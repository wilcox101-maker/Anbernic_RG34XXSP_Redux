#include "cardscripts.h"
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

#ifdef REDUX_PLATFORM_STOCK
#define CARDSCRIPTS_PATH "/mnt/data/redux/card_scripts"
#else
#define CARDSCRIPTS_PATH "/tmp/redux.card_scripts"
#endif

static const char *g_path;
static const char *path(void) { return g_path ? g_path : CARDSCRIPTS_PATH; }

void cardscripts_set_path(const char *p) { g_path = p; }

/* Only an exact "1" means On. A missing, empty or damaged file means Off. */
int cardscripts_enabled(void) {
    FILE *f = fopen(path(), "r");
    if (!f) return 0;
    char buf[4] = "";
    int on = fgets(buf, sizeof buf, f) && buf[0] == '1' && (buf[1] == '\n' || buf[1] == '\0');
    fclose(f);
    return on;
}

void cardscripts_set(int on) {
#ifdef REDUX_PLATFORM_STOCK
    if (!g_path && mkdir("/mnt/data/redux", 0755) != 0 && errno != EEXIST) return;
#endif
    if (!on) { remove(path()); return; }
    FILE *f = fopen(path(), "w");
    if (!f) return;
    fputs("1\n", f);
    fclose(f);
}
