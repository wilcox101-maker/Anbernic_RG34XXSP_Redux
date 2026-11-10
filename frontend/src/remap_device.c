#include "remap_device.h"

logical_button_t remap_device_key(int code) {
    switch (code) {
    case 304: return LB_A;      /* BTN_SOUTH */
    case 305: return LB_B;      /* BTN_EAST */
    case 306: return LB_X;      /* BTN_C */
    case 307: return LB_Y;      /* BTN_NORTH */
    case 308: return LB_NONE;   /* BTN_WEST = L1 */
    case 309: return LB_NONE;   /* BTN_Z = R1 */
    case 310: return LB_NONE;   /* BTN_TL = L2 */
    case 311: return LB_NONE;   /* BTN_TR = R2 */
    case 312: return LB_MENU;   /* BTN_TL2 = Menu */
    case 314: return LB_SELECT; /* BTN_SELECT */
    case 315: return LB_START;  /* BTN_START */
    default:  return LB_NONE;
    }
}

logical_button_t remap_device_axis(int code, int value) {
    int thr = DEV_STICK_THRESHOLD;
    switch (code) {
    /* The D-pad hat reports -1/0/+1, so it has no threshold. */
    case DEV_ABS_HAT0X:
        return value < 0 ? LB_LEFT : value > 0 ? LB_RIGHT : LB_NONE;
    case DEV_ABS_HAT0Y:
        return value < 0 ? LB_UP : value > 0 ? LB_DOWN : LB_NONE;
    case DEV_ABS_LEFT_X:
    case DEV_ABS_RIGHT_X:
        if (value < -thr) return LB_LEFT;
        if (value >  thr) return LB_RIGHT;
        return LB_NONE;
    case DEV_ABS_LEFT_Y:
    case DEV_ABS_RIGHT_Y:
        if (value < -thr) return LB_UP;
        if (value >  thr) return LB_DOWN;
        return LB_NONE;
    default:
        return LB_NONE;
    }
}
