#include "grid.h"
#include "font.h"
#include <string.h>

static unsigned char cells[GRID_ROWS][GRID_COLS];
static unsigned char invert[GRID_ROWS][GRID_COLS];
static SDL_Color accent = { 0, 255, 255, 255 };
static SDL_Color col_text = { 255, 255, 255, 255 }, col_hf = { 255, 255, 255, 255 }, col_win = { 0, 0, 0, 255 };
static int size_h, size_f, size_w;   /* requested (grid_set_sizes) */
static int plain;                    /* this frame at Default sizes */

/* Glyph atlas: 256 glyphs side by side, white on transparent, built once per renderer. */
static SDL_Texture *atlas;
static SDL_Renderer *atlas_owner;
static font_id_t atlas_font = FONT_COUNT;   /* font the atlas was built from */

/* The only non-ASCII characters the UI uses, and their CP437 codes. */
static const struct { uint32_t cp; unsigned char cp437; } box_map[] = {
    { 0x2500, 0xC4 }, /* ─ */  { 0x2502, 0xB3 }, /* │ */
    { 0x250C, 0xDA }, /* ┌ */  { 0x2510, 0xBF }, /* ┐ */
    { 0x2514, 0xC0 }, /* └ */  { 0x2518, 0xD9 }, /* ┘ */
    { 0x2593, 0xB2 }, /* ▓ */
};

void grid_clear(void) {
    memset(cells, ' ', sizeof cells);
    memset(invert, 0, sizeof invert);
    plain = 0;
}

void grid_set_sizes(int header, int footer, int window) {
    size_h = header == 1;
    size_f = footer == 1;
    size_w = window < 0 ? 0 : window > 2 ? 2 : window;
}

void grid_set_colors(uint8_t tr, uint8_t tg, uint8_t tb, uint8_t hr, uint8_t hg, uint8_t hb,
                     uint8_t wr, uint8_t wg, uint8_t wb) {
    col_text = (SDL_Color){ tr, tg, tb, 255 };
    col_hf   = (SDL_Color){ hr, hg, hb, 255 };
    col_win  = (SDL_Color){ wr, wg, wb, 255 };
}

void grid_plain_frame(void) { plain = 1; }

/* Pixel layout of one set of sizes. */
typedef struct { int th, fh, sy, sx, cap; } layout_t;

static layout_t layout_of(int h, int f, int w) {
    layout_t L;
    L.th = FONT_H * (1 + h);
    L.fh = FONT_H * (1 + f);
    L.sy = w >= 1 ? 2 : 1;
    L.sx = w == 2 ? 2 : 1;
    int body_px = GRID_ROWS * FONT_H - L.th - L.fh - 2 * FONT_H;   /* minus the box's top and bottom */
    L.cap = body_px / (FONT_H * L.sy);
    if (L.cap > BODY_ROWS) L.cap = BODY_ROWS;
    return L;
}

int grid_body_capacity(void) { return layout_of(size_h, size_f, size_w).cap; }

/* Body rows the current cells use (last non-blank or inverted body row + 1). */
static int body_used(void) {
    for (int i = BODY_ROWS - 1; i >= 0; i--)
        for (int c = 1; c < GRID_COLS - 1; c++)
            if (cells[ROW_BODY0 + i][c] != ' ' || invert[ROW_BODY0 + i][c]) return i + 1;
    return 0;
}

void grid_effective_sizes(int *header, int *footer, int *window) {
    int h = size_h, f = size_f, w = size_w;
    if (plain) h = f = w = 0;
    int used = body_used();
    while (used > layout_of(h, f, w).cap) {
        if (w > 0) w--;
        else if (f > 0) f--;
        else if (h > 0) h--;
        else break;
    }
    *header = h; *footer = f; *window = w;
}

/* Decode one UTF-8 character; returns its length (1 for invalid bytes). */
static int utf8_next(const char *s, uint32_t *cp) {
    const unsigned char *u = (const unsigned char *)s;
    if (u[0] < 0x80) { *cp = u[0]; return 1; }
    if ((u[0] & 0xE0) == 0xC0 && u[1]) { *cp = ((u[0] & 0x1F) << 6) | (u[1] & 0x3F); return 2; }
    if ((u[0] & 0xF0) == 0xE0 && u[1] && u[2]) {
        *cp = ((u[0] & 0x0F) << 12) | ((u[1] & 0x3F) << 6) | (u[2] & 0x3F); return 3;
    }
    *cp = '?'; return 1;
}

