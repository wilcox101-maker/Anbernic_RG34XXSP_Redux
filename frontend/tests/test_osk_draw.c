/* On-screen keyboard drawing (frontend/osk/osk_draw.c, mock-up A) into plain memory: no SDL, no
   device. Exit 0 = pass. */
#include "osk_draw.h"
#include "font.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fails;
static void expect(int ok, const char *what) { printf("%s %s\n", ok ? "ok  " : "FAIL", what); if (!ok) fails++; }

#define MAXW 1280
#define MAXH 720
#define PAD 0xDEADBEEFu

static uint32_t buf[MAXH * (MAXW + 16)];
static int W, H, STRIDE;

static void setup(int w, int h) { W = w; H = h; STRIDE = w + 16; for (int i = 0; i < H * STRIDE; i++) buf[i] = PAD; }

static int count(int x, int y, int w, int h, uint32_t c) {
    int n = 0;
    for (int j = 0; j < h; j++) for (int i = 0; i < w; i++) n += buf[(y + j) * STRIDE + x + i] == c;
    return n;
}

static int find(int page, const char *label, int *row, int *col) {
    for (int r = 0; r < OSK_ROWS; r++)
        for (int c = 0; c < osk_cols(page, r); c++)
            if (!strcmp(osk_key(page, r, c)->label, label)) { *row = r; *col = c; return 1; }
    return 0;
}

static int hide_esc(const osk_key_t *key, void *user) { (void)user; return key && !strcmp(key->label, "Esc"); }

static int draw(const osk_t *k, const osk_draw_style_t *st, osk_hidden_fn hid) {
    return osk_draw(buf, W, H, STRIDE, k, st, hid, NULL);
}

int main(void) {
    font_set_path("/tmp/redux-osk-draw-test-font-missing");   /* no saved font: VGA */
    font_load();
    osk_draw_style_t st;
    osk_draw_style_default(&st);
    osk_t k;
    osk_init(&k);
    int pw, ph, x, y, w, h, bx, by, row, col;

    /* Geometry (mock-up A): LCD and HDMI panels. */
    osk_draw_panel_size(720, &pw, &ph);
    expect(pw == 712 && ph == 240, "720 wide: panel 712 x 240");
    osk_draw_panel_size(1280, &pw, &ph);
    expect(pw == 1064 && ph == 304, "1280 wide: panel 1064 x 304");
    expect(osk_draw_box(700, 480, NULL, NULL, NULL, NULL) < 0, "a buffer narrower than the panel is refused");

    /* LCD: full screen buffer, keyboard at the bottom, the rest transparent, padding untouched. */
    setup(720, 480);
    expect(draw(&k, &st, NULL) == 0, "draws at 720 x 480");
    osk_draw_box(W, H, &bx, &by, &pw, &ph);
    expect(bx == 4 && by == 240, "panel at the bottom, centred");
    expect(count(0, 0, W, by, 0) == W * by, "everything above the panel is transparent");
    {
        int pad_ok = 1;
        for (int j = 0; j < H; j++) for (int i = W; i < STRIDE; i++) if (buf[j * STRIDE + i] != PAD) pad_ok = 0;
        expect(pad_ok, "stride padding is left alone");
    }
    expect(count(bx, by + 20, pw, 2, st.panel_bg) > 0, "the panel is blue");

    /* Every cap inside the panel, none overlapping its neighbour, on every page. */
    {
        int ok = 1;
        for (int p = 0; p < OSK_PAGES; p++)
            for (int r = 0; r < OSK_ROWS; r++) {
                int prev_end = -1;
                for (int c = 0; c < osk_cols(p, r); c++) {
                    osk_draw_key_rect(W, H, p, r, c, &x, &y, &w, &h);
                    if (x < bx + 8 || x + w > bx + pw - 8 || y < by + 16 || y + h > by + ph - 16 || x <= prev_end) ok = 0;
                    prev_end = x + w;
                }
            }
        expect(ok, "caps fit in the panel and keep their gaps (all pages)");
    }

    /* A cap: outline, face, highlight, shadow, label. The focus key (q) is yellow with a ring. */
    osk_draw_key_rect(W, H, 0, 0, 0, &x, &y, &w, &h);
    expect(w == 62 && h == 40, "a normal cap is 62 x 40");
    expect(count(x + 4, y + 4, w - 8, h - 8, st.focus_face) > 0 && count(x - 3, y - 3, w + 6, 1, st.focus_ring) == w + 6,
           "the focus key is yellow with a white ring");
    expect(count(x + 4, y + 4, w - 8, h - 8, st.cap_text) > 30, "its label is drawn (2x glyph)");
    find(0, "w", &row, &col);
    osk_draw_key_rect(W, H, 0, row, col, &x, &y, &w, &h);
    expect(count(x + 4, y + 4, w - 8, h - 8, st.cap_face) > 0, "other keys have the grey face");
    expect(buf[y * STRIDE + x] == st.cap_line && buf[(y + 1) * STRIDE + x + 1] == st.cap_hi &&
           buf[(y + h - 2) * STRIDE + x + w - 2] == st.cap_shadow, "outline, top-left highlight, bottom-right shadow");

    /* Shift latched: green with the triangle; letters drawn upper-case. */
    k.shift = 1;
    setup(720, 480); draw(&k, &st, NULL);
    find(0, "Shift", &row, &col);
    osk_draw_key_rect(W, H, 0, row, col, &x, &y, &w, &h);
    expect(count(x + 4, y + 4, w - 8, h - 8, st.latch_face) > 0, "latched Shift is green");
    k.shift = 0;

    /* Page tabs: the active page's tab is cyan. */
    k.page = 2;
    setup(720, 480); draw(&k, &st, NULL);
    expect(count(bx, by, pw, 16, st.tab_bg) > 0, "the active tab is drawn");
    find(2, "F10", &row, &col);
    osk_draw_key_rect(W, H, 2, row, col, &x, &y, &w, &h);
    expect(count(x + 4, y + 4, w - 8, h - 8, st.cap_text) > 30, "Fn page: F10 is drawn");

    /* The hidden-key callback leaves a gap: panel blue where Esc would be. */
    find(2, "Esc", &row, &col);
    setup(720, 480); draw(&k, &st, hide_esc);
    osk_draw_key_rect(W, H, 2, row, col, &x, &y, &w, &h);
    expect(count(x, y, w, h, st.panel_bg) == w * h, "a hidden key is not drawn");

    /* HDMI 1280 x 720: bigger caps. */
    k.page = 0;
    setup(1280, 720);
    expect(draw(&k, &st, NULL) == 0, "draws at 1280 x 720");
    osk_draw_key_rect(W, H, 0, 0, 0, &x, &y, &w, &h);
    expect(w == 96 && h == 56, "an HDMI cap is 96 x 56");

    /* A buffer only as tall as the panel works too (what redux-osk hands its layer). */
    setup(720, 240);
    expect(draw(&k, &st, NULL) == 0, "a panel-sized buffer works");
    expect(osk_draw(NULL, W, H, STRIDE, &k, &st, NULL, NULL) < 0 && osk_draw(buf, W, H, W - 1, &k, &st, NULL, NULL) < 0,
           "bad arguments are refused");

    printf("%s\n", fails ? "test_osk_draw: FAILED" : "test_osk_draw: all passed");
    return fails ? 1 : 0;
}
