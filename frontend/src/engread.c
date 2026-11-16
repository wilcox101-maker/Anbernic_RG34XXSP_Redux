/* Engineering Mode read layer (F5 step 1). Read-only: nothing here opens a file for writing. */
#include "engread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char root[256];

void engread_set_root(const char *r) {
    snprintf(root, sizeof root, "%s", r ? r : "");
}

static int read_text(const char *rel, char *buf, size_t n) {
    char path[512];
    snprintf(path, sizeof path, "%s%s", root, rel);
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    size_t got = fread(buf, 1, n - 1, f);
    fclose(f);
    buf[got] = '\0';
    while (got && (buf[got - 1] == '\n' || buf[got - 1] == '\r' || buf[got - 1] == ' '))
        buf[--got] = '\0';
    return 0;
}

static int read_int(const char *rel, int *out) {
    char buf[32];
    if (read_text(rel, buf, sizeof buf) != 0) return -1;
    char *end;
    long v = strtol(buf, &end, 10);
    if (end == buf) return -1;
    *out = (int)v;
    return 0;
}

static int cmp_int(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

int engread_parse_khz_list(const char *text, int *mhz, int max) {
    int n = 0;
    const char *p = text;
    while (p && *p && n < max) {
        char *end;
        long khz = strtol(p, &end, 10);
        if (end == p) { p++; continue; }
        p = end;
        if (khz <= 0) continue;
        int v = (int)(khz / 1000);
        int dup = 0;
        for (int i = 0; i < n; i++) if (mhz[i] == v) dup = 1;
        if (!dup) mhz[n++] = v;
    }
    qsort(mhz, (size_t)n, sizeof *mhz, cmp_int);
    return n;
}

#define CPUFREQ "/sys/devices/system/cpu/cpu0/cpufreq/"

int engread_cpu(eng_cpu_t *c) {
    memset(c, 0, sizeof *c);
    c->min_mhz = c->max_mhz = -1;
    int rc = read_text(CPUFREQ "scaling_governor", c->governor, sizeof c->governor);
    if (rc != 0) c->governor[0] = '\0';
    int v;
    if (read_int(CPUFREQ "scaling_min_freq", &v) == 0) c->min_mhz = v / 1000;
    if (read_int(CPUFREQ "scaling_max_freq", &v) == 0) c->max_mhz = v / 1000;
    char list[512];
    if (read_text(CPUFREQ "scaling_available_frequencies", list, sizeof list) == 0)
        c->n_opps = engread_parse_khz_list(list, c->opps_mhz, ENG_MAX_OPPS);
    return rc == 0 ? 0 : -1;
}

int engread_thermal(eng_zone_t *z, int max) {
    int n = 0;
    for (int i = 0; i < 32 && n < max; i++) {
        char rel[96];
        eng_zone_t *e = &z[n];
        memset(e, 0, sizeof *e);
        snprintf(rel, sizeof rel, "/sys/class/thermal/thermal_zone%d/type", i);
        if (read_text(rel, e->type, sizeof e->type) != 0) continue;
        int milli;
        snprintf(rel, sizeof rel, "/sys/class/thermal/thermal_zone%d/temp", i);
        e->temp_x10 = read_int(rel, &milli) == 0 ? milli / 100 : -1;
        for (int t = 0; t < ENG_MAX_TRIPS; t++) {
            eng_trip_t *tr = &e->trips[e->n_trips];
            snprintf(rel, sizeof rel, "/sys/class/thermal/thermal_zone%d/trip_point_%d_type", i, t);
            if (read_text(rel, tr->type, sizeof tr->type) != 0) break;
            snprintf(rel, sizeof rel, "/sys/class/thermal/thermal_zone%d/trip_point_%d_temp", i, t);
            tr->temp_x10 = read_int(rel, &milli) == 0 ? milli / 100 : -1;
            e->n_trips++;
        }
        n++;
    }
    return n;
}
