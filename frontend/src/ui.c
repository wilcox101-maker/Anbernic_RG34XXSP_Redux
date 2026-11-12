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
enum { SETTINGS_SYSTEM = 4, SETTINGS_UI = 6 };   /* Network lives on the main menu only (Dale, 2026-11-12) */
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
    /* One A tap must never run a destructive action. The clock starts at the
       first press; a repeat press does not move it. Release, or any other
       button, leaves the screen without running it. */
    if (screen == SCREEN_STOCK_CONFIRM || screen == SCREEN_RESTART_CONFIRM
        || screen == SCREEN_POWEROFF_CONFIRM) {
        if (btn == LB_A) {
            if (pressed) {
                if (!a_down_at && !stock_started)
                    a_down_at = now_ms ? now_ms : 1;
            } else if (a_down_at && !stock_started) {
                a_down_at = 0;   /* released before 3 s: stay, reset the bar */
                hold_ms = 0;
            }
        } else if (pressed) {
            a_down_at = 0;
            hold_ms = 0;
            back();
        }
        dirty = 1;
        return;
    }
    if (!pressed) return;

    /* Shoulders are not Wi-Fi navigation. L2/R2 are not logical buttons, so they never get here. */
    if ((screen == SCREEN_WIFI || screen == SCREEN_WIFI_KEY) &&
        (btn == LB_L1 || btn == LB_R1))
        return;

    if (screen == SCREEN_WIFI_KEY) { wifi_key_button(btn); return; }

    if (screen == SCREEN_ENG && (btn == LB_L1 || btn == LB_LEFT))  { switch_eng_tab(-1); return; }
    if (screen == SCREEN_ENG && (btn == LB_R1 || btn == LB_RIGHT)) { switch_eng_tab(+1); return; }

    if (screen == SCREEN_WIFI && btn == LB_Y) { wifi_rescan(); return; }

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
        if (action == SYSTEM_TOOLS) { go(SCREEN_SYSINFO); return; }
        if (action == SYSTEM_RESTART || action == SYSTEM_POWEROFF || action == SYSTEM_STOCK) {
            stock_msg[0] = '\0';
            stock_started = 0;
            go(action == SYSTEM_RESTART ? SCREEN_RESTART_CONFIRM
               : action == SYSTEM_POWEROFF ? SCREEN_POWEROFF_CONFIRM
               : SCREEN_STOCK_CONFIRM);
            return;
        }
        return;
    }
    if (btn == LB_A && screen == SCREEN_WIFI) {
        const wifi_ap_t *ap = wifi_ap(selected);
        if (!ap) return;
        if (ap->open) wifi_begin(ap, NULL);
        else wifi_open_key();
        return;
    }
    if (screen == SCREEN_WIFI_RESULT && btn == LB_A && !wifi_result_ok) {
        char ssid[WIFI_SSID_MAX];
        snprintf(ssid, sizeof ssid, "%s", wifi_result_ssid);
        ui_busy("Connecting", ssid);
        int rc = wifi_connect(ssid, wifi_retry[0] ? wifi_retry : NULL);
        wifi_show_result(ssid, rc);
        return;
    }
    if (btn == LB_B && screen == SCREEN_WIFI_RESULT) wipe_retry();
    if (btn == LB_B && screen != SCREEN_MAIN) back();
}

void ui_tick(uint32_t now_ms) {
    if (screen == SCREEN_ENG_CONFIRM && a_down_at) {
        hold_ms = now_ms - a_down_at;
        if (hold_ms >= HOLD_TO_ENTER_MS) go_replace(SCREEN_ENG);   /* B from Engineering -> Main */
        dirty = 1;
    }
    if ((screen == SCREEN_STOCK_CONFIRM || screen == SCREEN_RESTART_CONFIRM
         || screen == SCREEN_POWEROFF_CONFIRM) && a_down_at && !stock_started) {
        hold_ms = now_ms - a_down_at;
        if (hold_ms >= HOLD_DESTRUCT_MS) {
            a_down_at = 0;
            if (screen == SCREEN_STOCK_CONFIRM) {
                ui_busy("Restoring stock menu", NULL);
                int rc = stock_restore();
                if (rc == 0) {
                    stock_started = 1;
                    snprintf(stock_msg, sizeof stock_msg, "rebooting");
                } else if (rc == -2) {
                    snprintf(stock_msg, sizeof stock_msg, "no dmenu_ln.bak; refusing");
                } else {
                    snprintf(stock_msg, sizeof stock_msg, "restore failed");
                }
            } else {
                int rc = (screen == SCREEN_RESTART_CONFIRM) ? power_reboot() : power_poweroff();
                stock_started = 1;
                snprintf(stock_msg, sizeof stock_msg, "%s",
                         rc == 0 ? (screen == SCREEN_RESTART_CONFIRM ? "restarting" : "powering off")
                                 : "failed");
            }
        }
        dirty = 1;
    }
    char c[8];
    platform_clock(c, sizeof c);
    if (strcmp(c, last_clock) != 0) dirty = 1;
}

