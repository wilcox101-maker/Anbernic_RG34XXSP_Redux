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
#include "power.h"
#include "volume.h"
#include "brightness.h"
#include "sysinfo.h"
#include "apps.h"
#include "cardscripts.h"
#include "bootscreen.h"
#include "screentimeout.h"
#include "font.h"
#include "grid.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int fails;
static void expect(int cond, const char *what) { printf("%s %s\n", cond ? "ok  " : "FAIL", what); if (!cond) fails++; }
static void press(logical_button_t b) { ui_button(b, 1, 1); ui_button(b, 0, 2); }
static void down(int n) { while (n--) press(LB_DOWN); }
static void tap(logical_button_t b, int n) { while (n--) press(b); }

static int boot_mounts;
static int boot_hook(const char *cmd) {
    if (strcmp(cmd, BOOTSCREEN_MOUNT_CMD) == 0) boot_mounts++;
    return 0;
}

static void write_empty(const char *path) {
    FILE *f = fopen(path, "w");
    if (f) fclose(f);
}

static char nav_saw_scan, nav_saw_pw, nav_pw_ok, nav_saw_ifname;
static const char *nav_want_pw;
static char nav_busy[80];
static int nav_fail;

static char nav_last_ssid[64] = "Home";
static int nav_spawn(char *const argv[], char *out, size_t out_n, int *status) {
    snprintf(nav_busy, sizeof nav_busy, "%s", ui_last_busy());
    if (argv[3] && strcmp(argv[3], "GENERAL.STATE,GENERAL.CONNECTION") == 0) {
        if (status) *status = 0;
        if (out && out_n) snprintf(out, out_n, "GENERAL.STATE:100 (connected)\nGENERAL.CONNECTION:%s\n", nav_last_ssid);
        return 0;
    }
    if (argv[3] && strcmp(argv[3], "IP4.ADDRESS") == 0) {
        if (status) *status = 0;
        if (out && out_n) snprintf(out, out_n, "IP4.ADDRESS[1]:192.168.1.20/24\n");
        return 0;
    }
    if (status) *status = nav_fail;
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
    if (argv[1] && strcmp(argv[1], "dev") == 0 && argv[4])
        snprintf(nav_last_ssid, sizeof nav_last_ssid, "%s", argv[4]);
    for (int i = 0; argv[i]; i++) {
        if (strcmp(argv[i], "password") == 0 && argv[i + 1]) {
            nav_saw_pw = 1;
            if (nav_want_pw && strcmp(argv[i + 1], nav_want_pw) == 0) nav_pw_ok = 1;
        }
        if (strcmp(argv[i], "ifname") == 0 && argv[i + 1] && strcmp(argv[i + 1], "wlan0") == 0)
            nav_saw_ifname = 1;
    }
    if (!nav_want_pw) nav_pw_ok = !nav_saw_pw;
    if (out && out_n) {
        if (nav_fail) snprintf(out, out_n, "Error: Secrets were required, but not provided.\n");
        else out[0] = 0;
    }
    return 0;
}

static int vol_calls;
static char vol_cmd[80];
static int vol_hook(const char *cmd) {
    vol_calls++;
    snprintf(vol_cmd, sizeof vol_cmd, "%s", cmd);
    return 0;
}

