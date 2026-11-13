#define _POSIX_C_SOURCE 200809L
#include "screentimeout.h"
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

#ifdef REDUX_PLATFORM_STOCK
#define TIMEOUT_PATH "/mnt/data/redux/screen_timeout"
#else
#define TIMEOUT_PATH "/tmp/redux.screen_timeout"
#endif

static const int choices[] = SCREENTIMEOUT_CHOICES;
#define N_CHOICES ((int)(sizeof choices / sizeof choices[0]))

static const char *g_path;
static int g_minutes = SCREENTIMEOUT_DEFAULT;

void screentimeout_set_path(const char *path) { g_path = path; }
int screentimeout_minutes(void) { return g_minutes; }

static const char *path(void) { return g_path ? g_path : TIMEOUT_PATH; }

static int index_of(int minutes) {
    for (int i = 0; i < N_CHOICES; i++) if (choices[i] == minutes) return i;
    return -1;
}

static void save(void) {
#ifdef REDUX_PLATFORM_STOCK
    if (mkdir("/mnt/data/redux", 0755) != 0 && errno != EEXIST) return;
#endif
    FILE *f = fopen(path(), "w");
    if (!f) return;
    fprintf(f, "%d\n", g_minutes);
    fclose(f);
}

int screentimeout_step(int dir) {
    int i = index_of(g_minutes);
    if (i < 0) i = index_of(SCREENTIMEOUT_DEFAULT);
    i += dir > 0 ? 1 : dir < 0 ? -1 : 0;
    if (i < 0) i = 0;
    if (i >= N_CHOICES) i = N_CHOICES - 1;
    g_minutes = choices[i];
    save();
    return g_minutes;
}

void screentimeout_load(void) {
    g_minutes = SCREENTIMEOUT_DEFAULT;
    FILE *f = fopen(path(), "r");
    if (!f) return;
    int m;
    if (fscanf(f, "%d", &m) == 1 && index_of(m) >= 0) g_minutes = m;
    fclose(f);
}

int screentimeout_due(uint32_t idle_ms) {
    return g_minutes > 0 && idle_ms >= (uint32_t)g_minutes * 60000u;
}
