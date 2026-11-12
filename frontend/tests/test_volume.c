#define _POSIX_C_SOURCE 200809L
#include "volume.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int checks, fails;
static void check(int cond, const char *msg) {
    checks++;
    if (!cond) { printf("FAIL: %s\n", msg); fails++; }
}

static int calls;
static char last[80];
static int hook(const char *cmd) { calls++; snprintf(last, sizeof last, "%s", cmd); return 0; }

int main(void) {
    volume_set_run(hook);
    volume_set_path("/tmp/redux-volume-test");
    unlink("/tmp/redux-volume-test");

    check(volume_get() == 31, "default is 31");
    volume_step(+1);
    check(volume_get() == 31, "step up clamps at 31");
    check(calls == 1, "clamp still applies");
    check(strcmp(last, "amixer sset 'lineout volume' 31") == 0, "command is lineout volume");

    for (int i = 0; i < 40; i++) volume_step(-1);
    check(volume_get() == 0, "step down clamps at 0");
    check(strcmp(last, "amixer sset 'lineout volume' 0") == 0, "bottom command is 0");

    volume_step(+1);
    check(volume_get() == 1, "step up from 0 is 1");

    /* reload from the saved file */
    volume_set_run(NULL);
    volume_load();
    volume_set_run(hook);
    check(volume_get() == 1, "load reads the saved level");
    volume_apply();
    check(strcmp(last, "amixer sset 'lineout volume' 1") == 0, "apply uses the loaded level");

    unlink("/tmp/redux-volume-test");
    printf("%d checks, %d failed\n", checks, fails);
    return fails ? 1 : 0;
}
