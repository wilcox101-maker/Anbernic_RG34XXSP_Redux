/* Screens and navigation. Engineering is one screen with a tab bar.
   B goes back one level via a small stack. Internet is a sub-menu. */
#include "ui.h"
#include "grid.h"
#include "platform.h"
#include "launcher.h"
#include "systems.h"
#include <stdio.h>
#include <string.h>

#define HOLD_TO_ENTER_MS 2000
#define IDLE_WAIT_MS     1000
#define HOLD_WAIT_MS     50
#define BACK_MAX 8

static const char *main_items[] = {
    "Game Room", "Music", "Video", "Documents", "File Manager",
    "Internet", "Network", "Settings", "Engineering Mode"
};
enum { MAIN_GAME_ROOM = 0, MAIN_INTERNET = 5, MAIN_SETTINGS = 7, MAIN_ENGINEERING = 8 };

static const char *rom_items[] = {
    "Advance Wars", "Fire Emblem", "Golden Sun", "Metroid Fusion", "Pokemon Emerald"
};
static const char *rom_paths[] = {
    "/mnt/mmc/Roms/GBA/Advance Wars.gba",
    "/mnt/mmc/Roms/GBA/Fire Emblem.gba",
    "/mnt/mmc/Roms/GBA/Golden Sun.gba",
    "/mnt/mmc/Roms/GBA/Metroid Fusion.gba",
    "/mnt/mmc/Roms/GBA/Pokemon Emerald.gba"
};
static const char *rom_system = "GBA";
static const char *settings_items[] = {
    "Display", "Audio", "Input", "Power & Lid", "Network", "System", "Emulation"
};
static const char *internet_items[] = {
    "YouTube (if feasible)", "Web browser (text)"
};
static const char *eng_tabs[] = { "CPU", "Battery", "Thermal", "Display", "System" };
#define COUNT(a) ((int)(sizeof(a) / sizeof((a)[0])))

static screen_t screen;
static eng_tab_t eng_tab;
static int selected;
static int dirty;
static int should_exit;
static uint32_t a_down_at;
static uint32_t hold_ms;
static char last_clock[8];
static screen_t back_stack[BACK_MAX];
static int back_top;

static void push_back(screen_t from) {
    if (back_top < BACK_MAX) back_stack[back_top++] = from;
}

static void go(screen_t s) {
    push_back(screen);
    screen = s; selected = 0; a_down_at = 0; hold_ms = 0; dirty = 1;
}

static void back(void) {
    if (back_top > 0) screen = back_stack[--back_top];
    else screen = SCREEN_MAIN;
    selected = 0; a_down_at = 0; hold_ms = 0; dirty = 1;
}

void ui_init(void) { should_exit = 0; back_top = 0; eng_tab = ENG_CPU; go(SCREEN_MAIN); }
void ui_show(screen_t s) { back_top = 0; screen = s; selected = 0; dirty = 1; }
int ui_dirty(void) { return dirty; }
int ui_should_exit(void) { return should_exit; }

static int list_count(void) {
    switch (screen) {
    case SCREEN_MAIN:     return COUNT(main_items);
    case SCREEN_ROMS:     return COUNT(rom_items);
    case SCREEN_SETTINGS: return COUNT(settings_items);
    case SCREEN_INTERNET: return COUNT(internet_items);
    default:              return 0;
    }
}

static void switch_eng_tab(int delta) {
    eng_tab = (eng_tab + delta + ENG_TAB_COUNT) % ENG_TAB_COUNT;
    dirty = 1;
}

void ui_button(logical_button_t btn, int pressed, uint32_t now_ms) {
    if (screen == SCREEN_ENG_CONFIRM && btn == LB_A) {
        if (pressed) { a_down_at = now_ms ? now_ms : 1; hold_ms = 0; }
        else { a_down_at = 0; hold_ms = 0; }
        dirty = 1;
        return;
    }
    if (!pressed) return;

    if (screen == SCREEN_ENG && (btn == LB_L1 || btn == LB_LEFT))  { switch_eng_tab(-1); return; }
    if (screen == SCREEN_ENG && (btn == LB_R1 || btn == LB_RIGHT)) { switch_eng_tab(+1); return; }

    int n = list_count();
    if (n && btn == LB_UP)   { selected = (selected - 1 + n) % n; dirty = 1; }
    if (n && btn == LB_DOWN) { selected = (selected + 1) % n; dirty = 1; }

    if (btn == LB_A && screen == SCREEN_MAIN) {
        if (selected == MAIN_GAME_ROOM)   go(SCREEN_ROMS);
        if (selected == MAIN_INTERNET)    go(SCREEN_INTERNET);
        if (selected == MAIN_SETTINGS)    go(SCREEN_SETTINGS);
        if (selected == MAIN_ENGINEERING) go(SCREEN_ENG_CONFIRM);
        return;
    }
    if (btn == LB_A && screen == SCREEN_ROMS) {
        const system_entry_t *sys = systems_lookup(rom_system);
        if (sys && launcher_start(sys->target, sys->core_or_path, rom_paths[selected]) == 0)
            should_exit = 1;
        return;
    }
    if (btn == LB_B && screen != SCREEN_MAIN) back();
}

