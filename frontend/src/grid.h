#ifndef REDUX_GRID_H
#define REDUX_GRID_H

#include <SDL.h>
#include <stddef.h>
#include <stdint.h>

/* 90 x 30 character cells, 8 x 16 pixels each = 720 x 480. */
#define GRID_COLS 90
#define GRID_ROWS 30
#define FONT_W 8
#define FONT_H 16

/* Box layout used by every screen (docs/ui-spec.md section 2). */
#define ROW_TITLE   0
#define ROW_BOX_TOP 1
#define ROW_BODY0   2   /* first of 26 body rows */
#define BODY_ROWS   26
#define ROW_BOX_BOT 28
#define ROW_HELP    29

void grid_clear(void);

/* Write a UTF-8 string. ASCII and the box/shade characters the spec uses
 * (┌ ┐ └ ┘ ─ │ ▓) are mapped to CP437; anything else becomes '?'. Clipped at the edge. */
void grid_puts(int col, int row, const char *utf8, int inverted);

/* Same, but right-aligned so the last character lands in column GRID_COLS-1. */
void grid_puts_right(int row, const char *utf8);

/* Mark cells [c0, c1] of a row as inverted (accent bar, black text). */
void grid_invert(int row, int c0, int c1);

/* Draw the standard screen box on rows ROW_BOX_TOP..ROW_BOX_BOT. */
void grid_box(void);

/* Accent colour for the selection bar. Default BIOS cyan (0, 255, 255). */
void grid_set_accent(uint8_t r, uint8_t g, uint8_t b);

/* Render all cells. Glyphs are cached in one texture per renderer. */
void grid_present(SDL_Renderer *ren);

/* Text dump of the grid for tests: GRID_ROWS lines of UTF-8, each ending in '\n'.
 * A selected row shows its first two inverted cells as "▓▓", the spec's mock-up
 * convention. Returns bytes written (excluding the terminating NUL). */
size_t grid_dump(char *buf, size_t len);

#endif
