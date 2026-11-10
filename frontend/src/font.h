#ifndef REDUX_FONT_H
#define REDUX_FONT_H
#include <stdint.h>

/* 16 bytes per glyph, one byte per row, MSB = leftmost pixel. Index is a CP437 code. */
const uint8_t *font_glyph(unsigned char cp437);

#endif
