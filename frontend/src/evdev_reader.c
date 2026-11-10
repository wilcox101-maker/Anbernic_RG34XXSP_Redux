/* Read /dev/input/event1 (ANBERNIC-keys) and turn events into logical buttons.
   Sticks and the D-pad use edge detection: press when crossing the threshold,
   release when coming back inside the hysteresis band. */
#include "evdev_reader.h"
#include "remap_device.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>

#define DEV_PATH "/dev/input/event1"
#define PRESS_THR  1500
#define RELEASE_THR 1000

static int fd = -1;
static int stick_state[4];   /* left X/Y, right X/Y: -1, 0, +1 */

static logical_button_t axis_to_btn(int axis, int dir) {
    if (dir == 0) return BTN_NONE;
    if (axis == 0 || axis == 2) return dir < 0 ? BTN_LEFT : BTN_RIGHT;
    if (axis == 1 || axis == 3) return dir < 0 ? BTN_UP : BTN_DOWN;
    return BTN_NONE;
}

int evdev_open(void) {
    fd = open(DEV_PATH, O_RDONLY | O_NONBLOCK);
    return fd >= 0 ? 0 : -1;
}

void evdev_close(void) {
    if (fd >= 0) close(fd);
    fd = -1;
}

/* Call repeatedly. Invokes cb for each logical press or release. */
int evdev_poll(void (*cb)(logical_button_t, int pressed)) {
    if (fd < 0) return -1;
    struct input_event ev;
    while (read(fd, &ev, sizeof ev) == sizeof ev) {
        if (ev.type == EV_KEY) {
            logical_button_t b = remap_device_key(ev.code);
            if (b != BTN_NONE && cb) cb(b, ev.value);
        } else if (ev.type == EV_ABS) {
            int axis = -1;
            if (ev.code == DEV_ABS_LEFT_X) axis = 0;
            else if (ev.code == DEV_ABS_LEFT_Y) axis = 1;
            else if (ev.code == DEV_ABS_RIGHT_X) axis = 2;
            else if (ev.code == DEV_ABS_RIGHT_Y) axis = 3;
            else if (ev.code == DEV_ABS_HAT0X) axis = 0;
            else if (ev.code == DEV_ABS_HAT0Y) axis = 1;
            if (axis < 0) continue;

            int dir = 0;
            int v = ev.value;
            if (ev.code == DEV_ABS_HAT0X || ev.code == DEV_ABS_HAT0Y)
                dir = v;   /* -1 / 0 / +1 */
            else if (v <= -PRESS_THR) dir = -1;
            else if (v >= PRESS_THR) dir = 1;
            else if (v > -RELEASE_THR && v < RELEASE_THR) dir = 0;
            else dir = stick_state[axis];   /* hysteresis */

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
