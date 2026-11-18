/* Redux Terminal (Dale 2026-11-17 A; notes/2026-11-17-claude-question-terminal-apps.md).
 *
 *   redux-term                 a shell (bash)
 *   redux-term -e CMD ARGS...  run CMD; the terminal closes when it exits
 *   redux-term --hold -e CMD   ...and stays until a button after CMD ends (neofetch, speaker test)
 *   redux-term --version
 *
 * A pty runs the program; vt.c keeps the screen; this file draws it with Redux's own 8x16 fonts on
 * the 90 x 30 grid (720 x 480, pixel for pixel, so text is as sharp as the menu) and turns the
 * device buttons into keys. Embedded in dmenu.bin and written to /tmp at start (term.c); Apps
 * launches it through /tmp/.next like any other program, and dmenu_ln restarts the menu after it.
 *
 * Buttons: D-pad and left stick = arrows, A = Enter, B = Esc, X = Tab, Y = Backspace,
 * L1 / R1 = Page Up / Page Down, L2 / R2 = Home / End, START = F10, MENU = quit.
 * Mouse: the right stick moves a cursor, L3 = left click, R3 = right click, sent as xterm mouse
 * events to a program that asked for them (mc, htop, alsamixer). SELECT opens the on-screen keys
 * (osk.c): D-pad moves, A types, B or SELECT closes.
 * On a PC the keyboard types into it (for development). REDUX_TERM_DUMP=<file> writes the final
 * screen as text at exit (the CI smoke test) and REDUX_TERM_RAW=<file> records every byte the
 * program sent (for fixing a program that draws wrong); REDUX_TERM_SHOT=<file.bmp> saves the last
 * frame and REDUX_TERM_OSK=1 starts with the key picker open (screenshots for a PR). Nothing here
 * writes outside /tmp. */
#define _GNU_SOURCE
#include <SDL.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "vt.h"
#include "font.h"
#include "osk.h"
#include "cjk.h"
#include "remap.h"
#ifdef REDUX_PLATFORM_STOCK
#include <linux/input.h>
#include "remap_device.h"
#endif

#define FONT_W 8
#define FONT_H 16
#define SCREEN_W (VT_COLS * FONT_W)
#define SCREEN_H (VT_ROWS * FONT_H)
#define PRESENT_BUFFERS 3          /* page-flipped framebuffer on the device, as in main.c */
#define VIDEO_RETRY_MS 5000u
#define REPEAT_DELAY_MS 400
#define REPEAT_EVERY_MS 60
#define HOLD_AFTER_FAIL_MS 60000   /* a program that failed stays on screen this long */
#define MOUSE_HIDE_MS 4000         /* the cursor hides this long after the stick went still */
#define MOUSE_SPEED_PX_S 700.0f    /* at full deflection */
#define MOUSE_TICK_MS 16

#ifndef REDUX_BUILD
#define REDUX_BUILD "dev"
#endif

/* xterm's 16 colours. */
static const SDL_Color palette[16] = {
    {0,0,0,255}, {205,0,0,255}, {0,205,0,255}, {205,205,0,255}, {0,0,238,255}, {205,0,205,255},
    {0,205,205,255}, {229,229,229,255}, {127,127,127,255}, {255,0,0,255}, {0,255,0,255},
    {255,255,0,255}, {92,92,255,255}, {255,0,255,255}, {0,255,255,255}, {255,255,255,255},
};

