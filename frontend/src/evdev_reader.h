#ifndef REDUX_EVDEV_READER_H
#define REDUX_EVDEV_READER_H

#include "remap.h"

/* Open /dev/input/event1. Returns 0 on success. */
int evdev_open(void);

/* Close the device. */
void evdev_close(void);

/* Sleep until a button or the power key has input, or timeout_ms passes. Returns >0 if input. */
int evdev_wait(int timeout_ms);

/* Poll for events. Calls cb(btn, pressed) for each logical press or release.
   Sticks use edge detection with hysteresis. */
int evdev_poll(void (*cb)(logical_button_t btn, int pressed));

#endif
