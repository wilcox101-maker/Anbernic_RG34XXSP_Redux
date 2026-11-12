/* Navigation test: B goes back one level, Engineering tabs wrap, B from Engineering exits to Main.
   Also the ROM scanner: systems list, launch, and scrolling a long list.
   Exit code 0 = pass. Uses the desktop platform layer (no window is opened). */
#define _POSIX_C_SOURCE 200809L
#include "ui.h"
#include "launcher.h"
#include "romscan.h"
#include "systems.h"
#include "wifi.h"
#include "stockmenu.h"
#include "grid.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int fails;
static void expect(int cond, const char *what) { printf("%s %s\n", cond ? "ok  " : "FAIL", what); if (!cond) fails++; }
static void press(logical_button_t b) { ui_button(b, 1, 1); ui_button(b, 0, 2); }
static void down(int n) { while (n--) press(LB_DOWN); }
static void tap(logical_button_t b, int n) { while (n--) press(b); }

static void write_empty(const char *path) {
    FILE *f = fopen(path, "w");
    if (f) fclose(f);
}

static char nav_saw_scan, nav_saw_pw, nav_pw_ok, nav_saw_ifname;
static const char *nav_want_pw;
static char nav_busy[80];

static int nav_spawn(char *const argv[], char *out, size_t out_n, int *status) {
    snprintf(nav_busy, sizeof nav_busy, "%s", ui_last_busy());
    if (status) *status = 0;
    if (argv[1] && strcmp(argv[1], "-t") == 0) {
        static const char *const want[] = {
            "/usr/bin/nmcli", "-t", "-f", "SSID,SIGNAL,SECURITY", "dev", "wifi", "list", NULL
        };
        nav_saw_scan = 1;
        for (int i = 0; want[i]; i++)
            if (!argv[i] || strcmp(argv[i], want[i]) != 0) nav_saw_scan = 0;
        if (out && out_n) snprintf(out, out_n, "Home:40:WPA2\nCafe:90:\n");
        return 0;
    }
    nav_saw_pw = 0;
    nav_pw_ok = 0;
    nav_saw_ifname = 0;
    for (int i = 0; argv[i]; i++) {
        if (strcmp(argv[i], "password") == 0 && argv[i + 1]) {
            nav_saw_pw = 1;
            if (nav_want_pw && strcmp(argv[i + 1], nav_want_pw) == 0) nav_pw_ok = 1;
        }
        if (strcmp(argv[i], "ifname") == 0 && argv[i + 1] && strcmp(argv[i + 1], "wlan0") == 0)
            nav_saw_ifname = 1;
    }
    if (!nav_want_pw) nav_pw_ok = !nav_saw_pw;
    if (out && out_n) out[0] = 0;
    return 0;
}

static int stock_calls;
static int stock_cmd_ok;
static char stock_busy[80];
static int stock_hook(const char *command) {
    stock_calls++;
    snprintf(stock_busy, sizeof stock_busy, "%s", ui_last_busy());
    stock_cmd_ok = strcmp(command, STOCK_RESTORE_CMD) == 0;
    return stock_cmd_ok ? 0 : 1;
}

static int presents;
static void count_present(void) { presents++; }

