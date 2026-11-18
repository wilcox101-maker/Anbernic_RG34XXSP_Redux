/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * redux-osk: the global on-screen keyboard (Redux OSK plan D1, Dale 2026-11-18). A background program
 * Redux starts; Menu + Select opens a keyboard over the running app on its own display layer
 * (disp0 ch3/l0, osk_disp.c), reads the pad while open (EVIOCGRAB, osk_in.c) and types through a
 * uinput keyboard (osk_kbd.c). Layout and look: keyboard UI A (term/osk.c, osk/osk_draw.c).
 *
 *   redux-osk          run (refuses if another redux-osk is running)
 *   redux-osk --off    one-shot: turn the OSK layer off (clears a layer left by a crash) and exit
 *
 * Rules: never opens over the Redux menu (/tmp/redux-menu.up) or while RetroArch runs; closes on lid
 * close, power-key sleep and an HDMI change; hides the layer and releases the grab on exit. While
 * closed it holds no grab, commits nothing and sleeps in poll(). Writes only /tmp files.
 * Nothing here has run on the device yet.
 */
#define _DEFAULT_SOURCE
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "osk_core.h"
#include "osk_disp.h"
#include "osk_draw.h"
#include "osk_in.h"
#include "osk_kbd.h"
#include "font.h"

#define PID_PATH  "/tmp/redux-osk.pid"
#define LOG_PATH  "/tmp/redux-osk.log"
#define MENU_FLAG "/tmp/redux-menu.up"
#define LID_CODE  110   /* KEY_INSERT on axp2202-pek (event0), as redux_lidwatch */

static volatile sig_atomic_t quit;
static void on_signal(int s) { (void)s; quit = 1; }

static void logf_(const char *fmt, ...) {
    FILE *f = fopen(LOG_PATH, "a");
    if (!f) return;
    time_t t = time(NULL);
    struct tm tm;
    localtime_r(&t, &tm);
    fprintf(f, "%02d:%02d:%02d ", tm.tm_hour, tm.tm_min, tm.tm_sec);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);
    fputc('\n', f);
    fclose(f);
}

static int file_exists(const char *p) { return access(p, F_OK) == 0; }

/* Another redux-osk alive? (pid file + /proc/<pid>/comm) */
static int already_running(void) {
    FILE *f = fopen(PID_PATH, "r");
    if (!f) return 0;
    int pid = 0;
    if (fscanf(f, "%d", &pid) != 1) pid = 0;
    fclose(f);
    if (pid <= 0 || pid == getpid()) return 0;
    char path[64], comm[64] = "";
    snprintf(path, sizeof path, "/proc/%d/comm", pid);
    f = fopen(path, "r");
    if (!f) return 0;
    if (!fgets(comm, sizeof comm, f)) comm[0] = 0;
    fclose(f);
    return strncmp(comm, "redux-osk", 9) == 0;
}

/* RetroArch running? Scan /proc/<pid>/comm (only when the chord comes, never in a loop). */
static int retroarch_running(void) {
    DIR *d = opendir("/proc");
    if (!d) return 0;
    struct dirent *e;
    int found = 0;
    while (!found && (e = readdir(d))) {
        if (e->d_name[0] < '0' || e->d_name[0] > '9') continue;
        char path[300], comm[64] = "";
        snprintf(path, sizeof path, "/proc/%s/comm", e->d_name);
        FILE *f = fopen(path, "r");
        if (!f) continue;
        if (fgets(comm, sizeof comm, f) && strncmp(comm, "retroarch", 9) == 0) found = 1;
        fclose(f);
    }
    closedir(d);
    return found;
}

static int hdmi_state(void) {
    static const char *paths[] = { "/sys/class/extcon/hdmi/state", "/sys/class/switch/hdmi/state", NULL };
    for (int i = 0; paths[i]; i++) {
        FILE *f = fopen(paths[i], "r");
        if (!f) continue;
        char buf[64] = "";
        if (!fgets(buf, sizeof buf, f)) buf[0] = 0;
        fclose(f);
        return strstr(buf, "=1") || buf[0] == '1';
    }
    return 0;
}

typedef struct {
    osk_disp_t *disp;
    osk_buf_t buf[2];
    int back, have_bufs;
    uint32_t sw, sh;
    osk_rect_t rect;
} screen_t;

static void bufs_free(screen_t *s) {
    if (!s->have_bufs) return;
    osk_disp_buf_free(s->disp, &s->buf[0]);
    osk_disp_buf_free(s->disp, &s->buf[1]);
    s->have_bufs = 0;
}

/* (Re)size the two buffers to the current screen: width = screen width (osk_draw picks the cap size
   from it), height = the panel. */
static int bufs_fit(screen_t *s) {
    uint32_t w = 0, h = 0;
    if (osk_disp_screen_size(s->disp, &w, &h) < 0 || !w || !h) { w = 720; h = 480; }
    int pw, ph;
    osk_draw_panel_size((int)w, &pw, &ph);
    if (s->have_bufs && w == s->sw && h == s->sh) return 0;
    bufs_free(s);
    s->sw = w; s->sh = h;
    if (osk_disp_buf_new(s->disp, w, (uint32_t)ph, &s->buf[0]) < 0) return -1;
    if (osk_disp_buf_new(s->disp, w, (uint32_t)ph, &s->buf[1]) < 0) { osk_disp_buf_free(s->disp, &s->buf[0]); return -1; }
    s->have_bufs = 1;
    s->rect = osk_disp_rect_bottom(w, h, w, (uint32_t)ph);
    logf_("screen %ux%u, panel %dx%d", w, h, pw, ph);
    return 0;
}

