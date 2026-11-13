/* Screen timeout choices, save/load and the idle check. No device. Exit 0 = pass. */
#include <stdio.h>
#include "screentimeout.h"

static int fails;
static void expect(int ok, const char *what) { printf("%s %s\n", ok ? "ok  " : "FAIL", what); if (!ok) fails++; }

int main(void) {
    const char *p = "test_screentimeout.txt";
    remove(p);
    screentimeout_set_path(p);
    screentimeout_load();
    expect(screentimeout_minutes() == SCREENTIMEOUT_DEFAULT, "no file: default 5 min");
    expect(!screentimeout_due(5 * 60000u - 1), "not due just before 5 min");
    expect(screentimeout_due(5 * 60000u), "due at 5 min");
    expect(screentimeout_step(+1) == 10, "Right: 5 -> 10");
    for (int i = 0; i < 20; i++) screentimeout_step(+1);
    expect(screentimeout_minutes() == 30, "Right stops at 30");
    for (int i = 0; i < 20; i++) screentimeout_step(-1);
    expect(screentimeout_minutes() == 0, "Left stops at Off");
    expect(!screentimeout_due(0xFFFFFFFFu), "Off is never due");
    screentimeout_step(+1); screentimeout_step(+1);
    screentimeout_load();
    expect(screentimeout_minutes() == 2, "saved 2 min reloads");
    FILE *f = fopen(p, "w"); fputs("7\n", f); fclose(f);
    screentimeout_load();
    expect(screentimeout_minutes() == SCREENTIMEOUT_DEFAULT, "unknown value keeps the default");
    remove(p);
    printf("%d failed\n", fails);
    return fails ? 1 : 0;
}
