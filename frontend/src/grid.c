#include "grid.h"
#include "font.h"
#include <string.h>

static unsigned char cells[GRID_ROWS][GRID_COLS];
static unsigned char invert[GRID_ROWS][GRID_COLS];
static SDL_Color accent = { 0, 255, 255, 255 };

/* Glyph atlas: 256 glyphs side by side, white on transparent, built once per renderer. */
static SDL_Texture *atlas;
static SDL_Renderer *atlas_owner;

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
    if (atlas && atlas_owner == ren) return 0;
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
    return 0;
}

void grid_present(SDL_Renderer *ren) {
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);
    if (build_atlas(ren) != 0) return;

    /* Pass 1: accent bars. */
    SDL_SetRenderDrawColor(ren, accent.r, accent.g, accent.b, 255);
    for (int r = 0; r < GRID_ROWS; r++)
        for (int c = 0; c < GRID_COLS; c++)
            if (invert[r][c]) {
                SDL_Rect bar = { c * FONT_W, r * FONT_H, FONT_W, FONT_H };
                SDL_RenderFillRect(ren, &bar);
            }

    /* Pass 2: glyphs, white on black or black on the accent bar. */
    for (int r = 0; r < GRID_ROWS; r++)
        for (int c = 0; c < GRID_COLS; c++) {
            unsigned char ch = cells[r][c];
            if (ch == ' ') continue;
            Uint8 v = invert[r][c] ? 0 : 255;
            SDL_SetTextureColorMod(atlas, v, v, v);
            SDL_Rect src = { ch * FONT_W, 0, FONT_W, FONT_H };
            SDL_Rect dst = { c * FONT_W, r * FONT_H, FONT_W, FONT_H };
            SDL_RenderCopy(ren, atlas, &src, &dst);
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