uint32_t ui_wait_ms(void) {
    if (a_down_at && (screen == SCREEN_ENG_CONFIRM || screen == SCREEN_STOCK_CONFIRM
        || screen == SCREEN_RESTART_CONFIRM || screen == SCREEN_POWEROFF_CONFIRM))
        return HOLD_WAIT_MS;
    return IDLE_WAIT_MS;
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

/* "  [" starts at column 1, so the 40 fill cells are columns BAR_C0..BAR_C0+39.
 * The filled run is drawn as inverted blanks, which grid_present paints in the
 * accent colour -- the same colour as the selection bar. */
#define BAR_WIDTH 40
#define BAR_C0    4

static void draw_hold_bar(int line, uint32_t total_ms) {
    int filled = total_ms ? (int)(hold_ms * (uint32_t)BAR_WIDTH / total_ms) : 0;
    if (filled > BAR_WIDTH) filled = BAR_WIDTH;
    char bar[3 + BAR_WIDTH + 2];
    int n = snprintf(bar, sizeof bar, "  [");
    for (int i = 0; i < BAR_WIDTH; i++) bar[n++] = ' ';
    bar[n++] = ']';
    bar[n] = '\0';
    body(line, bar);
    if (filled > 0) grid_invert(ROW_BODY0 + line, BAR_C0, BAR_C0 + filled - 1);
}

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
        if (romscan_systems() == 0) body(0, "   (no ROMs)");
        else list_scanned(0, 1);
        break;
    case SCREEN_SETTINGS:
        frame("Settings", "A:Enter  B:Back  M:Menu", 1, 0);
        list(0, settings_items, COUNT(settings_items));
        break;
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
    case SCREEN_ENG_CONFIRM: {
        frame("[ENGINEERING]", "Hold A:Enter  B:Back", 0, 1);
        body(0, "  Engineering Mode shows live CPU, GPU, thermal and power controls.");
        body(1, "  Changes apply live and are only kept if you choose \"Save as profile\".");
        body(3, "  Hold A for 2 seconds to enter. Release to cancel.");
        draw_hold_bar(5, HOLD_TO_ENTER_MS);
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
        int window = n < ROM_LIST_WINDOW ? n : ROM_LIST_WINDOW;
        for (int i = 0; i < window; i++) {
            int idx = scroll + i;
            const wifi_ap_t *ap = wifi_ap(idx);
            char line[GRID_COLS];
            if (!ap) continue;
            snprintf(line, sizeof line, "   %-24.24s %3d  %.12s",
                     ap->ssid, ap->signal, ap->security);
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
    case SCREEN_WIFI_RESULT: {
        frame(wifi_result_ok ? "Connected" : "Not connected",
              wifi_result_ok ? "B:Back" : "A:Retry  B:Back", 1, 0);
        char line[GRID_COLS];
        snprintf(line, sizeof line, "  Network:  %.48s", wifi_result_ssid);
        body(0, line);
        body(1, wifi_result_ok ? "  Status:   connected" : "  Status:   failed");
        if (wifi_result_ok) {
            const char *ip = wifi_last_ip();
            snprintf(line, sizeof line, "  IP:       %.48s", (ip && ip[0]) ? ip : "none");
            body(2, line);
        } else {
            const char *why = wifi_last_reason();
            snprintf(line, sizeof line, "  Reason:   %.60s", (why && why[0]) ? why : "failed");
            body(2, line);
        }
        break;
    }
    case SCREEN_SYSTEM_DEFAULTS:
        frame("System Defaults", "A:Select  B:Back", 1, 0);
        for (int i = 0; i < N_SYSTEM_ROWS; i++) {
            char line[GRID_COLS];
            snprintf(line, sizeof line, "   %s", system_rows[i].label);
            body(i, line);
            if (i == selected) grid_invert(ROW_BODY0 + i, 1, GRID_COLS - 2);
        }
        break;
    case SCREEN_SYSINFO: {
        frame("System Tools", "B:Back", 1, 0);
        sysinfo_row_t rows[SYSINFO_MAX_ROWS];
        int n = sysinfo_collect(rows, SYSINFO_MAX_ROWS);
        for (int i = 0; i < n && i < BODY_ROWS; i++) {
            char line[GRID_COLS];
            snprintf(line, sizeof line, "  %-14.14s %.40s", rows[i].label, rows[i].value);
            body(i, line);
        }
        break;
    }
    case SCREEN_RESTART_CONFIRM:
        frame("Restart", "Hold A 3s:Restart  B:Cancel", 1, 0);
        body(0, "  Runs sync, then reboot.");
        body(2, "  Hold A for 3 seconds. Release, or any other button, cancels.");
        draw_hold_bar(4, HOLD_DESTRUCT_MS);
        if (stock_msg[0]) {
            char line[GRID_COLS];
            snprintf(line, sizeof line, "  %s", stock_msg);
            body(6, line);
        }
        break;
    case SCREEN_POWEROFF_CONFIRM:
        frame("Power Off", "Hold A 3s:Power Off  B:Cancel", 1, 0);
        body(0, "  Runs sync, then poweroff.");
        body(2, "  Hold A for 3 seconds. Release, or any other button, cancels.");
        draw_hold_bar(4, HOLD_DESTRUCT_MS);
        if (stock_msg[0]) {
            char line[GRID_COLS];
            snprintf(line, sizeof line, "  %s", stock_msg);
            body(6, line);
        }
        break;
    case SCREEN_STOCK_CONFIRM:
        frame("Stock Menu", "Hold A 3s:Restore  B:Cancel", 1, 0);
        body(0, "  Restores the stock launcher from dmenu_ln.bak.");
        body(1, "  Copies it to dmenu_ln.new, then renames that");
        body(2, "  over /mnt/vendor/ctrl/dmenu_ln, syncs, and reboots.");
        body(3, "  The backup file is kept. Refuses if it is missing.");
        body(5, "  Hold A for 3 seconds. Release, or any other button, cancels.");
        draw_hold_bar(7, HOLD_DESTRUCT_MS);
        if (stock_msg[0]) {
            char line[GRID_COLS];
            snprintf(line, sizeof line, "  %s", stock_msg);
            body(9, line);
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
