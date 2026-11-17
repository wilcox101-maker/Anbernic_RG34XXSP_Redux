#include "apps.h"
#include "vsync.h"
#include "launcher.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

/* Status column: "verified" only where a device result note backs it (Dale, 2026-11-17: update it as
   apps pass on the device). Music: notes/2026-11-13-dale-hw-result-app-center-ports-boot.md and
   #185 check 13. Video: #185 check 9 (plays, no buttons). Ebook (TF1): #185 check 16. Terminal and
   Task Monitor: 11-13 note and #185 check 13. The five Tools: notes/2026-11-13-dale-hw-result-143.md
   (#150). PDF: the stock launch.sh exits without a file argument, so it waits for a file picker. */
typedef struct {
    const char *name, *status;
    const char *dge;              /* .dge app (launch_target LAUNCH_DGE), or NULL */
    const char *bin, *env, *args; /* or a program run from its own folder (launcher_start_app) */
} app_t;
/* SimpleTerminal: on rootfs (/mnt/mod/ctrl), not the card. The mod starts it as
   HOME=/root ./SimpleTerminal; it takes -e <command>, and MENU quits. Task Monitor runs htop
   (/usr/bin/htop, Ubuntu package htop). Not stock: Dale installed it with apt; Dale's decision
   2026-11-13 is htop for all, so Redux must supply it on units that lack it (queue item). */
#define TERM_BIN "/mnt/mod/ctrl/SimpleTerminal" 
static const app_t apps[APP_COUNT] = {
    [APP_MUSIC]     = { "Music Player", "verified", "/mnt/vendor/bin/music/ap.dge" },
    [APP_VIDEO]     = { "Video Player", "plays; no buttons", "/mnt/vendor/bin/video/vp.dge" },
    [APP_EBOOK_TF1] = { "Ebook Reader (TF1)", "verified", "/mnt/vendor/bin/ebook/reader_mmc.dge" },
    [APP_EBOOK_TF2] = { "Ebook Reader (TF2)", "not verified", "/mnt/vendor/bin/ebook/reader_sdcard.dge" },
    /* File Manager is Midnight Commander in the terminal (Dale 2026-11-17: replaces the stock
       dinguxCommander; mc installed with apt, works, shows Chinese file names). */
    [APP_FILES]     = { "File Manager", "verified", NULL, TERM_BIN, "HOME=/root", "-e mc" },
    [APP_PDF]       = { "PDF Reader", "later (needs a file picker)", NULL },
    [APP_TERMINAL]  = { "Terminal", "verified", NULL, TERM_BIN, "HOME=/root", NULL },
    [APP_TOP]       = { "Task Monitor (htop)", "verified", NULL, TERM_BIN, "HOME=/root", "-e htop" },
    /* Tools: stock Ubuntu programs, each seen present with no missing libraries on Dale's unit
       (2026-11-13 check). The default ALSA card is the one Redux's volume (amixer) already uses.
       evtest reads the buttons device without grabbing it, so MENU still quits the terminal. */
    [APP_MIXER]        = { "Audio Mixer", "verified", NULL, TERM_BIN, "HOME=/root", "-e alsamixer" },
    [APP_NETWORK]      = { "Network Setup", "verified", NULL, TERM_BIN, "HOME=/root", "-e nmtui" },
    [APP_BLUETOOTH]    = { "Bluetooth", "verified", NULL, TERM_BIN, "HOME=/root", "-e bluetoothctl" },
    [APP_INPUT_TEST]   = { "Input Test", "verified", NULL, TERM_BIN, "HOME=/root", "-e evtest /dev/input/event1" },
    [APP_SPEAKER_TEST] = { "Speaker Test", "verified", NULL, TERM_BIN, "HOME=/root", "-e speaker-test -t wav -c 2 -l 1" },
    /* --off: no logo, so the text fits the terminal width (Dale 12:56: the logo ran off the right).
       It stays on screen until MENU quits the terminal. */
    [APP_NEOFETCH]     = { "NeoFetch", "verified", NULL, TERM_BIN, "HOME=/root", "-e sh -c 'neofetch --off; exec sleep 86400'" },
};

static const char *root = "";

/* Rows of the Apps list. File Manager, Music Player and Video Player are left out: they are on Main
   (Dale, 2026-11-13 12d: A; 2026-11-17: Music and Video too). Audio Mixer and Speaker Test live in
   Settings > Audio; Task Monitor, Bluetooth, Input Test and Network Setup in Settings (Dale,
   2026-11-17 11:49). Rows are sorted by name (Dale, 2026-11-17). */
static int listed(int i) {
    switch (i) {
    case APP_FILES: case APP_MUSIC: case APP_VIDEO:
    case APP_MIXER: case APP_SPEAKER_TEST:
    case APP_TOP: case APP_BLUETOOTH: case APP_INPUT_TEST: case APP_NETWORK:
        return 0;
    default:
        return 1;
    }
}

static int order[APP_COUNT], n_order = -1;

static void build_order(void) {
    if (n_order >= 0) return;
    n_order = 0;
    for (int i = 0; i < APP_COUNT; i++) if (listed(i)) order[n_order++] = i;
    for (int a = 1; a < n_order; a++)          /* insertion sort, case-insensitive by name */
        for (int b = a; b > 0 && strcasecmp(apps[order[b - 1]].name, apps[order[b]].name) > 0; b--) {
            int t = order[b]; order[b] = order[b - 1]; order[b - 1] = t;
        }
}

int apps_count(void) {
    build_order();
    return n_order;
}

int apps_list_id(int row) {
    build_order();
    return (row >= 0 && row < n_order) ? order[row] : -1;
}

int apps_row_of(int id) {
    build_order();
    for (int r = 0; r < n_order; r++) if (order[r] == id) return r;
    return -1;
}

const char *apps_name(int i) {
    return (i >= 0 && i < APP_COUNT) ? apps[i].name : "";
}

const char *apps_status(int i) {
    return (i >= 0 && i < APP_COUNT) ? apps[i].status : "";
}

int apps_launchable(int i) {
    return i >= 0 && i < APP_COUNT && (apps[i].dge != NULL || apps[i].bin != NULL);
}

void apps_set_root(const char *r) { root = r ? r : ""; }

int apps_launch(int i) {
    if (!apps_launchable(i)) return -1;
    char path[512];
    struct stat st;
    const char *bin = apps[i].dge ? apps[i].dge : apps[i].bin;
    snprintf(path, sizeof path, "%s%s", root, bin);
    if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) return -2;
    /* The video player hands playback back through /tmp/.next (launcher_start_video). */
    if (i == APP_VIDEO) return launcher_start_video(apps[i].dge) == 0 ? 0 : -1;
    /* SimpleTerminal tears (stock SDL presents without vsync): preload the vsync library for it
       (Dale's decision 2026-11-14, B). If it can't be written, launch without it. File Manager is
       not covered: it never calls a present function (see frontend/vsync/redux_vsync.c). */
    char env[300];
    if (apps[i].bin && strcmp(apps[i].bin, TERM_BIN) == 0 && vsync_install(env, sizeof env) == 0)
        launcher_set_preload(env);
    int rc = apps[i].dge ? launcher_start(LAUNCH_DGE, apps[i].dge, NULL)
                         : launcher_start_app(apps[i].bin, apps[i].env, apps[i].args);
    /* A failed launch must not leave the preload for the next .dge or app launch. */
    if (rc != 0) launcher_set_preload(NULL);
    return rc == 0 ? 0 : -1;
}
