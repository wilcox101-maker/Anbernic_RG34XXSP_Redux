#include "ui.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>

static const char *menu_items[] = {
    "Game Room",
    "Music",
    "Video",
    "Documents",
    "File Manager",
    "YouTube (if feasible)",
    "Network",
    "Settings",
    "Engineering Mode"
};
#define MENU_COUNT 9
static int selected = 0;

void ui_init(void) { selected = 0; }
void ui_shutdown(void) {}

void ui_handle(logical_button_t btn) {
    if (btn == BTN_UP)   selected = (selected - 1 + MENU_COUNT) % MENU_COUNT;
    if (btn == BTN_DOWN) selected = (selected + 1) % MENU_COUNT;
    /* A / B navigation stubs for later screens */
}

void ui_render(SDL_Renderer *ren) {
    (void)ren;
    /* Placeholder: real 8x16 grid renderer lands in grid.c + font.c.
       For now the window is black; full cell-perfect rendering is next. */
    char status[64];
    char clock[8];
    platform_clock(clock, sizeof clock);
    snprintf(status, sizeof status, "BAT %d%% %s CPU %dC %s",
             platform_battery_percent(),
             platform_battery_charging() ? "CHG" : "",
             platform_cpu_temp_c(), clock);
    (void)status;
}
