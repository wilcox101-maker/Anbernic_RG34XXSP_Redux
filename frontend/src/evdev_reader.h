#ifndef REDUX_EVDEV_READER_H
#define REDUX_EVDEV_READER_H

#include "remap.h"

/* Open /dev/input/event1. Returns 0 on success. */
int evdev_open(void);

/* Close the device. */
void evdev_close(void);

/* Poll for events. Calls cb(btn, pressed) for each logical press or release.
   Sticks use edge detection with hysteresis. */
int evdev_poll(void (*cb)(logical_button_t btn, int pressed));

#endif
