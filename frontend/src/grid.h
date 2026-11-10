#ifndef GRID_H
#define GRID_H

#include <SDL.h>

#define GRID_COLS 90
#define GRID_ROWS 30
#define FONT_W 8
#define FONT_H 16

void grid_init(SDL_Renderer *ren);
void grid_clear(void);
void grid_puts(int col, int row, const char *s, int inverted);
void grid_present(SDL_Renderer *ren);

#endif
