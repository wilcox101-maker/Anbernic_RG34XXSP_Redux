/* Writes the embedded redux-term to /tmp, like the lid watcher and heat guard. */
#include <stdio.h>
#include <sys/stat.h>
#include "term.h"

static const char *g_path;

void term_set_path(const char *path) { g_path = path; }
const char *term_path(void) { return g_path ? g_path : "/tmp/redux-term"; }

int term_install(void) {
    const char *path = term_path();
    char tmp[256];
    snprintf(tmp, sizeof tmp, "%s.new", path);
    FILE *f = fopen(tmp, "wb");
    if (!f) return -1;
    size_t w = fwrite(redux_term_bin, 1, redux_term_bin_len, f);
    if (fclose(f) != 0 || w != redux_term_bin_len) { remove(tmp); return -1; }
    if (chmod(tmp, 0755) != 0 || rename(tmp, path) != 0) { remove(tmp); return -1; }
    return 0;
}
