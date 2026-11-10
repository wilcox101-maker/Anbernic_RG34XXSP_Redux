#ifndef REDUX_UI_H
#define REDUX_UI_H

#include <stdint.h>
#include "remap.h"

typedef enum {
    SCREEN_MAIN, SCREEN_ROMS, SCREEN_SETTINGS, SCREEN_ENG_CONFIRM, SCREEN_CPU, SCREEN_BATTERY
} screen_t;

void ui_init(void);

/* A logical button went down (pressed = 1) or up (pressed = 0) at time now_ms. */
void ui_button(logical_button_t btn, int pressed, uint32_t now_ms);

/* Advance timers (the hold-A confirm). Call when a wait times out. */
void ui_tick(uint32_t now_ms);

/* 1 if something changed since the last ui_draw(). */
int ui_dirty(void);

/* How long main may sleep before the next ui_tick() is needed (ms). */
uint32_t ui_wait_ms(void);

/* Fill the grid for the current screen. grid_present() then puts it on screen. */
void ui_draw(void);

/* For tests: jump straight to a screen. */
void ui_show(screen_t s);

#endif
