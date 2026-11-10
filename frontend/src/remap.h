#ifndef REDUX_REMAP_H
#define REDUX_REMAP_H

/* Logical buttons. UI code only ever sees these. */
typedef enum {
    BTN_NONE = 0,
    BTN_UP,
    BTN_DOWN,
    BTN_LEFT,
    BTN_RIGHT,
    BTN_A,
    BTN_B,
    BTN_X,
    BTN_Y,
    BTN_MENU,
    BTN_START,
    BTN_SELECT
} logical_button_t;

/* Map a raw SDL keycode to a logical button. Returns BTN_NONE if unmapped. */
logical_button_t remap_key(int sdl_keycode);

#endif
