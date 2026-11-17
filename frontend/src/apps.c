#include "apps.h"
#include "launcher.h"
#include "pkgs.h"
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
/* Terminal programs run in Redux Terminal (frontend/term; Dale 2026-11-17 A, PR 3 of 3: the apps
   moved over from the stock SimpleTerminal, which Redux no longer starts). main writes it to /tmp
   at start (term.c); it takes -e <command> and --hold (wait for a button after the command ends),
   and MENU quits. Task Monitor runs htop (/usr/bin/htop, Ubuntu package htop; Dale installed it
   with apt, decision 2026-11-13 is htop for all: Settings > System > Packages supplies it). */
#define TERM_BIN "/tmp/redux-term"
static const app_t apps[APP_COUNT] = {
    [APP_MUSIC]     = { "Music Player", "verified", "/mnt/vendor/bin/music/ap.dge" },
    [APP_VIDEO]     = { "Video Player", "plays; no buttons", "/mnt/vendor/bin/video/vp.dge" },
    [APP_EBOOK_TF1] = { "Ebook Reader (TF1)", "verified", "/mnt/vendor/bin/ebook/reader_mmc.dge" },
    [APP_EBOOK_TF2] = { "Ebook Reader (TF2)", "not verified", "/mnt/vendor/bin/ebook/reader_sdcard.dge" },
    /* File Manager is Midnight Commander (Dale 2026-11-17: replaces the stock dinguxCommander; mc
       installed with apt, works, shows Chinese file names). Statuses below are "verified" only once
       a device result backs the Redux Terminal launch (they were verified in SimpleTerminal). */
    [APP_FILES]     = { "File Manager", "not verified", NULL, TERM_BIN, "HOME=/root", "-e mc" },
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
    [APP_SPEAKER_TEST] = { "Speaker Test", "not verified", NULL, TERM_BIN, "HOME=/root", "--hold -e speaker-test -t wav -c 2 -l 1" },
    /* --off: no logo, so the text fits the terminal width (Dale 12:56: the logo ran off the right).
       --hold keeps it on screen until a button. */
    [APP_NEOFETCH]     = { "NeoFetch", "not verified", NULL, TERM_BIN, "HOME=/root", "--hold -e neofetch --off" },
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

/* Rows: the hand-written apps above plus every catalog package (tools/extras.ini) that has an
   `app =` name and a `launch =` line and whose binary is installed (Dale 2026-11-17 A). Those get
   ids APP_COUNT + catalog index. The list is rebuilt when Apps opens (apps_rescan). */
#define MAX_ROWS (APP_COUNT + PKGS_MAX)
static int order[MAX_ROWS], n_order = -1;

static int catalog_installed(int k) {
    static const char *dirs[] = { "/usr/bin", "/usr/local/bin", "/bin", "/usr/games" };
    const char *bin = pkgs_binary(k);
    if (!bin[0]) return 0;
    char path[512];
    struct stat st;
    for (size_t d = 0; d < sizeof dirs / sizeof dirs[0]; d++) {
        snprintf(path, sizeof path, "%s%s/%s", root, dirs[d], bin);
        if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) return 1;
    }
    return 0;
}

static int catalog_row(int id) { return id >= APP_COUNT && id < APP_COUNT + pkgs_count() ? id - APP_COUNT : -1; }

static const char *name_of(int id) {
    int k = catalog_row(id);
    return k >= 0 ? pkgs_app(k) : (id >= 0 && id < APP_COUNT) ? apps[id].name : "";
}

void apps_rescan(void) { n_order = -1; }

static void build_order(void) {
    if (n_order >= 0) return;
    n_order = 0;
    for (int i = 0; i < APP_COUNT; i++) if (listed(i)) order[n_order++] = i;
    for (int k = 0; k < pkgs_count() && n_order < MAX_ROWS; k++)
        if (pkgs_app(k)[0] && pkgs_launch(k)[0] && catalog_installed(k)) order[n_order++] = APP_COUNT + k;
    for (int a = 1; a < n_order; a++)          /* insertion sort, case-insensitive by name */
        for (int b = a; b > 0 && strcasecmp(name_of(order[b - 1]), name_of(order[b])) > 0; b--) {
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

const char *apps_name(int i) { return name_of(i); }

const char *apps_status(int i) {
    if (catalog_row(i) >= 0) return "from Packages";
    return (i >= 0 && i < APP_COUNT) ? apps[i].status : "";
}

int apps_launchable(int i) {
    if (catalog_row(i) >= 0) return 1;
    return i >= 0 && i < APP_COUNT && (apps[i].dge != NULL || apps[i].bin != NULL);
}

void apps_set_root(const char *r) { root = r ? r : ""; n_order = -1; }

int apps_launch(int i) {
    if (!apps_launchable(i)) return -1;
    char path[512];
    struct stat st;
    int k = catalog_row(i);
    const char *bin = k >= 0 ? TERM_BIN : apps[i].dge ? apps[i].dge : apps[i].bin;
    snprintf(path, sizeof path, "%s%s", root, bin);
    if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) return -2;
    /* The video player hands playback back through /tmp/.next (launcher_start_video). */
    if (i == APP_VIDEO) return launcher_start_video(apps[i].dge) == 0 ? 0 : -1;
    if (k >= 0) return launcher_start_app(TERM_BIN, "HOME=/root", pkgs_launch(k)) == 0 ? 0 : -1;
    int rc = apps[i].dge ? launcher_start(LAUNCH_DGE, apps[i].dge, NULL)
                         : launcher_start_app(apps[i].bin, apps[i].env, apps[i].args);
    return rc == 0 ? 0 : -1;
}
