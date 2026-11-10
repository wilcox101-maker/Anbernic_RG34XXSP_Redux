#include "remap.h"
#include <SDL.h>

logical_button_t remap_key(int key) {
    switch (key) {
    case SDLK_UP:    case SDLK_w: return BTN_UP;
    case SDLK_DOWN:  case SDLK_s: return BTN_DOWN;
    case SDLK_LEFT:  case SDLK_a: return BTN_LEFT;
    case SDLK_RIGHT: case SDLK_d: return BTN_RIGHT;
    case SDLK_RETURN: case SDLK_z: return BTN_A;
    case SDLK_ESCAPE: case SDLK_x: return BTN_B;
    case SDLK_c: return BTN_X;
    case SDLK_v: return BTN_Y;
    case SDLK_m: return BTN_MENU;
    case SDLK_SPACE: return BTN_START;
    default: return BTN_NONE;
    }
}
