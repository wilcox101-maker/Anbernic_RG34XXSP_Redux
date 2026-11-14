#include <stdio.h>
#include <sys/stat.h>
#include "vsync.h"

static const char *g_path;

void vsync_set_path(const char *path) { g_path = path; }

int vsync_install(char *env, size_t n) {
    const char *path = g_path ? g_path : "/tmp/redux-vsync.so";
    char tmp[256];
    snprintf(tmp, sizeof tmp, "%s.new", path);
    FILE *f = fopen(tmp, "wb");
    if (!f) return -1;
    size_t w = fwrite(redux_vsync_so, 1, redux_vsync_so_len, f);
    if (fclose(f) != 0 || w != redux_vsync_so_len) { remove(tmp); return -1; }
    chmod(tmp, 0644);
    if (rename(tmp, path) != 0) { remove(tmp); return -1; }
    if ((size_t)snprintf(env, n, "LD_PRELOAD=%s", path) >= n) return -1;
    return 0;
}
