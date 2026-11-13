#define _POSIX_C_SOURCE 200809L
#include "brightness.h"
#include <stdio.h>
#include <unistd.h>

static int checks, fails;
static void check(int cond, const char *msg) {
    checks++;
    if (!cond) { printf("FAIL: %s\n", msg); fails++; }
}

static int calls;
static int last_written;
static int hook(int level) { calls++; last_written = level; return 0; }

int main(void) {
    brightness_set_write(hook);
    brightness_set_path("/tmp/redux-brightness-test");
    unlink("/tmp/redux-brightness-test");

    check(brightness_get() == BRIGHTNESS_DEFAULT, "default is 5");
    brightness_step(+1);
    check(brightness_get() == 6, "step up from default");
    check(calls == 1, "step applies");
    check(last_written == 6, "wrote 6");

    for (int i = 0; i < 30; i++) brightness_step(+1);
    check(brightness_get() == BRIGHTNESS_MAX, "step up clamps at 20");
    check(last_written == BRIGHTNESS_MAX, "max write is 20");

    for (int i = 0; i < 40; i++) brightness_step(-1);
    check(brightness_get() == BRIGHTNESS_MIN, "step down clamps at 1");
    check(last_written == BRIGHTNESS_MIN, "min write is 1, never 0");

    brightness_step(+1);
    check(brightness_get() == 2, "step up from 1 is 2");

    /* reload from the saved file */
    brightness_set_write(NULL);
    brightness_load();
    brightness_set_write(hook);
    check(brightness_get() == 2, "load reads the saved level");
    brightness_apply();
    check(brightness_last_written() == 2, "apply uses the loaded level");

    unlink("/tmp/redux-brightness-test");
    printf("%d checks, %d failed\n", checks, fails);
    return fails ? 1 : 0;
}
