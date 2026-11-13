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
#include "power.h"
#include "volume.h"
#include "brightness.h"
#include "sysinfo.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#define HOLD_TO_ENTER_MS 2000   /* Engineering Mode: hold A this long (ui-spec flag 3) */
#define HOLD_DESTRUCT_MS 3000   /* destructive confirms: Stock Menu, and later Restart / Power Off */
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
    "Display", "Audio", "Input", "Power & Lid", "System", "Emulation", "UI"
};
enum { SETTINGS_DISPLAY = 0, SETTINGS_SYSTEM = 4, SETTINGS_UI = 6 };   /* Network lives on the main menu only (Dale, 2026-11-12) */
enum { SYSTEM_STOCK = 0, SYSTEM_TOOLS = 1, SYSTEM_RESTART = 2, SYSTEM_POWEROFF = 3 };
typedef struct { const char *label; int selectable; int action; } system_row_t;
static const system_row_t system_rows[] = {
    { "System Tools",         1, SYSTEM_TOOLS },
    { "Restart",              1, SYSTEM_RESTART },
    { "Power Off",            1, SYSTEM_POWEROFF },
    { "Boot Screen   not yet", 0, -1 },
    { "",                     0, -1 },
    { "",                     0, -1 },
    { "",                     0, -1 },
    { "Stock Menu",           1, SYSTEM_STOCK },
    { "Reset All     not yet", 0, -1 }
};
#define N_SYSTEM_ROWS ((int)(sizeof system_rows / sizeof system_rows[0]))
static int system_selectable(int i) {
    return i >= 0 && i < N_SYSTEM_ROWS && system_rows[i].selectable;
}
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
static char wifi_result_ssid[WIFI_SSID_MAX];
static int wifi_result_ok;
static char wifi_retry[64];
static char stock_msg[48];

/* Colour presets (docs/ui-brief.md section 3). Default BIOS cyan. Saved to THEME_PATH. */
#ifdef REDUX_PLATFORM_STOCK
#define THEME_PATH "/mnt/data/redux/theme"
#else
#define THEME_PATH "/tmp/redux.theme"
#endif
typedef struct { const char *name; uint8_t r, g, b; } theme_preset_t;
static const theme_preset_t presets[] = {
    { "BIOS cyan", 0, 255, 255 },
    { "Amber",     255, 176, 0 },
    { "Green",     0, 255, 0 },
    { "White",     255, 255, 255 }
};
#define N_PRESETS ((int)(sizeof(presets) / sizeof(presets[0])))
static int theme_index;

static void theme_apply(int i) {
    if (i < 0 || i >= N_PRESETS) i = 0;
    theme_index = i;
    grid_set_accent(presets[i].r, presets[i].g, presets[i].b);
}

static void theme_save(void) {
#ifdef REDUX_PLATFORM_STOCK
    if (mkdir("/mnt/data/redux", 0755) != 0 && errno != EEXIST) return;
#endif
    FILE *f = fopen(THEME_PATH, "w");
    if (!f) return;
    fprintf(f, "%d\n", theme_index);
    fclose(f);
}

static void theme_load(void) {
    FILE *f = fopen(THEME_PATH, "r");
    int i = 0;
    if (f) {
        if (fscanf(f, "%d", &i) != 1) i = 0;
        fclose(f);
    }
    theme_apply(i);
}

static int stock_started;
static uint32_t a_down_at;
static uint32_t vol_until;   /* show VOL nn until this time */
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

static void clamp_scroll(void);
static int resume_load(void);

static void (*present_one)(void);
static char last_busy[80];

void ui_set_present(void (*fn)(void)) { present_one = fn; }
const char *ui_last_busy(void) { return last_busy; }

static void box_char(int col, int row, const char *ch) { grid_puts(col, row, ch, 0); }

