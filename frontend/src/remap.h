#ifndef REDUX_REMAP_H
#define REDUX_REMAP_H

/* Logical buttons. UI code only ever sees these. */
typedef enum {
    LB_NONE = 0,
    LB_UP,
    LB_DOWN,
    LB_LEFT,
    LB_RIGHT,
    LB_A,
    LB_B,
    LB_X,
    LB_Y,
    LB_L1,
    LB_R1,
    LB_MENU,
    LB_START,
    LB_SELECT,
    LB_VOL_UP,
    LB_VOL_DOWN
} logical_button_t;

/* Map a raw SDL keycode to a logical button. Returns LB_NONE if unmapped. */
logical_button_t remap_key(int sdl_keycode);

#endif