static vt_t vt;
static osk_t osk;
static FILE *evlog;
static long long evlog_t0;
static long long now_ms(void);
static void logf_(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
static void logf_(const char *fmt, ...) {
    if (!evlog) return;
    va_list ap;
    fprintf(evlog, "%6lld ", now_ms() - evlog_t0);
    va_start(ap, fmt); vfprintf(evlog, fmt, ap); va_end(ap);
    fputc('\n', evlog);
    fflush(evlog);
}
/* Mouse cursor in pixels; shown while moving and for MOUSE_HIDE_MS after. */
static float mouse_x = SCREEN_W / 2, mouse_y = SCREEN_H / 2;
static int mouse_shown, mouse_held = -1;        /* button held: 0 left, 2 right, -1 none */
static int mouse_cell_col = -1, mouse_cell_row = -1;
static long long mouse_moved_at;
static int stick_rx, stick_ry;                   /* right stick raw values */
static int pty_fd = -1;
static pid_t child = -1;
static SDL_Texture *atlas;

static long long now_ms(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (long long)t.tv_sec * 1000 + t.tv_nsec / 1000000;
}

static void write_all(const char *buf, int n) {
    while (n > 0 && pty_fd >= 0) {
        ssize_t w = write(pty_fd, buf, (size_t)n);
        if (w < 0) { if (errno == EINTR || errno == EAGAIN) continue; return; }
        buf += w; n -= (int)w;
    }
}

static void send_key(vt_key_t k) {
    char buf[16];
    int n = vt_key_bytes(&vt, k, buf, sizeof buf);
    write_all(buf, n);
}

/* ---- pty ---- */

static int spawn(char **argv) {
    int m = posix_openpt(O_RDWR | O_NOCTTY);
    if (m < 0 || grantpt(m) != 0 || unlockpt(m) != 0) { perror("openpt"); return -1; }
    const char *slave = ptsname(m);
    if (!slave) { perror("ptsname"); return -1; }
    struct winsize ws = { VT_ROWS, VT_COLS, SCREEN_W, SCREEN_H };
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return -1; }
    if (pid == 0) {
        setsid();
        int s = open(slave, O_RDWR);
        if (s < 0) _exit(127);
        ioctl(s, TIOCSCTTY, 0);
        ioctl(s, TIOCSWINSZ, &ws);
        dup2(s, 0); dup2(s, 1); dup2(s, 2);
        if (s > 2) close(s);
        close(m);
        setenv("TERM", "xterm-256color", 1);
        setenv("COLUMNS", "90", 1);
        setenv("LINES", "30", 1);
        /* English messages, UTF-8 text: stock starts the frontend with a Chinese locale, which every
         * terminal app inherited (Dale 11-18). Clear every override, then C.UTF-8 (always present). */
        static const char *lc[] = { "LANGUAGE", "LC_ALL", "LC_CTYPE", "LC_MESSAGES", "LC_TIME",
            "LC_NUMERIC", "LC_COLLATE", "LC_MONETARY", "LC_PAPER", "LC_NAME", "LC_ADDRESS",
            "LC_TELEPHONE", "LC_MEASUREMENT", "LC_IDENTIFICATION", NULL };
        for (int i = 0; lc[i]; i++) unsetenv(lc[i]);
        setenv("LANG", "C.UTF-8", 1);
        if (!getenv("HOME")) setenv("HOME", "/root", 1);
        unsetenv("LD_PRELOAD");   /* the vsync preload meant for SimpleTerminal, if it leaked in */
        signal(SIGINT, SIG_DFL); signal(SIGPIPE, SIG_DFL);
        execvp(argv[0], argv);
        fprintf(stderr, "redux-term: cannot run %s: %s\r\n", argv[0], strerror(errno));
        _exit(127);
    }
    fcntl(m, F_SETFL, fcntl(m, F_GETFL) | O_NONBLOCK);
    pty_fd = m;
    child = pid;
    return 0;
}

/* ---- drawing ---- */

static int build_atlas(SDL_Renderer *ren) {
    SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, 256 * FONT_W, FONT_H, 32, SDL_PIXELFORMAT_RGBA32);
    if (!s) return -1;
    SDL_LockSurface(s);
    for (int g = 0; g < 256; g++) {
        const uint8_t *glyph = font_glyph((unsigned char)g);
        for (int y = 0; y < FONT_H; y++) {
            Uint32 *row = (Uint32 *)((Uint8 *)s->pixels + y * s->pitch) + g * FONT_W;
            for (int x = 0; x < FONT_W; x++)
                row[x] = (glyph[y] & (0x80 >> x)) ? SDL_MapRGBA(s->format, 255, 255, 255, 255)
                                                  : SDL_MapRGBA(s->format, 0, 0, 0, 0);
        }
    }
    SDL_UnlockSurface(s);
    atlas = SDL_CreateTextureFromSurface(ren, s);
    SDL_FreeSurface(s);
    if (!atlas) return -1;
    SDL_SetTextureBlendMode(atlas, SDL_BLENDMODE_BLEND);
    return 0;
}

/* 16x16 CJK glyphs: uploaded into a 1024 x 1024 atlas (4096 slots, more than a screen can show)
   the first time each one is drawn; when the atlas is full it starts over. */
#define CJK_ATLAS_PX 1024
#define CJK_SLOTS ((CJK_ATLAS_PX / 16) * (CJK_ATLAS_PX / 16))
static SDL_Texture *cjk_atlas;
static uint32_t cjk_slot_cp[CJK_SLOTS];   /* code point in each slot, 0 = free */
static int cjk_used;

static int cjk_slot(SDL_Renderer *ren, uint32_t cp, const uint8_t *glyph) {
    if (!cjk_atlas) {
        cjk_atlas = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, CJK_ATLAS_PX, CJK_ATLAS_PX);
        if (!cjk_atlas) return -1;
        SDL_SetTextureBlendMode(cjk_atlas, SDL_BLENDMODE_BLEND);
    }
    unsigned h = (cp * 2654435761u) % CJK_SLOTS;
    for (int probe = 0; probe < CJK_SLOTS; probe++, h = (h + 1) % CJK_SLOTS) {
        if (cjk_slot_cp[h] == cp) return (int)h;
        if (cjk_slot_cp[h] == 0) break;
    }
    if (cjk_used >= CJK_SLOTS - 1) { memset(cjk_slot_cp, 0, sizeof cjk_slot_cp); cjk_used = 0; h = (cp * 2654435761u) % CJK_SLOTS; }
    static Uint32 px[16 * 16];
    for (int y = 0; y < 16; y++) {
        unsigned bits = ((unsigned)glyph[y * 2] << 8) | glyph[y * 2 + 1];
        for (int x = 0; x < 16; x++) px[y * 16 + x] = (bits & (0x8000u >> x)) ? 0xFFFFFFFFu : 0u;
    }
    SDL_Rect r = { (int)(h % (CJK_ATLAS_PX / 16)) * 16, (int)(h / (CJK_ATLAS_PX / 16)) * 16, 16, 16 };
    if (SDL_UpdateTexture(cjk_atlas, &r, px, 16 * 4) != 0) return -1;
    cjk_slot_cp[h] = cp;
    cjk_used++;
    return (int)h;
}

