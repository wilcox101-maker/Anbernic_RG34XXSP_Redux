#include <SDL.h>
#include <stdio.h>
#include "ui.h"
#include "remap.h"
static void shot(const char *name){
    SDL_Surface *s=SDL_CreateRGBSurfaceWithFormat(0,720,480,32,SDL_PIXELFORMAT_RGBA32);
    SDL_Renderer *r=SDL_CreateSoftwareRenderer(s);
    Uint64 t0=SDL_GetPerformanceCounter();
    for(int i=0;i<20;i++) ui_render(r);
    double ms=(SDL_GetPerformanceCounter()-t0)*1000.0/SDL_GetPerformanceFrequency()/20;
    SDL_SaveBMP(s,name); printf("%s  %.2f ms/frame (software renderer, x86 desktop)\n",name,ms);
    SDL_DestroyRenderer(r); SDL_FreeSurface(s);
}
int main(int c,char**v){(void)c;(void)v;
    SDL_Init(0); ui_init();
    shot("1-main.bmp");
    ui_handle(BTN_A); shot("2-roms.bmp"); ui_handle(BTN_B);
    for(int i=0;i<7;i++) ui_handle(BTN_DOWN); ui_handle(BTN_A); shot("3-settings.bmp"); ui_handle(BTN_B);
    for(int i=0;i<8;i++) ui_handle(BTN_DOWN); ui_handle(BTN_A); shot("4-cpu.bmp"); ui_handle(BTN_B);
    ui_handle(BTN_MENU); shot("5-battery.bmp");
    return 0;}
