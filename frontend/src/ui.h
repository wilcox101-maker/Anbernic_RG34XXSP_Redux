#ifndef UI_H
#define UI_H

#include <SDL.h>
#include "remap.h"

void ui_init(void);
void ui_shutdown(void);
void ui_handle(logical_button_t btn);
void ui_render(SDL_Renderer *ren);

#endif
