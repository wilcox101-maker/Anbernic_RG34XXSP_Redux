#include "grid.h"
#include <string.h>

static char cells[GRID_ROWS][GRID_COLS];
static int invert[GRID_ROWS][GRID_COLS];

void grid_init(SDL_Renderer *ren) {
    (void)ren;
    memset(cells, ' ', sizeof cells);
    memset(invert, 0, sizeof invert);
}

void grid_clear(void) {
    memset(cells, ' ', sizeof cells);
    memset(invert, 0, sizeof invert);
}

void grid_puts(int col, int row, const char *s, int inverted) {
    if (row < 0 || row >= GRID_ROWS) return;
    for (int i = 0; s[i] && col + i < GRID_COLS; i++) {
        cells[row][col + i] = s[i];
        invert[row][col + i] = inverted;
    }
}

void grid_present(SDL_Renderer *ren) {
    (void)ren;
    /* Full 8x16 bitmap blit lands with font.c. */
}
