/* Redux Terminal screen model: a VT100/xterm subset. See vt.h. */
#include "vt.h"
#include <string.h>

enum { S_GROUND, S_ESC, S_CSI, S_OSC, S_OSC_ESC, S_DCS, S_DCS_ESC, S_CHARSET };

static vt_cell_t blank(const vt_t *t) {
    vt_cell_t c = { ' ', t->fg, t->bg, 0 };
    /* Erase uses the current background (xterm bce), never bold/underline. */
    return c;
}

static void mark(vt_t *t) { t->dirty = 1; }

void vt_init(vt_t *t) {
    memset(t, 0, sizeof *t);
    t->fg = VT_DEFAULT_FG; t->bg = VT_DEFAULT_BG;
    t->saved_fg = VT_DEFAULT_FG; t->saved_bg = VT_DEFAULT_BG;
    t->top = 0; t->bot = VT_ROWS - 1;
    t->cursor_visible = 1;
    t->autowrap = 1;
    for (int c = 0; c < VT_COLS; c++) t->tabs[c] = (c % 8) == 0;
    vt_cell_t b = blank(t);
    for (int r = 0; r < VT_ROWS; r++)
        for (int c = 0; c < VT_COLS; c++) t->cells[r][c] = t->alt[r][c] = b;
    t->dirty = 1;
}

/* ---- UTF-8 to CP437 ---- */

