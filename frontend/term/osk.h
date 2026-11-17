#ifndef REDUX_OSK_H
#define REDUX_OSK_H

/* Redux Terminal on-screen keys (Select; Dale's plan A, PR 2). A small keyboard drawn over the
   bottom rows of the terminal while the program keeps running. D-pad moves, A types the key, B or
   Select closes it. Pure: layout and bytes only; redux_term.c draws it and sends the bytes. */

#define OSK_ROWS 6
#define OSK_MAX_COLS 16
#define OSK_HEIGHT (OSK_ROWS + 2)       /* with the box's top and bottom lines */

typedef struct {
    int row, col;        /* highlighted key */
    int shift, ctrl;     /* Shift stays on until pressed again; Ctrl applies to the next key */
    int open;
} osk_t;

typedef struct {
    const char *label;   /* what the key shows (Shift turns letters upper-case) */
    const char *bytes;   /* what it sends, or NULL for a mode key; "\x1b[5~" style for specials */
    const char *shifted; /* bytes with Shift on (NULL: upper-case of bytes, or the same) */
    int mode;            /* 1 Shift, 2 Ctrl, 0 a key */
    int width;           /* cells, label included */
} osk_key_t;

void osk_init(osk_t *k);
void osk_toggle(osk_t *k);
int osk_cols(int row);                      /* keys on that row */
const osk_key_t *osk_key(int row, int col);
/* Move the highlight; the column is clamped on the new row. */
void osk_move(osk_t *k, int drow, int dcol);
/* The highlighted key was pressed: bytes to send (0 for a mode key, which flips the mode instead).
   Ctrl turns a letter into its control byte and then switches off. */
int osk_press(osk_t *k, char *out, int cap);
/* Column (in cells) where key col of a row starts, for drawing. */
int osk_key_x(int row, int col);

#endif
