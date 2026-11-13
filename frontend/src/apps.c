#include "apps.h"
#include "launcher.h"
#include <stdio.h>
#include <sys/stat.h>

/* Every stock app is "not verified" until Dale runs it from Redux on the device.
   PDF: the stock launch.sh exits without a file argument, so it waits for a file picker. */
typedef struct { const char *name; const char *status; const char *dge; } app_t;
static const app_t apps[APP_COUNT] = {
    [APP_MUSIC]     = { "Music Player", "not verified", "/mnt/vendor/bin/music/ap.dge" },
    [APP_VIDEO]     = { "Video Player", "not verified", "/mnt/vendor/bin/video/vp.dge" },
    [APP_EBOOK_TF1] = { "Ebook Reader (TF1)", "not verified", "/mnt/vendor/bin/ebook/reader_mmc.dge" },
    [APP_EBOOK_TF2] = { "Ebook Reader (TF2)", "not verified", "/mnt/vendor/bin/ebook/reader_sdcard.dge" },
    [APP_FILES]     = { "File Manager", "not verified", "/mnt/vendor/bin/fileM/dinguxCommand_en.dge" },
    [APP_PDF]       = { "PDF Reader", "later (needs a file picker)", NULL },
    [APP_TERMINAL]  = { "Terminal", "later", NULL },
};

static const char *root = "";

int apps_count(void) { return APP_COUNT; }

const char *apps_name(int i) {
    return (i >= 0 && i < APP_COUNT) ? apps[i].name : "";
}

const char *apps_status(int i) {
    return (i >= 0 && i < APP_COUNT) ? apps[i].status : "";
}

int apps_launchable(int i) {
    return i >= 0 && i < APP_COUNT && apps[i].dge != NULL;
}

void apps_set_root(const char *r) { root = r ? r : ""; }

int apps_launch(int i) {
    if (!apps_launchable(i)) return -1;
    char path[512];
    struct stat st;
    snprintf(path, sizeof path, "%s%s", root, apps[i].dge);
    if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) return -2;
    return launcher_start(LAUNCH_DGE, apps[i].dge, NULL) == 0 ? 0 : -1;
}