void ui_tick(uint32_t now_ms) {
    if (screen == SCREEN_ENG_CONFIRM && a_down_at) {
        hold_ms = now_ms - a_down_at;
        if (hold_ms >= HOLD_TO_ENTER_MS) go(SCREEN_ENG);
        dirty = 1;
    }
    char c[8];
    platform_clock(c, sizeof c);
    if (strcmp(c, last_clock) != 0) dirty = 1;
}

uint32_t ui_wait_ms(void) {
    return (screen == SCREEN_ENG_CONFIRM && a_down_at) ? HOLD_WAIT_MS : IDLE_WAIT_MS;
}

static void draw_status(int wifi, int cpu_temp) {
    char clock[8], s[64];
    platform_clock(clock, sizeof clock);
    snprintf(last_clock, sizeof last_clock, "%s", clock);
    int n = snprintf(s, sizeof s, "BAT %d%%", platform_battery_percent());
    if (platform_battery_charging()) n += snprintf(s + n, sizeof s - n, " CHG");
    if (wifi && platform_wifi_connected()) n += snprintf(s + n, sizeof s - n, " WIFI");
    if (cpu_temp) n += snprintf(s + n, sizeof s - n, " CPU %dC", platform_cpu_temp_c());
    snprintf(s + n, sizeof s - n, " %s", clock);
    grid_puts_right(ROW_TITLE, s);
}

static void frame(const char *title, const char *help, int wifi, int cpu_temp) {
    grid_clear();
    grid_puts(0, ROW_TITLE, title, 0);
    draw_status(wifi, cpu_temp);
    grid_box();
    grid_puts(0, ROW_HELP, help, 0);
}

static void body(int line, const char *text) { grid_puts(1, ROW_BODY0 + line, text, 0); }

static void list(int first_line, const char *const *items, int n) {
    for (int i = 0; i < n; i++) {
        char line[GRID_COLS];
        snprintf(line, sizeof line, "   %s", items[i]);
        body(first_line + i, line);
        if (i == selected) grid_invert(ROW_BODY0 + first_line + i, 1, GRID_COLS - 2);
    }
}

static void draw_eng_tabs(void) {
    char line[GRID_COLS] = " ";
    for (int i = 0; i < ENG_TAB_COUNT; i++) {
        if (i == eng_tab) strcat(line, "\u2593");
        strcat(line, eng_tabs[i]);
        if (i == eng_tab) strcat(line, "\u2593");
        strcat(line, "  ");
    }
    body(0, line);
    if (eng_tab == ENG_CPU) grid_invert(ROW_BODY0, 1, 5);
}

void ui_draw(void) {
    char t[GRID_COLS];
    switch (screen) {
    case SCREEN_MAIN:
        frame("RG34XXSP Redux", "A:Select  B:Back  M:Menu  Start:Search", 1, 0);
        list(0, main_items, COUNT(main_items));
        break;
    case SCREEN_ROMS:
        frame("Game Room > Systems > Game Boy Advance",
              "A:Launch  B:Back  X:Favorites  Y:Info  Start:Search  Select:Random", 1, 0);
        body(0, "  Search: _");
        list(2, rom_items, COUNT(rom_items));
        body(2 + COUNT(rom_items), "   ...");
        break;
    case SCREEN_SETTINGS:
        frame("Settings", "A:Enter  B:Back  M:Menu", 1, 0);
        list(0, settings_items, COUNT(settings_items));
        break;
    case SCREEN_INTERNET:
        frame("Internet", "A:Enter  B:Back", 1, 0);
        list(0, internet_items, COUNT(internet_items));
        break;
    case SCREEN_ENG_CONFIRM: {
        frame("[ENGINEERING]", "Hold A:Enter  B:Back", 0, 1);
        body(0, "  Engineering Mode shows live CPU, GPU, thermal and power controls.");
        body(1, "  Changes apply live and are only kept if you choose \"Save as profile\".");
        body(3, "  Hold A for 2 seconds to enter. Release to cancel.");
        int width = 40, filled = (int)(hold_ms * (uint32_t)width / HOLD_TO_ENTER_MS);
        if (filled > width) filled = width;
        char bar[4 + 40 * 3 + 2] = "  [";
        for (int i = 0; i < width; i++) strcat(bar, i < filled ? "\u2593" : " ");
        strcat(bar, "]");
        body(5, bar);
        break;
    }
    case SCREEN_ENG:
        frame("[ENGINEERING]", "L1/R1:Tab  A:Edit  B:Exit  X:Save profile  Y:Reset", 0, 1);
        draw_eng_tabs();
        if (eng_tab == ENG_CPU) {
            body(2, "  Governor:     schedutil");
            body(3, "  Min freq:     480 MHz");
            body(4, "  Max freq:     1512 MHz");
            snprintf(t, sizeof t, "  Current:      %d MHz", platform_cpu_cur_mhz(0)); body(5, t);
        } else if (eng_tab == ENG_BATTERY) {
            int ma = platform_battery_ma();
            snprintf(t, sizeof t, "  Capacity:     %d %%", platform_battery_percent()); body(2, t);
            snprintf(t, sizeof t, "  Voltage:      %d.%02d V", platform_battery_mv() / 1000,
                     (platform_battery_mv() % 1000) / 10); body(3, t);
            body(4, platform_battery_ma_known() ? "  Current:      (known)" : "  Current:      unknown");
            (void)ma;
        } else {
            body(2, "  Not yet");
        }
        break;
    }
    dirty = 0;
}
