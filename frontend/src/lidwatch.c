#include <stdio.h>
#include <sys/stat.h>
#include "lidwatch.h"

static const char *g_path;

void lidwatch_set_path(const char *path) { g_path = path; }
const char *lidwatch_path(void) { return g_path ? g_path : "/tmp/redux-lidwatch"; }

int lidwatch_install(void) {
    const char *path = lidwatch_path();
    char tmp[256];
    snprintf(tmp, sizeof tmp, "%s.new", path);
    FILE *f = fopen(tmp, "wb");
    if (!f) return -1;
    size_t w = fwrite(redux_lidwatch_bin, 1, redux_lidwatch_bin_len, f);
    if (fclose(f) != 0 || w != redux_lidwatch_bin_len) { remove(tmp); return -1; }
    if (chmod(tmp, 0755) != 0 || rename(tmp, path) != 0) { remove(tmp); return -1; }
    return 0;
}
