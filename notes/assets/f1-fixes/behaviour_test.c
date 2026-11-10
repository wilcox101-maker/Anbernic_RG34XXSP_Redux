#include <SDL.h>
#include <stdio.h>
#include <assert.h>
#include "ui.h"
#include "grid.h"
#include "platform.h"
static int scr(void){ char b[9000]; ui_draw(); grid_dump(b,sizeof b);
  if(strstr(b,"[ENGINEERING] CPU")) return SCREEN_CPU; if(strstr(b,"Hold A for 2")) return SCREEN_ENG_CONFIRM;
  if(strstr(b,"RG34XXSP Redux")) return SCREEN_MAIN; return -1;}
static void shot(const char*n){ SDL_Surface*s=SDL_CreateRGBSurfaceWithFormat(0,720,480,32,SDL_PIXELFORMAT_RGBA32);
  SDL_Renderer*r=SDL_CreateSoftwareRenderer(s); Uint64 t0=SDL_GetPerformanceCounter();
  for(int i=0;i<50;i++){ui_draw();grid_present(r);} double ms=(SDL_GetPerformanceCounter()-t0)*1000.0/SDL_GetPerformanceFrequency()/50;
  SDL_SaveBMP(s,n); printf("%-16s %.2f ms/frame\n",n,ms); SDL_DestroyRenderer(r); SDL_FreeSurface(s);}
int main(void){ SDL_Init(0); platform_set_fixed_clock("12:34"); ui_init();
  for(int i=0;i<8;i++) ui_button(BTN_DOWN,1,0);
  ui_button(BTN_A,1,100); ui_button(BTN_A,0,150); assert(scr()==SCREEN_ENG_CONFIRM); /* selecting doesn't enter */
  ui_button(BTN_A,1,1000); ui_tick(2500); assert(scr()==SCREEN_ENG_CONFIRM);          /* 1.5 s: not yet */
  assert(ui_wait_ms()==50);
  ui_button(BTN_A,0,2600); ui_tick(4000); assert(scr()==SCREEN_ENG_CONFIRM);          /* released: cancelled */
  assert(ui_wait_ms()==1000);
  ui_button(BTN_A,1,5000); ui_tick(6000); shot("6-eng-confirm.bmp"); ui_tick(7001); assert(scr()==SCREEN_CPU);
  ui_button(BTN_B,1,8000); assert(scr()==SCREEN_MAIN);
  puts("hold-A confirm: ok (select alone, early release, 2 s hold)");
  ui_show(SCREEN_MAIN); shot("1-main.bmp"); ui_show(SCREEN_ROMS); shot("2-roms.bmp");
  ui_show(SCREEN_SETTINGS); shot("3-settings.bmp"); ui_show(SCREEN_CPU); shot("4-cpu.bmp");
  ui_show(SCREEN_BATTERY); shot("5-battery.bmp");
  grid_set_accent(0xFF,0xB0,0x00); ui_show(SCREEN_MAIN); shot("7-main-amber.bmp");
  return 0;}
