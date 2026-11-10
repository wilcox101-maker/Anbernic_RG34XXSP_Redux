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

#define BOOT_REPAINT_MS 15000u   /* repaint about once a second for this long after start */
#define PRESENT_BUFFERS 3   /* covers double and triple buffering */
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
        { SCREEN_CPU, "cpu" }, { SCREEN_BATTERY, "battery" },
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

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Window *win = SDL_CreateWindow("RG34XXSP Redux", SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED, SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN);
    SDL_Renderer *ren = win ? SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED) : NULL;
    if (!ren) ren = win ? SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE) : NULL;
    if (!ren) {
        fprintf(stderr, "SDL window/renderer: %s\n", SDL_GetError());
        if (win) SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

#ifdef REDUX_PLATFORM_STOCK
    if (evdev_open() != 0)
        fprintf(stderr, "warning: could not open /dev/input/event1\n");
#endif

    ui_init();
    const uint32_t start_ms = SDL_GetTicks();
    int running = 1;
    while (running && !ui_should_exit()) {
        /* For the first seconds after start, repaint even when nothing changed: at boot the
           first frames can land before the display shows them, and the stock boot logo stayed
           up until the next change (~45 s on the device, 2026-11-10). */
        if (ui_dirty() || SDL_GetTicks() - start_ms < BOOT_REPAINT_MS) {
            ui_draw();
            /* Draw the same frame into every buffer the display backend flips between. On the
               device (Anbernic SDL 2.0.12, page-flipped framebuffer) presenting once left the
               previous frame in the other buffer, so two highlights showed (Dale, 2026-11-10). */
            for (int i = 0; i < PRESENT_BUFFERS; i++) {
                grid_present(ren);
                SDL_RenderPresent(ren);
            }
        }
        SDL_Event e;
        int have_event;
#ifdef REDUX_PLATFORM_STOCK
        /* Sleep on the input devices, not on SDL: SDL never sees event0/event1, so waiting in
           SDL_WaitEventTimeout would delay every button press by up to ui_wait_ms() (1 s idle). */
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
                    grid_present(ren);   /* window uncovered: repaint the last frame */
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
    return 0;   /* on a launch, loadapp.sh now runs /tmp/.next */
}
