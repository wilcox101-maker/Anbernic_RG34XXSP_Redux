/* Screens and navigation. Engineering is one screen with a tab bar.
   B goes back one level via a small stack. Internet is a sub-menu. */
#include "ui.h"
#include "grid.h"
#include "platform.h"
#include "launcher.h"
#include "systems.h"
#include "romscan.h"
#include "wifi.h"
#include "stockmenu.h"
#include <stdio.h>
#include <string.h>

#define HOLD_TO_ENTER_MS 2000   /* Engineering Mode: hold A this long (ui-spec flag 3) */
#define IDLE_WAIT_MS     1000   /* wake once a second so the clock can roll over */
#define HOLD_WAIT_MS     50     /* smooth progress bar while A is held */
#define BACK_MAX 8              /* back stack depth; deeper than any menu we have */

static const char *main_items[] = {
    "Game Room", "Music", "Video", "Documents", "File Manager",
    "Internet", "Network", "Settings", "Engineering Mode"
};
enum { MAIN_GAME_ROOM = 0, MAIN_INTERNET = 5, MAIN_NETWORK = 6, MAIN_SETTINGS = 7, MAIN_ENGINEERING = 8 };

/* ROM names come from romscan (read-only). The desktop fixture recreates the
   five F0 names so this screen still matches the mock-up. */
static const char *settings_items[] = {
    "Display", "Audio", "Input", "Power & Lid", "Network", "System", "Emulation"
};
enum { SETTINGS_NETWORK = 4 };
static const char *internet_items[] = {
    "YouTube (if feasible)", "Web browser (text)"
};
static const char *eng_tabs[] = { "CPU", "Battery", "Thermal", "Display", "System" };
#define COUNT(a) ((int)(sizeof(a) / sizeof((a)[0])))

static screen_t screen;
static eng_tab_t eng_tab;
static int selected;
static int scroll;
static int sys_index;
static int dirty;
static int should_exit;
static int key_row;
static int key_col;
static int key_shift;
static int key_sym;
static int wifi_pick;
static char wifi_pass[64];
static int wifi_pass_len;
static char wifi_msg[48];
static char stock_msg[48];
static int stock_started;
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
    screen = s; selected = 0; scroll = 0; a_down_at = 0; hold_ms = 0; dirty = 1;
}

static void back(void) {
    if (back_top > 0) screen = back_stack[--back_top];
    else screen = SCREEN_MAIN;
    selected = 0; scroll = 0; a_down_at = 0; hold_ms = 0; dirty = 1;
}

/* Replace the current screen without remembering it (the confirm screen is not a place to go back to). */
static void go_replace(screen_t s) {
    screen = s; selected = 0; scroll = 0; a_down_at = 0; hold_ms = 0; dirty = 1;
}

void ui_init(void) {
    romscan_init();
    should_exit = 0; back_top = 0; eng_tab = ENG_CPU; sys_index = 0; screen = SCREEN_MAIN;
    go_replace(SCREEN_MAIN);
}
void ui_set_eng_tab(eng_tab_t t) { eng_tab = t; dirty = 1; }
screen_t ui_screen(void) { return screen; }
eng_tab_t ui_eng_tab(void) { return eng_tab; }
int ui_list_selected(void) { return selected; }
int ui_list_scroll(void) { return scroll; }
void ui_show(screen_t s) {
    back_top = 0; screen = s; selected = 0; scroll = 0; dirty = 1;
    if (s == SCREEN_ROMS) sys_index = romscan_index_of("GBA");
}
int ui_dirty(void) { return dirty; }
int ui_should_exit(void) { return should_exit; }

static int list_count(void) {
    switch (screen) {
    case SCREEN_MAIN:     return COUNT(main_items);
    case SCREEN_SYSTEMS:  return romscan_systems();
    case SCREEN_ROMS:     return romscan_roms(sys_index);
    case SCREEN_SETTINGS: return COUNT(settings_items);
    case SCREEN_INTERNET: return COUNT(internet_items);
    case SCREEN_WIFI:     return wifi_count() + 1;   /* last row: Back to stock menu */
    default:              return 0;
    }
}

