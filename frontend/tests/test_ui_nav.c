/* Navigation test: B goes back one level, Engineering tabs wrap, B from Engineering exits to Main.
   Also the ROM scanner: systems list, launch, and scrolling a long list.
   Exit code 0 = pass. Uses the desktop platform layer (no window is opened). */
#define _POSIX_C_SOURCE 200809L
#include "ui.h"
#include "launcher.h"
#include "romscan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int fails;
static void expect(int cond, const char *what) { printf("%s %s\n", cond ? "ok  " : "FAIL", what); if (!cond) fails++; }
static void press(logical_button_t b) { ui_button(b, 1, 1); ui_button(b, 0, 2); }
static void down(int n) { while (n--) press(LB_DOWN); }

static void write_empty(const char *path) {
    FILE *f = fopen(path, "w");
    if (f) fclose(f);
}

int main(void) {
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
    FILE *nf = fopen("/tmp/redux-next-test", "r");
    char line[512] = "";
    if (nf) { if (!fgets(line, sizeof line, nf)) line[0] = 0; fclose(nf); }
    expect(strstr(line, "mgba_libretro.so") != NULL, "GBA launches with mgba");
    expect(strstr(line, "Advance Wars.gba") != NULL, "launch line names the selected ROM");

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
    down(30);
    expect(ui_list_selected() == 30, "Down moves the selection to row 30");
    expect(ui_list_scroll() == 30 - ROM_LIST_WINDOW + 1, "the window scrolls to keep row 30 visible");
    press(LB_UP);
    expect(ui_screen() == SCREEN_ROMS, "Up on a ROM list does not leave the screen");

    /* Folder name PS is not a table key; the alias launches the PCSX entry. */
    mkdir("/tmp/redux-rom-alias", 0755);
    mkdir("/tmp/redux-rom-alias/PS", 0755);
    write_empty("/tmp/redux-rom-alias/PS/castlevania.chd");
    setenv("REDUX_ROMS", "/tmp/redux-rom-alias", 1);
    ui_init();
    expect(romscan_systems() == 1 && strcmp(romscan_key(0), "PCSX") == 0, "PS folder maps to the PCSX entry");
    launcher_set_next_path("/tmp/redux-next-ps");
    press(LB_A); press(LB_A); press(LB_A);
    nf = fopen("/tmp/redux-next-ps", "r");
    line[0] = 0;
    if (nf) { if (!fgets(line, sizeof line, nf)) line[0] = 0; fclose(nf); }
    expect(strstr(line, "pcsx") != NULL && strstr(line, "castlevania.chd") != NULL,
           "PS ROM launches the standalone pcsx line");

    printf("%d failed\n", fails);
    return fails ? 1 : 0;
}