int main(void) {
    remove(UI_RESUME_PATH);
    unsetenv("REDUX_ROMS");
    ui_init();
    expect(ui_screen() == SCREEN_MAIN, "starts on Main");
    press(LB_A);                       /* Game Room */
    expect(ui_screen() == SCREEN_SYSTEMS, "A on Game Room opens the systems list");
    expect(romscan_systems() == 1 && strcmp(romscan_key(0), "GBA") == 0, "desktop fixture is one GBA system");
    press(LB_A);
    expect(ui_screen() == SCREEN_ROMS, "A on a system opens its ROM list");
    expect(romscan_roms(0) == 5, "fixture has the five mock-up ROMs");
    press(LB_B);
    expect(ui_screen() == SCREEN_SYSTEMS, "B from the ROM list returns to the systems list");
    press(LB_B);
    expect(ui_screen() == SCREEN_MAIN, "B from the systems list returns to Main");

    down(5); press(LB_A);              /* Internet */
    expect(ui_screen() == SCREEN_INTERNET, "A on Internet opens the Internet menu");
    press(LB_B);
    expect(ui_screen() == SCREEN_MAIN, "B from Internet returns to Main");

    ui_init(); down(7); press(LB_A);   /* Settings */
    expect(ui_screen() == SCREEN_SETTINGS, "A on Settings opens Settings");
    press(LB_B);
    expect(ui_screen() == SCREEN_MAIN, "B from Settings returns to Main");

    ui_init(); down(8); press(LB_A);   /* Engineering Mode */
    expect(ui_screen() == SCREEN_ENG_CONFIRM, "A on Engineering opens the hold-to-enter screen");
    ui_button(LB_A, 1, 100); ui_tick(1000); ui_button(LB_A, 0, 1000);
    expect(ui_screen() == SCREEN_ENG_CONFIRM, "releasing A early cancels");
    ui_button(LB_A, 1, 2000); ui_tick(4100);
    expect(ui_screen() == SCREEN_ENG, "holding A 2 s enters Engineering");
    ui_button(LB_A, 0, 4200);
    expect(ui_eng_tab() == ENG_CPU, "Engineering opens on the CPU tab");
    press(LB_R1);
    expect(ui_eng_tab() == ENG_BATTERY, "R1 moves to Battery");
    press(LB_LEFT); press(LB_LEFT);
    expect(ui_eng_tab() == ENG_SYSTEM, "Left wraps from CPU to System");
    press(LB_B);
    expect(ui_screen() == SCREEN_MAIN, "B from Engineering exits to Main, not the confirm screen");

    /* Launch the first fixture ROM through the systems table. */
    launcher_set_next_path("/tmp/redux-next-test");
    ui_init();
    press(LB_A); press(LB_A); press(LB_A);
    expect(ui_should_exit() == 1, "A on a ROM asks main to exit");
    expect(strcmp(ui_last_busy(), "Loading ROM") == 0, "launch shows Loading ROM before exit");
    {
        char dump[8192];
        grid_dump(dump, sizeof dump);
        expect(strstr(dump, "Advance Wars") != NULL, "splash names the game");
        expect(strstr(dump, "RetroArch / mgba") != NULL, "splash names the engine");
        expect(strstr(dump, "TF1") != NULL, "splash names the card");
        expect(strstr(dump, "please wait") != NULL, "splash asks to wait");
        expect(strstr(dump, "MENU") != NULL, "RetroArch splash has the menu tip");
        expect(strstr(dump, "\u2593\u2593\u2593") == NULL, "splash has no progress bar");
    }
    FILE *sf = fopen(UI_RESUME_PATH, "r");
    expect(sf != NULL, "launch still wrote the resume file");
    if (sf) fclose(sf);
    FILE *nf = fopen("/tmp/redux-next-test", "r");
    char line[512] = "";
    if (nf) { if (!fgets(line, sizeof line, nf)) line[0] = 0; fclose(nf); }
    expect(strstr(line, "mgba_libretro.so") != NULL, "GBA launches with mgba");
    expect(strstr(line, "Advance Wars.gba") != NULL, "launch line names the selected ROM");
    remove(UI_RESUME_PATH);   /* this launch is not the resume case; don't leak it */

    /* A long list scrolls so the selection stays on screen. */
    const char *longroot = "/tmp/redux-rom-long/GBA";
    mkdir("/tmp/redux-rom-long", 0755);
    mkdir(longroot, 0755);
    for (int i = 0; i < 40; i++) {
        char path[128];
        snprintf(path, sizeof path, "%s/Game %02d.gba", longroot, i);
        write_empty(path);
    }
    setenv("REDUX_ROMS", "/tmp/redux-rom-long", 1);
    ui_init();
    press(LB_A); press(LB_A);
    expect(romscan_roms(0) == 40, "long folder scans 40 ROMs");
    /* Left/Right page by the 20-row window and stop at the ends. */
    press(LB_RIGHT);
    expect(ui_list_selected() == 20 && ui_list_scroll() == 20, "Right pages to row 20");
    press(LB_RIGHT);
    expect(ui_list_selected() == 39 && ui_list_scroll() == 20, "Right stops on the last row");
    press(LB_RIGHT);
    expect(ui_list_selected() == 39 && ui_list_scroll() == 20, "Right on the last row does not wrap");
    press(LB_LEFT);
    expect(ui_list_selected() == 19 && ui_list_scroll() == 0, "Left pages back a window");
    press(LB_LEFT);
    expect(ui_list_selected() == 0 && ui_list_scroll() == 0, "Left stops on the first row");
    down(5); press(LB_RIGHT);
    expect(ui_list_selected() == 25 && ui_list_scroll() == 20, "the cursor keeps its row on screen");
    press(LB_LEFT); press(LB_LEFT);
    expect(ui_screen() == SCREEN_ROMS && ui_list_selected() == 0, "paging never leaves the ROM list");
    press(LB_B);
    press(LB_RIGHT);
    expect(ui_screen() == SCREEN_SYSTEMS && ui_list_selected() == 0, "Right on a one-system list stays on it");
    press(LB_A);
    down(30);
    expect(ui_list_selected() == 30, "Down moves the selection to row 30");
    expect(ui_list_scroll() == 30 - ROM_LIST_WINDOW + 1, "the window scrolls to keep row 30 visible");
    press(LB_UP);
    expect(ui_screen() == SCREEN_ROMS, "Up on a ROM list does not leave the screen");
    press(LB_DOWN);
    launcher_set_next_path("/tmp/redux-next-resume");
    press(LB_A);
    expect(ui_should_exit() == 1, "launching the scrolled row exits");
    ui_init();
    expect(ui_screen() == SCREEN_ROMS, "resume opens the same ROM list");
    expect(ui_list_selected() == 30, "resume keeps the selected row");
    expect(ui_list_scroll() == 30 - ROM_LIST_WINDOW + 1, "resume keeps the scroll");
    {
        FILE *sf = fopen(UI_RESUME_PATH, "r");
        expect(sf == NULL, "resume file is removed after a valid read");
        if (sf) fclose(sf);
    }
    press(LB_B);
    expect(ui_screen() == SCREEN_SYSTEMS, "B after resume returns to Systems");
    press(LB_B);
    expect(ui_screen() == SCREEN_MAIN, "B from Systems returns to Main");
    {
        FILE *sf = fopen(UI_RESUME_PATH, "w");
        expect(sf != NULL, "can write an unknown-system resume file");
        if (sf) { fputs("roms\nMISSING\n0\n0\n", sf); fclose(sf); }
        ui_init();
        expect(ui_screen() == SCREEN_MAIN, "unknown system starts on Main");
        sf = fopen(UI_RESUME_PATH, "r");
        expect(sf == NULL, "unknown-system resume file is removed");
        if (sf) fclose(sf);
        sf = fopen(UI_RESUME_PATH, "w");
        expect(sf != NULL, "can write an out-of-range resume file");
        if (sf) { fputs("roms\nGBA\n999\n0\n", sf); fclose(sf); }
        ui_init();
        expect(ui_screen() == SCREEN_MAIN, "row out of range starts on Main");
        sf = fopen(UI_RESUME_PATH, "r");
        expect(sf == NULL, "out-of-range resume file is removed");
        if (sf) fclose(sf);
    }

    /* Folder name PS is not a table key; the alias launches the PS1 (RetroArch) entry. */
    mkdir("/tmp/redux-rom-alias", 0755);
    mkdir("/tmp/redux-rom-alias/PS", 0755);
    write_empty("/tmp/redux-rom-alias/PS/castlevania.chd");
    setenv("REDUX_ROMS", "/tmp/redux-rom-alias", 1);
    ui_init();
    expect(romscan_systems() == 1 && strcmp(romscan_key(0), "PS1") == 0, "PS folder maps to the PS1 entry");
    launcher_set_next_path("/tmp/redux-next-ps");
    press(LB_A); press(LB_A); press(LB_A);
    nf = fopen("/tmp/redux-next-ps", "r");
    line[0] = 0;
    if (nf) { if (!fgets(line, sizeof line, nf)) line[0] = 0; fclose(nf); }
    expect(strstr(line, "pcsx") != NULL && strstr(line, "castlevania.chd") != NULL,
           "PS ROM launches the standalone pcsx line");
    remove(UI_RESUME_PATH);

    /* Stock folders added 2026-11-12: each key has a RetroArch core, and real folders are listed by name. */
    {
        static const char *keys[] = { "A2600", "A5200", "A7800", "A800", "LYNX", "ATARIST", "AMIGA",
            "C64", "VIC20", "MSX", "DOS", "SCUMMVM", "EASYRPG", "ONS", "FDS", "GW", "POKE", "VB",
            "PICO", "SEGA32X", "MDCD", "SATURN", "N64", "DREAMCAST", "NAOMI", "ATOMISWAVE", "NEOCD" };
        int ok = 1;
        for (size_t i = 0; i < sizeof keys / sizeof keys[0]; i++) {
            const system_entry_t *e = systems_lookup(keys[i]);
            if (!e || e->target != LAUNCH_RETROARCH || !e->core_or_path || !strstr(e->core_or_path, "_libretro.so")) {
                ok = 0; fprintf(stderr, "no core for %s\n", keys[i]);
            }
        }
        expect(ok, "all 27 new stock folders have a RetroArch core");
        mkdir("/tmp/redux-rom-more", 0755);
        const char *dirs[4] = { "N64", "DREAMCAST", "PICO", "A2600" };
        const char *files[4] = { "mario.z64", "crazy.chd", "celeste.p8", "pitfall.a26" };
        for (int i = 0; i < 4; i++) {
            char path[96];
            snprintf(path, sizeof path, "/tmp/redux-rom-more/%s", dirs[i]);
            mkdir(path, 0755);
            snprintf(path, sizeof path, "/tmp/redux-rom-more/%s/%s", dirs[i], files[i]);
            write_empty(path);
        }
        setenv("REDUX_ROMS", "/tmp/redux-rom-more", 1);
        ui_init();
        int named = 0;
        for (int s = 0; s < romscan_systems(); s++) {
            const char *tl = romscan_title(s);
            if (!strcmp(tl, "Nintendo 64") || !strcmp(tl, "Dreamcast") || !strcmp(tl, "PICO-8") ||
                !strcmp(tl, "Atari 2600")) named++;
        }
        expect(romscan_systems() == 4 && named == 4, "N64, Dreamcast, PICO-8 and Atari 2600 are listed by name");
    }

    /* The stock MAME folder is listed and launches through the mame2003_plus core. */
    mkdir("/tmp/redux-rom-mame", 0755);
    mkdir("/tmp/redux-rom-mame/MAME", 0755);
    write_empty("/tmp/redux-rom-mame/MAME/dkong.zip");
    setenv("REDUX_ROMS", "/tmp/redux-rom-mame", 1);
    ui_init();
    press(LB_A);
    expect(romscan_systems() == 1 && strcmp(romscan_key(0), "MAME") == 0, "MAME folder is a system");
    expect(strcmp(romscan_title(0), "MAME (Arcade)") == 0, "MAME has its display name");
    launcher_set_next_path("/tmp/redux-next-mame");
    press(LB_A); press(LB_A);
    nf = fopen("/tmp/redux-next-mame", "r");
    line[0] = 0;
    if (nf) { if (!fgets(line, sizeof line, nf)) line[0] = 0; fclose(nf); }
    expect(strstr(line, "mame2003_plus_libretro.so") != NULL && strstr(line, "dkong.zip") != NULL,
           "MAME ROM launches with the mame2003_plus core");
    remove(UI_RESUME_PATH);

    /* Network opens the Wi-Fi screen. The spawn hook stands in for nmcli. */
    wifi_set_spawn(nav_spawn);
    ui_init();
    down(6); press(LB_A);
    expect(ui_screen() == SCREEN_WIFI, "A on Network opens Wi-Fi");
    expect(nav_saw_scan == 1, "opening Wi-Fi scans with the nmcli list command");
    expect(strcmp(nav_busy, "Scanning for Wi-Fi networks") == 0, "scan shows the busy screen first");
    expect(wifi_count() == 2, "the hook's two networks are listed");
    press(LB_L1); press(LB_R1);
    expect(ui_screen() == SCREEN_WIFI, "L1 and R1 stay on the Wi-Fi list");
    press(LB_A);
    expect(ui_screen() == SCREEN_WIFI_KEY, "a secured network opens the keyboard");
    press(LB_L1); press(LB_R1);
    expect(ui_screen() == SCREEN_WIFI_KEY, "L1 and R1 stay on the Wi-Fi keyboard");
    press(LB_A);                       /* key 1 */
    {
        char dump[8192];
        ui_draw();
        grid_dump(dump, sizeof dump);
        expect(strstr(dump, "Password: *") != NULL, "the password is drawn as stars");
        expect(strstr(dump, "Password: 1") == NULL, "the typed character is not drawn");
    }
    nav_want_pw = "1";
    press(LB_START);
    expect(ui_screen() == SCREEN_WIFI, "Start connects and returns to the list");
    expect(nav_saw_pw == 1 && nav_pw_ok == 1 && nav_saw_ifname == 1,
           "connect got the typed password and ifname wlan0");
    expect(strcmp(nav_busy, "Connecting to Home") == 0, "connect shows the busy screen first");
    {
        char dump[8192];
        ui_draw();
        grid_dump(dump, sizeof dump);
        expect(strstr(dump, "Password") == NULL, "the list screen has no password");
    }
    press(LB_A); press(LB_A); press(LB_B);
    expect(ui_screen() == SCREEN_WIFI, "B leaves the keyboard");
    nav_want_pw = NULL;
    nav_pw_ok = 0;
    press(LB_DOWN); press(LB_A);       /* Cafe, empty security */
    expect(ui_screen() == SCREEN_WIFI, "empty security connects without the keyboard");
    expect(nav_pw_ok == 1 && nav_saw_pw == 0, "that connect has no password argument");
    expect(strcmp(nav_busy, "Connecting to Cafe") == 0, "open-network connect shows the busy screen first");
    {
        char dump[8192];
        ui_draw();
        grid_dump(dump, sizeof dump);
        expect(strstr(dump, "Back to stock") == NULL, "Wi-Fi list has no Back to stock row");
    }

    ui_init();
    down(7); press(LB_A); down(4); press(LB_A);
    expect(ui_screen() == SCREEN_WIFI, "Settings Network opens the same Wi-Fi screen");
    press(LB_B);
    expect(ui_screen() == SCREEN_SETTINGS, "B from Wi-Fi returns to Settings");

    /* Symbol layer: type 0x20..0x5E (63 bytes, the password cap) and require
       wifi_connect to receive that exact string. Not printed. */
    ui_init();
    down(6); press(LB_A); press(LB_A);
    expect(ui_screen() == SCREEN_WIFI_KEY, "keyboard opens again for a symbol password");
    tap(LB_DOWN, 4);
    tap(LB_RIGHT, 3);
    press(LB_A);                 /* sym, cursor stays on the sym key */
    press(LB_DOWN);              /* first symbol row, column follows the sym key */
    tap(LB_LEFT, 3);             /* column 0 is 0x20 */
    for (int row = 0; row < 4; row++) {
        for (int c = 0; c < 16; c++) {
            press(LB_A);
            if (c + 1 < 16) press(LB_RIGHT);
        }
        if (row + 1 < 4) {
            press(LB_DOWN);
            tap(LB_LEFT, 15);
        }
    }
    {
        char expect_pw[64];
        char dump[8192];
        for (int i = 0; i < 63; i++) expect_pw[i] = (char)(0x20 + i);
        expect_pw[63] = '\0';
        ui_draw();
        grid_dump(dump, sizeof dump);
        {
            char *p = strstr(dump, "Password: ");
            int stars = 0;
            expect(p != NULL, "symbol password line is on screen");
            if (p) {
                p += 10; /* strlen("Password: ") */
                while (*p == '*') { stars++; p++; }
                expect(stars == 63, "symbol password is shown as 63 stars");
                expect(*p != '!' && *p != '"' && *p != '\\', "symbols are not drawn in the password");
            }
        }
        nav_want_pw = expect_pw;
        press(LB_START);
        expect(ui_screen() == SCREEN_WIFI, "Start sends the symbol password");
        expect(nav_saw_pw == 1 && nav_pw_ok == 1, "wifi_connect got the exact symbol password");
    }

    stock_set_run(stock_hook);
    stock_set_bak(0);
    stock_calls = 0;
    ui_init();
    down(7); press(LB_A); down(5); press(LB_A);
    expect(ui_screen() == SCREEN_SYSTEM_DEFAULTS, "Settings System opens System Defaults");
    {
        char dump[8192];
        ui_draw();
        grid_dump(dump, sizeof dump);
        expect(strstr(dump, "Stock Menu") != NULL, "Stock Menu is listed");
        expect(strstr(dump, "Boot Screen") != NULL && strstr(dump, "not yet") != NULL,
               "Boot Screen is shown and not yet");
        expect(strstr(dump, "Reset All") != NULL, "Reset All is shown");
    }
    press(LB_DOWN); press(LB_DOWN); press(LB_A);
    expect(ui_screen() == SCREEN_STOCK_CONFIRM, "only Stock Menu accepts A");
    press(LB_B);
    expect(ui_screen() == SCREEN_SYSTEM_DEFAULTS, "B cancels back to System Defaults");
    expect(stock_calls == 0, "cancel does not run S7");
    press(LB_A);
    press(LB_A);
    expect(ui_screen() == SCREEN_SYSTEM_DEFAULTS, "a single A press does not restore");
    expect(stock_calls == 0, "a single A press does not run S7");
    press(LB_A);
    ui_button(LB_A, 1, 1000);
    ui_tick(3900);
    ui_button(LB_A, 0, 3900);
    expect(ui_screen() == SCREEN_SYSTEM_DEFAULTS, "releasing at 2.9 s cancels");
    expect(stock_calls == 0, "2.9 s does not run S7");
    press(LB_A);
    ui_button(LB_A, 1, 5000);
    ui_tick(6000);
    ui_button(LB_B, 1, 6100);
    expect(ui_screen() == SCREEN_SYSTEM_DEFAULTS, "another button during the hold cancels");
    expect(stock_calls == 0, "cancel during the hold does not run S7");
    ui_button(LB_B, 0, 6200);
    ui_button(LB_A, 0, 6300);
    press(LB_A);
    ui_button(LB_A, 1, 8000);
    ui_button(LB_A, 1, 10000);   /* a repeat press must not restart the 3 s clock */
    ui_tick(11000);
    expect(ui_screen() == SCREEN_STOCK_CONFIRM, "a finished hold with no backup stays");
    expect(stock_calls == 0, "missing dmenu_ln.bak does not run S7");
    ui_button(LB_A, 0, 11100);
    stock_set_bak(1);
    ui_button(LB_A, 1, 12000);
    ui_tick(15000);
    expect(stock_calls == 1 && stock_cmd_ok, "holding A for 3 s runs the atomic S7 command");
    expect(strcmp(stock_busy, "Restoring stock menu") == 0, "restore shows the busy screen first");

    wifi_set_spawn(NULL);
    stock_set_run(NULL);
    stock_set_bak(-1);

    presents = 0;
    ui_set_present(count_present);
    ui_init();
    expect(presents == PRESENT_BUFFERS, "startup scan is presented 3 times");
    expect(strcmp(ui_last_busy(), "Scanning for ROMs") == 0, "startup scan shows Scanning for ROMs");

    printf("%d failed\n", fails);
    return fails ? 1 : 0;
}
