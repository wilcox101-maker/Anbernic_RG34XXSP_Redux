/* Redux on-screen keyboard layout (mock-up A). See osk.h. */
#include "osk.h"
#include <string.h>

#define K(l, b, w)      { l, b, NULL, 0, w }
#define KS(l, b, s, w)  { l, b, s, 0, w }
#define MODE(l, m, w)   { l, NULL, NULL, m, w }
#define BOTTOM MODE("Ctrl", 2, 3), K("Tab", "\t", 3), K("Space", " ", 8), K("Bksp", "\x7f", 3), K("Enter", "\r", 3)

/* Page abc */
static const osk_key_t a0[] = { K("q", "q", 2), K("w", "w", 2), K("e", "e", 2), K("r", "r", 2), K("t", "t", 2),
    K("y", "y", 2), K("u", "u", 2), K("i", "i", 2), K("o", "o", 2), K("p", "p", 2) };
static const osk_key_t a1[] = { K("a", "a", 2), K("s", "s", 2), K("d", "d", 2), K("f", "f", 2), K("g", "g", 2),
    K("h", "h", 2), K("j", "j", 2), K("k", "k", 2), K("l", "l", 2), KS("'", "'", "\"", 2) };
static const osk_key_t a2[] = { MODE("Shift", 1, 3), K("z", "z", 2), K("x", "x", 2), K("c", "c", 2), K("v", "v", 2),
    K("b", "b", 2), K("n", "n", 2), K("m", "m", 2), KS(",", ",", "<", 2), KS(".", ".", ">", 1) };
static const osk_key_t a3[] = { BOTTOM };

/* Page 123 */
static const osk_key_t n0[] = { KS("1", "1", "!", 2), KS("2", "2", "@", 2), KS("3", "3", "#", 2), KS("4", "4", "$", 2),
    KS("5", "5", "%", 2), KS("6", "6", "^", 2), KS("7", "7", "&", 2), KS("8", "8", "*", 2), KS("9", "9", "(", 2),
    KS("0", "0", ")", 2) };
static const osk_key_t n1[] = { KS("-", "-", "_", 2), KS("=", "=", "+", 2), KS("[", "[", "{", 2), KS("]", "]", "}", 2),
    KS("\\", "\\", "|", 2), KS(";", ";", ":", 2), KS("'", "'", "\"", 2), KS("`", "`", "~", 2), KS("/", "/", "?", 2),
    KS(",", ",", "<", 2) };
static const osk_key_t n2[] = { K("!", "!", 2), K("@", "@", 2), K("#", "#", 2), K("$", "$", 2), K("%", "%", 2),
    K("^", "^", 2), K("&", "&", 2), K("*", "*", 2), K("(", "(", 2), K(")", ")", 2) };
static const osk_key_t n3[] = { BOTTOM };

/* Page Fn: the arrows are drawn with CP437 arrow glyphs (0x18-0x1B). */
static const osk_key_t f0[] = { K("F1", "\x1bOP", 2), K("F2", "\x1bOQ", 2), K("F3", "\x1bOR", 2), K("F4", "\x1bOS", 2),
    K("F5", "\x1b[15~", 2), K("F6", "\x1b[17~", 2), K("F7", "\x1b[18~", 2), K("F8", "\x1b[19~", 2),
    K("F9", "\x1b[20~", 2), K("F10", "\x1b[21~", 2) };
static const osk_key_t f1[] = { K("Esc", "\x1b", 4), K("Ins", "\x1b[2~", 4), K("Del", "\x1b[3~", 4),
    K("Home", "\x1b[H", 4), K("End", "\x1b[F", 4) };
static const osk_key_t f2[] = { K("PgUp", "\x1b[5~", 4), K("PgDn", "\x1b[6~", 4), K("\x18", "\x1b[A", 4),
    K("_", "_", 2), K("+", "+", 2), K("{", "{", 2), K("}", "}", 2) };
static const osk_key_t f3[] = { K("~", "~", 2), K("|", "|", 2), K(":", ":", 2), K("\"", "\"", 2), K("<", "<", 2),
    K(">", ">", 2), K("?", "?", 2), K("\x1b", "\x1b[D", 2), K("\x19", "\x1b[B", 2), K("\x1a", "\x1b[C", 2) };

