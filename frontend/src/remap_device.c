#include "remap_device.h"

logical_button_t remap_device_key(int code) {
    switch (code) {
    case 304: return BTN_A;      /* BTN_SOUTH */
    case 305: return BTN_B;      /* BTN_EAST */
    case 306: return BTN_X;      /* BTN_C */
    case 307: return BTN_Y;      /* BTN_NORTH */
    case 308: return BTN_NONE;   /* BTN_WEST = L1 */
    case 309: return BTN_NONE;   /* BTN_Z = R1 */
    case 310: return BTN_NONE;   /* BTN_TL = L2 */
    case 311: return BTN_NONE;   /* BTN_TR = R2 */
    case 312: return BTN_MENU;   /* BTN_TL2 = Menu */
    case 314: return BTN_SELECT; /* BTN_SELECT */
    case 315: return BTN_START;  /* BTN_START */
    default:  return BTN_NONE;
    }
}

logical_button_t remap_device_axis(int code, int value) {
    int thr = DEV_STICK_THRESHOLD;
    switch (code) {
    case DEV_ABS_LEFT_X:
    case DEV_ABS_RIGHT_X:
    case DEV_ABS_HAT0X:
        if (value < -thr) return BTN_LEFT;
        if (value >  thr) return BTN_RIGHT;
        return BTN_NONE;
    case DEV_ABS_LEFT_Y:
    case DEV_ABS_RIGHT_Y:
    case DEV_ABS_HAT0Y:
        if (value < -thr) return BTN_UP;
        if (value >  thr) return BTN_DOWN;
        return BTN_NONE;
    default:
        return BTN_NONE;
    }
}
