/* Read device buttons and the power key.
   event1 (ANBERNIC-keys): sticks and buttons with edge detection.
   event0 (axp2202-pek): power key. On press, run stock pwr_new.sh. */
#include "evdev_reader.h"
#include "remap_device.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <time.h>
#include <linux/input.h>

/* <linux/input.h> defines BTN_LEFT/BTN_RIGHT (mouse, 0x110/0x111), BTN_A, BTN_START... as macros.
   Our logical buttons were once named BTN_* too, so here BTN_LEFT silently became 0x110 and Left/Right
   never reached the UI on the device (2026-11-10). They are LB_* now; keep it that way. */
_Static_assert(LB_LEFT < 64 && LB_RIGHT < 64 && LB_A < 64, "logical buttons clash with linux/input.h");

#define KEYS_PATH  "/dev/input/event1"
#define POWER_PATH "/dev/input/event0"
#define PRESS_THR  1500
#define RELEASE_THR 1000

static int keys_fd = -1;
static int power_fd = -1;
static int stick_state[6];   /* 0-3 sticks, 4-5 D-pad hat (own state, so stick and hat don't cancel) */

static logical_button_t axis_to_btn(int axis, int dir) {
    if (dir == 0) return LB_NONE;
    if (axis == 0 || axis == 2 || axis == 4) return dir < 0 ? LB_LEFT : LB_RIGHT;
    if (axis == 1 || axis == 3 || axis == 5) return dir < 0 ? LB_UP : LB_DOWN;
    return LB_NONE;
}

int evdev_open(void) {
    keys_fd = open(KEYS_PATH, O_RDONLY | O_NONBLOCK);
    power_fd = open(POWER_PATH, O_RDONLY | O_NONBLOCK);
    return (keys_fd >= 0) ? 0 : -1;
}

void evdev_close(void) {
    if (keys_fd >= 0) close(keys_fd);
    if (power_fd >= 0) close(power_fd);
    keys_fd = power_fd = -1;
}

/* On power key press, run stock pwr_new.sh. It decides sleep vs shutdown. */
#define POWER_QUIET_MS 1500   /* ignore the power key this long after pwr_new.sh returns */

static long long now_ms(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (long long)t.tv_sec * 1000 + t.tv_nsec / 1000000;
}

/* Throw away whatever queued up on a device (e.g. key events from before suspend and from the wake). */
static void drain(int fd) {
    struct input_event ev;
    while (fd >= 0 && read(fd, &ev, sizeof ev) == sizeof ev) { }
}

static long long power_quiet_until;

static void handle_power(void) {
    /* pwr_new.sh is a bash script (function, {0..2}); sh (dash) fails with a syntax error. */
    if (system("/bin/bash /mnt/mod/ctrl/pwr_new.sh") != 0)
        fprintf(stderr, "pwr_new.sh returned an error\n");
    /* pwr_new.sh returns after suspend+resume. The PMIC wakes on both key edges, so a held key
       woke the device at once and its queued press put it back to sleep: a sleep/wake loop
       while held (Dale's video, 2026-11-10). Drop the queued events and stay deaf briefly. */
    drain(power_fd);
    drain(keys_fd);
    power_quiet_until = now_ms() + POWER_QUIET_MS;
}

int evdev_wait(int timeout_ms) {
    struct pollfd p[2];
    int n = 0;
    if (keys_fd >= 0)  { p[n].fd = keys_fd;  p[n].events = POLLIN; n++; }
    if (power_fd >= 0) { p[n].fd = power_fd; p[n].events = POLLIN; n++; }
    if (n == 0) { usleep((useconds_t)timeout_ms * 1000); return 0; }
    return poll(p, (nfds_t)n, timeout_ms);
}

int evdev_poll(void (*cb)(logical_button_t, int pressed)) {
    if (keys_fd < 0) return -1;
    struct input_event ev;

    /* Power key (event0). */
    if (power_fd >= 0) {
        while (read(power_fd, &ev, sizeof ev) == sizeof ev) {
            if (ev.type == EV_KEY && ev.value == 1 && now_ms() >= power_quiet_until)
                handle_power();
        }
    }

    /* Buttons and sticks (event1). */
    while (read(keys_fd, &ev, sizeof ev) == sizeof ev) {
        if (ev.type == EV_KEY) {
            /* 0 release, 1 press, 2 autorepeat. Repeats would restart the hold-A timer. */
            if (ev.value == 2) continue;
            logical_button_t b = remap_device_key(ev.code);
            if (b != LB_NONE && cb) cb(b, ev.value);
        } else if (ev.type == EV_ABS) {
            int axis = -1;
            if (ev.code == DEV_ABS_LEFT_X) axis = 0;
            else if (ev.code == DEV_ABS_LEFT_Y) axis = 1;
            else if (ev.code == DEV_ABS_RIGHT_X) axis = 2;
            else if (ev.code == DEV_ABS_RIGHT_Y) axis = 3;
            else if (ev.code == DEV_ABS_HAT0X) axis = 4;
            else if (ev.code == DEV_ABS_HAT0Y) axis = 5;
            if (axis < 0) continue;

            int dir = 0;
            int v = ev.value;
            if (ev.code == DEV_ABS_HAT0X || ev.code == DEV_ABS_HAT0Y)
                dir = v < 0 ? -1 : v > 0 ? 1 : 0;
            else if (v <= -PRESS_THR) dir = -1;
            else if (v >= PRESS_THR) dir = 1;
            else if (v > -RELEASE_THR && v < RELEASE_THR) dir = 0;
            else dir = stick_state[axis];

            if (dir != stick_state[axis]) {
                if (stick_state[axis] != 0 && cb)
                    cb(axis_to_btn(axis, stick_state[axis]), 0);
                if (dir != 0 && cb)
                    cb(axis_to_btn(axis, dir), 1);
                stick_state[axis] = dir;
            }
        }
    }
    return 0;
}
