/* Navigation test: B goes back one level, Engineering tabs wrap, B from Engineering exits to Main.
   Exit code 0 = pass. Uses the desktop platform layer (no window is opened). */
#include "ui.h"
#include <stdio.h>

static int fails;
static void expect(int cond, const char *what) { printf("%s %s\n", cond ? "ok  " : "FAIL", what); if (!cond) fails++; }
static void press(logical_button_t b) { ui_button(b, 1, 1); ui_button(b, 0, 2); }
static void down(int n) { while (n--) press(LB_DOWN); }

int main(void) {
    ui_init();
    expect(ui_screen() == SCREEN_MAIN, "starts on Main");
    press(LB_A);                       /* Game Room */
    expect(ui_screen() == SCREEN_ROMS, "A on Game Room opens the ROM list");
    press(LB_B);
    expect(ui_screen() == SCREEN_MAIN, "B from the ROM list returns to Main");

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

    printf("%d failed\n", fails);
    return fails ? 1 : 0;
}