static const struct { uint16_t cp; unsigned char c; } cp437_map[] = {
    /* Latin-1 letters and symbols present in CP437 */
    {0x00A0,' '}, {0x00A1,0xAD}, {0x00A2,0x9B}, {0x00A3,0x9C}, {0x00A5,0x9D}, {0x00AA,0xA6},
    {0x00AB,0xAE}, {0x00AC,0xAA}, {0x00B0,0xF8}, {0x00B1,0xF1}, {0x00B2,0xFD}, {0x00B5,0xE6},
    {0x00B7,0xFA}, {0x00BA,0xA7}, {0x00BB,0xAF}, {0x00BC,0xAC}, {0x00BD,0xAB}, {0x00BF,0xA8},
    {0x00C4,0x8E}, {0x00C5,0x8F}, {0x00C6,0x92}, {0x00C7,0x80}, {0x00C9,0x90}, {0x00D1,0xA5},
    {0x00D6,0x99}, {0x00DC,0x9A}, {0x00DF,0xE1}, {0x00E0,0x85}, {0x00E1,0xA0}, {0x00E2,0x83},
    {0x00E4,0x84}, {0x00E5,0x86}, {0x00E6,0x91}, {0x00E7,0x87}, {0x00E8,0x8A}, {0x00E9,0x82},
    {0x00EA,0x88}, {0x00EB,0x89}, {0x00EC,0x8D}, {0x00ED,0xA1}, {0x00EE,0x8C}, {0x00EF,0x8B},
    {0x00F1,0xA4}, {0x00F2,0x95}, {0x00F3,0xA2}, {0x00F4,0x93}, {0x00F6,0x94}, {0x00F7,0xF6},
    {0x00F9,0x97}, {0x00FA,0xA3}, {0x00FB,0x96}, {0x00FC,0x81}, {0x00FF,0x98},
    {0x0192,0x9F}, {0x0393,0xE2}, {0x0398,0xE9}, {0x03A3,0xE4}, {0x03A6,0xE8}, {0x03A9,0xEA},
    {0x03B1,0xE0}, {0x03B4,0xEB}, {0x03B5,0xEE}, {0x03C0,0xE3}, {0x03C3,0xE5}, {0x03C4,0xE7},
    {0x03C6,0xED}, {0x2022,0x07}, {0x203C,0x13}, {0x207F,0xFC}, {0x20A7,0x9E}, {0x2190,0x1B},
    {0x2191,0x18}, {0x2192,0x1A}, {0x2193,0x19}, {0x2194,0x1D}, {0x2195,0x12}, {0x21A8,0x17},
    {0x2219,0xF9}, {0x221A,0xFB}, {0x221E,0xEC}, {0x221F,0x1C}, {0x2229,0xEF}, {0x2248,0xF7},
    {0x2261,0xF0}, {0x2264,0xF3}, {0x2265,0xF2}, {0x2310,0xA9}, {0x2320,0xF4}, {0x2321,0xF5},
    /* box drawing, blocks, shades */
    {0x2500,0xC4}, {0x2502,0xB3}, {0x250C,0xDA}, {0x2510,0xBF}, {0x2514,0xC0}, {0x2518,0xD9},
    {0x251C,0xC3}, {0x2524,0xB4}, {0x252C,0xC2}, {0x2534,0xC1}, {0x253C,0xC5},
    {0x2550,0xCD}, {0x2551,0xBA}, {0x2552,0xD5}, {0x2553,0xD6}, {0x2554,0xC9}, {0x2555,0xB8},
    {0x2556,0xB7}, {0x2557,0xBB}, {0x2558,0xD4}, {0x2559,0xD3}, {0x255A,0xC8}, {0x255B,0xBE},
    {0x255C,0xBD}, {0x255D,0xBC}, {0x255E,0xC6}, {0x255F,0xC7}, {0x2560,0xCC}, {0x2561,0xB5},
    {0x2562,0xB6}, {0x2563,0xB9}, {0x2564,0xD1}, {0x2565,0xD2}, {0x2566,0xCB}, {0x2567,0xCF},
    {0x2568,0xD0}, {0x2569,0xCA}, {0x256A,0xD8}, {0x256B,0xD7}, {0x256C,0xCE},
    {0x2580,0xDF}, {0x2584,0xDC}, {0x2588,0xDB}, {0x258C,0xDD}, {0x2590,0xDE},
    {0x2591,0xB0}, {0x2592,0xB1}, {0x2593,0xB2}, {0x25A0,0xFE}, {0x25AC,0x16}, {0x25B2,0x1E},
    {0x25BA,0x10}, {0x25BC,0x1F}, {0x25C4,0x11}, {0x25CB,0x09}, {0x25D8,0x08}, {0x25D9,0x0A},
    {0x263A,0x01}, {0x263B,0x02}, {0x263C,0x0F}, {0x2640,0x0C}, {0x2642,0x0B}, {0x2660,0x06},
    {0x2663,0x05}, {0x2665,0x03}, {0x2666,0x04}, {0x266A,0x0D}, {0x266B,0x0E},
    /* near enough: heavy/rounded/dashed lines to the plain ones, other blocks to shades */
    {0x2501,0xC4}, {0x2503,0xB3}, {0x250F,0xDA}, {0x2513,0xBF}, {0x2517,0xC0}, {0x251B,0xD9},
    {0x256D,0xDA}, {0x256E,0xBF}, {0x256F,0xD9}, {0x2570,0xC0}, {0x2504,0xC4}, {0x2506,0xB3},
    {0x2508,0xC4}, {0x250A,0xB3}, {0x254C,0xC4}, {0x254E,0xB3},
    {0x2581,0xDC}, {0x2582,0xDC}, {0x2583,0xDC}, {0x2585,0xDB}, {0x2586,0xDB}, {0x2587,0xDB},
    {0x2589,0xDB}, {0x258A,0xDB}, {0x258B,0xDD}, {0x258D,0xDD}, {0x258E,0xDD}, {0x258F,0xDD},
    {0x25A1,0xFE}, {0x25CF,0x07}, {0x25E6,0xF9}, {0x2026,'.'}, {0x2013,'-'}, {0x2014,'-'},
    {0x2018,'\''}, {0x2019,'\''}, {0x201C,'"'}, {0x201D,'"'}, {0x2713,0xFB}, {0x2714,0xFB},
    {0x2717,'x'}, {0x2718,'x'}, {0x2605,0x0F}, {0x2606,0x0F}, {0x00D7,'x'},
};

unsigned char vt_cp437(uint32_t cp) {
    if (cp >= 0x20 && cp < 0x7F) return (unsigned char)cp;
    for (size_t i = 0; i < sizeof cp437_map / sizeof cp437_map[0]; i++)
        if (cp437_map[i].cp == cp) return cp437_map[i].c;
    return 0;
}

