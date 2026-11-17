#ifndef REDUX_CJK_H
#define REDUX_CJK_H
#include <stddef.h>
#include <stdint.h>

/* 16x16 glyphs for the wide (CJK) cells of Redux Terminal: GNU Unifont, converted by
   tools/cjkconv.py into frontend/term/cjk.bin and embedded in redux-term (CJK decision A,
   notes/2026-11-17-dale-decision-terminal-cjk.md). 32 bytes per glyph, two bytes per row, MSB
   left. NULL when the table has no glyph for the code point (the terminal shows "?"). */
const uint8_t *cjk_glyph(uint32_t cp);

/* Tests: use another blob (NULL = the embedded one). Returns the number of ranges, -1 if bad. */
int cjk_set_blob(const unsigned char *blob, size_t len);

extern const unsigned char redux_cjk_bin[];
extern const size_t redux_cjk_bin_len;

#endif