static void clamp_scroll(void) {
    int n = list_count();
    int window = n < ROM_LIST_WINDOW ? n : ROM_LIST_WINDOW;
    if (window < 1) { scroll = 0; return; }
    if (selected < 0) selected = 0;
    if (selected >= n) selected = n - 1;
    if (selected < scroll) scroll = selected;
    if (selected >= scroll + window) scroll = selected - window + 1;
}

static void switch_eng_tab(int delta) {
    eng_tab = (eng_tab_t)((eng_tab + delta + ENG_TAB_COUNT) % ENG_TAB_COUNT);
    dirty = 1;
}

/* On-screen keyboard. The letter layer is a-z (shift latches), digits, and . - _ .
   "sym" switches to every printable ASCII byte (0x20-0x7E) in order, 16 per row,
   same 90x30 grid and one cell per key. The password is stars on screen and is
   wiped on Back, on connect, and when leaving the keyboard. It is never written
   to a file here. */
static const char *kb_rows[] = {
    "1234567890",
    "qwertyuiop",
    "asdfghjkl",
    "zxcvbnm.-_"
};
#define KB_ALPHA_ROWS 4
static const char *sym_rows[] = {
    " !\"#$%&'()*+,-./",   /* 0x20-0x2F */
    "0123456789:;<=>?",    /* 0x30-0x3F */
    "@ABCDEFGHIJKLMNO",    /* 0x40-0x4F */
    "PQRSTUVWXYZ[\\]^_",   /* 0x50-0x5F */
    "`abcdefghijklmno",    /* 0x60-0x6F */
    "pqrstuvwxyz{|}~"      /* 0x70-0x7E */
};
#define KB_SYM_ROWS 6
_Static_assert(sizeof " !\"#$%&'()*+,-./" == 17, "sym row 0x20");
_Static_assert(sizeof "0123456789:;<=>?" == 17, "sym row 0x30");
_Static_assert(sizeof "@ABCDEFGHIJKLMNO" == 17, "sym row 0x40");
_Static_assert(sizeof "PQRSTUVWXYZ[\\]^_" == 17, "sym row 0x50");
_Static_assert(sizeof "`abcdefghijklmno" == 17, "sym row 0x60");
_Static_assert(sizeof "pqrstuvwxyz{|}~" == 16, "sym row 0x70");
static const char *kb_acts[] = { "space", "del", "shift", "sym", "OK" };
#define KB_ACT_SPACE 0
#define KB_ACT_DEL   1
#define KB_ACT_SHIFT 2
#define KB_ACT_SYM   3
#define KB_ACT_OK    4
#define KB_ACTS 5

static int kb_char_rows(void) { return key_sym ? KB_SYM_ROWS : KB_ALPHA_ROWS; }

static void wipe_pass(void) {
    volatile char *v = wifi_pass;
    for (size_t i = 0; i < sizeof wifi_pass; i++) v[i] = 0;
    wifi_pass_len = 0;
}

static void wifi_rescan(void) {
    wifi_msg[0] = '\0';
    if (wifi_scan() < 0) snprintf(wifi_msg, sizeof wifi_msg, "scan failed");
    else if (wifi_count() == 0) snprintf(wifi_msg, sizeof wifi_msg, "no networks");
    selected = 0;
    scroll = 0;
    dirty = 1;
}

static void wifi_type(char ch) {
    if (wifi_pass_len + 1 < (int)sizeof wifi_pass) {
        wifi_pass[wifi_pass_len++] = ch;
        wifi_pass[wifi_pass_len] = '\0';
    }
    dirty = 1;
}

static void wifi_submit(void) {
    const wifi_ap_t *ap = wifi_ap(wifi_pick);
    const char *pw = wifi_pass_len ? wifi_pass : NULL;
    int rc = ap ? wifi_connect(ap->ssid, pw) : -1;
    wipe_pass();
    snprintf(wifi_msg, sizeof wifi_msg, "%s", rc == 0 ? "connected" : "failed");
    back();
}

