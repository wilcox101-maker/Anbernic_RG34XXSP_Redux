/* On-screen keyboard drawing (mock-up A). See osk_draw.h. */
#include "osk_draw.h"
#include "font.h"
#include <string.h>

#define RGB(v) (0xFF000000u | (uint32_t)(v))
#define CELL_W 8
#define CELL_H 16

void osk_draw_style_default(osk_draw_style_t *s) {
    s->panel_bg = RGB(0x0000AA); s->frame = RGB(0xFFFFFF); s->title = RGB(0xFFFF55);
    s->tab_bg = RGB(0x00AAAA); s->tab_fg = RGB(0x000000); s->tab_off = RGB(0x55FFFF); s->hint = RGB(0x55FFFF);
    s->cap_line = RGB(0x000000); s->cap_face = RGB(0xAAAAAA); s->cap_hi = RGB(0xFFFFFF);
    s->cap_shadow = RGB(0x555555); s->cap_text = RGB(0x000000);
    s->focus_face = RGB(0xFFFF55); s->focus_ring = RGB(0xFFFFFF); s->latch_face = RGB(0x55FF55);
    s->title_text = "Keyboard";
    s->hint_text = "D-pad move  A type  L1/R1 page  B close";
}

void osk_draw_metrics(int screen_w, osk_metrics_t *m) {
    if (screen_w >= 1200) { m->unit = 96; m->gap = 8; m->cap_h = 56; }
    else { m->unit = 62; m->gap = 6; m->cap_h = 40; }
}

static int kb_width(const osk_metrics_t *m) { return 10 * m->unit + 9 * m->gap; }

void osk_draw_panel_size(int screen_w, int *pw, int *ph) {
    osk_metrics_t m;
    osk_draw_metrics(screen_w, &m);
    int w = (kb_width(&m) + 32 + 7) / 8 * 8;
    int h = CELL_H + 8 + OSK_ROWS * m.cap_h + (OSK_ROWS - 1) * m.gap + 8 + CELL_H;
    h = (h + CELL_H - 1) / CELL_H * CELL_H;
    if (pw) *pw = w;
    if (ph) *ph = h;
}

int osk_draw_box(int w, int h, int *x, int *y, int *bw, int *bh) {
    int pw, ph;
    osk_draw_panel_size(w, &pw, &ph);
    if (w < pw || h < ph) return -1;
    if (x) *x = (w - pw) / 2;
    if (y) *y = h - ph;
    if (bw) *bw = pw;
    if (bh) *bh = ph;
    return 0;
}

int osk_draw_key_rect(int w, int h, int page, int row, int col, int *x, int *y, int *kw, int *kh) {
    const osk_key_t *key = osk_key(page, row, col);
    int bx, by, bw;
    if (!key || osk_draw_box(w, h, &bx, &by, &bw, NULL) != 0) return -1;
    osk_metrics_t m;
    osk_draw_metrics(w, &m);
    int x0 = bx + (bw - kb_width(&m)) / 2, y0 = by + CELL_H + 8;
    int x2 = osk_key_x2(page, row, col);
    if (x) *x = x0 + x2 * (m.unit + m.gap) / 2;
    if (y) *y = y0 + row * (m.cap_h + m.gap);
    if (kw) *kw = key->units2 * m.unit / 2 + (key->units2 - 2) * m.gap / 2;
    if (kh) *kh = m.cap_h;
    return 0;
}

typedef struct { uint32_t *px; int w, h, stride; } canvas_t;

static void fill(const canvas_t *c, int x, int y, int w, int h, uint32_t col) {
    for (int j = y < 0 ? 0 : y; j < y + h && j < c->h; j++) {
        uint32_t *p = c->px + (long)j * c->stride;
        for (int i = x < 0 ? 0 : x; i < x + w && i < c->w; i++) p[i] = col;
    }
}

/* One glyph, set bits only (transparent background), at scale 1 or 2. */
static void glyph(const canvas_t *c, int x, int y, unsigned char ch, uint32_t fg, int scale) {
    const uint8_t *g = font_glyph(ch);
    for (int j = 0; j < CELL_H; j++)
        for (int i = 0; i < CELL_W; i++)
            if (g[j] & (0x80u >> i)) fill(c, x + i * scale, y + j * scale, scale, scale, fg);
}

static void text(const canvas_t *c, int x, int y, const char *s, uint32_t fg, int scale) {
    for (int i = 0; s[i]; i++) glyph(c, x + i * CELL_W * scale, y, (unsigned char)s[i], fg, scale);
}

/* A label on its own background strip (title, tabs, hint sit on the frame line). */
static void label_on(const canvas_t *c, int x, int y, const char *s, uint32_t fg, uint32_t bg) {
    fill(c, x, y, (int)strlen(s) * CELL_W, CELL_H, bg);
    text(c, x, y, s, fg, 1);
}

