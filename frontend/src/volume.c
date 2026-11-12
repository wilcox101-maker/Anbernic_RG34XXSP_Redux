#define _POSIX_C_SOURCE 200809L
#include "volume.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef REDUX_PLATFORM_STOCK
#define VOLUME_PATH "/mnt/data/redux/volume"
#else
#define VOLUME_PATH "/tmp/redux.volume"
#endif

static volume_run_fn g_run;
static const char *g_path;
static int g_level = VOLUME_MAX;
static char g_last[80];

void volume_set_run(volume_run_fn fn) { g_run = fn; }
void volume_set_path(const char *path) { g_path = path; }
int volume_get(void) { return g_level; }
const char *volume_last_command(void) { return g_last; }

static const char *path(void) { return g_path ? g_path : VOLUME_PATH; }

static void save(void) {
#ifdef REDUX_PLATFORM_STOCK
    if (mkdir("/mnt/data/redux", 0755) != 0 && errno != EEXIST) return;
#endif
    FILE *f = fopen(path(), "w");
    if (!f) return;
    fprintf(f, "%d\n", g_level);
    fclose(f);
}

int volume_apply(void) {
    snprintf(g_last, sizeof g_last, "amixer sset 'lineout volume' %d", g_level);
    if (g_run) return g_run(g_last);
#ifdef REDUX_PLATFORM_STOCK
    pid_t pid = fork();
    if (pid < 0) return -1;
    if (pid == 0) {
        execlp("sh", "sh", "-c", g_last, (char *)NULL);
        _exit(127);
    }
    int st = 0;
    if (waitpid(pid, &st, 0) < 0) return -1;
    return (WIFEXITED(st) && WEXITSTATUS(st) == 0) ? 0 : -1;
#else
    return -1;   /* a desktop build must not touch the host mixer */
#endif
}

int volume_step(int dir) {
    int next = g_level + (dir > 0 ? 1 : dir < 0 ? -1 : 0);
    if (next < VOLUME_MIN) next = VOLUME_MIN;
    if (next > VOLUME_MAX) next = VOLUME_MAX;
    g_level = next;
    save();
    volume_apply();
    return g_level;
}

void volume_load(void) {
    FILE *f = fopen(path(), "r");
    if (!f) return;
    int n = g_level;
    if (fscanf(f, "%d", &n) == 1) {
        if (n < VOLUME_MIN) n = VOLUME_MIN;
        if (n > VOLUME_MAX) n = VOLUME_MAX;
        g_level = n;
    }
    fclose(f);
    volume_apply();
}