/* DEC special graphics (ESC ( 0): the line-drawing set curses uses when it does not trust UTF-8. */
static unsigned char dec_graphics(unsigned char ch) {
    switch (ch) {
    case 'j': return 0xD9; case 'k': return 0xBF; case 'l': return 0xDA; case 'm': return 0xC0;
    case 'n': return 0xC5; case 'q': return 0xC4; case 't': return 0xC3; case 'u': return 0xB4;
    case 'v': return 0xC1; case 'w': return 0xC2; case 'x': return 0xB3; case 'a': return 0xB1;
    case '`': return 0x04; case 'f': return 0xF8; case 'g': return 0xF1; case 'o': return 0xC4;
    case 'p': return 0xC4; case 'r': return 0xC4; case 's': return 0xC4; case 'y': return 0xF3;
    case 'z': return 0xF2; case '{': return 0xE3; case '|': return 0xF7; case '}': return 0x9C;
    case '~': return 0xFA; case '0': return 0xDB; case '_': return ' ';
    case '+': return 0x1A; case ',': return 0x1B; case '-': return 0x18; case '.': return 0x19;
    case 'h': return 0xB0; case 'i': return 0x0F;
    default:  return ch;
    }
}

/* ---- screen operations ---- */

static void scroll_up(vt_t *t, int top, int bot, int n) {
    if (n <= 0) return;
    if (n > bot - top + 1) n = bot - top + 1;
    vt_cell_t b = blank(t);
    for (int r = top; r <= bot; r++) {
        if (r + n <= bot) memcpy(t->cells[r], t->cells[r + n], sizeof t->cells[r]);
        else for (int c = 0; c < VT_COLS; c++) t->cells[r][c] = b;
    }
    mark(t);
}

static void scroll_down(vt_t *t, int top, int bot, int n) {
    if (n <= 0) return;
    if (n > bot - top + 1) n = bot - top + 1;
    vt_cell_t b = blank(t);
    for (int r = bot; r >= top; r--) {
        if (r - n >= top) memcpy(t->cells[r], t->cells[r - n], sizeof t->cells[r]);
        else for (int c = 0; c < VT_COLS; c++) t->cells[r][c] = b;
    }
    mark(t);
}

static void linefeed(vt_t *t) {
    if (t->row == t->bot) scroll_up(t, t->top, t->bot, 1);
    else if (t->row < VT_ROWS - 1) t->row++;
    t->wrap_pending = 0;
}

static void reverse_index(vt_t *t) {
    if (t->row == t->top) scroll_down(t, t->top, t->bot, 1);
    else if (t->row > 0) t->row--;
    t->wrap_pending = 0;
}

static void put_glyph(vt_t *t, unsigned char ch) {
    if (t->wrap_pending) {
        if (t->autowrap) { t->col = 0; linefeed(t); }
        else t->col = VT_COLS - 1;
        t->wrap_pending = 0;
    }
    if (t->insert_mode) {
        for (int c = VT_COLS - 1; c > t->col; c--) t->cells[t->row][c] = t->cells[t->row][c - 1];
    }
    vt_cell_t *cell = &t->cells[t->row][t->col];
    cell->ch = ch; cell->fg = t->fg; cell->bg = t->bg; cell->attr = t->attr;
    if (t->col == VT_COLS - 1) t->wrap_pending = 1;
    else t->col++;
    mark(t);
}

static void clamp_cursor(vt_t *t) {
    if (t->col < 0) t->col = 0;
    if (t->col >= VT_COLS) t->col = VT_COLS - 1;
    if (t->row < 0) t->row = 0;
    if (t->row >= VT_ROWS) t->row = VT_ROWS - 1;
    t->wrap_pending = 0;
}

static void move_to(vt_t *t, int row, int col) {
    if (t->origin_mode) {
        row += t->top;
        if (row > t->bot) row = t->bot;
        if (row < t->top) row = t->top;
    }
    t->row = row; t->col = col;
    clamp_cursor(t);
}

static void erase_row(vt_t *t, int r, int c0, int c1) {
    vt_cell_t b = blank(t);
    for (int c = c0; c <= c1 && c < VT_COLS; c++) if (c >= 0) t->cells[r][c] = b;
    mark(t);
}

static void swap_screen(vt_t *t, int to_alt) {
    if (to_alt == t->alt_active) return;
    for (int r = 0; r < VT_ROWS; r++)
        for (int c = 0; c < VT_COLS; c++) {
            vt_cell_t x = t->cells[r][c]; t->cells[r][c] = t->alt[r][c]; t->alt[r][c] = x;
        }
    t->alt_active = to_alt;
    mark(t);
}