static unsigned char to_cp437(uint32_t cp) {
    if (cp >= 0x20 && cp < 0x7F) return (unsigned char)cp;
    for (size_t i = 0; i < sizeof box_map / sizeof box_map[0]; i++)
        if (box_map[i].cp == cp) return box_map[i].cp437;
    return '?';
}

static size_t utf8_len(const char *s) {
    size_t n = 0; uint32_t cp;
    while (*s) { s += utf8_next(s, &cp); n++; }
    return n;
}

void grid_puts(int col, int row, const char *s, int inverted) {
    if (row < 0 || row >= GRID_ROWS) return;
    uint32_t cp;
    while (*s && col < GRID_COLS) {
        s += utf8_next(s, &cp);
        if (col >= 0) {
            cells[row][col] = to_cp437(cp);
            invert[row][col] = (unsigned char)(inverted != 0);
        }
        col++;
    }
}

void grid_puts_right(int row, const char *s) {
    grid_puts(GRID_COLS - (int)utf8_len(s), row, s, 0);
}

void grid_invert(int row, int c0, int c1) {
    if (row < 0 || row >= GRID_ROWS) return;
    for (int c = c0 < 0 ? 0 : c0; c <= c1 && c < GRID_COLS; c++) invert[row][c] = 1;
}

void grid_box(void) {
    cells[ROW_BOX_TOP][0] = 0xDA; cells[ROW_BOX_TOP][GRID_COLS - 1] = 0xBF;
    cells[ROW_BOX_BOT][0] = 0xC0; cells[ROW_BOX_BOT][GRID_COLS - 1] = 0xD9;
    for (int c = 1; c < GRID_COLS - 1; c++) cells[ROW_BOX_TOP][c] = cells[ROW_BOX_BOT][c] = 0xC4;
    for (int r = ROW_BOX_TOP + 1; r < ROW_BOX_BOT; r++) cells[r][0] = cells[r][GRID_COLS - 1] = 0xB3;
}

void grid_set_accent(uint8_t r, uint8_t g, uint8_t b) {
    accent.r = r; accent.g = g; accent.b = b;
}

static int build_atlas(SDL_Renderer *ren) {
    if (atlas && atlas_owner == ren && atlas_font == font_current()) return 0;
    if (atlas) SDL_DestroyTexture(atlas);
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
    atlas_owner = ren;
    atlas_font = font_current();
    return 0;
}

static void glyph(SDL_Renderer *ren, unsigned char ch, int x, int y, int w, int h, SDL_Color c) {
    if (ch == ' ') return;
    SDL_SetTextureColorMod(atlas, c.r, c.g, c.b);
    SDL_Rect src = { ch * FONT_W, 0, FONT_W, FONT_H };
    SDL_Rect dst = { x, y, w, h };
    SDL_RenderCopy(ren, atlas, &src, &dst);
}

static void bar(SDL_Renderer *ren, int x, int y, int w, int h) {
    SDL_SetRenderDrawColor(ren, accent.r, accent.g, accent.b, 255);
    SDL_Rect r = { x, y, w, h };
    SDL_RenderFillRect(ren, &r);
}

/* One full-width row (title, box edges, help) at height h, starting at pixel y. */
static void plain_row(SDL_Renderer *ren, int r, int y, int h, SDL_Color c) {
    static const SDL_Color black = { 0, 0, 0, 255 };
    for (int col = 0; col < GRID_COLS; col++) {
        if (invert[r][col]) bar(ren, col * FONT_W, y, FONT_W, h);
        glyph(ren, cells[r][col], col * FONT_W, y, FONT_W, h, invert[r][col] ? black : c);
    }
}