static int power_calls;
static char power_cmd[64];
static int power_hook(const char *cmd) {
    power_calls++;
    snprintf(power_cmd, sizeof power_cmd, "%s", cmd);
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

    down(6); press(LB_A);              /* Internet */
    expect(ui_screen() == SCREEN_INTERNET, "A on Internet opens the Internet menu");
    press(LB_B);
    expect(ui_screen() == SCREEN_MAIN, "B from Internet returns to Main");

    ui_init(); down(8); press(LB_A);   /* Settings */
    expect(ui_screen() == SCREEN_SETTINGS, "A on Settings opens Settings");
    press(LB_B);
    expect(ui_screen() == SCREEN_MAIN, "B from Settings returns to Main");

    ui_init(); down(9); press(LB_A);   /* Engineering Mode */
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

    /* TF1 + TF2: the same system on both cards is one merged, sorted list; a TF2-only system
       appears; alias folders PS and PSX merge; a missing card is skipped. */
    {
        const char *made[] = { "/tmp/redux-tf1", "/tmp/redux-tf1/GBA", "/tmp/redux-tf1/PS",
                               "/tmp/redux-tf2", "/tmp/redux-tf2/GBA", "/tmp/redux-tf2/N64",
                               "/tmp/redux-tf2/PSX" };
        for (size_t i = 0; i < sizeof made / sizeof made[0]; i++) mkdir(made[i], 0755);
        write_empty("/tmp/redux-tf1/GBA/Metroid Fusion.gba");
        write_empty("/tmp/redux-tf1/GBA/Advance Wars.gba");
        write_empty("/tmp/redux-tf2/GBA/Golden Sun.gba");
        write_empty("/tmp/redux-tf2/N64/mario.z64");
        write_empty("/tmp/redux-tf1/PS/castlevania.chd");
        write_empty("/tmp/redux-tf2/PSX/ff7.chd");
        setenv("REDUX_ROMS", "/tmp/redux-tf1:/tmp/redux-tf2:/tmp/redux-no-such-card", 1);
        ui_init();
        expect(romscan_systems() == 3, "TF1 + TF2 give GBA, N64 and PS1 once each");
        int g = -1, p = -1, n64 = -1;
        for (int s = 0; s < romscan_systems(); s++) {
            if (!strcmp(romscan_key(s), "GBA")) g = s;
            if (!strcmp(romscan_key(s), "PS1")) p = s;
            if (!strcmp(romscan_key(s), "N64")) n64 = s;
        }
        expect(g >= 0 && romscan_roms(g) == 3, "GBA merges both cards");
        expect(g >= 0 && !strcmp(romscan_rom_name(g, 0), "Advance Wars") &&
               !strcmp(romscan_rom_name(g, 1), "Golden Sun") &&
               !strcmp(romscan_rom_name(g, 2), "Metroid Fusion"), "merged list is sorted by name");
        expect(g >= 0 && strstr(romscan_rom_path(g, 1), "/tmp/redux-tf2/GBA/") != NULL,
               "a TF2 ROM keeps its TF2 path");
        expect(n64 >= 0 && romscan_roms(n64) == 1, "TF2-only system is listed");
        expect(p >= 0 && romscan_roms(p) == 2, "PS (TF1) and PSX (TF2) merge into PS1");
        unsetenv("REDUX_ROMS");
    }

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

    /* More than the old 4096 cap across systems: every system and ROM is still listed. */
    {
        const char *sysn[3] = { "GB", "GBC", "NES" };
        mkdir("/tmp/redux-rom-big", 0755);
        for (int s = 0; s < 3; s++) {
            char dir[64];
            snprintf(dir, sizeof dir, "/tmp/redux-rom-big/%s", sysn[s]);
            mkdir(dir, 0755);
            for (int i = 0; i < 1500; i++) {
                char path[128];
                snprintf(path, sizeof path, "%s/Game %04d.rom", dir, i);
                write_empty(path);
            }
        }
        setenv("REDUX_ROMS", "/tmp/redux-rom-big", 1);
        ui_init();
        int total = 0;
        for (int s = 0; s < romscan_systems(); s++) total += romscan_roms(s);
        expect(romscan_systems() == 3, "all three big systems are listed");
        expect(total == 4500, "4500 ROMs are listed, past the old 4096 cap");
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
    down(7); press(LB_A);
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
    expect(ui_screen() == SCREEN_WIFI_RESULT, "Start opens the result screen");
    expect(strcmp(ui_last_busy(), "Connecting") == 0, "connect shows Connecting first");
    expect(nav_saw_pw == 1 && nav_pw_ok == 1 && nav_saw_ifname == 1,
           "connect got the typed password and ifname wlan0");
    {
        char dump[8192];
        ui_draw();
        grid_dump(dump, sizeof dump);
        expect(strstr(dump, "Home") != NULL, "result names the network");
        expect(strstr(dump, "192.168.1.20") != NULL, "result shows the IP");
        expect(strstr(dump, "connected") != NULL, "result says connected");
    }
    press(LB_B);
    expect(ui_screen() == SCREEN_WIFI, "B from the result returns to the list");
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
    expect(ui_screen() == SCREEN_WIFI_RESULT, "empty security connects without the keyboard");
    expect(strcmp(ui_last_busy(), "Connecting") == 0, "open-network connect shows Connecting first");
    expect(nav_pw_ok == 1 && nav_saw_pw == 0, "that connect has no password argument");
    press(LB_B);
    expect(ui_screen() == SCREEN_WIFI, "B from an open-network result returns to the list");
    {
        char dump[8192];
        ui_draw();
        grid_dump(dump, sizeof dump);
        expect(strstr(dump, "Back to stock") == NULL, "Wi-Fi list has no Back to stock row");
    }
    nav_fail = 1;
    press(LB_UP); press(LB_A); press(LB_START);
    expect(ui_screen() == SCREEN_WIFI_RESULT, "a failed connect opens the result");
    {
        char dump[8192];
        ui_draw();
        grid_dump(dump, sizeof dump);
        expect(strstr(dump, "Secrets were required") != NULL, "failure shows nmcli's reason");
        expect(strstr(dump, "A:Retry") != NULL, "failure offers retry");
    }
    nav_fail = 0;
    press(LB_A);
    expect(ui_screen() == SCREEN_WIFI_RESULT, "retry stays on the result screen");
    {
        char dump[8192];
        ui_draw();
        grid_dump(dump, sizeof dump);
        expect(strstr(dump, "192.168.1.20") != NULL, "retry shows the IP after nmcli succeeds");
    }
    press(LB_B);

    ui_init();
    down(8); press(LB_A); down(4); press(LB_A);
    expect(ui_screen() == SCREEN_SYSTEM_DEFAULTS, "Settings has no Network; row 4 is System");
    press(LB_B);
    expect(ui_screen() == SCREEN_SETTINGS, "B from System returns to Settings");

    /* Symbol layer: type 0x20..0x5E (63 bytes, the password cap) and require
       wifi_connect to receive that exact string. Not printed. */
    ui_init();
    down(7); press(LB_A); press(LB_A);
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
        expect(ui_screen() == SCREEN_WIFI_RESULT, "Start sends the symbol password");
        expect(nav_saw_pw == 1 && nav_pw_ok == 1, "wifi_connect got the exact symbol password");
    }

    stock_set_run(stock_hook);
    stock_set_bak(0);
    stock_calls = 0;
    ui_init();
    down(8); press(LB_A); down(4); press(LB_A);
    expect(ui_screen() == SCREEN_SYSTEM_DEFAULTS, "Settings System opens System Defaults");
    {
        char dump[8192];
        ui_draw();
        grid_dump(dump, sizeof dump);
        expect(strstr(dump, "System Tools") != NULL, "System Tools is first");
        expect(strstr(dump, "Stock Menu") != NULL, "Stock Menu is listed");
        expect(strstr(dump, "Boot Screen   Stock") != NULL, "Boot Screen is shown as Stock");
        expect(strstr(dump, "Reset All     not yet") != NULL, "Reset All is still not yet");
        expect(strstr(dump, "Reset All") != NULL, "Reset All is shown");
    }
    /* cursor skips the blanks and Reset All */
    expect(ui_list_selected() == 0, "cursor starts on System Tools");
    press(LB_DOWN); press(LB_DOWN); press(LB_DOWN); press(LB_DOWN); press(LB_DOWN);
    expect(ui_list_selected() == 7, "five downs land on Stock Menu (via Boot Screen and Card Scripts)");
    press(LB_DOWN);
    expect(ui_list_selected() == 0, "down from Stock Menu wraps to System Tools");
    press(LB_UP);
    expect(ui_list_selected() == 7, "up from System Tools wraps to Stock Menu");
    press(LB_A);
    expect(ui_screen() == SCREEN_STOCK_CONFIRM, "Stock Menu accepts A");
    press(LB_B);
    expect(ui_screen() == SCREEN_SYSTEM_DEFAULTS, "B cancels back to System Defaults");
    expect(stock_calls == 0, "cancel does not run S7");
    press(LB_UP);   /* back() leaves the cursor on System Tools; Up wraps to Stock Menu */
    press(LB_A);
    press(LB_A);
    expect(ui_screen() == SCREEN_STOCK_CONFIRM, "a short A stays on the confirm");
    expect(stock_calls == 0, "a short A does not run S7");
    ui_button(LB_A, 1, 1000);
    ui_tick(3900);
    ui_button(LB_A, 0, 3900);
    expect(ui_screen() == SCREEN_STOCK_CONFIRM, "releasing at 2.9 s stays on the confirm");
    expect(stock_calls == 0, "2.9 s does not run S7");
    ui_button(LB_A, 1, 5000);
    ui_tick(6000);
    ui_button(LB_B, 1, 6100);
    expect(ui_screen() == SCREEN_SYSTEM_DEFAULTS, "another button during the hold leaves");
    expect(stock_calls == 0, "cancel during the hold does not run S7");
    ui_button(LB_B, 0, 6200);
    ui_button(LB_A, 0, 6300);
    press(LB_UP);   /* back() leaves the cursor on System Tools; Up wraps to Stock Menu */
    press(LB_A);
    ui_button(LB_A, 1, 8000);
    ui_button(LB_A, 1, 10000);   /* a repeat press must not restart the 3 s clock */
    ui_tick(11000);
    expect(ui_screen() == SCREEN_STOCK_CONFIRM, "a finished hold with no backup stays");
    expect(stock_calls == 0, "missing dmenu_ln.bak does not run S7");
    ui_button(LB_A, 0, 11100);
    stock_set_bak(1);
    press(LB_B);
    press(LB_UP);   /* back() leaves the cursor on System Tools; Up wraps to Stock Menu */
    press(LB_A);
    ui_button(LB_A, 1, 12000);
    ui_tick(15000);
    expect(stock_calls == 1 && stock_cmd_ok, "holding A for 3 s runs the atomic S7 command");
    expect(strcmp(stock_busy, "Restoring stock menu") == 0, "restore shows the busy screen first");

    wifi_set_spawn(NULL);
    stock_set_run(NULL);
    stock_set_bak(-1);

    mkdir("/tmp/redux-sysinfo-nav", 0755);
    mkdir("/tmp/redux-sysinfo-nav/sys", 0755);
    mkdir("/tmp/redux-sysinfo-nav/sys/class", 0755);
    mkdir("/tmp/redux-sysinfo-nav/sys/class/power_supply", 0755);
    mkdir("/tmp/redux-sysinfo-nav/sys/class/power_supply/axp2202-battery", 0755);
    mkdir("/tmp/redux-sysinfo-nav/sys/devices", 0755);
    mkdir("/tmp/redux-sysinfo-nav/sys/devices/system", 0755);
    mkdir("/tmp/redux-sysinfo-nav/sys/devices/system/cpu", 0755);
    mkdir("/tmp/redux-sysinfo-nav/sys/devices/system/cpu/cpu0", 0755);
    mkdir("/tmp/redux-sysinfo-nav/sys/devices/system/cpu/cpu0/cpufreq", 0755);
    {
        FILE *f = fopen("/tmp/redux-sysinfo-nav/sys/class/power_supply/axp2202-battery/temp", "w");
        if (f) { fputs("300\n", f); fclose(f); }
        f = fopen("/tmp/redux-sysinfo-nav/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "w");
        if (f) { fputs("schedutil\n", f); fclose(f); }
    }
    sysinfo_set_root("/tmp/redux-sysinfo-nav");
    ui_init();
    down(8); press(LB_A); down(4); press(LB_A); press(LB_A);
    expect(ui_screen() == SCREEN_SYSINFO, "System Tools opens from System Defaults");
    {
        char dump[8192];
        ui_draw();
        grid_dump(dump, sizeof dump);
        expect(strstr(dump, "unknown (no sensor)") != NULL, "System Tools does not show the dummy battery temp");
        expect(strstr(dump, "schedutil") != NULL, "System Tools shows the CPU governor");
    }
    press(LB_B);
    expect(ui_screen() == SCREEN_SYSTEM_DEFAULTS, "B leaves System Tools");
    sysinfo_set_root(NULL);

    presents = 0;
    ui_set_present(count_present);
    ui_init();
    expect(presents == PRESENT_BUFFERS, "startup scan is presented 3 times");
    expect(strcmp(ui_last_busy(), "Scanning for ROMs") == 0, "startup scan shows Scanning for ROMs");

    /* Colour presets: Settings → UI opens Colors; A selects and saves. */
    remove("/tmp/redux.theme");
    ui_init();
    down(8); press(LB_A);          /* Settings */
    down(6); press(LB_A);          /* UI (last row) */
    expect(ui_screen() == SCREEN_UI_COLORS, "Settings → UI opens Colors");
    down(1); press(LB_A);          /* Amber */
    {
        char dump[8192];
        ui_draw();
        grid_dump(dump, sizeof dump);
        expect(strstr(dump, "Amber") != NULL, "Colors lists Amber");
        expect(strstr(dump, "(current)") != NULL, "selected preset is marked current");
    }
    press(LB_B);
    expect(ui_screen() == SCREEN_SETTINGS, "B from Colors returns to Settings");
    /* reload confirms the save */
    ui_init();
    down(8); press(LB_A); down(6); press(LB_A);
    {
        char dump[8192];
        ui_draw();
        grid_dump(dump, sizeof dump);
        expect(strstr(dump, "Amber  (current)") != NULL, "saved accent reloads as current");
    }
    remove("/tmp/redux.theme");

    /* Display: Settings → Display; Left/Right steps brightness 1-20 and saves; B returns. */
    remove("/tmp/redux.brightness");
    ui_init();
    down(8); press(LB_A);          /* Settings */
    press(LB_A);                   /* Display (row 0) */
    expect(ui_screen() == SCREEN_DISPLAY, "Settings → Display opens Display");
    {
        int start = brightness_get();
        press(LB_RIGHT);
        expect(brightness_get() == start + 1 || start == BRIGHTNESS_MAX, "Right raises brightness");
        for (int i = 0; i < 40; i++) press(LB_LEFT);
        expect(brightness_get() == BRIGHTNESS_MIN, "Left stops at the minimum, never 0");
        press(LB_RIGHT); press(LB_RIGHT);
        char dump[8192];
        ui_draw();
        grid_dump(dump, sizeof dump);
        expect(strstr(dump, " 3/20") != NULL, "Display shows the level out of 20");
        expect(ui_screen() == SCREEN_DISPLAY, "Left/Right stay on Display");
    }
    press(LB_B);
    expect(ui_screen() == SCREEN_SETTINGS, "B from Display returns to Settings");
    ui_init();
    expect(brightness_get() == 3, "saved brightness reloads");
    remove("/tmp/redux.brightness");

    /* Display row 2: Screen Timeout (Off..30 min, default 5), Down selects it, Left/Right change it. */
    remove("/tmp/redux.screen_timeout");
    ui_init();
    down(8); press(LB_A); press(LB_A);             /* Settings > Display */
    {
        int b = brightness_get();
        press(LB_DOWN); press(LB_RIGHT);
        char dump[8192];
        ui_draw();
        grid_dump(dump, sizeof dump);
        expect(strstr(dump, "Screen Timeout  10 min") != NULL, "Down + Right: Screen Timeout 5 -> 10 min");
        expect(brightness_get() == b, "changing the timeout leaves brightness alone");
        for (int i = 0; i < 10; i++) press(LB_LEFT);
        ui_draw();
        grid_dump(dump, sizeof dump);
        expect(strstr(dump, "Screen Timeout  Off") != NULL, "Left stops at Off");
    }
    remove("/tmp/redux.screen_timeout");

    /* Display row 3: Font (docs/ui-spec.md 8a). Right steps VGA -> Spleen, saved, reloaded;
       Left from VGA wraps to Chunky; the preview shows the box characters. */
    remove("/tmp/redux.font");
    ui_init();
    expect(font_current() == FONT_VGA, "no saved font: VGA");
    down(8); press(LB_A); press(LB_A);             /* Settings > Display */
    {
        char dump[8192];
        press(LB_DOWN); press(LB_DOWN); press(LB_RIGHT);
        ui_draw();
        grid_dump(dump, sizeof dump);
        expect(font_current() == FONT_SPLEEN && strstr(dump, "Font  < Spleen >") != NULL,
               "Down Down Right: Font VGA -> Spleen");
        expect(strstr(dump, "\u250c\u2500\u2500\u2500\u2510") != NULL, "Font row shows the box preview");
        expect(screentimeout_minutes() == SCREENTIMEOUT_DEFAULT, "changing the font leaves the timeout alone");
        ui_init();
        expect(font_current() == FONT_SPLEEN, "saved font reloads");
        down(8); press(LB_A); press(LB_A); press(LB_DOWN); press(LB_DOWN);
        press(LB_LEFT); press(LB_LEFT);
        expect(font_current() == FONT_CHUNKY, "Left from VGA wraps to Chunky");
        FILE *f = fopen("/tmp/redux.font", "w");
        if (f) { fputs("comic\n", f); fclose(f); }
        ui_init();
        expect(font_current() == FONT_VGA, "unknown saved font falls back to VGA");
    }
    {
        /* A glyph Unifont lacks is drawn from VGA; one it has is its own. */
        int missing = -1;
        for (int c = 0x21; c < 256 && missing < 0; c++) {
            int blank = 1;
            for (int y = 0; y < 16; y++) if (font_unifont_8x16[c][y]) blank = 0;
            if (blank && c != 0x7F) missing = c;
        }
        const uint8_t *vga_a = font_glyph('A'), *vga_m = missing >= 0 ? font_glyph((unsigned char)missing) : NULL;
        FILE *f = fopen("/tmp/redux.font", "w");
        if (f) { fputs("unifont\n", f); fclose(f); }
        font_load();
        expect(font_current() == FONT_UNIFONT && font_glyph('A') != vga_a, "Unifont draws its own A");
        expect(missing >= 0 && font_glyph((unsigned char)missing) == vga_m, "a glyph Unifont lacks comes from VGA");
        remove("/tmp/redux.font");
        font_load();
    }
    remove("/tmp/redux.font");

    /* Restart / Power Off: one A does nothing; 3 s hold runs the hook once; B cancels. */
    power_set_run(power_hook);
    power_calls = 0;
    ui_init();
    down(8); press(LB_A); down(4); press(LB_A); press(LB_DOWN); press(LB_A);
    expect(ui_screen() == SCREEN_RESTART_CONFIRM, "Restart opens the confirm");
    press(LB_A);
    expect(ui_screen() == SCREEN_RESTART_CONFIRM, "a short A stays on Restart confirm");
    expect(power_calls == 0, "a short A does not run reboot");
    press(LB_B);
    expect(ui_screen() == SCREEN_SYSTEM_DEFAULTS, "B leaves Restart confirm");
    press(LB_DOWN); press(LB_A);
    ui_button(LB_A, 1, 20000);
    ui_tick(22900);
    ui_button(LB_B, 1, 23000);
    expect(ui_screen() == SCREEN_SYSTEM_DEFAULTS, "B during Restart hold cancels");
    expect(power_calls == 0, "cancelled Restart does not run");
    ui_button(LB_B, 0, 23100);
    press(LB_DOWN); press(LB_A);
    ui_button(LB_A, 1, 24000);
    ui_tick(27000);
    expect(ui_screen() == SCREEN_RESTART_CONFIRM, "3 s hold stays on Restart confirm");
    expect(power_calls == 1, "3 s hold runs the hook once");
    expect(strcmp(power_cmd, "sync && reboot") == 0, "Restart runs sync && reboot");

    power_calls = 0;
    ui_init();
    down(8); press(LB_A); down(4); press(LB_A); press(LB_DOWN); press(LB_DOWN); press(LB_A);
    expect(ui_screen() == SCREEN_POWEROFF_CONFIRM, "Power Off opens the confirm");
    press(LB_A);
    expect(ui_screen() == SCREEN_POWEROFF_CONFIRM, "a short A stays on Power Off confirm");
    expect(power_calls == 0, "a short A does not run poweroff");
    press(LB_B);
    expect(ui_screen() == SCREEN_SYSTEM_DEFAULTS, "B leaves Power Off confirm");
    press(LB_DOWN); press(LB_DOWN); press(LB_A);
    ui_button(LB_A, 1, 30000);
    ui_tick(33000);
    expect(power_calls == 1, "3 s hold runs poweroff once");
    expect(strcmp(power_cmd, "sync && poweroff") == 0, "Power Off runs sync && poweroff");

    /* Volume: a press shows VOL nn; it does not leave a confirm screen. */
    volume_set_run(vol_hook);
    volume_set_path("/tmp/redux-volume-nav");
    unlink("/tmp/redux-volume-nav");
    ui_init();
    ui_button(LB_VOL_DOWN, 1, 1000);
    {
        char dump[8192];
        ui_draw();
        grid_dump(dump, sizeof dump);
        expect(strstr(dump, "VOL 30") != NULL, "vol down shows VOL 30");
    }
    expect(strcmp(vol_cmd, "amixer sset 'lineout volume' 30") == 0, "vol down runs amixer");
    ui_tick(2500);
    {
        char dump[8192];
        ui_draw();
        grid_dump(dump, sizeof dump);
        expect(strstr(dump, "VOL ") == NULL, "VOL overlay clears after 1.5 s");
    }
    /* on a confirm screen, volume must not cancel */
    down(8); press(LB_A); down(4); press(LB_A); press(LB_DOWN); press(LB_A);
    expect(ui_screen() == SCREEN_RESTART_CONFIRM, "on Restart confirm");
    ui_button(LB_VOL_UP, 1, 3000);
    expect(ui_screen() == SCREEN_RESTART_CONFIRM, "volume does not leave the confirm");
    unlink("/tmp/redux-volume-nav");

    /* App Center (menu option A). Desktop has no stock binaries until a fixture root is set. */
    launcher_set_next_path("/tmp/redux-next-apps");
    remove("/tmp/redux-next-apps");
    apps_set_root("/tmp/redux-apps-none");
    ui_init();
    down(5); press(LB_A);
    expect(ui_screen() == SCREEN_APPS, "A on Apps opens the App Center");
    {
        char dump[8192];
        ui_draw();
        grid_dump(dump, sizeof dump);
        expect(strstr(dump, "Music Player") && strstr(dump, "not verified") && strstr(dump, "Terminal"),
               "Apps lists stock apps as not verified, and Terminal");
        expect(strstr(dump, "File Manager") == NULL, "File Manager is on Main only, not in Apps");
        expect(apps_row_of(APP_FILES) == -1 && apps_count() == APP_COUNT - 1
               && apps_list_id(apps_row_of(APP_PDF)) == APP_PDF, "Apps rows map to app ids without File Manager");
    }
    press(LB_A);
    expect(ui_should_exit() == 0 && ui_screen() == SCREEN_APPS, "a missing app does not launch");
    {
        char dump[8192];
        ui_draw();
        grid_dump(dump, sizeof dump);
        expect(strstr(dump, "Not installed on this device") != NULL, "missing app says not installed");
    }
    down(apps_row_of(APP_PDF)); press(LB_A);
    expect(ui_should_exit() == 0, "PDF Reader is not launchable yet");
    press(LB_B);
    expect(ui_screen() == SCREEN_MAIN, "B from Apps returns to Main");

    mkdir("/tmp/redux-apps", 0755); mkdir("/tmp/redux-apps/mnt", 0755);
    mkdir("/tmp/redux-apps/mnt/vendor", 0755); mkdir("/tmp/redux-apps/mnt/vendor/bin", 0755);
    mkdir("/tmp/redux-apps/mnt/vendor/bin/music", 0755);
    write_empty("/tmp/redux-apps/mnt/vendor/bin/music/ap.dge");
    apps_set_root("/tmp/redux-apps");
    ui_init();
    down(1); press(LB_A);              /* Music, straight from Main */
    expect(ui_should_exit() == 1, "Music on Main launches the stock music app");
    expect(strcmp(ui_last_busy(), "Loading App") == 0, "app launch shows Loading App");
    {
        char line[256] = "";
        FILE *f = fopen("/tmp/redux-next-apps", "r");
        if (f) { if (!fgets(line, sizeof line, f)) line[0] = '\0'; fclose(f); }
        expect(strcmp(line, "cd '/mnt/vendor/bin/music' && AUDIODEV=hw:2,0 ./'ap.dge'\n") == 0,
               "music launch line cds into its folder");
    }
    remove("/tmp/redux-next-apps");
    remove("/tmp/redux-apps/mnt/vendor/bin/music/ap.dge");

    /* Task Monitor: SimpleTerminal from rootfs with -e htop (/usr/bin/htop; not stock, see apps.c). */
    mkdir("/tmp/redux-apps/mnt/mod", 0755); mkdir("/tmp/redux-apps/mnt/mod/ctrl", 0755);
    write_empty("/tmp/redux-apps/mnt/mod/ctrl/SimpleTerminal");
    ui_init();
    down(5); press(LB_A);              /* Apps */
    down(apps_row_of(APP_TOP)); press(LB_A);
    expect(ui_should_exit() == 1, "Task Monitor launches");
    {
        char line[256] = "";
        FILE *f = fopen("/tmp/redux-next-apps", "r");
        if (f) { if (!fgets(line, sizeof line, f)) line[0] = '\0'; fclose(f); }
        expect(strcmp(line, "cd '/mnt/mod/ctrl' && HOME=/root ./'SimpleTerminal' -e htop\n") == 0,
               "Task Monitor runs SimpleTerminal -e htop from its folder");
    }
    remove("/tmp/redux-next-apps");

    /* After an app from the Apps list, Redux restarts on the Apps list at the same row. */
    ui_init();
    expect(ui_screen() == SCREEN_APPS, "after an Apps launch, Redux restarts on the Apps list");
    expect(ui_list_selected() == apps_row_of(APP_TOP), "the Apps list keeps the launched row");
    {
        FILE *sf = fopen(UI_RESUME_PATH, "r");
        expect(sf == NULL, "apps resume file is removed after a valid read");
        if (sf) fclose(sf);
    }
    press(LB_B);
    expect(ui_screen() == SCREEN_MAIN, "B after an apps resume returns to Main");
    {
        FILE *sf = fopen(UI_RESUME_PATH, "w");
        if (sf) { fputs("apps\n-\n999\n0\n", sf); fclose(sf); }
        ui_init();
        expect(ui_screen() == SCREEN_MAIN, "apps row out of range starts on Main");
    }
    /* Music from Main: no Apps resume, Redux comes back on Main. */
    write_empty("/tmp/redux-apps/mnt/vendor/bin/music/ap.dge");
    ui_init();
    down(1); press(LB_A);
    expect(ui_should_exit() == 1, "Music from Main launches");
    ui_init();
    expect(ui_screen() == SCREEN_MAIN, "a launch from Main comes back to Main");
    remove("/tmp/redux-next-apps");
    remove("/tmp/redux-apps/mnt/vendor/bin/music/ap.dge");

    /* Tools (Core Apps A): each runs a stock program in SimpleTerminal. */
    {
        static const struct { int app; const char *args; } tools[] = {
            { APP_MIXER, "-e alsamixer" }, { APP_NETWORK, "-e nmtui" },
            { APP_BLUETOOTH, "-e bluetoothctl" }, { APP_INPUT_TEST, "-e evtest /dev/input/event1" },
            { APP_SPEAKER_TEST, "-e speaker-test -t wav -c 2 -l 1" },
        };
        for (unsigned t = 0; t < sizeof tools / sizeof tools[0]; t++) {
            ui_init();
            down(5); press(LB_A);              /* Apps */
            down(apps_row_of(tools[t].app)); press(LB_A);
            char line[256] = "", want[256];
            FILE *f = fopen("/tmp/redux-next-apps", "r");
            if (f) { if (!fgets(line, sizeof line, f)) line[0] = '\0'; fclose(f); }
            snprintf(want, sizeof want, "cd '/mnt/mod/ctrl' && HOME=/root ./'SimpleTerminal' %s\n", tools[t].args);
            char label[96];
            snprintf(label, sizeof label, "Tools: %s launches", apps_name(tools[t].app));
            expect(ui_should_exit() == 1 && strcmp(line, want) == 0, label);
            remove("/tmp/redux-next-apps");
            remove(UI_RESUME_PATH);   /* each tool starts from Main */
        }
    }
    remove("/tmp/redux-apps/mnt/mod/ctrl/SimpleTerminal");
    apps_set_root("");

    /* Card Scripts switch (Ports option B): Off by default, 3 s hold to turn on, one press off. */
    cardscripts_set_path("/tmp/redux-nav-cardscripts");
    cardscripts_set(0);
    ui_init();
    down(8); press(LB_A); down(4); press(LB_A);       /* Settings > System */
    down(4);
    expect(ui_list_selected() == 4, "Card Scripts is the row after Boot Screen");
    {
        char dump[8192];
        ui_draw();
        grid_dump(dump, sizeof dump);
        expect(strstr(dump, "Card Scripts  Off") != NULL, "Card Scripts shows Off by default");
    }
    press(LB_A);
    expect(ui_screen() == SCREEN_SCRIPTS_CONFIRM, "A on Card Scripts (Off) opens the hold-to-confirm");
    ui_button(LB_A, 1, 10000); ui_tick(11000); ui_button(LB_A, 0, 11000);
    expect(!cardscripts_enabled(), "a short hold does not turn it on");
    ui_button(LB_A, 1, 20000); ui_tick(23100);
    expect(cardscripts_enabled(), "a 3 s hold turns Card Scripts on");
    ui_button(LB_A, 0, 23200);
    press(LB_B);
    expect(ui_screen() == SCREEN_SYSTEM_DEFAULTS, "B returns to System Defaults");
    {
        char dump[8192];
        ui_draw();
        grid_dump(dump, sizeof dump);
        expect(strstr(dump, "Card Scripts  On") != NULL, "row shows On");
    }
    down(4); press(LB_A);
    expect(!cardscripts_enabled() && ui_screen() == SCREEN_SYSTEM_DEFAULTS, "one press turns it off");
    remove("/tmp/redux-nav-cardscripts");

    /* Boot Screen manager: 3 s hold installs, a second visit restores. Temp folder, no device. */
    {
        static unsigned char img[921654];
        const char *files[2] = { "/tmp/redux-nav-boot/mnt/bootlogo.bmp", "/tmp/redux-nav-boot/redux.bmp" };
        mkdir("/tmp/redux-nav-boot", 0755); mkdir("/tmp/redux-nav-boot/mnt", 0755);
        remove("/tmp/redux-nav-boot/backup.bmp"); remove("/tmp/redux-nav-boot/marker");
        for (int k = 0; k < 2; k++) {
            memset(img, k ? 0x22 : 0x11, sizeof img);
            memset(img, 0, 54);
            img[0] = 'B'; img[1] = 'M';
            img[2] = 0x36; img[3] = 0x10; img[4] = 0x0e;          /* 921654 */
            img[10] = 54; img[14] = 40;
            img[18] = 0x80; img[19] = 0x02;                       /* 640 */
            img[22] = 0xe0; img[23] = 0x01;                       /* 480 */
            img[26] = 1; img[28] = 24;
            FILE *f = fopen(files[k], "wb");
            if (f) { fwrite(img, 1, sizeof img, f); fclose(f); }
        }
        bootscreen_set_paths("/tmp/redux-nav-boot/mnt", files[1], "/tmp/redux-nav-boot/backup.bmp",
                             "/tmp/redux-nav-boot/marker");
        bootscreen_set_run(boot_hook);
        ui_init();
        down(8); press(LB_A); down(4); press(LB_A);       /* Settings > System */
        down(3);
        expect(ui_list_selected() == 3, "Boot Screen is the row after Power Off");
        press(LB_A);
        expect(ui_screen() == SCREEN_BOOT_CONFIRM, "A on Boot Screen opens the hold-to-confirm");
        ui_button(LB_A, 1, 30000); ui_tick(31000); ui_button(LB_A, 0, 31000);
        expect(!bootscreen_is_redux(), "a short hold installs nothing");
        ui_button(LB_A, 1, 40000); ui_tick(43100);
        expect(bootscreen_is_redux(), "a 3 s hold installs the Redux boot screen");
        ui_tick(47000);
        expect(bootscreen_is_redux() && boot_mounts == 1, "holding on does not run it a second time");
        ui_button(LB_A, 0, 47100);
        {
            char dump[8192];
            ui_draw();
            grid_dump(dump, sizeof dump);
            expect(strstr(dump, "Done. Shows at the next power-on.") != NULL, "the result is shown");
        }
        press(LB_B);
        {
            char dump[8192];
            ui_draw();
            grid_dump(dump, sizeof dump);
            expect(strstr(dump, "Boot Screen   Redux") != NULL, "row shows Redux");
        }
        down(3); press(LB_A);
        ui_button(LB_A, 1, 50000); ui_tick(53100); ui_button(LB_A, 0, 53200);
        expect(!bootscreen_is_redux() && boot_mounts == 2, "a second 3 s hold restores stock");
        press(LB_B);
        remove(files[0]); remove(files[1]);
        remove("/tmp/redux-nav-boot/backup.bmp");
    }

    /* M (Menu) returns to Main from anywhere; on a confirm screen it cancels instead. */
    ui_init();
    down(8); press(LB_A); press(LB_A);             /* Settings > Display */
    expect(ui_screen() == SCREEN_DISPLAY, "Settings > Display is open");
    press(LB_MENU);
    expect(ui_screen() == SCREEN_MAIN && ui_list_selected() == 0, "M on Display goes to Main");
    press(LB_B);
    expect(ui_screen() == SCREEN_MAIN, "B on Main after M stays on Main (back stack cleared)");
    down(8); press(LB_A); down(4); press(LB_A); down(5); press(LB_A);   /* Stock Menu confirm */
    expect(ui_screen() == SCREEN_STOCK_CONFIRM, "Stock Menu confirm is open");
    press(LB_MENU);
    expect(ui_screen() == SCREEN_SYSTEM_DEFAULTS, "M on a confirm screen cancels back one level");

    printf("%d failed\n", fails);
    return fails ? 1 : 0;
}