static SDL_Color dim(SDL_Color c) { c.r /= 2; c.g /= 2; c.b /= 2; return c; }

/* The key picker as cells over the bottom rows: light grey box, black text, the chosen key yellow,
   an active Shift/Ctrl cyan. */
static void osk_cells(vt_cell_t out[OSK_HEIGHT][VT_COLS]) {
    vt_cell_t bg = { ' ', 0, 7, 0, 0, 0 };
    for (int r = 0; r < OSK_HEIGHT; r++) for (int c = 0; c < VT_COLS; c++) out[r][c] = bg;
    out[0][0].ch = 0xDA; out[0][VT_COLS - 1].ch = 0xBF;
    out[OSK_HEIGHT - 1][0].ch = 0xC0; out[OSK_HEIGHT - 1][VT_COLS - 1].ch = 0xD9;
    for (int c = 1; c < VT_COLS - 1; c++) out[0][c].ch = out[OSK_HEIGHT - 1][c].ch = 0xC4;
    for (int r = 1; r < OSK_HEIGHT - 1; r++) out[r][0].ch = out[r][VT_COLS - 1].ch = 0xB3;
    const char *title = " Keys: D-pad moves, A types, B closes ";
    for (int i = 0; title[i] && i + 2 < VT_COLS - 1; i++) out[0][i + 2].ch = (unsigned char)title[i];
    for (int r = 0; r < OSK_ROWS; r++) {
        for (int c = 0; c < osk_cols(r); c++) {
            const osk_key_t *k = osk_key(r, c);
            int x = osk_key_x(r, c), chosen = r == osk.row && c == osk.col;
            int active = (k->mode == 1 && osk.shift) || (k->mode == 2 && osk.ctrl);
            for (int i = 0; i < k->width + 2 && x + i < VT_COLS - 1; i++) {
                vt_cell_t *cell = &out[r + 1][x + i];
                cell->ch = (i == 0 || i == k->width + 1) ? ' ' : (unsigned char)k->label[i - 1];
                if (osk.shift && !k->mode && !k->shifted && cell->ch >= 'a' && cell->ch <= 'z' && k->width == 1)
                    cell->ch = (unsigned char)(cell->ch - 'a' + 'A');
                if (osk.shift && k->shifted && i == 1) cell->ch = (unsigned char)k->shifted[0];
                cell->bg = chosen ? 11 : active ? 14 : 7;
            }
        }
    }
}

