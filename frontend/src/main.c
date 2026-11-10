/* RG34XXSP Redux frontend.
 *
 *   rg34xxsp-redux                     run in a 720x480 window
 *   rg34xxsp-redux --accent RRGGBB     choose the accent colour (default 00FFFF, BIOS cyan)
 *   rg34xxsp-redux --dump              print the five F0 screens as text and exit (tests)
 *
 * Redraws only when something changes (input, the hold-A timer, the clock),
 * so an idle menu costs almost nothing on battery.
 * On device, a successful launch sets ui_should_exit(); main exits so loadapp.sh runs /tmp/.next.
 * Stock builds read /dev/input/event1 and event0 directly. */
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grid.h"
#include "platform.h"
#include "remap.h"
#include "ui.h"
#ifdef REDUX_PLATFORM_STOCK
#include "evdev_reader.h"
#endif

#define VIDEO_RETRY_MS 30000u
#define VIDEO_RETRY_STEP_MS 500u
#define BOOT_REPAINT_MS 15000u
#define PRESENT_BUFFERS 3
#define SCREEN_W (GRID_COLS * FONT_W)
#define SCREEN_H (GRID_ROWS * FONT_H)

static int parse_hex_color(const char *s, Uint8 *r, Uint8 *g, Uint8 *b) {
    if (!s || strlen(s) != 6) return -1;
    char *end;
    unsigned long v = strtoul(s, &end, 16);
    if (*end) return -1;
    *r = (Uint8)(v >> 16); *g = (Uint8)(v >> 8); *b = (Uint8)v;
    return 0;
}

/* Print each F0 mock-up screen as text, with the clock fixed to the spec's 12:34. */
static int dump_screens(void) {
    static const struct { screen_t s; const char *name; } shots[] = {
        { SCREEN_MAIN, "main" }, { SCREEN_ROMS, "roms" }, { SCREEN_SETTINGS, "settings" },
        { SCREEN_ENG, "eng" }, { SCREEN_INTERNET, "internet" },
    };
    static char buf[GRID_ROWS * (GRID_COLS * 3 + 1) + 1];
    platform_set_fixed_clock("12:34");
    for (size_t i = 0; i < sizeof shots / sizeof shots[0]; i++) {
        ui_show(shots[i].s);
        ui_draw();
        grid_dump(buf, sizeof buf);
        printf("=== %s ===\n%s", shots[i].name, buf);
    }
    return 0;
}

#ifdef REDUX_PLATFORM_STOCK
static void on_device_button(logical_button_t btn, int pressed) {
    ui_button(btn, pressed, SDL_GetTicks());
}
#endif

int main(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--dump")) { ui_init(); return dump_screens(); }
        if (!strcmp(argv[i], "--accent") && i + 1 < argc) {
            Uint8 r, g, b;
            if (parse_hex_color(argv[++i], &r, &g, &b) != 0) {
                fprintf(stderr, "--accent wants RRGGBB, e.g. FFB000 for amber\n");
                return 2;
            }
            grid_set_accent(r, g, b);
            continue;
        }
        fprintf(stderr, "usage: %s [--accent RRGGBB] [--dump]\n", argv[0]);
        return 2;
    }

    SDL_Window *win = NULL;
    SDL_Renderer *ren = NULL;
    for (uint32_t waited = 0;; waited += VIDEO_RETRY_STEP_MS) {
        if (SDL_Init(SDL_INIT_VIDEO) == 0) {
            win = SDL_CreateWindow("RG34XXSP Redux", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN);
            ren = win ? SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED) : NULL;
            if (!ren) ren = win ? SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE) : NULL;
            if (ren) {
                if (waited) fprintf(stderr, "video ready after %u ms of retries\n", (unsigned)waited);
                break;
            }
            if (win) SDL_DestroyWindow(win);
            win = NULL;
        }
        if (waited == 0 || waited + VIDEO_RETRY_STEP_MS >= VIDEO_RETRY_MS)
            fprintf(stderr, "SDL video not ready (%u ms): %s\n", (unsigned)waited, SDL_GetError());
        SDL_Quit();
        if (waited + VIDEO_RETRY_STEP_MS >= VIDEO_RETRY_MS) return 1;
        SDL_Delay(VIDEO_RETRY_STEP_MS);
    }

#ifdef REDUX_PLATFORM_STOCK
    if (evdev_open() != 0)
        fprintf(stderr, "warning: could not open /dev/input/event1\n");
#endif

    ui_init();
    const uint32_t start_ms = SDL_GetTicks();
    int running = 1;
    while (running && !ui_should_exit()) {
        if (ui_dirty() || SDL_GetTicks() - start_ms < BOOT_REPAINT_MS) {
            ui_draw();
            for (int i = 0; i < PRESENT_BUFFERS; i++) {
                grid_present(ren);
                SDL_RenderPresent(ren);
            }
        }
        SDL_Event e;
        int have_event;
#ifdef REDUX_PLATFORM_STOCK
        evdev_wait((int)ui_wait_ms());
        evdev_poll(on_device_button);
        have_event = SDL_PollEvent(&e);
#else
        have_event = SDL_WaitEventTimeout(&e, (int)ui_wait_ms());
#endif
        if (have_event) {
            do {
                if (e.type == SDL_QUIT) running = 0;
                if ((e.type == SDL_KEYDOWN && !e.key.repeat) || e.type == SDL_KEYUP) {
                    logical_button_t btn = remap_key(e.key.keysym.sym);
                    if (btn != LB_NONE) ui_button(btn, e.type == SDL_KEYDOWN, SDL_GetTicks());
                }
                if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_EXPOSED) {
                    grid_present(ren);
                    SDL_RenderPresent(ren);
                }
            } while (SDL_PollEvent(&e));
        }
        ui_tick(SDL_GetTicks());
    }

#ifdef REDUX_PLATFORM_STOCK
    evdev_close();
#endif
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
