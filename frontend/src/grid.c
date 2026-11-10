#include "grid.h"
#include <string.h>

extern const uint8_t *font_glyph(int c);

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
    /* BIOS cyan accent, white text, black background */
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);

    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            int inv = invert[r][c];
            const uint8_t *glyph = font_glyph(cells[r][c]);
            int x0 = c * FONT_W;
            int y0 = r * FONT_H;

            if (inv) {
                SDL_SetRenderDrawColor(ren, 0, 255, 255, 255); /* cyan bar */
                SDL_Rect bar = { x0, y0, FONT_W, FONT_H };
                SDL_RenderFillRect(ren, &bar);
                SDL_SetRenderDrawColor(ren, 0, 0, 0, 255); /* black text */
            } else {
                SDL_SetRenderDrawColor(ren, 255, 255, 255, 255); /* white text */
            }

            for (int gy = 0; gy < FONT_H; gy++) {
                uint8_t bits = glyph[gy];
                for (int gx = 0; gx < FONT_W; gx++) {
                    if (bits & (0x80 >> gx)) {
                        SDL_RenderDrawPoint(ren, x0 + gx, y0 + gy);
                    }
                }
            }
        }
    }
}