void grid_present(SDL_Renderer *ren) {
    static const SDL_Color black = { 0, 0, 0, 255 };
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);
    if (build_atlas(ren) != 0) return;

    int h, f, w;
    grid_effective_sizes(&h, &f, &w);
    layout_t L = layout_of(h, f, w);
    const int H = GRID_ROWS * FONT_H, W = GRID_COLS * FONT_W;

    if (plain) {
        /* Pop-up: every row at Default, title and help rows in the header/footer colour. */
        for (int r = 0; r < GRID_ROWS; r++)
            plain_row(ren, r, r * FONT_H, FONT_H, (r == ROW_TITLE || r == ROW_HELP) ? col_hf : col_text);
        return;
    }

    int top = L.th, bot = H - L.fh - FONT_H;           /* pixel rows of the box's top and bottom edges */
    int rh = FONT_H * L.sy, cw = FONT_W * L.sx;

    /* Window background inside the box. */
    SDL_SetRenderDrawColor(ren, col_win.r, col_win.g, col_win.b, 255);
    SDL_Rect inside = { FONT_W, top + FONT_H, W - 2 * FONT_W, bot - top - FONT_H };
    SDL_RenderFillRect(ren, &inside);

    plain_row(ren, ROW_TITLE, 0, L.th, col_hf);
    plain_row(ren, ROW_BOX_TOP, top, FONT_H, col_text);
    plain_row(ren, ROW_BOX_BOT, bot, FONT_H, col_text);
    plain_row(ren, ROW_HELP, H - L.fh, L.fh, col_hf);

    /* Box sides: each visible row's own edge cells (a screen may mark them), at the row's height;
       below the last row, plain edge glyphs down to the box bottom. */
    for (int i = 0; i < L.cap; i++) {
        int r = ROW_BODY0 + i, y = top + FONT_H + i * rh;
        for (int k = 0; k < 2; k++) {
            int c = k ? GRID_COLS - 1 : 0, x = k ? W - FONT_W : 0;
            if (invert[r][c]) bar(ren, x, y, FONT_W, rh);
            glyph(ren, cells[r][c], x, y, FONT_W, rh, invert[r][c] ? black : col_text);
        }
    }
    for (int y = top + FONT_H + L.cap * rh; y < bot; y += FONT_H) {
        int gh = bot - y < FONT_H ? bot - y : FONT_H;
        glyph(ren, 0xB3, 0, y, FONT_W, gh, col_text);
        glyph(ren, 0xB3, W - FONT_W, y, FONT_W, gh, col_text);
    }

    /* Body cells 1..88 of each visible row, scaled; Largest clips at the right edge. */
    for (int i = 0; i < L.cap; i++) {
        int r = ROW_BODY0 + i, y = top + FONT_H + i * rh;
        for (int c = 1; c < GRID_COLS - 1; c++) {
            int x = FONT_W + (c - 1) * cw;
            if (x + cw > W - FONT_W) break;
            if (invert[r][c]) bar(ren, x, y, cw, rh);
            glyph(ren, cells[r][c], x, y, cw, rh, invert[r][c] ? black : col_text);
        }
    }
}

static size_t put_utf8(char *buf, size_t len, size_t at, unsigned char cp437) {
    uint32_t cp = cp437;
    for (size_t i = 0; i < sizeof box_map / sizeof box_map[0]; i++)
        if (box_map[i].cp437 == cp437) cp = box_map[i].cp;
    char tmp[3]; size_t n;
    if (cp < 0x80) { tmp[0] = (char)cp; n = 1; }
    else { tmp[0] = (char)(0xE0 | (cp >> 12)); tmp[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
           tmp[2] = (char)(0x80 | (cp & 0x3F)); n = 3; }
    if (at + n < len) memcpy(buf + at, tmp, n);
    return at + n;
}

size_t grid_dump(char *buf, size_t len) {
    size_t at = 0;
    for (int r = 0; r < GRID_ROWS; r++) {
        int marked = 0;
        for (int c = 0; c < GRID_COLS; c++) {
            unsigned char ch = cells[r][c];
            /* Spec convention: the selected row starts with "▓▓". */
            if (invert[r][c] && ch == ' ' && marked < 2) { ch = 0xB2; marked++; }
            at = put_utf8(buf, len, at, ch);
        }
        if (at + 1 < len) buf[at] = '\n';
        at++;
    }
    if (len) buf[at < len ? at : len - 1] = '\0';
    return at;
}
