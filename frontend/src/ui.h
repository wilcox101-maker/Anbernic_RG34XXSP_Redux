#ifndef REDUX_UI_H
#define REDUX_UI_H

#include <stdint.h>
#include "remap.h"

typedef enum {
    SCREEN_MAIN, SCREEN_ROMS, SCREEN_SETTINGS, SCREEN_ENG_CONFIRM,
    SCREEN_ENG, SCREEN_INTERNET, SCREEN_SYSTEMS, SCREEN_WIFI, SCREEN_WIFI_KEY,
    SCREEN_SYSTEM_DEFAULTS, SCREEN_STOCK_CONFIRM
} screen_t;

/* Engineering tabs. Only CPU and Battery have content now. */
typedef enum {
    ENG_CPU = 0, ENG_BATTERY, ENG_THERMAL, ENG_DISPLAY, ENG_SYSTEM, ENG_TAB_COUNT
} eng_tab_t;

void ui_init(void);

/* For tests: choose the Engineering tab; read the current screen and tab. */
void ui_set_eng_tab(eng_tab_t t);
screen_t ui_screen(void);
eng_tab_t ui_eng_tab(void);

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

/* 1 if a launch was requested and main should exit so loadapp.sh runs /tmp/.next. */
int ui_should_exit(void);

/* For tests: the highlighted row, and how far a long list has scrolled. */
int ui_list_selected(void);
int ui_list_scroll(void);

#endif
