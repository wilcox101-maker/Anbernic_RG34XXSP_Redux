/* Build history for Engineering > Update (step 1). Temp folders only. */
#include "buildhist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int fails;
static void expect(int cond, const char *what) {
    printf("%s %s\n", cond ? "ok  " : "FAIL", what);
    if (!cond) fails++;
}

int main(void) {
    char base[] = "/tmp/redux-buildhist-XXXXXX";
    if (!mkdtemp(base)) { perror("mkdtemp"); return 2; }
    char dir[300];
    snprintf(dir, sizeof dir, "%s/builds", base);        /* created on first write */
    buildhist_set_dir(dir);
    buildhist_entry_t e[BUILDHIST_MAX], g;

    expect(buildhist_load(e, BUILDHIST_MAX) == 0, "no history yet");
    expect(buildhist_last_good(&g) == -1 && g.sha[0] == '\0', "no last known working yet");
    expect(buildhist_menu_reached("dev", 1794900000) == 0 && buildhist_load(e, 5) == 0, "dev builds are not recorded");
    expect(buildhist_menu_reached("bad;sha", 1794900000) == 0 && buildhist_load(e, 5) == 0, "a bad sha is not recorded");

    expect(buildhist_menu_reached("070427e", 1794900000) == 0, "first build recorded");
    int n = buildhist_load(e, BUILDHIST_MAX);
    expect(n == 1 && strcmp(e[0].sha, "070427e") == 0 && e[0].menu && strcmp(e[0].how, "manual") == 0,
           "entry: sha, reached the menu, manual");
    expect(strlen(e[0].when) == 16 && e[0].when[4] == '-' && e[0].when[13] == ':', "time stored as YYYY-MM-DD HH:MM");
    expect(buildhist_menu_reached("070427e", 1794900100) == 0 && buildhist_load(e, BUILDHIST_MAX) == 1,
           "same build again: no new line");

    /* A build that never reached the menu (written by step 2 later), then a good one by hand. */
    char path[320];
    snprintf(path, sizeof path, "%s/history", dir);
    FILE *f = fopen(path, "a");
    fputs("2917a5c 2026-11-17 10:00 nomenu update\n", f);
    fputs("garbage line\n", f);
    fclose(f);
    n = buildhist_load(e, BUILDHIST_MAX);
    expect(n == 2 && strcmp(e[0].sha, "2917a5c") == 0 && !e[0].menu, "newest first; bad line skipped");
    expect(buildhist_last_good(&g) == 0 && strcmp(g.sha, "070427e") == 0, "last known working skips the failed build");
    expect(buildhist_menu_reached("2917a5c", 1794900200) == 0, "the update build reaches the menu");
    n = buildhist_load(e, BUILDHIST_MAX);
    expect(n == 2 && e[0].menu && strcmp(e[0].how, "update") == 0, "marked working, how kept");

    /* At most BUILDHIST_MAX lines are kept. */
    for (int i = 0; i < 15; i++) {
        char sha[16];
        snprintf(sha, sizeof sha, "abc%04x", i);
        buildhist_menu_reached(sha, 1794900300 + i);
    }
    n = buildhist_load(e, BUILDHIST_MAX);
    expect(n == BUILDHIST_MAX && strcmp(e[0].sha, "abc000e") == 0 && strcmp(e[9].sha, "abc0005") == 0,
           "only the newest 10 kept");
    snprintf(path, sizeof path, "%s/history.tmp", dir);
    struct stat st;
    expect(stat(path, &st) != 0, "no temp file left");

    printf("%s\n", fails ? "FAILED" : "all passed");
    return fails ? 1 : 0;
}
