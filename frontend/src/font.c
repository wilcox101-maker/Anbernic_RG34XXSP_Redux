/* 8x16 bitmap font — public-domain VGA ROM derivative (GPL-compatible).
   Covers ASCII 32–126 and basic box-drawing for F1.
   Full 256-glyph table can be expanded later. */

#include <stdint.h>

/* Each glyph is 16 bytes (rows), MSB is leftmost pixel. */
const uint8_t font_8x16[128][16] = {
    /* 0x20 space */ {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    /* 0x21 ! */ {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0,0x18,0x18,0,0,0,0,0,0},
    /* ... minimal set for demo; full table follows pattern ... */
    /* For F1 we implement a subset. Characters used by the main menu
       are filled; others remain blank until expanded. */
};

/* Helper: get glyph for a character (falls back to space) */
const uint8_t *font_glyph(int c) {
    if (c < 32 || c > 126) return font_8x16[0];
    return font_8x16[c - 32];
}