static void save_cursor(vt_t *t) {
    t->saved_row = t->row; t->saved_col = t->col;
    t->saved_fg = t->fg; t->saved_bg = t->bg; t->saved_attr = t->attr;
}

static void restore_cursor(vt_t *t) {
    t->row = t->saved_row; t->col = t->saved_col;
    t->fg = t->saved_fg; t->bg = t->saved_bg; t->attr = t->saved_attr;
    clamp_cursor(t);
}

static void reply(vt_t *t, const char *s) {
    int n = (int)strlen(s);
    if (t->reply_len + n <= (int)sizeof t->reply) { memcpy(t->reply + t->reply_len, s, (size_t)n); t->reply_len += n; }
}

/* 256-colour index to the nearest of the 16 ANSI colours: a channel at level 3 of 5 or more is on;
   a colour with a channel at the top level is the bright variant (xterm's 196 is bright red). */
static uint8_t color256(int n) {
    if (n < 16) return (uint8_t)n;
    if (n >= 232) return n < 238 ? 0 : n < 244 ? 8 : n < 250 ? 7 : 15;   /* grey ramp */
    n -= 16;
    int r = n / 36, g = (n / 6) % 6, b = n % 6;
    uint8_t idx = (uint8_t)((r >= 3 ? 1 : 0) | (g >= 3 ? 2 : 0) | (b >= 3 ? 4 : 0));
    if (idx == 0) return (r + g + b) > 0 ? 8 : 0;   /* dark colours that are not black: dark grey */
    int top = r == 5 || g == 5 || b == 5;
    return top ? (uint8_t)(idx | 8) : idx;
}

/* True colour to the 16: a channel at 128 or more is on; an "off" channel still at 64 or more
   makes it the bright (pastel) variant, as does pure white. xterm's blue #0000ee stays blue. */
static uint8_t color_rgb(int r, int g, int b) {
    int on_r = r >= 128, on_g = g >= 128, on_b = b >= 128;
    uint8_t idx = (uint8_t)((on_r ? 1 : 0) | (on_g ? 2 : 0) | (on_b ? 4 : 0));
    if (idx == 0) return r + g + b > 96 ? 8 : 0;
    int pastel = (!on_r && r >= 64) || (!on_g && g >= 64) || (!on_b && b >= 64);
    int white = idx == 7 && r >= 240 && g >= 240 && b >= 240;
    return (pastel || white) ? (uint8_t)(idx | 8) : idx;
}

static void sgr(vt_t *t) {
    if (t->nparams == 0) { t->params[0] = 0; t->nparams = 1; }
    for (int i = 0; i < t->nparams; i++) {
        int p = t->params[i];
        if (p == 0) { t->fg = VT_DEFAULT_FG; t->bg = VT_DEFAULT_BG; t->attr = 0; }
        else if (p == 1) t->attr |= VT_BOLD;
        else if (p == 2) t->attr |= VT_DIM;
        else if (p == 4) t->attr |= VT_UNDERLINE;
        else if (p == 7) t->attr |= VT_INVERSE;
        else if (p == 22) t->attr &= (uint8_t)~(VT_BOLD | VT_DIM);
        else if (p == 24) t->attr &= (uint8_t)~VT_UNDERLINE;
        else if (p == 27) t->attr &= (uint8_t)~VT_INVERSE;
        else if (p >= 30 && p <= 37) t->fg = (uint8_t)(p - 30);
        else if (p == 39) t->fg = VT_DEFAULT_FG;
        else if (p >= 40 && p <= 47) t->bg = (uint8_t)(p - 40);
        else if (p == 49) t->bg = VT_DEFAULT_BG;
        else if (p >= 90 && p <= 97) t->fg = (uint8_t)(p - 90 + 8);
        else if (p >= 100 && p <= 107) t->bg = (uint8_t)(p - 100 + 8);
        else if ((p == 38 || p == 48) && i + 1 < t->nparams) {
            uint8_t c = p == 38 ? t->fg : t->bg;
            if (t->params[i + 1] == 5 && i + 2 < t->nparams) { c = color256(t->params[i + 2]); i += 2; }
            else if (t->params[i + 1] == 2 && i + 4 < t->nparams) {
                c = color_rgb(t->params[i + 2], t->params[i + 3], t->params[i + 4]); i += 4;
            } else break;
            if (p == 38) t->fg = c; else t->bg = c;
        }
    }
}

