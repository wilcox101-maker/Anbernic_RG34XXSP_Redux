/* 8x16 bitmap font — public-domain VGA ROM derivative (GPL-compatible).
   Minimal set for F1 main menu. Full table can be expanded. */

#include <stdint.h>

/* Space and a few glyphs. Each is 16 bytes, MSB leftmost. */
static const uint8_t glyph_space[16] = {0};
static const uint8_t glyph_A[16] = {
    0x18,0x18,0x24,0x24,0x42,0x42,0x7E,0x42,0x42,0x42,0x42,0,0,0,0,0
};
/* Additional glyphs would follow the same pattern. */

const uint8_t *font_glyph(int c) {
    if (c == 'A') return glyph_A;
    return glyph_space;
}
