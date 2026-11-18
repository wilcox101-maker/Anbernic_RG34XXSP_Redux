#ifndef REDUX_OSK_DRAW_H
#define REDUX_OSK_DRAW_H

/* On-screen keyboard drawing (mock-up A, Dale 2026-11-18 13:42): a BIOS/DOS panel (blue, double-
   line frame, title, page tabs, hint line) with framed key caps (outline, face, light top/left
   edge, dark bottom/right shadow), Redux's 8x16 font at 2x where a label fits. Focus: yellow face
   and a white ring; latched Shift/Ctrl: green face and a small triangle. Shared by Redux Terminal
   and the global OSK. Draws into a caller-owned ARGB8888 buffer; no SDL, no device access.

   Geometry follows the buffer width: under 1200 px (the 720 x 480 LCD) caps are 62 x 40 with 6 px
   gaps (panel 712 x 240); from 1200 px (HDMI 1280 x 720) 96 x 56 with 8 px gaps (panel 1064 x
   304). The panel sits at the bottom of the buffer, centred; every pixel outside it is
   0x00000000 (transparent), so the buffer can be as small as the panel. */

#include <stdint.h>
#include "osk.h"

typedef struct {
    uint32_t panel_bg, frame, title, tab_bg, tab_fg, tab_off, hint;
    uint32_t cap_line, cap_face, cap_hi, cap_shadow, cap_text;
    uint32_t focus_face, focus_ring, latch_face;
    const char *title_text, *hint_text;
} osk_draw_style_t;

/* BIOS/DOS palette (the VGA colours Redux's menus use). */
void osk_draw_style_default(osk_draw_style_t *s);

typedef struct { int unit, gap, cap_h; } osk_metrics_t;
void osk_draw_metrics(int screen_w, osk_metrics_t *m);

/* Panel size for a screen width (both multiples of 8 and 16 px). */
void osk_draw_panel_size(int screen_w, int *pw, int *ph);

/* Panel rectangle inside a w x h buffer. Returns 0, or -1 if the buffer is too small. */
int osk_draw_box(int w, int h, int *x, int *y, int *bw, int *bh);

/* Pixel rectangle of key (page, row, col) in a w x h buffer. Returns 0, or -1. */
int osk_draw_key_rect(int w, int h, int page, int row, int col, int *x, int *y, int *kw, int *kh);

/* Returns non-zero if a key is left off the keyboard (no cap drawn; a gap on the panel). */
typedef int (*osk_hidden_fn)(const osk_key_t *key, void *user);

/* Draw the keyboard for k (its page, highlight and latches). stride in pixels, >= w. hidden may be
   NULL. Returns 0, or -1 (nothing written) for a bad argument or a buffer smaller than the panel. */
int osk_draw(uint32_t *px, int w, int h, int stride, const osk_t *k, const osk_draw_style_t *s,
             osk_hidden_fn hidden, void *user);

#endif