static void dec_mode(vt_t *t, int set) {
    for (int i = 0; i < t->nparams; i++) {
        switch (t->params[i]) {
        case 1:    t->app_cursor = set; break;
        case 6:    t->origin_mode = set; move_to(t, 0, 0); break;
        case 7:    t->autowrap = set; break;
        case 25:   t->cursor_visible = set; mark(t); break;
        case 1000: case 1002: case 1003:
            t->mouse_mode = set ? t->params[i] : 0; break;
        case 1006: t->mouse_sgr = set; break;
        case 47: case 1047:
            swap_screen(t, set); break;
        case 1049:
            if (set) { save_cursor(t); swap_screen(t, 1); t->top = 0; t->bot = VT_ROWS - 1;
                       for (int r = 0; r < VT_ROWS; r++) erase_row(t, r, 0, VT_COLS - 1); }
            else { swap_screen(t, 0); restore_cursor(t); }
            break;
        default: break;   /* ?12 blink, ?1004 focus, ?2004 bracketed paste, ...: ignored */
        }
    }
}

static void csi(vt_t *t, unsigned char f) {
    int p0 = t->nparams > 0 ? t->params[0] : 0;
    int p1 = t->nparams > 1 ? t->params[1] : 0;
    int n = p0 > 0 ? p0 : 1;
    if (t->priv == '?') {
        if (f == 'h') dec_mode(t, 1);
        else if (f == 'l') dec_mode(t, 0);
        return;
    }
    if (t->priv) return;   /* > = etc.: XTVERSION, modifyOtherKeys and friends: ignored */
    switch (f) {
    case 'A': t->row -= n; if (t->row < t->top) t->row = t->top; t->wrap_pending = 0; break;
    case 'B': t->row += n; if (t->row > t->bot) t->row = t->bot; t->wrap_pending = 0; break;
    case 'C': t->col += n; clamp_cursor(t); break;
    case 'D': t->col -= n; clamp_cursor(t); break;
    case 'E': t->row += n; t->col = 0; if (t->row > t->bot) t->row = t->bot; t->wrap_pending = 0; break;
    case 'F': t->row -= n; t->col = 0; if (t->row < t->top) t->row = t->top; t->wrap_pending = 0; break;
    case 'G': case '`': t->col = n - 1; clamp_cursor(t); break;
    case 'd': t->row = n - 1; clamp_cursor(t); break;
    case 'H': case 'f': move_to(t, (p0 > 0 ? p0 : 1) - 1, (p1 > 0 ? p1 : 1) - 1); break;
    case 'J':
        if (p0 == 0) { erase_row(t, t->row, t->col, VT_COLS - 1);
                       for (int r = t->row + 1; r < VT_ROWS; r++) erase_row(t, r, 0, VT_COLS - 1); }
        else if (p0 == 1) { for (int r = 0; r < t->row; r++) erase_row(t, r, 0, VT_COLS - 1);
                            erase_row(t, t->row, 0, t->col); }
        else if (p0 == 2 || p0 == 3) for (int r = 0; r < VT_ROWS; r++) erase_row(t, r, 0, VT_COLS - 1);
        t->wrap_pending = 0;
        break;
    case 'K':
        if (p0 == 0) erase_row(t, t->row, t->col, VT_COLS - 1);
        else if (p0 == 1) erase_row(t, t->row, 0, t->col);
        else if (p0 == 2) erase_row(t, t->row, 0, VT_COLS - 1);
        t->wrap_pending = 0;
        break;
    case 'L': if (t->row >= t->top && t->row <= t->bot) scroll_down(t, t->row, t->bot, n); t->wrap_pending = 0; break;
    case 'M': if (t->row >= t->top && t->row <= t->bot) scroll_up(t, t->row, t->bot, n); t->wrap_pending = 0; break;
    case 'P': {   /* delete characters */
        if (n > VT_COLS - t->col) n = VT_COLS - t->col;
        for (int c = t->col; c < VT_COLS; c++)
            t->cells[t->row][c] = c + n < VT_COLS ? t->cells[t->row][c + n] : blank(t);
        mark(t); t->wrap_pending = 0; break;
    }
    case '@': {   /* insert blanks */
        if (n > VT_COLS - t->col) n = VT_COLS - t->col;
        for (int c = VT_COLS - 1; c >= t->col + n; c--) t->cells[t->row][c] = t->cells[t->row][c - n];
        for (int c = t->col; c < t->col + n; c++) t->cells[t->row][c] = blank(t);
        mark(t); t->wrap_pending = 0; break;
    }
    case 'X': erase_row(t, t->row, t->col, t->col + n - 1); t->wrap_pending = 0; break;
    case 'S': scroll_up(t, t->top, t->bot, n); break;
    case 'T': scroll_down(t, t->top, t->bot, n); break;
    case 'b': {   /* repeat the last glyph */
        unsigned char last = t->col > 0 ? t->cells[t->row][t->col - 1].ch : ' ';
        for (int i = 0; i < n && i < VT_COLS; i++) put_glyph(t, last);
        break;
    }
    case 'g': if (p0 == 3) memset(t->tabs, 0, sizeof t->tabs); else if (p0 == 0) t->tabs[t->col] = 0; break;
    case 'h': if (p0 == 4) t->insert_mode = 1; break;
    case 'l': if (p0 == 4) t->insert_mode = 0; break;
    case 'm': sgr(t); break;
    case 'n':
        if (p0 == 6) {
            /* Rows and columns are at most two digits. */
            char buf[16]; int k = 0, r = t->row + 1, c = t->col + 1;
            buf[k++] = 0x1B; buf[k++] = '[';
            if (r >= 10) buf[k++] = (char)('0' + r / 10);
            buf[k++] = (char)('0' + r % 10);
            buf[k++] = ';';
            if (c >= 10) buf[k++] = (char)('0' + c / 10);
            buf[k++] = (char)('0' + c % 10);
            buf[k++] = 'R'; buf[k] = 0;
            reply(t, buf);
        }
        else if (p0 == 5) reply(t, "\x1b[0n");
        break;
    case 'c': reply(t, "\x1b[?1;2c"); break;   /* VT100 with AVO */
    case 'r':
        if (t->nparams < 2 || p1 <= 0) p1 = VT_ROWS;
        if (p0 <= 0) p0 = 1;
        if (p1 > VT_ROWS) p1 = VT_ROWS;
        if (p0 < p1) { t->top = p0 - 1; t->bot = p1 - 1; move_to(t, 0, 0); }
        break;
    case 's': save_cursor(t); break;
    case 'u': restore_cursor(t); break;
    default: break;   /* t (window ops), q (cursor style), ...: ignored */
    }
}