static void draw(SDL_Renderer *ren, const char *footer) {
    static vt_cell_t overlay[OSK_HEIGHT][VT_COLS];
    int ov_top = osk.open ? VT_ROWS - OSK_HEIGHT : VT_ROWS;
    if (osk.open) osk_cells(overlay);
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);
    for (int r = 0; r < VT_ROWS; r++) {
        for (int c = 0; c < VT_COLS; c++) {
            vt_cell_t cell = r >= ov_top ? overlay[r - ov_top][c] : vt.cells[r][c];
            if (cell.wide == 2 && r < ov_top && c > 0 && vt.cells[r][c - 1].wide == 1 && cjk_glyph(vt.cells[r][c - 1].cp))
                continue;   /* drawn with its first half, background included */
            int fgi = cell.fg, bgi = cell.bg;
            if ((cell.attr & VT_BOLD) && fgi < 8) fgi += 8;   /* bold as bright, the console way */
            SDL_Color fg = palette[fgi & 15], bg = palette[bgi & 15];
            if (cell.attr & VT_DIM) fg = dim(fg);
            if (cell.attr & VT_INVERSE) { SDL_Color x = fg; fg = bg; bg = x; }
            int at_cursor = vt.cursor_visible && r == vt.row && c == vt.col && !footer && r < ov_top;
            if (at_cursor) { SDL_Color x = fg; fg = bg; bg = x; if (fg.r == bg.r && fg.g == bg.g && fg.b == bg.b) bg = palette[7]; }
            SDL_Rect dst = { c * FONT_W, r * FONT_H, FONT_W, FONT_H };
            if (bg.r || bg.g || bg.b) {
                SDL_SetRenderDrawColor(ren, bg.r, bg.g, bg.b, 255);
                SDL_RenderFillRect(ren, &dst);
            }
            /* A wide (CJK) glyph: 16x16 from Unifont over both cells; "??" when the table lacks it,
               so the columns still line up with what the program laid out. */
            unsigned char ch = cell.wide ? '?' : cell.ch;
            if (cell.wide == 1 && c + 1 < VT_COLS && vt.cells[r][c + 1].wide == 2 && r < ov_top) {
                const uint8_t *g = cjk_glyph(cell.cp);
                int slot = g ? cjk_slot(ren, cell.cp, g) : -1;
                if (slot >= 0) {
                    SDL_SetTextureColorMod(cjk_atlas, fg.r, fg.g, fg.b);
                    SDL_Rect src = { (slot % (CJK_ATLAS_PX / 16)) * 16, (slot / (CJK_ATLAS_PX / 16)) * 16, 16, 16 };
                    SDL_Rect wd = { dst.x, dst.y, 16, 16 };
                    /* The second cell's background first, so the glyph's right half sits on it. */
                    vt_cell_t c2 = vt.cells[r][c + 1];
                    SDL_Color bg2 = palette[((c2.attr & VT_INVERSE) ? c2.fg : c2.bg) & 15];
                    if (bg2.r || bg2.g || bg2.b) { SDL_SetRenderDrawColor(ren, bg2.r, bg2.g, bg2.b, 255); SDL_Rect d2 = { dst.x + FONT_W, dst.y, FONT_W, FONT_H }; SDL_RenderFillRect(ren, &d2); }
                    SDL_RenderCopy(ren, cjk_atlas, &src, &wd);
                    ch = ' ';
                }
            }
            if (ch != ' ') {
                SDL_SetTextureColorMod(atlas, fg.r, fg.g, fg.b);
                SDL_Rect src = { ch * FONT_W, 0, FONT_W, FONT_H };
                SDL_RenderCopy(ren, atlas, &src, &dst);
            }
            if (cell.attr & VT_UNDERLINE) {
                SDL_SetRenderDrawColor(ren, fg.r, fg.g, fg.b, 255);
                SDL_Rect u = { dst.x, dst.y + FONT_H - 1, FONT_W, 1 };
                SDL_RenderFillRect(ren, &u);
            }
        }
    }
    if (footer) {
        /* The program ended: one inverted line at the bottom, over the screen it left. */
        SDL_Rect bar = { 0, SCREEN_H - FONT_H, SCREEN_W, FONT_H };
        SDL_SetRenderDrawColor(ren, 229, 229, 229, 255);
        SDL_RenderFillRect(ren, &bar);
        SDL_SetTextureColorMod(atlas, 0, 0, 0);
        for (int c = 0; footer[c] && c < VT_COLS; c++) {
            SDL_Rect src = { (unsigned char)footer[c] * FONT_W, 0, FONT_W, FONT_H };
            SDL_Rect dst = { c * FONT_W, SCREEN_H - FONT_H, FONT_W, FONT_H };
            SDL_RenderCopy(ren, atlas, &src, &dst);
        }
    }
    if (mouse_shown && !footer) {
        /* The pointer: a yellow frame round the cell under it, plus a dot at the exact point. */
        SDL_Rect cellr = { (int)mouse_x / FONT_W * FONT_W, (int)mouse_y / FONT_H * FONT_H, FONT_W, FONT_H };
        SDL_SetRenderDrawColor(ren, 255, 255, 0, 255);
        SDL_RenderDrawRect(ren, &cellr);
        SDL_Rect dot = { (int)mouse_x - 1, (int)mouse_y - 1, 3, 3 };
        SDL_RenderFillRect(ren, &dot);
    }
}

static void present(SDL_Renderer *ren, const char *footer) {
    for (int i = 0; i < PRESENT_BUFFERS; i++) { draw(ren, footer); SDL_RenderPresent(ren); }
    logf_("frame osk=%d mouse=%d cursor=%d,%d%s", osk.open, mouse_shown, vt.row, vt.col, footer ? " footer" : "");
    vt.dirty = 0;
}

/* ---- input ---- */

static int quit_requested;
static int any_button;              /* a press after the program ended closes the terminal */
static int dirty_input;             /* the picker or the pointer changed: redraw */

/* Held button with repeat: directions, page keys and Backspace repeat after REPEAT_DELAY_MS. */
static int held_btn = -1;           /* logical_button_t, -1 none */
static long long held_since, held_last;

static void mouse_report(int button, int release, int motion) {
    char buf[32];
    int col = (int)mouse_x / FONT_W, row = (int)mouse_y / FONT_H;
    int n = vt_mouse_bytes(&vt, button, release, motion, col, row, buf, sizeof buf);
    if (n) write_all(buf, n);
}

