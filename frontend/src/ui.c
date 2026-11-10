#include "ui.h"
#include "grid.h"
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

void ui_init(void) { selected = 0; grid_init(NULL); }
void ui_shutdown(void) {}

void ui_handle(logical_button_t btn) {
    if (btn == BTN_UP)   selected = (selected - 1 + MENU_COUNT) % MENU_COUNT;
    if (btn == BTN_DOWN) selected = (selected + 1) % MENU_COUNT;
}

void ui_render(SDL_Renderer *ren) {
    grid_clear();

    char status[64];
    char clock[8];
    platform_clock(clock, sizeof clock);
    snprintf(status, sizeof status, "BAT %d%% %s CPU %dC %s",
             platform_battery_percent(),
             platform_battery_charging() ? "CHG" : "",
             platform_cpu_temp_c(), clock);

    grid_puts(0, 0, "RG34XXSP Redux", 0);
    grid_puts(90 - (int)strlen(status), 0, status, 0);

    /* Box top */
    grid_puts(0, 1, "+--------------------------------------------------------------------------------------+", 0);

    for (int i = 0; i < MENU_COUNT; i++) {
        char line[90];
        snprintf(line, sizeof line, "|%s%-86s|",
                 (i == selected) ? ">>" : "  ", menu_items[i]);
        grid_puts(0, 2 + i, line, i == selected);
    }

    /* Box bottom */
    grid_puts(0, 28, "+--------------------------------------------------------------------------------------+", 0);
    grid_puts(0, 29, "A:Select  B:Back  M:Menu  Start:Search", 0);

    grid_present(ren);
}
