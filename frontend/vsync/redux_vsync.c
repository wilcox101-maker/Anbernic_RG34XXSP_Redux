/* redux_vsync: a small LD_PRELOAD library that waits for the next vertical blank before an SDL
   app shows a frame. Redux preloads it only for the stock apps that tear (SimpleTerminal, File
   Manager; Dale's decision 2026-11-14, option B). Stock SDL 2.0.12 presents without vsync and
   SDL_RENDER_VSYNC has no effect on the device.

   It wraps the SDL2 and SDL1 calls that put a frame on screen. Each wrapper first waits with
   FBIO_WAITFORVSYNC on the framebuffer, then calls the real function (found with RTLD_NEXT).
   If the device can't open the framebuffer or the ioctl fails, the library switches itself off
   for the rest of the run and every call passes straight through, so it can never stop an app.

   REDUX_VSYNC_FB overrides the framebuffer path (tests). REDUX_VSYNC=0 turns it off. */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#ifndef FBIO_WAITFORVSYNC
#define FBIO_WAITFORVSYNC _IOW('F', 0x20, unsigned int)
#endif

static int state;   /* 0 = not tried yet, 1 = waiting works, -1 = off */
static int fb = -1;

int redux_vsync_state(void) { return state; }

static void wait_vblank(void) {
    if (state < 0) return;
    if (state == 0) {
        const char *off = getenv("REDUX_VSYNC");
        const char *path = getenv("REDUX_VSYNC_FB");
        if (off && strcmp(off, "0") == 0) { state = -1; return; }
        fb = open(path && path[0] ? path : "/dev/fb0", O_RDWR | O_CLOEXEC);
        if (fb < 0) {
            state = -1;
            fprintf(stderr, "[vsync] off: open %s: %s\n", path && path[0] ? path : "/dev/fb0", strerror(errno));
            return;
        }
    }
    unsigned int crtc = 0;
    if (ioctl(fb, FBIO_WAITFORVSYNC, &crtc) != 0) {
        if (state == 0) fprintf(stderr, "[vsync] off: FBIO_WAITFORVSYNC: %s\n", strerror(errno));
        state = -1;
        close(fb);
        fb = -1;
        return;
    }
    if (state == 0) fprintf(stderr, "[vsync] on\n");
    state = 1;
}

typedef void (*present_fn)(void *);
typedef int (*surface_fn)(void *);
typedef void (*rect_fn)(void *, int, int, unsigned, unsigned);

static void *next(const char *name) { return dlsym(RTLD_NEXT, name); }

/* SDL2 */
void SDL_RenderPresent(void *renderer) {
    static present_fn real;
    if (!real) real = (present_fn)next("SDL_RenderPresent");
    wait_vblank();
    if (real) real(renderer);
}

int SDL_UpdateWindowSurface(void *window) {
    static surface_fn real;
    if (!real) real = (surface_fn)next("SDL_UpdateWindowSurface");
    wait_vblank();
    return real ? real(window) : -1;
}

void SDL_GL_SwapWindow(void *window) {
    static present_fn real;
    if (!real) real = (present_fn)next("SDL_GL_SwapWindow");
    wait_vblank();
    if (real) real(window);
}

/* SDL 1.2 (st-sdl builds can use it) */
int SDL_Flip(void *screen) {
    static surface_fn real;
    if (!real) real = (surface_fn)next("SDL_Flip");
    wait_vblank();
    return real ? real(screen) : -1;
}

void SDL_UpdateRect(void *screen, int x, int y, unsigned w, unsigned h) {
    static rect_fn real;
    if (!real) real = (rect_fn)next("SDL_UpdateRect");
    wait_vblank();
    if (real) real(screen, x, y, w, h);
}
