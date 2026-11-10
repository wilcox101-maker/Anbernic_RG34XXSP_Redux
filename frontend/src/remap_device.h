#ifndef REDUX_REMAP_DEVICE_H
#define REDUX_REMAP_DEVICE_H

#include "remap.h"

/* Device remap for the RG34XXSP. Codes from Dale's evtest on ANBERNIC-keys (event1).
   Both sticks and the D-pad produce the same logical buttons. */

/* Absolute axis codes. */
#define DEV_ABS_LEFT_X   2   /* ABS_Z */
#define DEV_ABS_LEFT_Y   3   /* ABS_RX */
#define DEV_ABS_RIGHT_X  4   /* ABS_RY */
#define DEV_ABS_RIGHT_Y  5   /* ABS_RZ */
#define DEV_ABS_HAT0X   16
#define DEV_ABS_HAT0Y   17

/* Stick range is -4096..+4096. Threshold for digital conversion. */
#define DEV_STICK_THRESHOLD 1500

/* Map a raw device key code to a logical button. */
logical_button_t remap_device_key(int code);

/* Map a stick/hat axis value to a logical button (or BTN_NONE if inside deadzone). */
logical_button_t remap_device_axis(int code, int value);

#endif
