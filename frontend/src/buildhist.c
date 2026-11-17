#define _POSIX_C_SOURCE 200809L
#include "buildhist.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static char g_dir[256] = BUILDHIST_DIR;

void buildhist_set_dir(const char *dir) { snprintf(g_dir, sizeof g_dir, "%s", dir ? dir : BUILDHIST_DIR); }

static int sha_ok(const char *s) {
    size_t n = strlen(s);
    if (n < 7 || n > 15) return 0;
    for (size_t i = 0; i < n; i++)
        if (!((s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'f'))) return 0;
    return 1;
}

/* All lines, oldest first. Bad lines are skipped. */
static int read_all(buildhist_entry_t *e, int max) {
    char path[300], line[160];
    snprintf(path, sizeof path, "%s/history", g_dir);
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    int n = 0;
    while (fgets(line, sizeof line, f)) {
        buildhist_entry_t x;
        char date[12], tm[8], menu[8];
        memset(&x, 0, sizeof x);
        if (sscanf(line, "%15s %11s %7s %7s %11s", x.sha, date, tm, menu, x.how) != 5) continue;
        if (!sha_ok(x.sha)) continue;
        snprintf(x.when, sizeof x.when, "%.10s %.5s", date, tm);
        x.menu = strcmp(menu, "menu") == 0;
        if (n == max) { memmove(e, e + 1, sizeof *e * (size_t)(max - 1)); n--; }   /* keep the newest */
        e[n++] = x;
    }
    fclose(f);
    return n;
}

static int write_all(const buildhist_entry_t *e, int n) {
    char path[300], tmp[310];
    if (mkdir(g_dir, 0755) != 0 && errno != EEXIST) return -1;
    snprintf(path, sizeof path, "%s/history", g_dir);
    snprintf(tmp, sizeof tmp, "%s.tmp", path);
    int fd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return -1;
    int ok = 1;
    for (int i = (n > BUILDHIST_MAX ? n - BUILDHIST_MAX : 0); i < n && ok; i++) {
        char line[96];
        int len = snprintf(line, sizeof line, "%s %s %s %s\n", e[i].sha, e[i].when,
                           e[i].menu ? "menu" : "nomenu", e[i].how[0] ? e[i].how : "manual");
        ok = len > 0 && write(fd, line, (size_t)len) == len;
    }
    if (ok && fsync(fd) != 0) ok = 0;
    if (close(fd) != 0) ok = 0;
    if (!ok || rename(tmp, path) != 0) { remove(tmp); return -1; }
    return 0;
}

int buildhist_load(buildhist_entry_t *e, int max) {
    buildhist_entry_t all[BUILDHIST_MAX + 1];
    int n = read_all(all, BUILDHIST_MAX + 1);
    int k = 0;
    for (int i = n - 1; i >= 0 && k < max; i--) e[k++] = all[i];
    return k;
}

int buildhist_menu_reached(const char *sha, time_t now) {
    if (!sha || !sha_ok(sha)) return 0;              /* "dev" desktop builds: nothing to record */
    buildhist_entry_t all[BUILDHIST_MAX + 1];
    int n = read_all(all, BUILDHIST_MAX);
    if (n > 0 && strcmp(all[n - 1].sha, sha) == 0) {
        if (all[n - 1].menu) return 0;               /* already marked: no write */
        all[n - 1].menu = 1;
    } else {
        buildhist_entry_t x;
        memset(&x, 0, sizeof x);
        snprintf(x.sha, sizeof x.sha, "%s", sha);
        struct tm tmv;
        if (localtime_r(&now, &tmv)) strftime(x.when, sizeof x.when, "%Y-%m-%d %H:%M", &tmv);
        else snprintf(x.when, sizeof x.when, "0000-00-00 00:00");
        x.menu = 1;
        snprintf(x.how, sizeof x.how, "manual");
        all[n++] = x;
    }
    return write_all(all, n);
}

int buildhist_last_good(buildhist_entry_t *out) {
    buildhist_entry_t e[BUILDHIST_MAX];
    int n = buildhist_load(e, BUILDHIST_MAX);
    for (int i = 0; i < n; i++)
        if (e[i].menu) { *out = e[i]; return 0; }
    memset(out, 0, sizeof *out);
    return -1;
}
