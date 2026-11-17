/* Redux Terminal (Dale 2026-11-17 A; notes/2026-11-17-claude-question-terminal-apps.md).
 *
 *   redux-term                 a shell (bash)
 *   redux-term -e CMD ARGS...  run CMD; the terminal closes when it exits
 *   redux-term --version
 *
 * A pty runs the program; vt.c keeps the screen; this file draws it with Redux's own 8x16 fonts on
 * the 90 x 30 grid (720 x 480, pixel for pixel, so text is as sharp as the menu) and turns the
 * device buttons into keys. Embedded in dmenu.bin and written to /tmp at start (term.c); Apps
 * launches it through /tmp/.next like any other program, and dmenu_ln restarts the menu after it.
 *
 * Buttons (PR 1): D-pad and left stick = arrows, A = Enter, B = Esc, X = Tab, Y = Backspace,
 * L1 / R1 = Page Up / Page Down, L2 / R2 = Home / End, START = F10, MENU = quit.
 * Mouse (right stick, L3 / R3) and the SELECT on-screen keys come in PR 2.
 * On a PC the keyboard types into it (for development). REDUX_TERM_DUMP=<file> writes the final
 * screen as text at exit (the CI smoke test) and REDUX_TERM_RAW=<file> records every byte the
 * program sent (for fixing a program that draws wrong). Nothing here writes outside /tmp. */
#define _GNU_SOURCE
#include <SDL.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
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
        if (!getenv("LANG") && !getenv("LC_ALL")) setenv("LANG", "C.UTF-8", 1);   /* UTF-8 lines in mc */
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

static SDL_Color dim(SDL_Color c) { c.r /= 2; c.g /= 2; c.b /= 2; return c; }

static void draw(SDL_Renderer *ren, const char *footer) {
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);
    for (int r = 0; r < VT_ROWS; r++) {
        for (int c = 0; c < VT_COLS; c++) {
            vt_cell_t cell = vt.cells[r][c];
            int fgi = cell.fg, bgi = cell.bg;
            if ((cell.attr & VT_BOLD) && fgi < 8) fgi += 8;   /* bold as bright, the console way */
            SDL_Color fg = palette[fgi & 15], bg = palette[bgi & 15];
            if (cell.attr & VT_DIM) fg = dim(fg);
            if (cell.attr & VT_INVERSE) { SDL_Color x = fg; fg = bg; bg = x; }
            int at_cursor = vt.cursor_visible && r == vt.row && c == vt.col && !footer;
            if (at_cursor) { SDL_Color x = fg; fg = bg; bg = x; if (fg.r == bg.r && fg.g == bg.g && fg.b == bg.b) bg = palette[7]; }
            SDL_Rect dst = { c * FONT_W, r * FONT_H, FONT_W, FONT_H };
            if (bg.r || bg.g || bg.b) {
                SDL_SetRenderDrawColor(ren, bg.r, bg.g, bg.b, 255);
                SDL_RenderFillRect(ren, &dst);
            }
            if (cell.ch != ' ') {
                SDL_SetTextureColorMod(atlas, fg.r, fg.g, fg.b);
                SDL_Rect src = { cell.ch * FONT_W, 0, FONT_W, FONT_H };
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
}

static void present(SDL_Renderer *ren, const char *footer) {
    for (int i = 0; i < PRESENT_BUFFERS; i++) { draw(ren, footer); SDL_RenderPresent(ren); }
    vt.dirty = 0;
}

/* ---- input ---- */

static int quit_requested;
static int any_button;              /* a press after the program ended closes the terminal */
static int held_key = -1;           /* a repeating key held down (vt_key_t), -1 none */
static long long held_since, held_last;

/* 1 if the key repeats while held. */
static int repeats(vt_key_t k) {
    return k == VTK_UP || k == VTK_DOWN || k == VTK_LEFT || k == VTK_RIGHT || k == VTK_PGUP ||
           k == VTK_PGDN || k == VTK_BACKSPACE;
}

static void key_down(vt_key_t k) {
    any_button = 1;
    send_key(k);
    if (repeats(k)) { held_key = k; held_since = held_last = now_ms(); }
}

static void key_up(vt_key_t k) {
    if (held_key == (int)k) held_key = -1;
}

static void repeat_tick(void) {
    if (held_key < 0) return;
    long long t = now_ms();
    if (t - held_since >= REPEAT_DELAY_MS && t - held_last >= REPEAT_EVERY_MS) {
        send_key((vt_key_t)held_key);
        held_last = t;
    }
}

#ifdef REDUX_PLATFORM_STOCK
static int keys_fd = -1;
static int stick_dir[6];   /* edge state per axis: left X/Y, right X/Y, hat X/Y */

/* Logical button to key. -1: no key. */
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

static void logical(logical_button_t b, int pressed) {
    if (b == LB_MENU) { if (pressed) { quit_requested = 1; any_button = 1; } return; }
    int k = button_key(b);
    if (k < 0) { if (pressed) any_button = 1; return; }
    if (pressed) key_down((vt_key_t)k); else key_up((vt_key_t)k);
}

static logical_button_t axis_btn(int axis, int dir) {
    if (dir == 0) return LB_NONE;
    if (axis == 0 || axis == 4) return dir < 0 ? LB_LEFT : LB_RIGHT;
    if (axis == 1 || axis == 5) return dir < 0 ? LB_UP : LB_DOWN;
    return LB_NONE;   /* right stick: the mouse, PR 2 */
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

/* PC keyboard (development). */
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
    case SDLK_F12: if (down) quit_requested = 1; return;   /* MENU on a PC */
    default: break;
    }
    if (k >= 0) { if (down) key_down((vt_key_t)k); else key_up((vt_key_t)k); return; }
    /* Ctrl + letter: the control byte (Ctrl-C, Ctrl-L, ...). SDL repeats these for us. */
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
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--version")) { printf("redux-term %s\n", REDUX_BUILD); return 0; }
        if (!strcmp(argv[i], "-e") && i + 1 < argc) { cmd = argv + i + 1; break; }
        fprintf(stderr, "usage: redux-term [-e CMD ARGS...]\n");
        return 2;
    }

    font_load();   /* the font the user chose in Settings > UI */
    vt_init(&vt);

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
        int timeout = held_key >= 0 ? REPEAT_EVERY_MS : vt.dirty ? 0 : 30;
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
                /* A clean exit after a real run just closes; a failure, or an exit within two
                   seconds, stays so the message can be read (nothing else on the device shows it). */
                if (exit_status == 0 && ended - started > 2000) break;
                snprintf(footer, sizeof footer, " %s ended (exit %d). Press MENU to close. ",
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
        if (vt.dirty) present(ren, pty_fd < 0 ? footer : NULL);
    }

    stop_child();
    if (raw) fclose(raw);
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
    SDL_DestroyTexture(atlas);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
