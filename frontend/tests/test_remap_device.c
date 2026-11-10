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
    check(remap_device_key(304) == LB_A, "BTN_SOUTH -> A");
    check(remap_device_key(312) == LB_MENU, "BTN_TL2 -> Menu");
    check(remap_device_key(999) == LB_NONE, "unknown key");

    /* Axis thresholds (press at 1500, release below) */
    check(remap_device_axis(DEV_ABS_LEFT_X, 2000) == LB_RIGHT, "left X right");
    check(remap_device_axis(DEV_ABS_LEFT_X, -2000) == LB_LEFT, "left X left");
    check(remap_device_axis(DEV_ABS_LEFT_X, 1000) == LB_NONE, "inside deadzone");
    check(remap_device_axis(DEV_ABS_HAT0Y, -1) == LB_UP, "hat up");
    check(remap_device_axis(DEV_ABS_HAT0X, 1) == LB_RIGHT, "hat right");
    check(remap_device_axis(DEV_ABS_HAT0X, 0) == LB_NONE, "hat centred");
    check(remap_device_axis(DEV_ABS_RIGHT_Y, 2000) == LB_DOWN, "right Y down");

    printf("%d checks, %d failed\n", checks, fails);
    return fails ? 1 : 0;
}