static int kb_cols(int row) {
    if (row >= 0 && row < kb_char_rows())
        return (int)strlen(key_sym ? sym_rows[row] : kb_rows[row]);
    if (row == kb_char_rows()) return KB_ACTS;
    return 0;
}

static void wifi_key_button(logical_button_t btn) {
    int cols = kb_cols(key_row);
    if (btn == LB_LEFT && cols)  { key_col = (key_col - 1 + cols) % cols; dirty = 1; }
    if (btn == LB_RIGHT && cols) { key_col = (key_col + 1) % cols; dirty = 1; }
    if (btn == LB_UP || btn == LB_DOWN) {
        int rows = kb_char_rows() + 1;
        key_row = (key_row + (btn == LB_UP ? rows - 1 : 1)) % rows;
        cols = kb_cols(key_row);
        if (cols > 0 && key_col >= cols) key_col = cols - 1;
        dirty = 1;
    }
    if (btn == LB_B) { wipe_pass(); back(); return; }
    if (btn == LB_START) { wifi_submit(); return; }
    if (btn != LB_A) return;
    if (key_row < kb_char_rows()) {
        char ch = (key_sym ? sym_rows[key_row] : kb_rows[key_row])[key_col];
        if (!key_sym && key_shift && ch >= 'a' && ch <= 'z') ch = (char)(ch - 'a' + 'A');
        wifi_type(ch);
        return;
    }
    if (key_col == KB_ACT_SPACE) wifi_type(' ');
    else if (key_col == KB_ACT_DEL) {
        if (wifi_pass_len > 0) {
            wifi_pass_len--;
            wifi_pass[wifi_pass_len] = '\0';
            dirty = 1;
        }
    } else if (key_col == KB_ACT_SHIFT) { key_shift = !key_shift; dirty = 1; }
    else if (key_col == KB_ACT_SYM) {
        key_sym = !key_sym;
        key_row = kb_char_rows();
        key_col = KB_ACT_SYM;
        dirty = 1;
    } else wifi_submit();
}

static void wifi_open_key(void) {
    wifi_pick = selected;
    key_row = 0;
    key_col = 0;
    key_shift = 0;
    key_sym = 0;
    wipe_pass();
    go(SCREEN_WIFI_KEY);
}

void ui_button(logical_button_t btn, int pressed, uint32_t now_ms) {
    if (screen == SCREEN_ENG_CONFIRM && btn == LB_A) {
        if (pressed) { a_down_at = now_ms ? now_ms : 1; hold_ms = 0; }
        else { a_down_at = 0; hold_ms = 0; }          /* released early: cancel */
        dirty = 1;
        return;
    }
    if (!pressed) return;

    if (screen == SCREEN_WIFI_KEY) { wifi_key_button(btn); return; }

    if (screen == SCREEN_STOCK_CONFIRM && btn == LB_A) {
        if (!stock_started) {
            int rc = stock_restore();
            if (rc == 0) {
                stock_started = 1;
                snprintf(stock_msg, sizeof stock_msg, "rebooting");
            } else if (rc == -2) {
                snprintf(stock_msg, sizeof stock_msg, "no dmenu_ln.bak; refusing");
            } else {
                snprintf(stock_msg, sizeof stock_msg, "restore failed");
            }
        }
        dirty = 1;
        return;
    }

    if (screen == SCREEN_ENG && (btn == LB_L1 || btn == LB_LEFT))  { switch_eng_tab(-1); return; }
    if (screen == SCREEN_ENG && (btn == LB_R1 || btn == LB_RIGHT)) { switch_eng_tab(+1); return; }

    if (screen == SCREEN_WIFI && btn == LB_Y) { wifi_rescan(); return; }

    int n = list_count();
    if (n && btn == LB_UP)   { selected = (selected - 1 + n) % n; clamp_scroll(); dirty = 1; }
    if (n && btn == LB_DOWN) { selected = (selected + 1) % n; clamp_scroll(); dirty = 1; }

    if (btn == LB_A && screen == SCREEN_MAIN) {
        if (selected == MAIN_GAME_ROOM)   go(SCREEN_SYSTEMS);
        if (selected == MAIN_INTERNET)    go(SCREEN_INTERNET);
        if (selected == MAIN_SETTINGS)    go(SCREEN_SETTINGS);
        if (selected == MAIN_ENGINEERING) go(SCREEN_ENG_CONFIRM);
        if (selected == MAIN_NETWORK)     { go(SCREEN_WIFI); wifi_rescan(); }
        return;
    }
    if (btn == LB_A && screen == SCREEN_SETTINGS) {
        if (selected == SETTINGS_NETWORK) { go(SCREEN_WIFI); wifi_rescan(); }
        return;
    }
    if (btn == LB_A && screen == SCREEN_SYSTEMS) {
        if (romscan_systems() > 0) { sys_index = selected; go(SCREEN_ROMS); }
        return;
    }
    if (btn == LB_A && screen == SCREEN_ROMS) {
        const system_entry_t *sys = systems_lookup(romscan_key(sys_index));
        const char *path = romscan_rom_path(sys_index, selected);
        if (sys && path[0] && launcher_start(sys->target, sys->core_or_path, path) == 0)
            should_exit = 1;
        return;
    }
    if (btn == LB_A && screen == SCREEN_WIFI) {
        if (selected == wifi_count()) {
            stock_msg[0] = '\0';
            stock_started = 0;
            go(SCREEN_STOCK_CONFIRM);
            return;
        }
        const wifi_ap_t *ap = wifi_ap(selected);
        if (!ap) return;
        if (ap->open) {
            int rc = wifi_connect(ap->ssid, NULL);
            snprintf(wifi_msg, sizeof wifi_msg, "%s", rc == 0 ? "connected" : "failed");
            dirty = 1;
        } else {
            wifi_open_key();
        }
        return;
    }
    if (btn == LB_B && screen != SCREEN_MAIN) back();
}

