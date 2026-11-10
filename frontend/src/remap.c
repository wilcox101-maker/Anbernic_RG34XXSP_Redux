#include "remap.h"
#include <SDL.h>

logical_button_t remap_key(int key) {
    switch (key) {
    case SDLK_UP:    case SDLK_w: return LB_UP;
    case SDLK_DOWN:  case SDLK_s: return LB_DOWN;
    case SDLK_LEFT:  case SDLK_a: return LB_LEFT;
    case SDLK_RIGHT: case SDLK_d: return LB_RIGHT;
    case SDLK_RETURN: case SDLK_z: return LB_A;
    case SDLK_ESCAPE: case SDLK_x: return LB_B;
    case SDLK_q: case SDLK_PAGEUP:   return LB_L1;
    case SDLK_e: case SDLK_PAGEDOWN: return LB_R1;
    case SDLK_c: return LB_X;
    case SDLK_v: return LB_Y;
    case SDLK_m: return LB_MENU;
    case SDLK_SPACE: return LB_START;
    default: return LB_NONE;
    }
}
