#ifndef REDUX_OSK_H
#define REDUX_OSK_H

/* Redux on-screen keyboard layout, shared by the Redux Terminal picker and the global OSK
   (Dale 2026-11-18 13:42, mock-up A: notes/2026-11-18-dale-decision-osk-ui.md). Three pages of
   at most 10 key units a row, 4 rows: abc (letters), 123 (digits and symbols), Fn (F-keys,
   editing keys, arrows, the remaining symbols). D-pad moves, A types, L1/R1 change page, B closes.
   Shift stays on until pressed again; Ctrl applies to the next key; both survive a page change.
   Pure: layout and bytes only. osk_draw.c draws it; redux_term.c sends the bytes to the pty and
   redux-osk turns keys into key codes. */

#define OSK_PAGES 3
#define OSK_ROWS 4
#define OSK_MAX_COLS 12
#define OSK_ROW_UNITS2 20   /* a row is 10 key units wide, counted in half units */

typedef struct {
    int page, row, col;  /* highlighted key */
    int shift, ctrl;     /* Shift stays on until pressed again; Ctrl applies to the next key */
    int open;
} osk_t;

typedef struct {
    const char *label;   /* what the key shows (Shift turns letters upper-case) */
    const char *bytes;   /* what it sends, or NULL for a mode key; "\x1b[5~" style for specials */
    const char *shifted; /* bytes (and label) with Shift on; NULL: upper-case of a letter, or the same */
    int mode;            /* 1 Shift, 2 Ctrl, 0 a key */
    int units2;          /* width in half key units (2 = one normal key) */
} osk_key_t;

void osk_init(osk_t *k);
void osk_toggle(osk_t *k);
const char *osk_page_name(int page);         /* "abc", "123", "Fn" */
int osk_cols(int page, int row);             /* keys on that row of that page (0 for an empty row) */
const osk_key_t *osk_key(int page, int row, int col);
const osk_key_t *osk_cur(const osk_t *k);    /* the highlighted key */
/* Where key col starts on its row, in half units from the row's left edge. */
int osk_key_x2(int page, int row, int col);
/* Move the highlight; rows and columns wrap. Changing rows lands on the key under the middle of
   the current one. */
void osk_move(osk_t *k, int drow, int dcol);
/* Next (+1) or previous (-1) page, wrapping; keeps the row and the position along it. */
void osk_page(osk_t *k, int dpage);
/* The highlighted key was pressed: bytes to send (0 for a mode key, which flips the mode instead).
   Ctrl turns a letter into its control byte and then switches off. */
int osk_press(osk_t *k, char *out, int cap);
/* The label a key shows with the given Shift state. Writes at most cap-1 chars plus NUL. */
int osk_label(const osk_key_t *key, int shift, char *out, int cap);

#endif