#define ROW(r) { r, (int)(sizeof r / sizeof r[0]) }
static const struct { const osk_key_t *keys; int n; } pages[OSK_PAGES][OSK_ROWS] = {
    { ROW(a0), ROW(a1), ROW(a2), ROW(a3) },
    { ROW(n0), ROW(n1), ROW(n2), ROW(n3) },
    { ROW(f0), ROW(f1), ROW(f2), ROW(f3) },
};
static const char *names[OSK_PAGES] = { "abc", "123", "Fn" };

void osk_init(osk_t *k) { memset(k, 0, sizeof *k); }   /* page abc, on "q" */

void osk_toggle(osk_t *k) { k->open = !k->open; k->ctrl = 0; }

const char *osk_page_name(int page) { return page >= 0 && page < OSK_PAGES ? names[page] : "?"; }

int osk_cols(int page, int row) {
    return (page >= 0 && page < OSK_PAGES && row >= 0 && row < OSK_ROWS) ? pages[page][row].n : 0;
}

const osk_key_t *osk_key(int page, int row, int col) {
    if (col < 0 || col >= osk_cols(page, row)) return NULL;
    return &pages[page][row].keys[col];
}

const osk_key_t *osk_cur(const osk_t *k) { return osk_key(k->page, k->row, k->col); }

int osk_key_x2(int page, int row, int col) {
    int x = 0;
    for (int c = 0; c < col && c < osk_cols(page, row); c++) x += pages[page][row].keys[c].units2;
    return x;
}

/* The key on (page, row) whose span holds half-unit position x2 (the last key if past the end). */
static int col_at(int page, int row, int x2) {
    int n = osk_cols(page, row), best = 0;
    for (int c = 0; c < n; c++) if (osk_key_x2(page, row, c) <= x2) best = c;
    return best;
}

static int mid_x2(const osk_t *k) {
    const osk_key_t *key = osk_cur(k);
    return osk_key_x2(k->page, k->row, k->col) + (key ? key->units2 / 2 : 0);
}

void osk_move(osk_t *k, int drow, int dcol) {
    if (drow) {
        int x = mid_x2(k);
        k->row = ((k->row + drow) % OSK_ROWS + OSK_ROWS) % OSK_ROWS;
        k->col = col_at(k->page, k->row, x);
    }
    if (dcol) {
        int n = osk_cols(k->page, k->row);
        if (n) k->col = ((k->col + dcol) % n + n) % n;
    }
}

void osk_page(osk_t *k, int dpage) {
    int x = mid_x2(k);
    k->page = ((k->page + dpage) % OSK_PAGES + OSK_PAGES) % OSK_PAGES;
    k->col = col_at(k->page, k->row, x);
}

int osk_label(const osk_key_t *key, int shift, char *out, int cap) {
    if (!out || cap < 1) return 0;
    out[0] = 0;
    if (!key) return 0;
    const char *src = key->label;
    char one[2] = { 0, 0 };
    if (shift && !key->mode) {
        if (key->shifted) src = key->shifted;
        else if (key->label[0] >= 'a' && key->label[0] <= 'z' && !key->label[1]) {
            one[0] = (char)(key->label[0] - 'a' + 'A'); src = one;
        }
    }
    int n = (int)strlen(src);
    if (n > cap - 1) n = cap - 1;
    memcpy(out, src, (size_t)n);
    out[n] = 0;
    return n;
}

int osk_press(osk_t *k, char *out, int cap) {
    const osk_key_t *key = osk_cur(k);
    if (!key) return 0;
    if (key->mode == 1) { k->shift = !k->shift; return 0; }
    if (key->mode == 2) { k->ctrl = !k->ctrl; return 0; }
    const char *b = key->bytes;
    char one[2] = { 0, 0 };
    if (k->ctrl && b[0] && !b[1]) {
        /* Ctrl + one key: its control byte (Ctrl-C = 3, Ctrl-Space = NUL); other keys unchanged. */
        char c = b[0];
        if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 1);
        else if (c >= '@' && c <= '_') c = (char)(c - '@');
        else if (c == ' ') c = 0;
        k->ctrl = 0;
        if (cap < 1) return 0;
        out[0] = c;
        return 1;
    }
    if (k->shift) {
        if (key->shifted) b = key->shifted;
        else if (b[0] >= 'a' && b[0] <= 'z' && !b[1]) { one[0] = (char)(b[0] - 'a' + 'A'); b = one; }
    }
    int n = (int)strlen(b);
    if (n > cap) n = cap;
    memcpy(out, b, (size_t)n);
    return n;
}