/* Right stick: move the pointer; report motion to a program that wants it. */
static void mouse_tick(long long now) {
    static long long last;
    if (!last) last = now;
    float dt = (float)(now - last) / 1000.0f;
    last = now;
    float dx = stick_rx / 4096.0f, dy = stick_ry / 4096.0f;
    if (dx > -0.15f && dx < 0.15f) dx = 0;   /* deadzone */
    if (dy > -0.15f && dy < 0.15f) dy = 0;
    if (dx || dy) {
        /* Speed grows with the square of the deflection: fine control near the centre. */
        mouse_x += dx * (dx < 0 ? -dx : dx) * MOUSE_SPEED_PX_S * dt;
        mouse_y += dy * (dy < 0 ? -dy : dy) * MOUSE_SPEED_PX_S * dt;
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_x > SCREEN_W - 1) mouse_x = SCREEN_W - 1;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_y > SCREEN_H - 1) mouse_y = SCREEN_H - 1;
        mouse_moved_at = now;
        if (!mouse_shown) { mouse_shown = 1; }
        dirty_input = 1;
        int col = (int)mouse_x / FONT_W, row = (int)mouse_y / FONT_H;
        if (col != mouse_cell_col || row != mouse_cell_row) {
            mouse_cell_col = col; mouse_cell_row = row;
            mouse_report(mouse_held, 0, 1);
        }
    } else if (mouse_shown && now - mouse_moved_at > MOUSE_HIDE_MS && mouse_held < 0) {
        mouse_shown = 0;
        dirty_input = 1;
    }
}

static int mouse_active(void) { return stick_rx > 600 || stick_rx < -600 || stick_ry > 600 || stick_ry < -600; }

/* Button to key, outside the picker. -1: no key. */
static int button_key(logical_button_t b) {
    switch (b) {
    case LB_UP: return VTK_UP;       case LB_DOWN: return VTK_DOWN;
    case LB_LEFT: return VTK_LEFT;   case LB_RIGHT: return VTK_RIGHT;
    case LB_A: return VTK_ENTER;     case LB_B: return VTK_ESC;
    case LB_X: return VTK_TAB;       case LB_Y: return VTK_BACKSPACE;
    case LB_L1: return VTK_PGUP;     case LB_R1: return VTK_PGDN;
    case LB_L2: return VTK_HOME;     case LB_R2: return VTK_END;
    case LB_START: return VTK_F10;
    default: return -1;
    }
}

static int repeats(logical_button_t b) {
    return b == LB_UP || b == LB_DOWN || b == LB_LEFT || b == LB_RIGHT || b == LB_L1 || b == LB_R1 || b == LB_Y;
}

/* One press (or repeat) of a logical button. */
static void act(logical_button_t b) {
    if (osk.open) {
        switch (b) {
        case LB_UP: osk_move(&osk, -1, 0); dirty_input = 1; return;
        case LB_DOWN: osk_move(&osk, 1, 0); dirty_input = 1; return;
        case LB_LEFT: osk_move(&osk, 0, -1); dirty_input = 1; return;
        case LB_RIGHT: osk_move(&osk, 0, 1); dirty_input = 1; return;
        case LB_A: { char buf[16]; int n = osk_press(&osk, buf, sizeof buf); if (n) write_all(buf, n); dirty_input = 1; return; }
        case LB_B: case LB_SELECT: osk_toggle(&osk); dirty_input = 1; logf_("picker closed"); return;
        default: break;   /* X, Y, L1/R1, L2/R2, Start still reach the program */
        }
    } else if (b == LB_SELECT) { osk_toggle(&osk); dirty_input = 1; logf_("picker opened"); return; }
    int k = button_key(b);
    if (k >= 0) send_key((vt_key_t)k);
}

static void logical(logical_button_t b, int pressed) {
    logf_("button %d %s osk=%d", (int)b, pressed ? "down" : "up", osk.open);
    if (pressed) any_button = 1;
    if (b == LB_MENU) { if (pressed) quit_requested = 1; return; }
    if (b == LB_L3 || b == LB_R3) {
        /* Clicks go to the program only while it wants the mouse. */
        int button = b == LB_L3 ? 0 : 2;
        if (pressed) { mouse_held = button; mouse_shown = 1; mouse_moved_at = now_ms(); mouse_report(button, 0, 0); }
        else { mouse_report(button, 1, 0); if (mouse_held == button) mouse_held = -1; }
        dirty_input = 1;
        return;
    }
    if (pressed) {
        act(b);
        if (repeats(b)) { held_btn = b; held_since = held_last = now_ms(); }
    } else if (held_btn == (int)b) {
        held_btn = -1;
    }
}

static void repeat_tick(void) {
    if (held_btn < 0) return;
    long long t = now_ms();
    if (t - held_since >= REPEAT_DELAY_MS && t - held_last >= REPEAT_EVERY_MS) {
        act((logical_button_t)held_btn);
        held_last = t;
    }
}

#ifdef REDUX_PLATFORM_STOCK
static int keys_fd = -1;
static int stick_dir[6];   /* edge state per axis: left X/Y, right X/Y, hat X/Y */

