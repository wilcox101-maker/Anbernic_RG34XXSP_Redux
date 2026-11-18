#ifndef REDUX_OSK_DRAW_H
#define REDUX_OSK_DRAW_H

/* Global on-screen keyboard: software drawing (queue item 0, Co-Programmer card in
   docs/process/roles.md). Renders the Redux Terminal key layout (frontend/term/osk.c, unchanged)
   into a caller-owned ARGB8888 buffer with Redux's 8x16 UI fonts (font_glyph). No SDL, no device
   access: redux-osk copies the result into its ION buffer on disp0 ch3/l0.

   Geometry: one cell = 8 x 16 px. The box is OSK_HEIGHT cells tall (8 rows = 128 px) and as many
   whole cells wide as the buffer allows; it sits at the bottom of the buffer, horizontally centred.
   Every pixel outside the box is 0x00000000 (fully transparent). */

#include <stdint.h>
#include "osk.h"

#define OSK_CELL_W 8
#define OSK_CELL_H 16
#define OSK_BOX_ROWS OSK_HEIGHT
#define OSK_BOX_H (OSK_BOX_ROWS * OSK_CELL_H)   /* 128 px */
/* Narrowest box: the widest row (row 0) fills cells 0-79, the right edge is cell 80: 81 cells, 648 px. */
#define OSK_MIN_COLS 81

typedef struct {
    uint32_t box_bg;     /* box and key background */
    uint32_t box_fg;     /* box lines and title */
    uint32_t key_fg;     /* key labels */
    uint32_t sel_bg, sel_fg;   /* highlighted key */
    uint32_t mode_bg, mode_fg; /* Shift / Ctrl while latched on */
    const char *title;   /* drawn on the top line; NULL for none */
} osk_draw_style_t;

/* Returns non-zero if a key is left off the keyboard (drawn as box background, label hidden).
   Kept generic; no key is hidden today (Dale 12:47: no OSK while RetroArch runs). */
typedef int (*osk_hidden_fn)(const osk_key_t *key, void *user);

/* Redux Terminal's look: light grey box, black text, highlighted key yellow, latched mode cyan. */
void osk_draw_style_default(osk_draw_style_t *s);

/* Settings > UI colours (uistyle.c): pass uis_colors[uistyle_get(UIS_TEXT / UIS_HFTEXT /
   UIS_WINCOLOR)] as 0xRRGGBB. Box = window colour, labels = menu text colour, lines and title =
   header/footer text colour, highlighted key = the two swapped (Settings already keeps menu text
   and window colour apart, so it always shows), latched mode = Light Cyan on black. */
void osk_draw_style_ui(osk_draw_style_t *s, uint32_t text_rgb, uint32_t hftext_rgb, uint32_t win_rgb);

/* The label a key shows with the current Shift state (Shift: letters upper-case, a key with a
   shifted symbol shows that symbol). Writes at most cap-1 chars plus NUL; returns the length. */
int osk_draw_label(const osk_key_t *key, int shift, char *out, int cap);

/* Box rectangle inside a w x h buffer. Returns 0, or -1 if the buffer is too small. */
int osk_draw_box(int w, int h, int *x, int *y, int *bw, int *bh);

/* Pixel rectangle of key (row, col) in a w x h buffer (the key's cells, one space each side
   included, as the highlight covers them). Returns 0, or -1 for no such key / buffer too small. */
int osk_draw_key_rect(int w, int h, int row, int col, int *x, int *y, int *kw, int *kh);

/* Draw the whole keyboard. stride is in pixels (uint32_t units), >= w. hidden may be NULL.
   Returns 0, or -1 (nothing written) if an argument is bad or the buffer is smaller than
   OSK_MIN_COLS * 8 x OSK_BOX_H. */
int osk_draw(uint32_t *px, int w, int h, int stride, const osk_t *k, const osk_draw_style_t *s,
             osk_hidden_fn hidden, void *user);

#endif