static void esc(vt_t *t, unsigned char ch) {
    switch (ch) {
    case '7': save_cursor(t); break;
    case '8': restore_cursor(t); break;
    case 'D': linefeed(t); break;
    case 'E': t->col = 0; linefeed(t); break;
    case 'M': reverse_index(t); break;
    case 'H': t->tabs[t->col] = 1; break;
    case 'c': vt_init(t); break;   /* RIS */
    case '=': case '>': break;   /* keypad modes */
    case 'Z': reply(t, "\x1b[?1;2c"); break;
    default: break;
    }
}

static void control(vt_t *t, unsigned char ch) {
    switch (ch) {
    case 0x07: t->bell = 1; break;
    case 0x08: if (t->wrap_pending) t->wrap_pending = 0; else if (t->col > 0) t->col--; break;
    case 0x09: { t->wrap_pending = 0; int c = t->col + 1;
                 while (c < VT_COLS - 1 && !t->tabs[c]) c++;
                 t->col = c < VT_COLS ? c : VT_COLS - 1; break; }
    case 0x0A: case 0x0B: case 0x0C: linefeed(t); break;
    case 0x0D: t->col = 0; t->wrap_pending = 0; break;
    case 0x0E: t->shift = 1; break;
    case 0x0F: t->shift = 0; break;
    default: break;
    }
}

static void glyph_cp(vt_t *t, uint32_t cp) {
    unsigned char ch;
    if (cp < 0x80) {
        ch = (unsigned char)cp;
        if (t->charset[t->shift] == 1) ch = dec_graphics(ch);
    } else {
        ch = vt_cp437(cp);
        if (!ch) ch = '?';
    }
    put_glyph(t, ch);
}

