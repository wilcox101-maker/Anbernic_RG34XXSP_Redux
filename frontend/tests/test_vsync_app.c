/* Child of test_vsync: an "app" linked against the fake SDL, run with the vsync library in
   LD_PRELOAD, as SimpleTerminal and File Manager are on the device. Prints what happened. */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
void SDL_RenderPresent(void *r);
int SDL_Flip(void *s);
extern int fake_presents;
int main(void) {
    SDL_RenderPresent(0);
    SDL_RenderPresent(0);
    SDL_Flip(0);
    int (*state)(void) = (int (*)(void))dlsym(RTLD_DEFAULT, "redux_vsync_state");
    printf("presents=%d state=%d\n", fake_presents, state ? state() : 99);
    return 0;
}