static logical_button_t axis_btn(int axis, int dir) {
    if (dir == 0) return LB_NONE;
    if (axis == 0 || axis == 4) return dir < 0 ? LB_LEFT : LB_RIGHT;
    if (axis == 1 || axis == 5) return dir < 0 ? LB_UP : LB_DOWN;
    return LB_NONE;   /* right stick: the mouse */
}

static void read_buttons(void) {
    struct input_event ev;
    while (keys_fd >= 0 && read(keys_fd, &ev, sizeof ev) == sizeof ev) {
        if (ev.type == EV_KEY) {
            if (ev.value == 2) continue;   /* our own repeat timing */
            logical_button_t b = remap_device_key(ev.code);
            if (b != LB_NONE) logical(b, ev.value);
        } else if (ev.type == EV_ABS) {
            int axis = ev.code == DEV_ABS_LEFT_X ? 0 : ev.code == DEV_ABS_LEFT_Y ? 1 :
                       ev.code == DEV_ABS_RIGHT_X ? 2 : ev.code == DEV_ABS_RIGHT_Y ? 3 :
                       ev.code == DEV_ABS_HAT0X ? 4 : ev.code == DEV_ABS_HAT0Y ? 5 : -1;
            if (axis < 0) continue;
            int v = ev.value, dir;
            if (axis == 2) { stick_rx = v; continue; }
            if (axis == 3) { stick_ry = v; continue; }
            if (axis >= 4) dir = v < 0 ? -1 : v > 0 ? 1 : 0;
            else if (v <= -DEV_STICK_THRESHOLD) dir = -1;
            else if (v >= DEV_STICK_THRESHOLD) dir = 1;
            else if (v > -1000 && v < 1000) dir = 0;
            else dir = stick_dir[axis];
            if (dir != stick_dir[axis]) {
                if (stick_dir[axis]) logical(axis_btn(axis, stick_dir[axis]), 0);
                if (dir) logical(axis_btn(axis, dir), 1);
                stick_dir[axis] = dir;
            }
        }
    }
}
#endif

/* Scripted buttons (REDUX_TERM_SCRIPT), applied as their time comes. */
static const char *script;
static long long script_t0;

static int button_named(const char *name, size_t n) {
    static const struct { const char *name; logical_button_t b; } names[] = {
        {"UP", LB_UP}, {"DOWN", LB_DOWN}, {"LEFT", LB_LEFT}, {"RIGHT", LB_RIGHT}, {"A", LB_A}, {"B", LB_B},
        {"X", LB_X}, {"Y", LB_Y}, {"L1", LB_L1}, {"R1", LB_R1}, {"L2", LB_L2}, {"R2", LB_R2}, {"L3", LB_L3},
        {"R3", LB_R3}, {"START", LB_START}, {"SELECT", LB_SELECT}, {"MENU", LB_MENU},
    };
    for (size_t i = 0; i < sizeof names / sizeof names[0]; i++)
        if (strlen(names[i].name) == n && strncmp(names[i].name, name, n) == 0) return names[i].b;
    return -1;
}

static void script_tick(long long now) {
    while (script && *script) {
        while (*script == ';' || *script == ' ') script++;
        if (!*script) break;
        char *end;
        long ms = strtol(script, &end, 10);
        if (end == script) { script = NULL; break; }
        if (now - script_t0 < ms) break;
        const char *p = end;
        while (*p == ' ') p++;
        const char *name = p;
        while (*p && *p != ' ') p++;
        int b = button_named(name, (size_t)(p - name));
        int pressed = (int)strtol(p, &end, 10);
        script = end;
        if (b >= 0) logical((logical_button_t)b, pressed);
    }
}

/* PC keyboard (development): plain keys go to the program; F11 opens the picker; F12 is MENU;
   the keypad 8/2/4/6 and 5 drive the mouse (hold 5 = left click, 0 = right click). */
