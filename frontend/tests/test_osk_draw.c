/* Global OSK drawing (frontend/osk/osk_draw.c) into plain memory: no SDL, no device. Exit 0 = pass. */
#include "osk_draw.h"
#include "font.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fails;
static void expect(int ok, const char *what) { printf("%s %s\n", ok ? "ok  " : "FAIL", what); if (!ok) fails++; }

#define W 720
#define H 200
#define STRIDE 736   /* wider than W: the padding must stay untouched */
#define PAD 0xDEADBEEFu

static uint32_t buf[H * STRIDE];

static void reset(void) { for (int i = 0; i < H * STRIDE; i++) buf[i] = PAD; }

/* Count pixels of colour c in a rectangle. */
static int count(int x, int y, int w, int h, uint32_t c) {
    int n = 0;
    for (int j = 0; j < h; j++) for (int i = 0; i < w; i++) n += buf[(y + j) * STRIDE + x + i] == c;
    return n;
}

static int find(const char *label, int *row, int *col) {
    for (int r = 0; r < OSK_ROWS; r++)
        for (int c = 0; c < osk_cols(r); c++)
            if (!strcmp(osk_key(r, c)->label, label)) { *row = r; *col = c; return 1; }
    return 0;
}

/* The glyph cell for character i of the key's label (i = 0 is the first label char). */
static int glyph_matches(int row, int col, int i, unsigned char ch, uint32_t fg, uint32_t bg) {
    int x, y;
    osk_draw_key_rect(W, H, row, col, &x, &y, NULL, NULL);
    x += (i + 1) * OSK_CELL_W;
    const uint8_t *g = font_glyph(ch);
    for (int j = 0; j < OSK_CELL_H; j++)
        for (int b = 0; b < OSK_CELL_W; b++)
            if (buf[(y + j) * STRIDE + x + b] != ((g[j] & (0x80u >> b)) ? fg : bg)) return 0;
    return 1;
}