static void frame_box(const canvas_t *c, int x, int y, int w, int h, uint32_t col) {
    int cols = w / CELL_W, rows = h / CELL_H;
    for (int i = 0; i < cols; i++) {
        glyph(c, x + i * CELL_W, y, i == 0 ? 0xC9 : i == cols - 1 ? 0xBB : 0xCD, col, 1);
        glyph(c, x + i * CELL_W, y + (rows - 1) * CELL_H, i == 0 ? 0xC8 : i == cols - 1 ? 0xBC : 0xCD, col, 1);
    }
    for (int r = 1; r < rows - 1; r++) {
        glyph(c, x, y + r * CELL_H, 0xBA, col, 1);
        glyph(c, x + (cols - 1) * CELL_W, y + r * CELL_H, 0xBA, col, 1);
    }
}

static void cap(const canvas_t *c, int x, int y, int w, int h, const char *label, int focus, int latched,
                const osk_draw_style_t *s) {
    uint32_t face = focus ? s->focus_face : latched ? s->latch_face : s->cap_face;
    fill(c, x, y, w, h, s->cap_line);
    fill(c, x + 1, y + 1, w - 2, h - 2, face);
    fill(c, x + 1, y + 1, w - 2, 2, s->cap_hi);
    fill(c, x + 1, y + 1, 2, h - 2, s->cap_hi);
    fill(c, x + 1, y + h - 4, w - 2, 3, s->cap_shadow);
    fill(c, x + w - 4, y + 1, 3, h - 2, s->cap_shadow);
    if (focus)
        for (int d = 2; d <= 3; d++) {
            fill(c, x - d, y - d, w + 2 * d, 1, s->focus_ring); fill(c, x - d, y + h + d - 1, w + 2 * d, 1, s->focus_ring);
            fill(c, x - d, y - d, 1, h + 2 * d, s->focus_ring); fill(c, x + w + d - 1, y - d, 1, h + 2 * d, s->focus_ring);
        }
    int n = (int)strlen(label);
    int scale = (n * CELL_W * 2 <= w - 12 && CELL_H * 2 <= h - 6) ? 2 : 1;
    int tw = n * CELL_W * scale, th = CELL_H * scale;
    text(c, x + (w - tw) / 2, y + (h - 3 - th) / 2, label, s->cap_text, scale);
    if (latched) glyph(c, x + w - 13, y + 3, 0x1E, s->cap_text, 1);   /* small triangle */
}

int osk_draw(uint32_t *px, int w, int h, int stride, const osk_t *k, const osk_draw_style_t *s,
             osk_hidden_fn hidden, void *user) {
    int bx, by, bw, bh;
    if (!px || !k || !s || stride < w || osk_draw_box(w, h, &bx, &by, &bw, &bh) != 0) return -1;
    const canvas_t c = { px, w, h, stride };
    for (int j = 0; j < h; j++) memset(px + (long)j * stride, 0, (size_t)w * sizeof *px);

    fill(&c, bx, by, bw, bh, s->panel_bg);
    frame_box(&c, bx, by, bw, bh, s->frame);
    if (s->title_text) {
        char t[64];
        int n = 0;
        t[n++] = ' ';
        for (int i = 0; s->title_text[i] && n < 60; i++) t[n++] = s->title_text[i];
        t[n++] = ' '; t[n] = 0;
        label_on(&c, bx + 2 * CELL_W, by, t, s->title, s->panel_bg);
    }
    /* Page tabs, right-aligned on the top line: " abc " " 123 " " Fn ", one cell apart. */
    int tabs_w = 0;
    for (int p = 0; p < OSK_PAGES; p++) tabs_w += ((int)strlen(osk_page_name(p)) + 3) * CELL_W;
    int tx = bx + bw - 2 * CELL_W - tabs_w;
    for (int p = 0; p < OSK_PAGES; p++) {
        char t[16];
        int n = 0;
        const char *name = osk_page_name(p);
        t[n++] = ' ';
        for (int i = 0; name[i] && n < 12; i++) t[n++] = name[i];
        t[n++] = ' '; t[n] = 0;
        if (p == k->page) label_on(&c, tx, by, t, s->tab_fg, s->tab_bg);
        else label_on(&c, tx, by, t, s->tab_off, s->panel_bg);
        tx += (n + 1) * CELL_W;
    }
    if (s->hint_text) {
        char t[96];
        int n = 0;
        t[n++] = ' ';
        for (int i = 0; s->hint_text[i] && n < 92; i++) t[n++] = s->hint_text[i];
        t[n++] = ' '; t[n] = 0;
        label_on(&c, bx + 2 * CELL_W, by + bh - CELL_H, t, s->hint, s->panel_bg);
    }

    for (int r = 0; r < OSK_ROWS; r++) {
        for (int col = 0; col < osk_cols(k->page, r); col++) {
            const osk_key_t *key = osk_key(k->page, r, col);
            if (hidden && hidden(key, user)) continue;
            int x = 0, y = 0, kw = 0, kh = 0;
            if (osk_draw_key_rect(w, h, k->page, r, col, &x, &y, &kw, &kh) != 0) continue;
            char label[16];
            osk_label(key, k->shift, label, (int)sizeof label);
            int latched = (key->mode == 1 && k->shift) || (key->mode == 2 && k->ctrl);
            cap(&c, x, y, kw, kh, label, r == k->row && col == k->col, latched, s);
        }
    }
    return 0;
}