static void sdl_key(const SDL_KeyboardEvent *e, int down) {
    int k = -1;
    switch (e->keysym.sym) {
    case SDLK_UP: k = VTK_UP; break;         case SDLK_DOWN: k = VTK_DOWN; break;
    case SDLK_LEFT: k = VTK_LEFT; break;     case SDLK_RIGHT: k = VTK_RIGHT; break;
    case SDLK_RETURN: case SDLK_KP_ENTER: k = VTK_ENTER; break;
    case SDLK_ESCAPE: k = VTK_ESC; break;    case SDLK_TAB: k = VTK_TAB; break;
    case SDLK_BACKSPACE: k = VTK_BACKSPACE; break;
    case SDLK_PAGEUP: k = VTK_PGUP; break;   case SDLK_PAGEDOWN: k = VTK_PGDN; break;
    case SDLK_HOME: k = VTK_HOME; break;     case SDLK_END: k = VTK_END; break;
    case SDLK_INSERT: k = VTK_INSERT; break; case SDLK_DELETE: k = VTK_DELETE; break;
    case SDLK_F1: k = VTK_F1; break;   case SDLK_F2: k = VTK_F2; break;  case SDLK_F3: k = VTK_F3; break;
    case SDLK_F4: k = VTK_F4; break;   case SDLK_F5: k = VTK_F5; break;  case SDLK_F6: k = VTK_F6; break;
    case SDLK_F7: k = VTK_F7; break;   case SDLK_F8: k = VTK_F8; break;  case SDLK_F9: k = VTK_F9; break;
    case SDLK_F10: k = VTK_F10; break;
    case SDLK_F11: if (down && !e->repeat) logical(LB_SELECT, 1); return;
    case SDLK_F12: if (down) quit_requested = 1; return;
    case SDLK_KP_8: stick_ry = down ? -4096 : 0; return;
    case SDLK_KP_2: stick_ry = down ? 4096 : 0; return;
    case SDLK_KP_4: stick_rx = down ? -4096 : 0; return;
    case SDLK_KP_6: stick_rx = down ? 4096 : 0; return;
    case SDLK_KP_5: if (!e->repeat) logical(LB_L3, down); return;
    case SDLK_KP_0: if (!e->repeat) logical(LB_R3, down); return;
    default: break;
    }
    if (osk.open && down && !e->repeat) {
        /* With the picker open the arrow keys and Enter drive it, like the D-pad and A. */
        logical_button_t b = k == VTK_UP ? LB_UP : k == VTK_DOWN ? LB_DOWN : k == VTK_LEFT ? LB_LEFT :
                             k == VTK_RIGHT ? LB_RIGHT : k == VTK_ENTER ? LB_A : k == VTK_ESC ? LB_B : LB_NONE;
        if (b != LB_NONE) { act(b); return; }
    }
    if (k >= 0) { if (down) { any_button = 1; send_key((vt_key_t)k); } return; }   /* SDL repeats for us */
    /* Ctrl + letter: the control byte (Ctrl-C, Ctrl-L, ...). */
    if (down && (e->keysym.mod & KMOD_CTRL) && e->keysym.sym >= 'a' && e->keysym.sym <= 'z') {
        char c = (char)(e->keysym.sym - 'a' + 1);
        write_all(&c, 1);
    }
}

/* ---- main ---- */

static void stop_child(void) {
    if (child > 0) {
        kill(-child, SIGHUP);
        for (int i = 0; i < 20 && waitpid(child, NULL, WNOHANG) == 0; i++) usleep(50000);
        if (waitpid(child, NULL, WNOHANG) == 0) { kill(-child, SIGKILL); waitpid(child, NULL, 0); }
        child = -1;
    }
}

int main(int argc, char **argv) {
    char *shell[] = { "bash", NULL };
    char **cmd = shell;
    int hold = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--version")) { printf("redux-term %s\n", REDUX_BUILD); return 0; }
        if (!strcmp(argv[i], "--hold")) { hold = 1; continue; }
        if (!strcmp(argv[i], "-e") && i + 1 < argc) { cmd = argv + i + 1; break; }
        fprintf(stderr, "usage: redux-term [--hold] [-e CMD ARGS...]\n");
        return 2;
    }

    {
        const char *lp = getenv("REDUX_TERM_LOG");
#ifdef REDUX_PLATFORM_STOCK
        if (!lp) lp = "/tmp/redux-term.log";
#endif
        if (lp) evlog = fopen(lp, "w");
        evlog_t0 = now_ms();
        logf_("start %s %s", REDUX_BUILD, cmd[0]);
    }
    font_load();   /* the font the user chose in Settings > UI */
    vt_init(&vt);
    osk_init(&osk);
    if (getenv("REDUX_TERM_OSK")) { osk.open = 1; mouse_shown = 1; mouse_moved_at = now_ms() + 60000; }

    SDL_Window *win = NULL;
    SDL_Renderer *ren = NULL;
    for (uint32_t waited = 0;; waited += 250) {
        if (SDL_Init(SDL_INIT_VIDEO) == 0) {
            win = SDL_CreateWindow("Redux Terminal", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN);
            ren = win ? SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED) : NULL;
            if (!ren) ren = win ? SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE) : NULL;
            if (ren) break;
            if (win) SDL_DestroyWindow(win);
            win = NULL;
        }
        SDL_Quit();
        if (waited >= VIDEO_RETRY_MS) { fprintf(stderr, "redux-term: no video: %s\n", SDL_GetError()); return 1; }
        SDL_Delay(250);
    }
    if (build_atlas(ren) != 0) { fprintf(stderr, "redux-term: font atlas: %s\n", SDL_GetError()); return 1; }
    SDL_ShowCursor(SDL_DISABLE);
    SDL_StartTextInput();

    if (spawn(cmd) != 0) return 1;
    long long started = now_ms();
    script = getenv("REDUX_TERM_SCRIPT");
    script_t0 = started;
#ifdef REDUX_PLATFORM_STOCK
    keys_fd = open("/dev/input/event1", O_RDONLY | O_NONBLOCK);
    if (keys_fd < 0) fprintf(stderr, "redux-term: warning: could not open /dev/input/event1\n");
