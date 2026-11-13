#define _POSIX_C_SOURCE 200809L
#include "brightness.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef REDUX_PLATFORM_STOCK
#define BRIGHTNESS_PATH "/mnt/data/redux/brightness"
#define BRIGHTNESS_SYSFS "/sys/class/power_supply/axp2202-battery/brightness"
#else
#define BRIGHTNESS_PATH "/tmp/redux.brightness"
#endif

static brightness_write_fn g_write;
static const char *g_path;
static int g_level = BRIGHTNESS_DEFAULT;
static int g_last = -1;

void brightness_set_write(brightness_write_fn fn) { g_write = fn; }
void brightness_set_path(const char *path) { g_path = path; }
int brightness_get(void) { return g_level; }
int brightness_last_written(void) { return g_last; }

static const char *path(void) { return g_path ? g_path : BRIGHTNESS_PATH; }

static void save(void) {
#ifdef REDUX_PLATFORM_STOCK
    if (mkdir("/mnt/data/redux", 0755) != 0 && errno != EEXIST) return;
#endif
    FILE *f = fopen(path(), "w");
    if (!f) return;
    fprintf(f, "%d\n", g_level);
    fclose(f);
}

int brightness_apply(void) {
    g_last = g_level;
    if (g_write) return g_write(g_level);
#ifdef REDUX_PLATFORM_STOCK
    FILE *f = fopen(BRIGHTNESS_SYSFS, "w");
    if (!f) return -1;
    int rc = fprintf(f, "%d\n", g_level);
    fclose(f);
    return (rc > 0) ? 0 : -1;
#else
    return -1;   /* a desktop build must not touch sysfs */
#endif
}

int brightness_step(int dir) {
    int next = g_level + (dir > 0 ? 1 : dir < 0 ? -1 : 0);
    if (next < BRIGHTNESS_MIN) next = BRIGHTNESS_MIN;
    if (next > BRIGHTNESS_MAX) next = BRIGHTNESS_MAX;
    g_level = next;
    save();
    brightness_apply();
    return g_level;
}

void brightness_load(void) {
    FILE *f = fopen(path(), "r");
    if (!f) return;
    int n = g_level;
    if (fscanf(f, "%d", &n) == 1) {
        if (n < BRIGHTNESS_MIN) n = BRIGHTNESS_MIN;
        if (n > BRIGHTNESS_MAX) n = BRIGHTNESS_MAX;
        g_level = n;
    }
    fclose(f);
    brightness_apply();
}
