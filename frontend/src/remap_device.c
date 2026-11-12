#include "remap_device.h"

logical_button_t remap_device_key(int code) {
    switch (code) {
    case 304: return LB_A;      /* BTN_SOUTH */
    case 305: return LB_B;      /* BTN_EAST */
    case 306: return LB_Y;      /* BTN_C = Y (device, 2026-11-12) */
    case 307: return LB_X;      /* BTN_NORTH = X (device, 2026-11-12) */
    case 308: return LB_L1;     /* BTN_WEST = L1 */
    case 309: return LB_R1;     /* BTN_Z = R1 */
    /* Dale's evtest, 2026-11-12 (Select, Start, L2, R2 in order): the device follows the DT,
       not the 2026-11-10 table. Select and Start send BTN_TL/BTN_TR; L2 and R2 send
       BTN_SELECT/BTN_START. */
    case 310: return LB_SELECT; /* BTN_TL = Select */
    case 311: return LB_START;  /* BTN_TR = Start */
    case 312: return LB_MENU;   /* BTN_TL2 = Menu (also sends KEY_GOTO 354, ignored) */
    case 314: return LB_NONE;   /* BTN_SELECT = L2 */
    case 315: return LB_NONE;   /* BTN_START = R2 */
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