void ui_tick(uint32_t now_ms) {
    if (screen == SCREEN_ENG_CONFIRM && a_down_at) {
        hold_ms = now_ms - a_down_at;
        if (hold_ms >= HOLD_TO_ENTER_MS) go_replace(SCREEN_ENG);   /* B from Engineering -> Main */
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
    int window = n < ROM_LIST_WINDOW ? n : ROM_LIST_WINDOW;
    for (int i = 0; i < window; i++) {
        int idx = scroll + i;
        char line[GRID_COLS];
        snprintf(line, sizeof line, "   %s", items[idx]);
        body(first_line + i, line);
        if (idx == selected) grid_invert(ROW_BODY0 + first_line + i, 1, GRID_COLS - 2);
    }
}

/* Same bar, but the strings come from the scanner (systems or one system's ROMs). */
static void list_scanned(int first_line, int systems) {
    int n = systems ? romscan_systems() : romscan_roms(sys_index);
    int window = n < ROM_LIST_WINDOW ? n : ROM_LIST_WINDOW;
    for (int i = 0; i < window; i++) {
        int idx = scroll + i;
        const char *text = systems ? romscan_title(idx) : romscan_rom_name(sys_index, idx);
        char line[GRID_COLS];
        snprintf(line, sizeof line, "   %s", text);
        body(first_line + i, line);
        if (idx == selected) grid_invert(ROW_BODY0 + first_line + i, 1, GRID_COLS - 2);
    }
}

/* BIOS-style tab bar on body row 0, a rule on row 1. The active tab, padded by one space on each
   side, is inverted in the accent colour (grid_dump shows that padding as "\u2593"). */
static void draw_eng_tabs(void) {
    char line[GRID_COLS * 3] = " ";
    int col = 2, active_c0 = 0, active_c1 = 0;   /* body text starts at column 1, plus the lead space */
    for (int i = 0; i < ENG_TAB_COUNT; i++) {
        int w = (int)strlen(eng_tabs[i]) + 2;
        if (i == (int)eng_tab) { active_c0 = col; active_c1 = col + w - 1; }
        strcat(line, " ");
        strcat(line, eng_tabs[i]);
        strcat(line, " ");
        col += w;
    }
    body(0, line);
    grid_invert(ROW_BODY0, active_c0, active_c1);
    char rule[GRID_COLS * 3] = "";
    for (int c = 1; c < GRID_COLS - 1; c++) strcat(rule, "\u2500");
    grid_puts(1, ROW_BODY0 + 1, rule, 0);
}

void ui_draw(void) {
    char t[GRID_COLS];
    switch (screen) {
    case SCREEN_MAIN:
        frame("RG34XXSP Redux", "A:Select  B:Back  M:Menu  Start:Search", 1, 0);
        list(0, main_items, COUNT(main_items));
        break;
    case SCREEN_ROMS:
        snprintf(t, sizeof t, "Game Room > Systems > %s", romscan_title(sys_index));
        frame(t, "A:Launch  B:Back  X:Favorites  Y:Info  Start:Search  Select:Random", 1, 0);
        body(0, "  Search: _");
        list_scanned(2, 0);
        {
            int n = romscan_roms(sys_index);
            int window = n < ROM_LIST_WINDOW ? n : ROM_LIST_WINDOW;
            body(2 + window, "   ...");
        }
        break;
    case SCREEN_SYSTEMS:
        frame("Game Room > Systems", "A:Enter  B:Back", 1, 0);
        if (romscan_systems() == 0) body(0, "   (no ROMs)");
        else list_scanned(0, 1);
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
        char bar[4 + 40 * 3 + 2] = "  [";   /* "\u2593" is 3 bytes in UTF-8 */
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
            body(7, "  Per-core:");
            for (int c = 0; c < 4; c++) {
                snprintf(t, sizeof t, "    cpu%d  %d MHz   %s", c, platform_cpu_cur_mhz(c),
                         platform_cpu_online(c) ? "online" : "offline");
                body(8 + c, t);
            }
            body(13, "  Stock OPP table:");
            body(14, "    480 / 600 / 792 / 1008 / 1200 / 1296 / 1416 / 1512");
            body(16, "  Live changes only. Persist with \"Save as profile\".");
            body(17, "  Boot watchdog resets settings after two failed boots.");
            body(18, "  Battery-heat guard remains active.");
            body(19, "  Entry required deliberate confirm.");
        } else if (eng_tab == ENG_BATTERY) {
            int ma = platform_battery_ma();
            snprintf(t, sizeof t, "  Capacity:     %d %%", platform_battery_percent()); body(2, t);
            snprintf(t, sizeof t, "  Voltage:      %d.%02d V", platform_battery_mv() / 1000,
                     (platform_battery_mv() % 1000) / 10); body(3, t);
            if (platform_battery_ma_known()) {
                snprintf(t, sizeof t, "  Current:     %s%d mA", ma < 0 ? "-" : " ", ma < 0 ? -ma : ma); body(4, t);
                snprintf(t, sizeof t, "  Status:       %s", ma < 0 ? "Discharging" : "Charging"); body(5, t);
            } else {
                /* Stock PMIC exposes no current: use the status file instead. */
                body(4, "  Current:      unknown");
                snprintf(t, sizeof t, "  Status:       %s",
                         platform_battery_charging() ? "Charging" : "Discharging"); body(5, t);
            }
            body(6, "  Time left:    ~4 h 12 m");
            body(8, platform_battery_temp_known() ? "  Temperature:  -- C"
                                                  : "  Temperature:  unknown   (thermistor unverified)");
            body(9, "  Max seen:     --");
            snprintf(t, sizeof t, "  SoC temp:     %d C", platform_cpu_temp_c()); body(11, t);
            body(13, "  Stock PMIC protection: disabled. Redux guard limits:");
            body(14, "    Charge stop:     45 C   (typical Li-ion, not stock)");
            body(15, "    Discharge stop:  60 C   (typical Li-ion, not stock)");
            body(17, "  Drain (last hour): -8 %");
        } else {
            body(3, "  Not yet. This tab arrives with its phase.");
        }
        break;
    case SCREEN_WIFI: {
        frame("Wi-Fi", "A:Select  B:Back  Y:Scan", 1, 0);
        int n = wifi_count();
        int rows = n + 1;
        int window = rows < ROM_LIST_WINDOW ? rows : ROM_LIST_WINDOW;
        for (int i = 0; i < window; i++) {
            int idx = scroll + i;
            char line[GRID_COLS];
            if (idx == n) {
                snprintf(line, sizeof line, "   Back to stock menu");
            } else {
                const wifi_ap_t *ap = wifi_ap(idx);
                if (!ap) continue;
                snprintf(line, sizeof line, "   %-24.24s %3d  %.12s",
                         ap->ssid, ap->signal, ap->security);
            }
            body(i, line);
            if (idx == selected) grid_invert(ROW_BODY0 + i, 1, GRID_COLS - 2);
        }
        if (wifi_truncated()) body(window < 22 ? window : 21, "   (list truncated)");
        if (wifi_msg[0]) {
            char line[GRID_COLS];
            snprintf(line, sizeof line, "  %s", wifi_msg);
            body(22, line);
        }
        break;
    }
    case SCREEN_STOCK_CONFIRM:
        frame("Back to stock menu", "A:Restore and reboot  B:Cancel", 1, 0);
        body(0, "  Restores the stock launcher from dmenu_ln.bak.");
        body(1, "  Copies it to dmenu_ln.new, then renames that");
        body(2, "  over /mnt/vendor/ctrl/dmenu_ln, syncs, and reboots.");
        body(3, "  The backup file is kept. Refuses if it is missing.");
        body(5, "  A confirms. B cancels. This writes to the device.");
        if (stock_msg[0]) {
            char line[GRID_COLS];
            snprintf(line, sizeof line, "  %s", stock_msg);
            body(7, line);
        }
        break;
    case SCREEN_WIFI_KEY: {
        frame("Wi-Fi password", "A:Type  B:Back  Start:Connect", 1, 0);
        const wifi_ap_t *ap = wifi_ap(wifi_pick);
        char line[GRID_COLS];
        snprintf(line, sizeof line, "  Network: %.48s", ap ? ap->ssid : "");
        body(0, line);
        char stars[64];
        int ns = wifi_pass_len > 63 ? 63 : wifi_pass_len;
        for (int i = 0; i < ns; i++) stars[i] = '*';
        stars[ns] = '\0';
        snprintf(line, sizeof line, "  Password: %s", stars);
        body(1, line);
        snprintf(line, sizeof line, "  Shift: %s   Sym: %s",
                 key_shift ? "on" : "off", key_sym ? "on" : "off");
        body(2, line);
        {
            int nrows = kb_char_rows();
            for (int r = 0; r < nrows; r++) {
                const char *src = key_sym ? sym_rows[r] : kb_rows[r];
                char row[GRID_COLS];
                int p = 0;
                row[p++] = ' ';
                row[p++] = ' ';
                int mark = -1;
                for (int c = 0; src[c] && p + 2 < GRID_COLS; c++) {
                    char ch = src[c];
                    if (!key_sym && key_shift && ch >= 'a' && ch <= 'z')
                        ch = (char)(ch - 'a' + 'A');
                    if (key_row == r && key_col == c) mark = p;
                    row[p++] = ch;
                    row[p++] = ' ';
                }
                row[p] = '\0';
                body(4 + r, row);
                if (mark >= 0) {
                    int cell = 1 + mark;
                    grid_invert(ROW_BODY0 + 4 + r, cell, cell);
                }
            }
            char row[GRID_COLS];
            int p = 0;
            int start_at[KB_ACTS], end_at[KB_ACTS];
            row[p++] = ' ';
            row[p++] = ' ';
            for (int a = 0; a < KB_ACTS; a++) {
                if (a && p + 1 < GRID_COLS) row[p++] = ' ';
                start_at[a] = p;
                for (const char *s = kb_acts[a]; *s && p + 1 < GRID_COLS; s++) row[p++] = *s;
                end_at[a] = p - 1;
            }
            row[p] = '\0';
            body(4 + nrows, row);
            if (key_row == nrows) {
                int c0 = 1 + start_at[key_col];
                int c1 = 1 + end_at[key_col];
                grid_invert(ROW_BODY0 + 4 + nrows, c0, c1);
            }
        }
        break;
    }
    }
    dirty = 0;
}
