/* Redux Terminal on-screen keys. See osk.h. */
#include "osk.h"
#include <string.h>

#define K(l, b)        { l, b, NULL, 0, (int)sizeof(l) - 1 }
#define KS(l, b, s)    { l, b, s, 0, (int)sizeof(l) - 1 }
#define MODE(l, m)     { l, NULL, NULL, m, (int)sizeof(l) - 1 }

static const osk_key_t row0[] = {
    K("F1", "\x1bOP"), K("F2", "\x1bOQ"), K("F3", "\x1bOR"), K("F4", "\x1bOS"), K("F5", "\x1b[15~"),
    K("F6", "\x1b[17~"), K("F7", "\x1b[18~"), K("F8", "\x1b[19~"), K("F9", "\x1b[20~"), K("F10", "\x1b[21~"),
    K("Esc", "\x1b"), K("Ins", "\x1b[2~"), K("Del", "\x1b[3~"),
    K("Home", "\x1b[H"), K("End", "\x1b[F"), K("PgUp", "\x1b[5~"), K("PgDn", "\x1b[6~"),
};
static const osk_key_t row1[] = {
    KS("`", "`", "~"), KS("1", "1", "!"), KS("2", "2", "@"), KS("3", "3", "#"), KS("4", "4", "$"),
    KS("5", "5", "%"), KS("6", "6", "^"), KS("7", "7", "&"), KS("8", "8", "*"), KS("9", "9", "("),
    KS("0", "0", ")"), KS("-", "-", "_"), KS("=", "=", "+"), K("Backspace", "\x7f"),
};
static const osk_key_t row2[] = {
    K("Tab", "\t"), K("q", "q"), K("w", "w"), K("e", "e"), K("r", "r"), K("t", "t"), K("y", "y"),
    K("u", "u"), K("i", "i"), K("o", "o"), K("p", "p"), KS("[", "[", "{"), KS("]", "]", "}"),
    KS("\\", "\\", "|"),
};
static const osk_key_t row3[] = {
    MODE("Shift", 1), K("a", "a"), K("s", "s"), K("d", "d"), K("f", "f"), K("g", "g"), K("h", "h"),
    K("j", "j"), K("k", "k"), K("l", "l"), KS(";", ";", ":"), KS("'", "'", "\""), K("Enter", "\r"),
};
static const osk_key_t row4[] = {
    MODE("Ctrl", 2), K("z", "z"), K("x", "x"), K("c", "c"), K("v", "v"), K("b", "b"), K("n", "n"),
    K("m", "m"), KS(",", ",", "<"), KS(".", ".", ">"), KS("/", "/", "?"), K("Space", " "),
};
static const osk_key_t row5[] = {   /* the shifted symbols without Shift, for shell lines */
    K("~", "~"), K("!", "!"), K("@", "@"), K("#", "#"), K("$", "$"), K("%", "%"), K("^", "^"),
    K("&", "&"), K("*", "*"), K("(", "("), K(")", ")"), K("_", "_"), K("+", "+"), K("{", "{"),
    K("}", "}"), K("|", "|"), K(":", ":"), K("\"", "\""), K("<", "<"), K(">", ">"), K("?", "?"),
};

static const struct { const osk_key_t *keys; int n; } rows[OSK_ROWS] = {
    { row0, (int)(sizeof row0 / sizeof row0[0]) }, { row1, (int)(sizeof row1 / sizeof row1[0]) },
    { row2, (int)(sizeof row2 / sizeof row2[0]) }, { row3, (int)(sizeof row3 / sizeof row3[0]) },
    { row4, (int)(sizeof row4 / sizeof row4[0]) }, { row5, (int)(sizeof row5 / sizeof row5[0]) },
};

void osk_init(osk_t *k) { memset(k, 0, sizeof *k); k->row = 2; k->col = 1; }   /* on "q" */

void osk_toggle(osk_t *k) { k->open = !k->open; k->ctrl = 0; }

int osk_cols(int row) { return (row >= 0 && row < OSK_ROWS) ? rows[row].n : 0; }

const osk_key_t *osk_key(int row, int col) {
    if (row < 0 || row >= OSK_ROWS || col < 0 || col >= rows[row].n) return NULL;
    return &rows[row].keys[col];
}

int osk_key_x(int row, int col) {
    int x = 1;   /* inside the box's left edge */
    for (int c = 0; c < col && c < osk_cols(row); c++) x += rows[row].keys[c].width + 2;   /* a space each side */
    return x;
}

void osk_move(osk_t *k, int drow, int dcol) {
    if (drow) {
        /* Keep the same screen position when changing rows: land on the key under the cursor. */
        int x = osk_key_x(k->row, k->col);
        int r = k->row + drow;
        if (r < 0) r = OSK_ROWS - 1;
        if (r >= OSK_ROWS) r = 0;
        k->row = r;
        int best = 0;
        for (int c = 0; c < osk_cols(r); c++) if (osk_key_x(r, c) <= x) best = c;
        k->col = best;
    }
    if (dcol) {
        int n = osk_cols(k->row);
        k->col = (k->col + dcol + n) % n;
    }
}

int osk_press(osk_t *k, char *out, int cap) {
    const osk_key_t *key = osk_key(k->row, k->col);
    if (!key) return 0;
    if (key->mode == 1) { k->shift = !k->shift; return 0; }
    if (key->mode == 2) { k->ctrl = !k->ctrl; return 0; }
    const char *b = key->bytes;
    char one[2] = { 0, 0 };
    int n;
    if (k->ctrl && b[0] && !b[1]) {
        /* Ctrl + one key: its control byte (Ctrl-C = 3, Ctrl-Space = NUL); other keys unchanged. */
        char c = b[0];
        if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 1);
        else if (c >= '@' && c <= '_') c = (char)(c - '@');
        else if (c == ' ') c = 0;
        one[0] = c;
        k->ctrl = 0;
        if (cap < 1) return 0;
        out[0] = one[0];
        return 1;
    }
    if (k->shift) {
        if (key->shifted) b = key->shifted;
        else if (b[0] >= 'a' && b[0] <= 'z' && !b[1]) { one[0] = (char)(b[0] - 'a' + 'A'); b = one; }
    }
    n = (int)strlen(b);
    if (n > cap) n = cap;
    memcpy(out, b, (size_t)n);
    return n;
}
