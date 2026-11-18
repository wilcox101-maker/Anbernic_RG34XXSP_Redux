/* Global on-screen keyboard: software drawing. See osk_draw.h. */
#include "osk_draw.h"
#include "font.h"
#include <string.h>

#define OPAQUE(rgb) (0xFF000000u | ((rgb) & 0xFFFFFFu))

void osk_draw_style_default(osk_draw_style_t *s) {
    s->box_bg = OPAQUE(0xE5E5E5); s->box_fg = OPAQUE(0x000000); s->key_fg = OPAQUE(0x000000);
    s->sel_bg = OPAQUE(0xFFFF00); s->sel_fg = OPAQUE(0x000000);
    s->mode_bg = OPAQUE(0x00FFFF); s->mode_fg = OPAQUE(0x000000);
    s->title = " Keyboard: D-pad moves, A types, B or Menu+Select closes ";
}

void osk_draw_style_ui(osk_draw_style_t *s, uint32_t text_rgb, uint32_t hftext_rgb, uint32_t win_rgb) {
    osk_draw_style_default(s);
    s->box_bg = OPAQUE(win_rgb); s->box_fg = OPAQUE(hftext_rgb); s->key_fg = OPAQUE(text_rgb);
    s->sel_bg = OPAQUE(text_rgb); s->sel_fg = OPAQUE(win_rgb);
    s->mode_bg = OPAQUE(0x55FFFF); s->mode_fg = OPAQUE(0x000000);   /* uis_colors Light Cyan */
}

int osk_draw_label(const osk_key_t *key, int shift, char *out, int cap) {
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

int osk_draw_box(int w, int h, int *x, int *y, int *bw, int *bh) {
    int cols = w / OSK_CELL_W;
    if (w <= 0 || h < OSK_BOX_H || cols < OSK_MIN_COLS) return -1;
    if (x) *x = (w - cols * OSK_CELL_W) / 2;
    if (y) *y = h - OSK_BOX_H;
    if (bw) *bw = cols * OSK_CELL_W;
    if (bh) *bh = OSK_BOX_H;
    return 0;
}

int osk_draw_key_rect(int w, int h, int row, int col, int *x, int *y, int *kw, int *kh) {
    int bx, by;
    const osk_key_t *key = osk_key(row, col);
    if (!key || osk_draw_box(w, h, &bx, &by, NULL, NULL) != 0) return -1;
    if (x) *x = bx + osk_key_x(row, col) * OSK_CELL_W;
    if (y) *y = by + (row + 1) * OSK_CELL_H;
    if (kw) *kw = (key->width + 2) * OSK_CELL_W;
    if (kh) *kh = OSK_CELL_H;
    return 0;
}

static void fill(uint32_t *px, int stride, int x, int y, int w, int h, uint32_t c) {
    for (int j = 0; j < h; j++) {
        uint32_t *p = px + (long)(y + j) * stride + x;
        for (int i = 0; i < w; i++) p[i] = c;
    }
}

/* One 8x16 glyph at (x, y): set bits fg, clear bits bg. */
static void glyph(uint32_t *px, int stride, int x, int y, unsigned char ch, uint32_t fg, uint32_t bg) {
    const uint8_t *g = font_glyph(ch);
    for (int j = 0; j < OSK_CELL_H; j++) {
        uint32_t *p = px + (long)(y + j) * stride + x;
        for (int i = 0; i < OSK_CELL_W; i++) p[i] = (g[j] & (0x80u >> i)) ? fg : bg;
    }
}

int osk_draw(uint32_t *px, int w, int h, int stride, const osk_t *k, const osk_draw_style_t *s,
             osk_hidden_fn hidden, void *user) {
    int bx, by, bw;
    if (!px || !k || !s || stride < w || osk_draw_box(w, h, &bx, &by, &bw, NULL) != 0) return -1;
    int cols = bw / OSK_CELL_W, last = cols - 1;

    for (int j = 0; j < h; j++) memset(px + (long)j * stride, 0, (size_t)w * sizeof *px);
    fill(px, stride, bx, by, bw, OSK_BOX_H, s->box_bg);

    /* Frame with CP437 single lines, as the terminal draws it. */
    for (int c = 0; c < cols; c++) {
        unsigned char top = c == 0 ? 0xDA : c == last ? 0xBF : 0xC4;
        unsigned char bot = c == 0 ? 0xC0 : c == last ? 0xD9 : 0xC4;
        glyph(px, stride, bx + c * OSK_CELL_W, by, top, s->box_fg, s->box_bg);
        glyph(px, stride, bx + c * OSK_CELL_W, by + (OSK_BOX_ROWS - 1) * OSK_CELL_H, bot, s->box_fg, s->box_bg);
    }
    for (int r = 1; r < OSK_BOX_ROWS - 1; r++) {
        glyph(px, stride, bx, by + r * OSK_CELL_H, 0xB3, s->box_fg, s->box_bg);
        glyph(px, stride, bx + last * OSK_CELL_W, by + r * OSK_CELL_H, 0xB3, s->box_fg, s->box_bg);
    }
    if (s->title)
        for (int i = 0; s->title[i] && i + 2 < last; i++)
            glyph(px, stride, bx + (i + 2) * OSK_CELL_W, by, (unsigned char)s->title[i], s->box_fg, s->box_bg);

    for (int r = 0; r < OSK_ROWS; r++) {
        for (int c = 0; c < osk_cols(r); c++) {
            const osk_key_t *key = osk_key(r, c);
            if (hidden && hidden(key, user)) continue;   /* gap: box background stays */
            int chosen = r == k->row && c == k->col;
            int latched = (key->mode == 1 && k->shift) || (key->mode == 2 && k->ctrl);
            uint32_t fg = chosen ? s->sel_fg : latched ? s->mode_fg : s->key_fg;
            uint32_t bg = chosen ? s->sel_bg : latched ? s->mode_bg : s->box_bg;
            char label[16];
            int n = osk_draw_label(key, k->shift, label, (int)sizeof label);
            int x0 = osk_key_x(r, c), y = by + (r + 1) * OSK_CELL_H;
            for (int i = 0; i < key->width + 2 && x0 + i < last; i++) {
                int li = i - 1;
                unsigned char ch = (li >= 0 && li < n) ? (unsigned char)label[li] : ' ';
                glyph(px, stride, bx + (x0 + i) * OSK_CELL_W, y, ch, fg, bg);
            }
        }
    }
    return 0;
}