int main(void) {
    font_set_path("/tmp/redux-osk-draw-test-font-missing");   /* no saved font: VGA */
    font_load();
    osk_draw_style_t st;
    osk_draw_style_default(&st);
    osk_t k;
    osk_init(&k);
    int ra = 0, x, y, w, h, row, col;

    /* Geometry. */
    expect(osk_draw_box(W, H, &x, &y, &w, &h) == 0 && x == 0 && y == H - 128 && w == 720 && h == 128,
           "720x200 band: box 720x128 at the bottom");
    expect(osk_draw_box(653, H, &x, NULL, &w, NULL) == 0 && w == 648 && x == 2, "odd width: whole cells, centred");
    expect(osk_draw_box(OSK_MIN_COLS * 8 - 1, H, NULL, NULL, NULL, NULL) == -1, "narrower than the widest row: refused");
    expect(osk_draw_box(W, 127, NULL, NULL, NULL, NULL) == -1, "shorter than 128 px: refused");
    int fits = 1;
    for (int r = 0; r < OSK_ROWS; r++)
        for (int c = 0; c < osk_cols(r); c++)
            if (osk_draw_key_rect(OSK_MIN_COLS * 8, 128, r, c, &x, NULL, &w, NULL) || x + w > (OSK_MIN_COLS - 1) * 8) fits = 0;
    expect(fits, "every key fits inside the frame at the minimum width");
    expect(osk_draw_key_rect(W, H, OSK_ROWS, 0, &x, &y, &w, &h) == -1 && osk_draw_key_rect(W, H, 0, 99, &x, &y, &w, &h) == -1,
           "no such key: -1");

    /* Bad arguments write nothing. */
    reset();
    expect(osk_draw(buf, 600, H, STRIDE, &k, &st, NULL, NULL) == -1 && buf[0] == PAD, "too small: -1, untouched");
    expect(osk_draw(buf, W, H, W - 1, &k, &st, NULL, NULL) == -1 && buf[0] == PAD, "stride < w: -1, untouched");
    expect(osk_draw(NULL, W, H, STRIDE, &k, &st, NULL, NULL) == -1, "NULL buffer: -1");

    /* Default draw. */
    reset();
    expect(osk_draw(buf, W, H, STRIDE, &k, &st, NULL, NULL) == 0, "draws into a 720x200 band");
    expect(count(0, 0, W, H - 128, 0) == W * (H - 128), "above the box: fully transparent");
    int pad_ok = 1;
    for (int j = 0; j < H; j++) for (int i = W; i < STRIDE; i++) if (buf[j * STRIDE + i] != PAD) pad_ok = 0;
    expect(pad_ok, "stride padding untouched");
    expect(count(0, H - 128, W, 128, 0) == 0, "box fully opaque");
    {   /* frame corners */
        const uint8_t *g = font_glyph(0xDA);
        int ok = 1;
        for (int j = 0; j < 16; j++) for (int b = 0; b < 8; b++)
            if (buf[(H - 128 + j) * STRIDE + b] != ((g[j] & (0x80u >> b)) ? st.box_fg : st.box_bg)) ok = 0;
        expect(ok, "top-left corner is CP437 0xDA in the line colour");
    }
    find("q", &row, &col);
    expect(k.row == row && k.col == col, "osk_init highlights q");
    osk_draw_key_rect(W, H, row, col, &x, &y, &w, &h);
    expect(count(x, y, w, h, st.sel_bg) + count(x, y, w, h, st.sel_fg) == w * h && count(x, y, w, h, st.sel_fg) > 0,
           "highlighted key: only the highlight colours, label drawn");
    expect(glyph_matches(row, col, 0, 'q', st.sel_fg, st.sel_bg), "highlighted key shows q from the current font");
    find("w", &row, &col);
    expect(glyph_matches(row, col, 0, 'w', st.key_fg, st.box_bg), "other keys: label colour on the box");
    find("Backspace", &row, &col);
    int okb = 1;
    for (int i = 0; i < 9; i++) okb &= glyph_matches(row, col, i, (unsigned char)"Backspace"[i], st.key_fg, st.box_bg);
    expect(okb, "long labels drawn in full");
    {   /* title on the top line */
        int okt = 1;
        for (int i = 0; i < 9; i++) {
            const uint8_t *g = font_glyph((unsigned char)st.title[i]);
            for (int j = 0; j < 16; j++) for (int b = 0; b < 8; b++)
                if (buf[(H - 128 + j) * STRIDE + (i + 2) * 8 + b] != ((g[j] & (0x80u >> b)) ? st.box_fg : st.box_bg)) okt = 0;
        }
        expect(okt, "title on the top line");
    }

    /* Shift latched: letters upper-case, shifted symbols, Shift key in the mode colours. */
    char lab[16];
    find("q", &row, &col);
    expect(osk_draw_label(osk_key(row, col), 1, lab, sizeof lab) == 1 && !strcmp(lab, "Q"), "label: Shift q -> Q");
    find("1", &row, &col);
    expect(osk_draw_label(osk_key(row, col), 1, lab, sizeof lab) == 1 && !strcmp(lab, "!"), "label: Shift 1 -> !");
    find("Tab", &row, &col);
    expect(osk_draw_label(osk_key(row, col), 1, lab, sizeof lab) == 3 && !strcmp(lab, "Tab"), "label: Shift Tab unchanged");
    find("Shift", &row, &col);
    expect(osk_draw_label(osk_key(row, col), 1, lab, sizeof lab) == 5 && !strcmp(lab, "Shift"), "label: mode key unchanged");
    expect(osk_draw_label(osk_key(row, col), 0, lab, 3) == 2 && !strcmp(lab, "Sh"), "label: cut to cap");
    k.shift = 1;
    reset();
    osk_draw(buf, W, H, STRIDE, &k, &st, NULL, NULL);
    find("w", &row, &col);
    expect(glyph_matches(row, col, 0, 'W', st.key_fg, st.box_bg), "Shift on: w drawn as W");
    find("=", &row, &col);
    expect(glyph_matches(row, col, 0, '+', st.key_fg, st.box_bg), "Shift on: = drawn as +");
    find("Shift", &row, &col);
    expect(glyph_matches(row, col, 0, 'S', st.mode_fg, st.mode_bg), "Shift on: Shift key in the mode colours");
    find("Ctrl", &row, &col);
    expect(glyph_matches(row, col, 0, 'C', st.key_fg, st.box_bg), "Ctrl off: plain");
    k.shift = 0; k.ctrl = 1;
    reset();
    osk_draw(buf, W, H, STRIDE, &k, &st, NULL, NULL);
    expect(glyph_matches(row, col, 0, 'C', st.mode_fg, st.mode_bg), "Ctrl latched: mode colours");
    k.ctrl = 0;

    /* D4 B: Esc and F1 left off while RetroArch runs. */
    int er, ec, fr, fc;
    find("Esc", &er, &ec); find("F1", &fr, &fc);
    reset();
    osk_draw(buf, W, H, STRIDE, &k, &st, osk_draw_hide_ra, &ra);
    osk_draw_key_rect(W, H, er, ec, &x, &y, &w, &h);
    expect(count(x, y, w, h, st.key_fg) > 0, "RetroArch not running: Esc shown");
    ra = 1;
    reset();
    osk_draw(buf, W, H, STRIDE, &k, &st, osk_draw_hide_ra, &ra);
    osk_draw_key_rect(W, H, er, ec, &x, &y, &w, &h);
    int esc_gone = count(x, y, w, h, st.box_bg) == w * h;
    osk_draw_key_rect(W, H, fr, fc, &x, &y, &w, &h);
    expect(esc_gone && count(x, y, w, h, st.box_bg) == w * h, "RetroArch running: Esc and F1 blank");
    find("F2", &row, &col);
    expect(glyph_matches(row, col, 0, 'F', st.key_fg, st.box_bg), "RetroArch running: F2 still shown");
    expect(!osk_draw_hide_ra(osk_key(fr, fc), NULL), "hide_ra with no flag: nothing hidden");

    /* Settings > UI colours. */
    osk_draw_style_t ui;
    osk_draw_style_ui(&ui, 0xFFFFFF, 0xFFFF55, 0x0000AA);
    expect(ui.box_bg == 0xFF0000AAu && ui.key_fg == 0xFFFFFFFFu && ui.box_fg == 0xFFFFFF55u, "UI style: window, text, header colours");
    expect(ui.sel_bg == 0xFFFFFFFFu && ui.sel_fg == 0xFF0000AAu, "UI style: highlight swaps text and window");
    ra = 0;
    reset();
    osk_draw(buf, W, H, STRIDE, &k, &ui, NULL, NULL);
    find("w", &row, &col);
    expect(glyph_matches(row, col, 0, 'w', ui.key_fg, ui.box_bg), "UI style applied to keys");

    /* Font follows Settings > Display. */
    font_set_path("/tmp/redux-osk-draw-test-font");
    FILE *f = fopen("/tmp/redux-osk-draw-test-font", "w");
    if (f) { fputs("spleen\n", f); fclose(f); }
    font_load();
    reset();
    osk_draw(buf, W, H, STRIDE, &k, &st, NULL, NULL);
    expect(font_current() == FONT_SPLEEN && glyph_matches(row, col, 0, 'w', st.key_fg, st.box_bg)
           && memcmp(font_glyph('w'), font_spleen_8x16['w'], 16) == 0, "drawn with the chosen UI font (Spleen)");
    remove("/tmp/redux-osk-draw-test-font");

    printf("%s\n", fails ? "FAILED" : "all passed");
    return fails ? 1 : 0;
}
