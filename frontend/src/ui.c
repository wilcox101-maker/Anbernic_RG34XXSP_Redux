#include "ui.h"
#include "grid.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>

typedef enum { SCREEN_MAIN, SCREEN_ROMS, SCREEN_SETTINGS, SCREEN_CPU, SCREEN_BATTERY } screen_t;
static screen_t screen = SCREEN_MAIN;
static int selected = 0;

static const char *main_items[] = {
    "Game Room", "Music", "Video", "Documents", "File Manager",
    "YouTube (if feasible)", "Network", "Settings", "Engineering Mode"
};
#define MAIN_COUNT 9

static const char *rom_items[] = {
    "Advance Wars", "Fire Emblem", "Golden Sun", "Metroid Fusion", "Pokemon Emerald"
};
#define ROM_COUNT 5

static const char *settings_items[] = {
    "Display", "Audio", "Input", "Power & Lid", "Network", "System", "Emulation"
};
#define SETTINGS_COUNT 7

void ui_init(void) { screen = SCREEN_MAIN; selected = 0; grid_init(NULL); }
void ui_shutdown(void) {}

void ui_handle(logical_button_t btn) {
    int count = (screen == SCREEN_MAIN) ? MAIN_COUNT :
                (screen == SCREEN_ROMS) ? ROM_COUNT :
                (screen == SCREEN_SETTINGS) ? SETTINGS_COUNT : 1;
    if (btn == BTN_UP)   selected = (selected - 1 + count) % count;
    if (btn == BTN_DOWN) selected = (selected + 1) % count;
    if (btn == BTN_A) {
        if (screen == SCREEN_MAIN && selected == 0) { screen = SCREEN_ROMS; selected = 0; }
        else if (screen == SCREEN_MAIN && selected == 7) { screen = SCREEN_SETTINGS; selected = 0; }
        else if (screen == SCREEN_MAIN && selected == 8) { screen = SCREEN_CPU; selected = 0; }
    }
    if (btn == BTN_B && screen != SCREEN_MAIN) { screen = SCREEN_MAIN; selected = 0; }
    if (btn == BTN_MENU && screen == SCREEN_MAIN) { screen = SCREEN_BATTERY; selected = 0; }
}

static void draw_status(void) {
    char status[64], clock[8];
    platform_clock(clock, sizeof clock);
    snprintf(status, sizeof status, "BAT %d%% %s CPU %dC %s",
             platform_battery_percent(),
             platform_battery_charging() ? "CHG" : "",
             platform_cpu_temp_c(), clock);
    grid_puts(0, 0, "RG34XXSP Redux", 0);
    grid_puts(90 - (int)strlen(status), 0, status, 0);
}

void ui_render(SDL_Renderer *ren) {
    grid_clear();
    draw_status();

    if (screen == SCREEN_MAIN) {
        for (int i = 0; i < MAIN_COUNT; i++) {
            char line[90];
            snprintf(line, sizeof line, "%s%s", (i == selected) ? "> " : "  ", main_items[i]);
            grid_puts(0, 2 + i, line, i == selected);
        }
        grid_puts(0, 29, "A:Select  B:Back  M:Menu  Start:Search", 0);
    } else if (screen == SCREEN_ROMS) {
        grid_puts(0, 0, "Game Room > GBA", 0);
        for (int i = 0; i < ROM_COUNT; i++) {
            char line[90];
            snprintf(line, sizeof line, "%s%s", (i == selected) ? "> " : "  ", rom_items[i]);
            grid_puts(0, 3 + i, line, i == selected);
        }
        grid_puts(0, 29, "A:Launch  B:Back", 0);
    } else if (screen == SCREEN_SETTINGS) {
        grid_puts(0, 0, "Settings", 0);
        for (int i = 0; i < SETTINGS_COUNT; i++) {
            char line[90];
            snprintf(line, sizeof line, "%s%s", (i == selected) ? "> " : "  ", settings_items[i]);
            grid_puts(0, 2 + i, line, i == selected);
        }
        grid_puts(0, 29, "A:Enter  B:Back", 0);
    } else if (screen == SCREEN_CPU) {
        grid_puts(0, 0, "[ENGINEERING] CPU", 0);
        grid_puts(0, 2, "Governor: schedutil", 0);
        grid_puts(0, 3, "Max freq: 1512 MHz", 0);
        grid_puts(0, 4, "Current:  1008 MHz", 0);
        grid_puts(0, 29, "B:Back", 0);
    } else if (screen == SCREEN_BATTERY) {
        grid_puts(0, 0, "Battery", 0);
        grid_puts(0, 2, "Capacity: 87 %", 0);
        grid_puts(0, 3, "Temp:     unknown (unverified)", 0);
        grid_puts(0, 29, "B:Back", 0);
    }

    grid_present(ren);
}