static void show(screen_t *s, const osk_t *k) {
    osk_draw_style_t st;
    osk_draw_style_default(&st);
    st.hint_text = "D-pad move  A type  L1/R1 page  B close";
    osk_buf_t *b = &s->buf[s->back];
    if (osk_draw(b->px, (int)b->w, (int)b->h, (int)b->w, k, &st, NULL, NULL) != 0) return;
    if (osk_disp_show(s->disp, s->rect, b) < 0) logf_("show failed");
    s->back ^= 1;
}

static int open_event0(void) { return open("/dev/input/event0", O_RDONLY | O_NONBLOCK | O_CLOEXEC); }

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--off") == 0) {
        int r = osk_disp_off_once(NULL);
        return r < 0 ? 1 : 0;
    }
    if (argc > 1) { fprintf(stderr, "usage: redux-osk [--off]\n"); return 2; }
    if (already_running()) { fprintf(stderr, "redux-osk: already running\n"); return 0; }

    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_signal;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    signal(SIGPIPE, SIG_IGN);

    FILE *pf = fopen(PID_PATH, "w");
    if (pf) { fprintf(pf, "%d\n", (int)getpid()); fclose(pf); }
    font_load();   /* the font chosen in Settings > UI; VGA when none is saved */

    osk_disp_off_once(NULL);   /* a layer left by an earlier crash */
    screen_t scr;
    memset(&scr, 0, sizeof scr);
    osk_in_t *in = NULL;
    osk_kbd_t *kbd = NULL;
    int r;
    if ((r = osk_disp_open(&scr.disp, NULL)) < 0) { logf_("disp open failed (%d)", r); goto out; }
    if ((r = osk_in_open(&in, NULL, NULL, NULL)) < 0) { logf_("keypad open failed (%d)", r); goto out; }
    if ((r = osk_kbd_open(&kbd, NULL)) < 0) { logf_("uinput keyboard failed (%d)", r); goto out; }   /* before any app starts (DR-1) */
    int ev0 = open_event0();
    logf_("started, pid %d", (int)getpid());

    osk_core_t core;
    osk_core_init(&core);
    int hdmi = hdmi_state();
    long opened_at = 0;

    while (!quit) {
        struct pollfd p[2] = { { osk_in_poll_fd(in), POLLIN, 0 }, { ev0, POLLIN, 0 } };
        /* Closed: sleep until a key. Open: wake once a second for HDMI and the grab check. */
        int n = poll(p, ev0 >= 0 ? 2 : 1, core.k.open ? 1000 : -1);
        if (n < 0 && errno != EINTR) break;
        osk_act_t a = { OSK_ACT_NONE, NULL, 0 };

        if (ev0 >= 0 && (p[1].revents & POLLIN)) {
            struct input_event e;
            while (read(ev0, &e, sizeof e) == (ssize_t)sizeof e)
                if (e.type == EV_KEY && e.value == 1 && (e.code == LID_CODE || e.code == KEY_POWER)) {
                    osk_act_t c = osk_core_force_close(&core);
                    if (c.kind != OSK_ACT_NONE) { a = c; logf_("closed: %s", e.code == KEY_POWER ? "power key" : "lid"); }
                }
        }
        if (core.k.open && a.kind == OSK_ACT_NONE) {
            int h = hdmi_state();
            if (h != hdmi || file_exists(MENU_FLAG)) {
                a = osk_core_force_close(&core);
                logf_("closed: %s", h != hdmi ? "HDMI change" : "menu up");
            } else if (!osk_in_is_grabbed(in) && opened_at && time(NULL) - opened_at >= 2) {
                a = osk_core_force_close(&core);   /* another program holds the pad (EBUSY) */
                logf_("closed: no grab (another program holds the pad)");
            }
            hdmi = h;
        }
        if (a.kind == OSK_ACT_CLOSE) { osk_disp_hide(scr.disp); osk_in_grab(in, 0); opened_at = 0; a.kind = OSK_ACT_NONE; }

        osk_in_event_t ev;
        while (!quit && (r = osk_in_next_event(in, &ev)) == 1) {
            if (ev.kind == OSK_IN_TOGGLE && !core.k.open) {
                core.menu_up = file_exists(MENU_FLAG);
                core.retroarch = core.menu_up ? 0 : retroarch_running();
            }
            a = osk_core_event(&core, &ev);
            switch (a.kind) {
            case OSK_ACT_OPEN:
                hdmi = hdmi_state();
                if (bufs_fit(&scr) < 0) { logf_("no buffers"); osk_core_force_close(&core); break; }
                osk_in_grab(in, 1);
                opened_at = (long)time(NULL);
                show(&scr, &core.k);
                logf_("opened");
                break;
            case OSK_ACT_CLOSE:
                osk_disp_hide(scr.disp);
                osk_in_grab(in, 0);
                opened_at = 0;
                logf_("closed");
                break;
            case OSK_ACT_TYPE:
                if (osk_kbd_type(kbd, a.key, a.mods) < 0) logf_("type failed: %s", a.key->label);
                show(&scr, &core.k);
                break;
            case OSK_ACT_REDRAW:
                show(&scr, &core.k);
                break;
            case OSK_ACT_REFUSED:
                logf_("chord ignored: %s", core.menu_up ? "Redux menu" : "RetroArch");
                break;
            default: break;
            }
        }
        if (r < 0 && r != -EAGAIN) { logf_("keypad read failed (%d)", r); break; }
    }
    logf_("stopping");
    if (ev0 >= 0) close(ev0);
out:
    if (scr.disp) { osk_disp_hide(scr.disp); bufs_free(&scr); osk_disp_close(scr.disp); }
    osk_disp_off_once(NULL);
    if (in) osk_in_close(in);
    if (kbd) osk_kbd_close(kbd);
    unlink(PID_PATH);
    return 0;
}