void ui_busy(const char *title, const char *detail) {
    char store[8][88];
    int n = 0, saw_wait = 0;
    if (!title) title = "";
    snprintf(last_busy, sizeof last_busy, "%s", title);
    if (detail && detail[0]) {
        const char *p = detail;
        while (n < 8) {
            const char *nl = strchr(p, '\n');
            size_t len = nl ? (size_t)(nl - p) : strlen(p);
            if (len > 80) len = 80;
            memcpy(store[n], p, len);
            store[n][len] = '\0';
            if (strstr(store[n], "wait")) saw_wait = 1;
            n++;
            if (!nl) break;
            p = nl + 1;
            if (!*p && nl) break;
        }
    }
    if (!saw_wait && n < 8) snprintf(store[n++], sizeof store[0], "Please wait...");

    int text_max = (int)strlen(title);
    for (int i = 0; i < n; i++) {
        int L = (int)strlen(store[i]);
        if (L > text_max) text_max = L;
    }
    int inner = text_max + 2;          /* one space of padding on each side */
    if (inner < 24) inner = 24;
    if (inner > GRID_COLS - 6) inner = GRID_COLS - 6;
    int box_w = inner + 2;
    int top = (GRID_ROWS - (n + 2)) / 2;
    int left = (GRID_COLS - box_w) / 2;
    if (top < 0) top = 0;

    grid_clear();
    box_char(left, top, "┌");
    box_char(left + 1, top, " ");   /* two spaces so the dump's ▓▓ mark is not the title */
    box_char(left + 2, top, " ");
    for (int c = 3; c < box_w - 1; c++) box_char(left + c, top, "─");
    box_char(left + box_w - 1, top, "┐");
    grid_invert(top, left, left + box_w - 1);
    char tshow[64];
    snprintf(tshow, sizeof tshow, "%s", title);
    if ((int)strlen(tshow) > inner - 2) tshow[inner - 2] = '\0';
    int tlen = (int)strlen(tshow);
    int tcol = left + 3 + (box_w - 4 - tlen) / 2;
    if (tcol < left + 3) tcol = left + 3;
    grid_puts(tcol, top, tshow, 1);
    for (int i = 0; i < n; i++) {
        int r = top + 1 + i;
        box_char(left, r, "│");
        box_char(left + box_w - 1, r, "│");
        if (!store[i][0]) {
            for (int c = 1; c < box_w - 1; c++) box_char(left + c, r, "─");
        } else {
            grid_puts(left + 2, r, store[i], 0);
        }
    }
    int bot = top + n + 1;
    box_char(left, bot, "└");
    for (int c = 1; c < box_w - 1; c++) box_char(left + c, bot, "─");
    box_char(left + box_w - 1, bot, "┘");

    dirty = 0;
    if (present_one)
        for (int i = 0; i < PRESENT_BUFFERS; i++) present_one();
}

static void engine_name(const system_entry_t *sys, char *out, size_t n) {
    if (!sys) { snprintf(out, n, "unknown"); return; }
    if (sys->target == LAUNCH_RETROARCH) {
        const char *core = sys->core_or_path ? sys->core_or_path : "";
        const char *cut = strstr(core, "_libretro");
        char shortn[40];
        size_t L = cut ? (size_t)(cut - core) : strlen(core);
        if (L >= sizeof shortn) L = sizeof shortn - 1;
        memcpy(shortn, core, L);
        shortn[L] = '\0';
        if (shortn[0]) snprintf(out, n, "RetroArch / %s", shortn);
        else snprintf(out, n, "RetroArch");
    } else if (sys->target == LAUNCH_PPSSPP) snprintf(out, n, "PPSSPP");
    else if (sys->target == LAUNCH_DRASTIC) snprintf(out, n, "Drastic");
    else if (sys->target == LAUNCH_PCSX) snprintf(out, n, "PCSX");
    else if (sys->target == LAUNCH_PORTMASTER) snprintf(out, n, "PortMaster");
    else snprintf(out, n, "Standalone");
}

static const char *card_of(const char *path) {
    if (path && (strstr(path, "/mnt/sdcard") || strstr(path, "/sdcard/") || strstr(path, "TF2")))
        return "TF2";
    return "TF1";
}

