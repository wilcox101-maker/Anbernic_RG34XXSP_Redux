/* Screens and navigation. F2: ROM select calls the launcher. */
#include "ui.h"
#include "grid.h"
#include "platform.h"
#include "launcher.h"
#include <stdio.h>
#include <string.h>

#define HOLD_TO_ENTER_MS 2000
#define IDLE_WAIT_MS     1000
#define HOLD_WAIT_MS     50

static const char *main_items[] = {
    "Game Room", "Music", "Video", "Documents", "File Manager",
    "YouTube (if feasible)", "Network", "Settings", "Engineering Mode"
};
enum { MAIN_GAME_ROOM = 0, MAIN_SETTINGS = 7, MAIN_ENGINEERING = 8 };

/* Demo ROMs. Real F2 reads the stock folders. */
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
static const char *settings_items[] = {
    "Display", "Audio", "Input", "Power & Lid", "Network", "System", "Emulation"
};
#define COUNT(a) ((int)(sizeof(a) / sizeof((a)[0])))

static screen_t screen;
static int selected;
static int dirty;
static uint32_t a_down_at;
static uint32_t hold_ms;
static char last_clock[8];

static void go(screen_t s) { screen = s; selected = 0; a_down_at = 0; hold_ms = 0; dirty = 1; }

void ui_init(void) { go(SCREEN_MAIN); }
void ui_show(screen_t s) { go(s); }
int ui_dirty(void) { return dirty; }

static int list_count(void) {
    switch (screen) {
    case SCREEN_MAIN:     return COUNT(main_items);
    case SCREEN_ROMS:     return COUNT(rom_items);
    case SCREEN_SETTINGS: return COUNT(settings_items);
    default:              return 0;
    }
}

void ui_button(logical_button_t btn, int pressed, uint32_t now_ms) {
    if (screen == SCREEN_ENG_CONFIRM && btn == BTN_A) {
        if (pressed) { a_down_at = now_ms ? now_ms : 1; hold_ms = 0; }
        else { a_down_at = 0; hold_ms = 0; }
        dirty = 1;
        return;
    }
    if (!pressed) return;

    int n = list_count();
    if (n && btn == BTN_UP)   { selected = (selected - 1 + n) % n; dirty = 1; }
    if (n && btn == BTN_DOWN) { selected = (selected + 1) % n; dirty = 1; }

    if (btn == BTN_A && screen == SCREEN_MAIN) {
        if (selected == MAIN_GAME_ROOM)   go(SCREEN_ROMS);
        if (selected == MAIN_SETTINGS)    go(SCREEN_SETTINGS);
        if (selected == MAIN_ENGINEERING) go(SCREEN_ENG_CONFIRM);
        return;
    }
    if (btn == BTN_A && screen == SCREEN_ROMS) {
        /* Launch the selected ROM via RetroArch (GBA core). */
        if (launcher_start(LAUNCH_RETROARCH, "mgba_libretro.so", rom_paths[selected]) == 0) {
            /* main() will exit so loadapp.sh runs /tmp/.next. */
        }
        return;
    }
    if ((btn == BTN_LEFT || btn == BTN_RIGHT) && (screen == SCREEN_CPU || screen == SCREEN_BATTERY)) {
        go(screen == SCREEN_CPU ? SCREEN_BATTERY : SCREEN_CPU);
        return;
    }
    if (btn == BTN_B && screen != SCREEN_MAIN) go(SCREEN_MAIN);
}

void ui_tick(uint32_t now_ms) {
    if (screen == SCREEN_ENG_CONFIRM && a_down_at) {
        hold_ms = now_ms - a_down_at;
        if (hold_ms >= HOLD_TO_ENTER_MS) go(SCREEN_CPU);
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
    case SCREEN_ENG_CONFIRM: {
        frame("[ENGINEERING]", "Hold A:Enter  B:Back", 0, 1);
        body(0, "  Engineering Mode shows live CPU, GPU, thermal and power controls.");
        body(1, "  Changes apply live and are only kept if you choose \"Save as profile\".");
        body(3, "  Hold A for 2 seconds to enter. Release to cancel.");
        int width = 40, filled = (int)(hold_ms * (uint32_t)width / HOLD_TO_ENTER_MS);
        if (filled > width) filled = width;
        char bar[4 + 40 * 3 + 2] = "  [";
        for (int i = 0; i < width; i++) strcat(bar, i < filled ? "▓" : " ");
        strcat(bar, "]");
        body(5, bar);
        break;
    }
    case SCREEN_CPU:
        frame("[ENGINEERING] CPU", "A:Edit  B:Back  X:Save as profile  Y:Reset to stock", 0, 1);
        body(0, "  Governor:     schedutil");
        body(1, "  Min freq:     480 MHz");
        body(2, "  Max freq:     1512 MHz");
        snprintf(t, sizeof t, "  Current:      %d MHz", platform_cpu_cur_mhz(0)); body(3, t);
        body(5, "  Per-core:");
        for (int c = 0; c < 4; c++) {
            snprintf(t, sizeof t, "    cpu%d  %d MHz   %s", c, platform_cpu_cur_mhz(c),
                     platform_cpu_online(c) ? "online" : "offline");
            body(6 + c, t);
        }
        body(11, "  Stock OPP table:");
        body(12, "    480 / 600 / 792 / 1008 / 1200 / 1296 / 1416 / 1512");
        body(14, "  Live changes only. Persist with \"Save as profile\".");
        body(15, "  Boot watchdog resets settings after two failed boots.");
        body(16, "  Battery-heat guard remains active.");
        body(17, "  Entry required deliberate confirm.");
        break;
    case SCREEN_BATTERY: {
        frame("Battery", "A:Refresh  B:Back  M:Menu", 0, 0);
        int ma = platform_battery_ma();
        snprintf(t, sizeof t, "  Capacity:     %d %%", platform_battery_percent()); body(0, t);
        snprintf(t, sizeof t, "  Voltage:      %d.%02d V", platform_battery_mv() / 1000,
                 (platform_battery_mv() % 1000) / 10); body(1, t);
        snprintf(t, sizeof t, "  Current:     %s%d mA", ma < 0 ? "-" : " ", ma < 0 ? -ma : ma); body(2, t);
        snprintf(t, sizeof t, "  Status:       %s", ma < 0 ? "Discharging" : "Charging"); body(3, t);
        body(4, "  Time left:    ~4 h 12 m");
        body(6, platform_battery_temp_known() ? "  Temperature:  -- C"
                                              : "  Temperature:  unknown   (thermistor unverified)");
        body(7, "  Max seen:     --");
        snprintf(t, sizeof t, "  SoC temp:     %d C", platform_cpu_temp_c()); body(9, t);
        body(11, "  Stock PMIC protection: disabled. Redux guard limits:");
        body(12, "    Charge stop:     45 C   (typical Li-ion, not stock)");
        body(13, "    Discharge stop:  60 C   (typical Li-ion, not stock)");
        body(15, "  Drain (last hour): -8 %");
        break;
    }
    }
    dirty = 0;
}