#endif

    FILE *raw = getenv("REDUX_TERM_RAW") ? fopen(getenv("REDUX_TERM_RAW"), "wb") : NULL;
    int exit_status = -1;          /* set when the program ends */
    char footer[VT_COLS + 1] = "";
    long long ended = 0;
    present(ren, NULL);

    while (!quit_requested) {
        struct pollfd p[2]; int n = 0;
        if (pty_fd >= 0) { p[n].fd = pty_fd; p[n].events = POLLIN; n++; }
#ifdef REDUX_PLATFORM_STOCK
        if (keys_fd >= 0) { p[n].fd = keys_fd; p[n].events = POLLIN; n++; }
#endif
        int timeout = (held_btn >= 0 || mouse_active() || script) ? MOUSE_TICK_MS : (vt.dirty || dirty_input) ? 0 : 30;
        if (n) poll(p, (nfds_t)n, timeout); else SDL_Delay((Uint32)timeout);

        /* Program output. */
        if (pty_fd >= 0) {
            unsigned char buf[8192];
            ssize_t r;
            int rounds = 0;
            while ((r = read(pty_fd, buf, sizeof buf)) > 0 && rounds++ < 64) {
                if (raw) fwrite(buf, 1, (size_t)r, raw);
                vt_feed(&vt, buf, (size_t)r);
            }
            if (r == 0 || (r < 0 && errno != EAGAIN && errno != EINTR)) {
                /* EIO: the program closed the slave side, i.e. it ended. */
                int st = 0;
                waitpid(child, &st, 0);
                child = -1;
                close(pty_fd); pty_fd = -1;
                exit_status = WIFEXITED(st) ? WEXITSTATUS(st) : WIFSIGNALED(st) ? 128 + WTERMSIG(st) : 1;
                ended = now_ms();
                /* A clean exit after a real run just closes; a failure, an exit within two seconds,
                   or --hold stays so the screen can be read (nothing else on the device shows it). */
                if (exit_status == 0 && ended - started > 2000 && !hold) break;
                if (exit_status == 0) snprintf(footer, sizeof footer, " Finished. Press a button to close. ");
                else snprintf(footer, sizeof footer, " %s ended with error %d. Press a button to close. ",
                              cmd[0], exit_status);
                any_button = 0;
                vt.dirty = 1;
            }
            char rep[64];
            int rn = vt_take_reply(&vt, rep, sizeof rep);
            if (rn) write_all(rep, rn);
            vt.bell = 0;
        }
        if (pty_fd < 0) {
            long long hold = HOLD_AFTER_FAIL_MS;
            if (getenv("REDUX_TERM_HOLD_MS")) hold = atoll(getenv("REDUX_TERM_HOLD_MS"));   /* tests */
            if (any_button || now_ms() - ended > hold) break;
        }

#ifdef REDUX_PLATFORM_STOCK
        read_buttons();
#endif
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit_requested = 1;
            else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) sdl_key(&e.key, e.type == SDL_KEYDOWN);
            else if (e.type == SDL_TEXTINPUT) { any_button = 1; write_all(e.text.text, (int)strlen(e.text.text)); }
            else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_EXPOSED) vt.dirty = 1;
        }
        repeat_tick();
        script_tick(now_ms());
        mouse_tick(now_ms());
        if (vt.dirty || dirty_input) { present(ren, pty_fd < 0 ? footer : NULL); dirty_input = 0; }
    }

    stop_child();
    logf_("exit status=%d quit=%d", exit_status, quit_requested);
    if (evlog) fclose(evlog);
    if (raw) fclose(raw);
    const char *shot = getenv("REDUX_TERM_SHOT");
    if (shot) {
        draw(ren, pty_fd < 0 && footer[0] ? footer : NULL);
        SDL_Surface *sf = SDL_CreateRGBSurfaceWithFormat(0, SCREEN_W, SCREEN_H, 32, SDL_PIXELFORMAT_RGBA32);
        if (sf && SDL_RenderReadPixels(ren, NULL, sf->format->format, sf->pixels, sf->pitch) == 0) SDL_SaveBMP(sf, shot);
        else fprintf(stderr, "redux-term: screenshot failed: %s\n", SDL_GetError());
        if (sf) SDL_FreeSurface(sf);
    }
    const char *dump_path = getenv("REDUX_TERM_DUMP");
    if (dump_path) {
        /* Both screens: a curses program has switched back to the (blank) main screen by then. */
        static char dump[VT_ROWS * (VT_COLS + 1) + 1];
        FILE *f = fopen(dump_path, "w");
        if (f) {
            vt_dump(&vt, dump, sizeof dump); fputs(dump, f);
            fputs("--- other screen ---\n", f);
            memcpy(vt.cells, vt.alt, sizeof vt.cells);
            vt_dump(&vt, dump, sizeof dump); fputs(dump, f);
            fclose(f);
        }
    }
    if (cjk_atlas) SDL_DestroyTexture(cjk_atlas);
    SDL_DestroyTexture(atlas);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