static void show_loading(const system_entry_t *sys, const char *path) {
    char engine[64], detail[512];
    engine_name(sys, engine, sizeof engine);
    const char *tip = (sys && sys->target == LAUNCH_RETROARCH)
        ? "\nTip: press MENU in-game to open the RetroArch menu" : "";
    snprintf(detail, sizeof detail,
             "Game    %.48s\nSystem  %.32s\nEngine  %.48s\nCard    %s\n"
             "\nStarting emulator, please wait...%s",
             romscan_rom_name(sys_index, selected), romscan_title(sys_index),
             engine, card_of(path), tip);
    ui_busy("Loading ROM", detail);
}

void ui_init(void) {
    should_exit = 0; back_top = 0; eng_tab = ENG_CPU; sys_index = 0; screen = SCREEN_MAIN;
    theme_load();
    volume_load();
    brightness_load();
    ui_busy("Scanning for ROMs", NULL);
    romscan_init();
    go_replace(SCREEN_MAIN);
    resume_load();   /* valid resume replaces Main; the file is removed either way */
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
    case SCREEN_UI_COLORS: return N_PRESETS;
    case SCREEN_DISPLAY: return 1;
    case SCREEN_INTERNET: return COUNT(internet_items);
    case SCREEN_WIFI:     return wifi_count();
    case SCREEN_SYSTEM_DEFAULTS: return N_SYSTEM_ROWS;
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

/* Left/Right on the Game Room lists: move a whole window. The cursor keeps its row on
   screen, like turning a page. Stops at the ends (no wrap), so it can't overshoot. */
static void page(int dir) {
    int n = list_count();
    if (n < 1) return;
    int window = n < ROM_LIST_WINDOW ? n : ROM_LIST_WINDOW;
    int last_top = n - window;
    selected += dir * window;
    scroll += dir * window;
    if (selected < 0) selected = 0;
    if (selected > n - 1) selected = n - 1;
    if (scroll < 0) scroll = 0;
    if (scroll > last_top) scroll = last_top;
    clamp_scroll();
    dirty = 1;
}

/* Just before /tmp/.next: so the next start opens this list instead of Main.
   Four lines: screen, system key, row, scroll. Only "roms" is restored. */
static void resume_save(void) {
    FILE *f = fopen(UI_RESUME_PATH, "w");
    if (!f) return;
    fprintf(f, "roms\n%s\n%d\n%d\n", romscan_key(sys_index), selected, scroll);
    fclose(f);
}

static void resume_clear(void) { remove(UI_RESUME_PATH); }

static int resume_load(void) {
    FILE *f = fopen(UI_RESUME_PATH, "r");
    if (!f) return 0;
    char screen_tok[32], key[64], rowb[32], scrb[32];
    int got = fgets(screen_tok, sizeof screen_tok, f)
           && fgets(key, sizeof key, f)
           && fgets(rowb, sizeof rowb, f)
           && fgets(scrb, sizeof scrb, f);
    fclose(f);
    remove(UI_RESUME_PATH);
    if (!got) return 0;
    screen_tok[strcspn(screen_tok, "\r\n")] = 0;
    key[strcspn(key, "\r\n")] = 0;
    if (strcmp(screen_tok, "roms") != 0 || !key[0]) return 0;
    int sys = -1;
    for (int i = 0; i < romscan_systems(); i++)
        if (strcmp(romscan_key(i), key) == 0) { sys = i; break; }
    int row = -1, scr = 0;
    if (sys < 0 || sscanf(rowb, "%d", &row) != 1) return 0;
    if (row < 0 || row >= romscan_roms(sys)) return 0;
    sscanf(scrb, "%d", &scr);
    /* Same path as Game Room → Systems → ROM list, so B walks back through both. */
    push_back(SCREEN_MAIN);
    push_back(SCREEN_SYSTEMS);
    screen = SCREEN_ROMS;
    sys_index = sys;
    selected = row;
    scroll = scr < 0 ? 0 : scr;
    clamp_scroll();
    dirty = 1;
    return 1;
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

static void wipe_retry(void) {
    volatile char *v = wifi_retry;
    for (size_t i = 0; i < sizeof wifi_retry; i++) v[i] = 0;
}

static void wifi_show_result(const char *ssid, int rc) {
    snprintf(wifi_result_ssid, sizeof wifi_result_ssid, "%s", ssid ? ssid : "");
    wifi_result_ok = rc == 0;
    if (rc == 0) wipe_retry();
    if (screen == SCREEN_WIFI_KEY) back();
    if (screen != SCREEN_WIFI_RESULT) go(SCREEN_WIFI_RESULT);
    else dirty = 1;
}

static void wifi_begin(const wifi_ap_t *ap, const char *pw) {
    wipe_retry();
    if (pw && pw[0]) snprintf(wifi_retry, sizeof wifi_retry, "%s", pw);
    ui_busy("Connecting", ap ? ap->ssid : "");
    int rc = ap ? wifi_connect(ap->ssid, (pw && pw[0]) ? pw : NULL) : -1;
    wifi_show_result(ap ? ap->ssid : "", rc);
}

static void wifi_rescan(void) {
    ui_busy("Scanning for Wi-Fi networks", NULL);
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
    wifi_begin(ap, pw);
    wipe_pass();
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
    else if (key_col == KB_ACT_SYM) { key_sym = !key_sym; key_row = 0; key_col = 0; dirty = 1; }
    else if (key_col == KB_ACT_OK) wifi_submit();
}

void ui_button(logical_button_t btn, int pressed, uint32_t now_ms) {
    if (btn == LB_NONE) return;
    if (screen == SCREEN_WIFI_KEY) { wifi_key_button(btn); return; }

    if (screen == SCREEN_ENG && (btn == LB_L1 || btn == LB_LEFT))  { switch_eng_tab(-1); return; }
    if (screen == SCREEN_ENG && (btn == LB_R1 || btn == LB_RIGHT)) { switch_eng_tab(+1); return; }

    if (screen == SCREEN_WIFI && btn == LB_Y) { wifi_rescan(); return; }

    if (screen == SCREEN_DISPLAY && (btn == LB_LEFT || btn == LB_RIGHT)) {
        brightness_step(btn == LB_RIGHT ? +1 : -1);
        dirty = 1;
        return;
    }

    if ((screen == SCREEN_ROMS || screen == SCREEN_SYSTEMS) && (btn == LB_LEFT || btn == LB_RIGHT)) {
        page(btn == LB_RIGHT ? +1 : -1);
        return;
    }

    int n = list_count();
    if (n && (btn == LB_UP || btn == LB_DOWN)) {
        int dir = btn == LB_DOWN ? 1 : -1;
        int next = selected;
        for (int i = 0; i < n; i++) {
            next = (next + dir + n) % n;
            if (screen != SCREEN_SYSTEM_DEFAULTS || system_selectable(next)) break;
        }
        selected = next;
        clamp_scroll();
        dirty = 1;
    }

    if (btn == LB_A && screen == SCREEN_MAIN) {
        if (selected == MAIN_GAME_ROOM)   go(SCREEN_SYSTEMS);
        if (selected == MAIN_INTERNET)    go(SCREEN_INTERNET);
        if (selected == MAIN_SETTINGS)    go(SCREEN_SETTINGS);
        if (selected == MAIN_ENGINEERING) go(SCREEN_ENG_CONFIRM);
        if (selected == MAIN_NETWORK)     { go(SCREEN_WIFI); wifi_rescan(); }
        return;
    }
    if (btn == LB_A && screen == SCREEN_SETTINGS) {
        if (selected == SETTINGS_DISPLAY) go(SCREEN_DISPLAY);
        if (selected == SETTINGS_SYSTEM)  go(SCREEN_SYSTEM_DEFAULTS);
        if (selected == SETTINGS_UI)      go(SCREEN_UI_COLORS);
        return;
    }
    if (btn == LB_A && screen == SCREEN_UI_COLORS) {
        theme_apply(selected);
        theme_save();
        dirty = 1;
        return;
    }
    if (btn == LB_A && screen == SCREEN_SYSTEMS) {
        if (romscan_systems() > 0) { sys_index = selected; go(SCREEN_ROMS); }
        return;
    }
    if (btn == LB_A && screen == SCREEN_ROMS) {
        const system_entry_t *sys = systems_lookup(romscan_key(sys_index));
        const char *path = romscan_rom_path(sys_index, selected);
        if (sys && path[0]) {
            resume_save();
            volume_apply();
            if (launcher_start(sys->target, sys->core_or_path, path) == 0) {
                show_loading(sys, path);
                should_exit = 1;
            } else
                resume_clear();
        }
        return;
    }
    if (btn == LB_A && screen == SCREEN_SYSTEM_DEFAULTS) {
        if (!system_selectable(selected)) return;
        int action = system_rows[selected].action;
        if (action == SYSTEM_TOOLS) go(SCREEN_SYSINFO);
        if (action == SYSTEM_RESTART) go(SCREEN_RESTART_CONFIRM);
        if (action == SYSTEM_POWEROFF) go(SCREEN_POWEROFF_CONFIRM);
        if (action == SYSTEM_STOCK) go(SCREEN_STOCK_CONFIRM);
        return;
    }
    if (btn == LB_A && screen == SCREEN_STOCK_CONFIRM) {
        if (stock_started) return;
        stock_started = 1;
        if (stockmenu_start() == 0) {
            ui_busy("Stock Menu", "Returning to the stock launcher...\nPlease wait...");
            should_exit = 1;
        } else {
            stock_started = 0;
            snprintf(stock_msg, sizeof stock_msg, "failed");
            dirty = 1;
        }
        return;
    }
    if (btn == LB_A && screen == SCREEN_RESTART_CONFIRM) {
        if (stock_started) return;
        stock_started = 1;
        if (power_restart() == 0) {
            ui_busy("Restart", "Restarting...\nPlease wait...");
            should_exit = 1;
        } else {
            stock_started = 0;
            snprintf(stock_msg, sizeof stock_msg, "failed");
            dirty = 1;
        }
        return;
    }
    if (btn == LB_A && screen == SCREEN_POWEROFF_CONFIRM) {
        if (stock_started) return;
        stock_started = 1;
        if (power_off() == 0) {
            ui_busy("Power Off", "Powering off...\nPlease wait...");
            should_exit = 1;
        } else {
            stock_started = 0;
            snprintf(stock_msg, sizeof stock_msg, "failed");
            dirty = 1;
        }
        return;
    }
    if (btn == LB_B) { back(); return; }
}

void ui_tick(uint32_t now_ms) {
    if (a_down_at && now_ms - a_down_at >= hold_ms) {
        /* hold complete */
        a_down_at = 0;
        dirty = 1;
    }
    if (vol_until && now_ms >= vol_until) { vol_until = 0; dirty = 1; }
}

uint32_t ui_wait_ms(void) {
    if (a_down_at) return HOLD_WAIT_MS;
    if (vol_until) return 100;
    return IDLE_WAIT_MS;
}

static void frame(const char *title, const char *keys, int clock, int battery) {
    grid_clear();
    char t[GRID_COLS];
    snprintf(t, sizeof t, "%s", title);
    grid_puts(1, 0, t, 1);
    if (clock) {
        char clk[8];
        platform_clock(clk, sizeof clk);
        grid_puts(GRID_COLS - 7, 0, clk, 1);
    }
    if (battery) {
        char bat[8];
        platform_battery(bat, sizeof bat);
        grid_puts(GRID_COLS - 7, 1, bat, 1);
    }
    grid_puts(1, GRID_ROWS - 1, keys, 1);
}

static void body(int row, const char *text) {
    grid_puts(1, ROW_BODY0 + row, text, 0);
}

static void list(int first_line, const char **items, int n) {
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
   side, is inverted in the accent colour (grid_dump shows that padding as "▓"). */
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
    for (int c = 1; c < GRID_COLS - 1; c++) strcat(rule, "─");
    grid_puts(1, ROW_BODY0 + 1, rule, 0);
}

void ui_draw(void) {
    char t[GRID_COLS];
    switch (screen) {
    case SCREEN_MAIN: {
        char id[32], title[GRID_COLS];
        snprintf(id, sizeof id, REDUX_ID_FMT, REDUX_BUILD);
        snprintf(title, sizeof title, "RG34XXSP Redux  %s", id);
        frame(title, "A:Select  B:Back  M:Menu  Start:Search", 1, 0);
        list(0, main_items, COUNT(main_items));
        break;
    }
    case SCREEN_ROMS:
        snprintf(t, sizeof t, "Game Room > Systems > %s", romscan_title(sys_index));
        frame(t, "A:Launch  B:Back  X:Favorites  Y:Info  Start:Search  Select:Random  Left/Right:Page", 1, 0);
        body(0, "  Search: _");
        list_scanned(2, 0);
        {
            int n = romscan_roms(sys_index);
            int window = n < ROM_LIST_WINDOW ? n : ROM_LIST_WINDOW;
            body(2 + window, "   ...");
        }
        break;
    case SCREEN_SYSTEMS:
        frame("Game Room > Systems", "A:Enter  B:Back  Left/Right:Page", 1, 0);
        list_scanned(0, 1);
        break;
    case SCREEN_SETTINGS:
        frame("Settings", "A:Enter  B:Back  M:Menu", 1, 0);
        list(0, settings_items, COUNT(settings_items));
        break;
    case SCREEN_DISPLAY: {
        frame("Settings > Display", "Left/Right: Brightness  B:Back", 1, 0);
        char bline[GRID_COLS];
        snprintf(bline, sizeof bline, "   Brightness   %d", brightness_get());
        body(2, bline);
        break;
    }
    case SCREEN_UI_COLORS: {
        frame("Colors", "A:Select  B:Back  M:Menu", 1, 0);
        for (int i = 0; i < N_PRESETS; i++) {
            char line[80];
            snprintf(line, sizeof line, "  %s%s", presets[i].name,
                     i == theme_index ? "  (current)" : "");
            body(i, line);
            if (i == selected) grid_invert(ROW_BODY0 + i, 0, 1);
        }
        body(N_PRESETS + 1, "  Black background, white text, one accent.");
        body(N_PRESETS + 2, "  Saved. Survives reboot on the device.");
        break;
    }
    case SCREEN_INTERNET:
        frame("Internet", "A:Enter  B:Back", 1, 0);
        list(0, internet_items, COUNT(internet_items));
        break;
    case SCREEN_WIFI:
        frame("Network", "A:Connect  B:Back  Y:Rescan", 1, 0);
        if (wifi_msg[0]) body(0, wifi_msg);
        else if (wifi_count() == 0) body(0, "  no networks");
        else {
            for (int i = 0; i < wifi_count() && i < ROM_LIST_WINDOW; i++) {
                const wifi_ap_t *ap = wifi_ap(scroll + i);
                char line[GRID_COLS];
                snprintf(line, sizeof line, "   %s  %d%%", ap->ssid, ap->signal);
                body(i, line);
                if (scroll + i == selected) grid_invert(ROW_BODY0 + i, 1, GRID_COLS - 2);
            }
        }
        break;
    case SCREEN_WIFI_KEY:
        frame("Password", "A:Type  B:Back  Start:Connect", 1, 0);
        body(0, wifi_ap(wifi_pick)->ssid);
        {
            char stars[65];
            for (int i = 0; i < wifi_pass_len && i < 64; i++) stars[i] = '*';
            stars[wifi_pass_len] = '\0';
            body(1, stars);
        }
        {
            int nrows = kb_char_rows();
            for (int r = 0; r < nrows; r++) {
                const char *row = key_sym ? sym_rows[r] : kb_rows[r];
                char line[GRID_COLS] = " ";
                for (int c = 0; c < (int)strlen(row); c++) {
                    strcat(line, " ");
                    char ch[2] = { row[c], 0 };
                    strcat(line, ch);
                }
                body(3 + r, line);
                if (r == key_row) {
                    int c0 = 1 + key_col * 2;
                    int c1 = c0 + 1;
                    grid_invert(ROW_BODY0 + 3 + r, c0, c1);
                }
            }
            char acts[GRID_COLS] = " ";
            int start_at[KB_ACTS], end_at[KB_ACTS];
            int pos = 1;
            for (int a = 0; a < KB_ACTS; a++) {
                start_at[a] = pos;
                strcat(acts, kb_acts[a]);
                pos += (int)strlen(kb_acts[a]);
                end_at[a] = pos - 1;
                strcat(acts, "  ");
                pos += 2;
            }
            body(3 + nrows, acts);
            if (key_row == nrows) {
                int c0 = 1 + start_at[key_col];
                int c1 = 1 + end_at[key_col];
                grid_invert(ROW_BODY0 + 4 + nrows, c0, c1);
            }
        }
        break;
    case SCREEN_WIFI_RESULT:
        frame(wifi_result_ok ? "Connected" : "Not connected", "A:Retry  B:Back", 1, 0);
        body(2, wifi_result_ssid);
        body(3, wifi_result_ok ? "IP: (see Network)" : "Failed");
        break;
    case SCREEN_SYSTEM_DEFAULTS:
        frame("System", "A:Enter  B:Back", 1, 0);
        for (int i = 0; i < N_SYSTEM_ROWS; i++) {
            char line[GRID_COLS];
            snprintf(line, sizeof line, "   %s", system_rows[i].label);
            body(i, line);
            if (i == selected && system_rows[i].selectable)
                grid_invert(ROW_BODY0 + i, 1, GRID_COLS - 2);
        }
        break;
    case SCREEN_STOCK_CONFIRM:
        frame("Stock Menu", "A:Hold 3s  B:Back", 1, 0);
        body(2, "Return to the stock launcher?");
        body(3, stock_msg[0] ? stock_msg : "Hold A");
        break;
    case SCREEN_RESTART_CONFIRM:
        frame("Restart", "A:Hold 3s  B:Back", 1, 0);
        body(2, "Restart the device?");
        body(3, stock_msg[0] ? stock_msg : "Hold A");
        break;
    case SCREEN_POWEROFF_CONFIRM:
        frame("Power Off", "A:Hold 3s  B:Back", 1, 0);
        body(2, "Power off the device?");
        body(3, stock_msg[0] ? stock_msg : "Hold A");
        break;
    case SCREEN_SYSINFO:
        frame("System Tools", "B:Back", 1, 0);
        {
            char line[GRID_COLS];
            snprintf(line, sizeof line, "  Battery: %s", sysinfo_battery());
            body(0, line);
            snprintf(line, sizeof line, "  CPU: %s", sysinfo_cpu());
            body(1, line);
            snprintf(line, sizeof line, "  Thermal: %s", sysinfo_thermal());
            body(2, line);
            snprintf(line, sizeof line, "  RAM: %s", sysinfo_ram());
            body(3, line);
            snprintf(line, sizeof line, "  Mounts: %s", sysinfo_mounts());
            body(4, line);
        }
        break;
    case SCREEN_ENG_CONFIRM:
        frame("Engineering Mode", "A:Hold 2s  B:Back", 1, 0);
        body(2, "Enter Engineering Mode?");
        body(3, "Hold A");
        break;
    case SCREEN_ENG:
        frame("Engineering", "L1/R1:Tabs  B:Back", 1, 0);
        draw_eng_tabs();
        if (eng_tab == ENG_CPU) {
            body(2, "  Governor: schedutil");
            body(3, "  Freq: 480-1512 MHz");
        } else if (eng_tab == ENG_BATTERY) {
            body(2, "  Battery: unknown (no sensor)");
        } else {
            body(2, "  (not yet)");
        }
        break;
    }
    dirty = 0;
}
