#ifndef REDUX_FONT_H
#define REDUX_FONT_H
#include <stdint.h>

/* UI fonts (docs/ui-spec.md section 8a, setting `[display] font`). Every font is an 8x16 cell, so
   the 90 x 30 grid and every screen stay the same whichever is chosen.
   16 bytes per glyph, one byte per row, MSB = leftmost pixel. Index is a CP437 code.
   A glyph the chosen font lacks is drawn from VGA. */

typedef enum { FONT_VGA, FONT_SPLEEN, FONT_UNIFONT, FONT_THIN, FONT_CHUNKY, FONT_COUNT } font_id_t;
#define FONT_DEFAULT FONT_VGA

/* Glyph of the current font. */
const uint8_t *font_glyph(unsigned char cp437);

font_id_t font_current(void);
const char *font_name(font_id_t id);   /* shown in Settings > Display, e.g. "Spleen" */
const char *font_key(font_id_t id);    /* saved value, e.g. "spleen" */

/* Next or previous font (dir +1 / -1), wrapping, and save. Returns the new font. */
font_id_t font_step(int dir);

/* Load the saved font; a missing or unknown value keeps VGA. */
void font_load(void);

/* Reset All: forget the saved font and go back to VGA. */
void font_reset(void);

/* Tests point at a temp file. NULL restores the build default
   (/mnt/data/redux/font on the device, /tmp/redux.font on desktop). */
void font_set_path(const char *path);

/* Generated tables (font_extra.c, tools/fontconv.py). */
extern const uint8_t font_spleen_8x16[256][16];
extern const uint8_t font_unifont_8x16[256][16];
extern const uint8_t font_thin_8x16[256][16];
extern const uint8_t font_chunky_8x16[256][16];

#endif