void vt_feed(vt_t *t, const unsigned char *buf, size_t n) {
    for (size_t i = 0; i < n; i++) {
        unsigned char b = buf[i];

        /* A control byte acts in every state except inside an OSC/DCS string (where ESC and BEL end it). */
        if (b == 0x1B && t->state != S_OSC && t->state != S_DCS) {
            t->state = S_ESC; t->utf_left = 0; continue;
        }
        if (b == 0x18 || b == 0x1A) { t->state = S_GROUND; continue; }   /* CAN, SUB */

        switch (t->state) {
        case S_GROUND:
            if (b < 0x20 || b == 0x7F) { if (b != 0x7F) control(t, b); t->utf_left = 0; }
            else if (b < 0x80) { t->utf_left = 0; glyph_cp(t, b); }
            else if (t->utf_left > 0 && (b & 0xC0) == 0x80) {
                t->utf_cp = (t->utf_cp << 6) | (b & 0x3F);
                if (--t->utf_left == 0) glyph_cp(t, t->utf_cp);
            } else if ((b & 0xE0) == 0xC0) { t->utf_cp = b & 0x1F; t->utf_left = 1; }
            else if ((b & 0xF0) == 0xE0) { t->utf_cp = b & 0x0F; t->utf_left = 2; }
            else if ((b & 0xF8) == 0xF0) { t->utf_cp = b & 0x07; t->utf_left = 3; }
            else { t->utf_left = 0; put_glyph(t, '?'); }
            break;
        case S_ESC:
            if (b == '[') { t->state = S_CSI; t->nparams = 0; t->priv = 0; t->inter = 0;
                            memset(t->params, 0, sizeof t->params); }
            else if (b == ']') { t->state = S_OSC; t->osc_len = 0; }
            else if (b == 'P' || b == '^' || b == '_' || b == 'X') t->state = S_DCS;
            else if (b == '(' || b == ')' || b == '*' || b == '+') { t->state = S_CHARSET; t->inter = (char)b; }
            else if (b == '#' || b == ' ' || b == '%') { t->state = S_CHARSET; t->inter = 0; }   /* DECALN etc.: eat one byte */
            else { t->state = S_GROUND; esc(t, b); }
            break;
        case S_CHARSET:
            if (t->inter == '(') t->charset[0] = b == '0' ? 1 : 0;
            else if (t->inter == ')') t->charset[1] = b == '0' ? 1 : 0;
            t->state = S_GROUND;
            break;
        case S_CSI:
            if (b >= '0' && b <= '9') {
                if (t->nparams == 0) t->nparams = 1;
                int *p = &t->params[t->nparams - 1];
                if (*p < 100000) *p = *p * 10 + (b - '0');
            } else if (b == ';' || b == ':') {
                if (t->nparams == 0) t->nparams = 1;
                if (t->nparams < 16) t->params[t->nparams++] = 0;
            } else if (b >= 0x3C && b <= 0x3F) t->priv = b;          /* < = > ? */
            else if (b >= 0x20 && b <= 0x2F) t->inter = (char)b;      /* intermediates */
            else if (b >= 0x40 && b <= 0x7E) {
                t->state = S_GROUND;
                if (!t->inter) csi(t, b);                             /* "ESC [ ... SP q" etc.: ignored */
            } else if (b < 0x20) control(t, b);
            else t->state = S_GROUND;
            break;
        case S_OSC:
            if (b == 0x07) { t->state = S_GROUND; if (t->osc_len > 1 && (t->osc[0] == '0' || t->osc[0] == '2')) t->title_changed = 1; }
            else if (b == 0x1B) t->state = S_OSC_ESC;
            else if (t->osc_len < (int)sizeof t->osc - 1) t->osc[t->osc_len++] = (char)b;
            break;
        case S_OSC_ESC:
            /* ESC \ ends the string; any other ESC sequence after an unterminated OSC starts fresh. */
            if (b == '\\') { t->state = S_GROUND; if (t->osc_len > 1 && (t->osc[0] == '0' || t->osc[0] == '2')) t->title_changed = 1; }
            else { t->state = S_ESC; i--; }
            break;
        case S_DCS:
            if (b == 0x07) t->state = S_GROUND;
            else if (b == 0x1B) t->state = S_DCS_ESC;
            break;
        case S_DCS_ESC:
            if (b == '\\') t->state = S_GROUND;
            else { t->state = S_ESC; i--; }
            break;
        default:
            t->state = S_GROUND;
            break;
        }
    }
}

