#include "remap_device.h"
#include <stdio.h>

static int checks;
static int fails;

static void check(int cond, const char *msg) {
    checks++;
    if (!cond) { printf("FAIL: %s\n", msg); fails++; }
}

int main(void) {
    /* Key mapping */
    check(remap_device_key(304) == BTN_A, "BTN_SOUTH -> A");
    check(remap_device_key(312) == BTN_MENU, "BTN_TL2 -> Menu");
    check(remap_device_key(999) == BTN_NONE, "unknown key");

    /* Axis thresholds (press at 1500, release below) */
    check(remap_device_axis(DEV_ABS_LEFT_X, 2000) == BTN_RIGHT, "left X right");
    check(remap_device_axis(DEV_ABS_LEFT_X, -2000) == BTN_LEFT, "left X left");
    check(remap_device_axis(DEV_ABS_LEFT_X, 1000) == BTN_NONE, "inside deadzone");
    check(remap_device_axis(DEV_ABS_HAT0Y, -1) == BTN_UP, "hat up");
    check(remap_device_axis(DEV_ABS_HAT0X, 1) == BTN_RIGHT, "hat right");
    check(remap_device_axis(DEV_ABS_HAT0X, 0) == BTN_NONE, "hat centred");
    check(remap_device_axis(DEV_ABS_RIGHT_Y, 2000) == BTN_DOWN, "right Y down");

    printf("%d checks, %d failed\n", checks, fails);
    return fails ? 1 : 0;
}
