#include "apps.h"
#include "launcher.h"
#include <stdio.h>
#include <sys/stat.h>

/* Every stock app is "not verified" until Dale runs it from Redux on the device.
   PDF: the stock launch.sh exits without a file argument, so it waits for a file picker. */
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
    [APP_MUSIC]     = { "Music Player", "not verified", "/mnt/vendor/bin/music/ap.dge" },
    [APP_VIDEO]     = { "Video Player", "not verified", "/mnt/vendor/bin/video/vp.dge" },
    [APP_EBOOK_TF1] = { "Ebook Reader (TF1)", "not verified", "/mnt/vendor/bin/ebook/reader_mmc.dge" },
    [APP_EBOOK_TF2] = { "Ebook Reader (TF2)", "not verified", "/mnt/vendor/bin/ebook/reader_sdcard.dge" },
    [APP_FILES]     = { "File Manager", "not verified", "/mnt/vendor/bin/fileM/dinguxCommand_en.dge" },
    [APP_PDF]       = { "PDF Reader", "later (needs a file picker)", NULL },
    [APP_TERMINAL]  = { "Terminal", "not verified", NULL, TERM_BIN, "HOME=/root", NULL },
    [APP_TOP]       = { "Task Monitor (htop)", "not verified", NULL, TERM_BIN, "HOME=/root", "-e htop" },
    /* Tools: stock Ubuntu programs, each seen present with no missing libraries on Dale's unit
       (2026-11-13 check). The default ALSA card is the one Redux's volume (amixer) already uses.
       evtest reads the buttons device without grabbing it, so MENU still quits the terminal. */
    [APP_MIXER]        = { "Audio Mixer", "not verified", NULL, TERM_BIN, "HOME=/root", "-e alsamixer" },
    [APP_NETWORK]      = { "Network Setup", "not verified", NULL, TERM_BIN, "HOME=/root", "-e nmtui" },
    [APP_BLUETOOTH]    = { "Bluetooth", "not verified", NULL, TERM_BIN, "HOME=/root", "-e bluetoothctl" },
    [APP_INPUT_TEST]   = { "Input Test", "not verified", NULL, TERM_BIN, "HOME=/root", "-e evtest /dev/input/event1" },
    [APP_SPEAKER_TEST] = { "Speaker Test", "not verified", NULL, TERM_BIN, "HOME=/root", "-e speaker-test -t wav -c 2 -l 1" },
};

static const char *root = "";

/* Rows of the Apps list. File Manager is left out: it is on Main (Dale, 2026-11-13, 12d: A). */
static int listed(int i) { return i != APP_FILES; }

int apps_count(void) {
    int n = 0;
    for (int i = 0; i < APP_COUNT; i++) n += listed(i);
    return n;
}

int apps_list_id(int row) {
    for (int i = 0, r = 0; i < APP_COUNT; i++)
        if (listed(i) && r++ == row) return i;
    return -1;
}

int apps_row_of(int id) {
    for (int i = 0, r = 0; i < APP_COUNT; i++) {
        if (i == id) return listed(i) ? r : -1;
        r += listed(i);
    }
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
    if (apps[i].dge) return launcher_start(LAUNCH_DGE, apps[i].dge, NULL) == 0 ? 0 : -1;
    return launcher_start_app(apps[i].bin, apps[i].env, apps[i].args) == 0 ? 0 : -1;
}