int vt_take_reply(vt_t *t, char *out, int cap) {
    int n = t->reply_len < cap ? t->reply_len : cap;
    memcpy(out, t->reply, (size_t)n);
    t->reply_len = 0;
    return n;
}

int vt_key_bytes(const vt_t *t, vt_key_t k, char *out, int cap) {
    const char *s;
    char arrow[4] = { 0x1B, t->app_cursor ? 'O' : '[', 0, 0 };
    switch (k) {
    case VTK_UP:    arrow[2] = 'A'; s = arrow; break;
    case VTK_DOWN:  arrow[2] = 'B'; s = arrow; break;
    case VTK_RIGHT: arrow[2] = 'C'; s = arrow; break;
    case VTK_LEFT:  arrow[2] = 'D'; s = arrow; break;
    case VTK_HOME:  arrow[2] = 'H'; s = arrow; break;
    case VTK_END:   arrow[2] = 'F'; s = arrow; break;
    case VTK_ENTER: s = "\r"; break;
    case VTK_ESC:   s = "\x1b"; break;
    case VTK_TAB:   s = "\t"; break;
    case VTK_BACKSPACE: s = "\x7f"; break;
    case VTK_SPACE: s = " "; break;
    case VTK_PGUP:  s = "\x1b[5~"; break;
    case VTK_PGDN:  s = "\x1b[6~"; break;
    case VTK_INSERT: s = "\x1b[2~"; break;
    case VTK_DELETE: s = "\x1b[3~"; break;
    case VTK_F1:  s = "\x1bOP"; break;
    case VTK_F2:  s = "\x1bOQ"; break;
    case VTK_F3:  s = "\x1bOR"; break;
    case VTK_F4:  s = "\x1bOS"; break;
    case VTK_F5:  s = "\x1b[15~"; break;
    case VTK_F6:  s = "\x1b[17~"; break;
    case VTK_F7:  s = "\x1b[18~"; break;
    case VTK_F8:  s = "\x1b[19~"; break;
    case VTK_F9:  s = "\x1b[20~"; break;
    case VTK_F10: s = "\x1b[21~"; break;
    default: return 0;
    }
    int n = (int)strlen(s);
    if (n > cap) n = cap;
    memcpy(out, s, (size_t)n);
    return n;
}

static int put_num(char *out, int at, int v) {
    char tmp[8]; int n = 0;
    if (v <= 0) tmp[n++] = '0';
    while (v > 0 && n < 7) { tmp[n++] = (char)('0' + v % 10); v /= 10; }
    while (n > 0) out[at++] = tmp[--n];
    return at;
}

int vt_mouse_bytes(const vt_t *t, int button, int release, int motion, int col, int row, char *out, int cap) {
    if (!t->mouse_mode || cap < 12) return 0;
    if (motion) {
        if (t->mouse_mode == 1000) return 0;                       /* clicks only */
        if (t->mouse_mode == 1002 && button < 0) return 0;         /* drag needs a held button */
    }
    int cb = button < 0 ? 3 : button;                              /* 3 = no button (motion) */
    if (motion) cb += 32;
    int n = 0;
    if (t->mouse_sgr) {
        out[n++] = 0x1B; out[n++] = '['; out[n++] = '<';
        n = put_num(out, n, cb); out[n++] = ';';
        n = put_num(out, n, col + 1); out[n++] = ';';
        n = put_num(out, n, row + 1);
        out[n++] = release ? 'm' : 'M';
    } else {
        /* X10 encoding: one byte each, offset 32; a release is button 3; fits our 90 x 30. */
        if (release) cb = (cb & 32) | 3;
        out[n++] = 0x1B; out[n++] = '['; out[n++] = 'M';
        out[n++] = (char)(32 + cb); out[n++] = (char)(33 + col); out[n++] = (char)(33 + row);
    }
    return n;
}

size_t vt_dump(const vt_t *t, char *buf, size_t len) {
    size_t at = 0;
    for (int r = 0; r < VT_ROWS; r++) {
        for (int c = 0; c < VT_COLS; c++) {
            unsigned char ch = t->cells[r][c].ch;
            if (ch < 0x20 || ch >= 0x7F) ch = '?';
            if (at + 1 < len) buf[at] = (char)ch;
            at++;
        }
        if (at + 1 < len) buf[at] = '\n';
        at++;
    }
    if (len) buf[at < len ? at : len - 1] = '\0';
    return at;
}
