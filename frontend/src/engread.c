#define _POSIX_C_SOURCE 200809L
/* Engineering Mode read layer (F5 step 1). Read-only: nothing here opens a file for writing. */
#include "engread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>

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
    if (read_text(CPUFREQ "scaling_available_governors", list, sizeof list) == 0) {
        char *save = NULL;
        for (char *t = strtok_r(list, " \t", &save); t && c->n_govs < 8; t = strtok_r(NULL, " \t", &save))
            if (strlen(t) < ENG_NAME) snprintf(c->govs[c->n_govs++], ENG_NAME, "%s", t);
    }
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

#define DEVFREQ "/sys/class/devfreq/gpu/"

int engread_gpu(eng_gpu_t *g) {
    memset(g, 0, sizeof *g);
    g->cur_mhz = g->min_mhz = g->max_mhz = -1;
    if (read_text(DEVFREQ "governor", g->governor, sizeof g->governor) != 0) return -1;
    g->present = 1;
    int v;
    if (read_int(DEVFREQ "cur_freq", &v) == 0) g->cur_mhz = v / 1000000;
    if (read_int(DEVFREQ "min_freq", &v) == 0) g->min_mhz = v / 1000000;
    if (read_int(DEVFREQ "max_freq", &v) == 0) g->max_mhz = v / 1000000;
    char list[512];
    if (read_text(DEVFREQ "available_frequencies", list, sizeof list) == 0) {
        /* Hz: parse as kHz-list after dropping three digits per value */
        int khz[ENG_MAX_OPPS];
        int n = engread_parse_khz_list(list, khz, ENG_MAX_OPPS);   /* gives Hz/1000 */
        for (int i = 0; i < n; i++) g->freqs_mhz[i] = khz[i] / 1000;
        g->n_freqs = n;
    }
    return 0;
}

#define BATT "/sys/class/power_supply/axp2202-battery/"

void engread_battery(eng_batt_t *b) {
    memset(b, 0, sizeof *b);
    if (read_int(BATT "time_to_empty_now", &b->time_to_empty_s) != 0) b->time_to_empty_s = -1;
    if (read_int(BATT "time_to_full_now", &b->time_to_full_s) != 0) b->time_to_full_s = -1;
    if (read_text(BATT "health", b->health, sizeof b->health) != 0) b->health[0] = '\0';
}

int engread_mounts(eng_mount_t *m) {
    static const char *want[ENG_MAX_MOUNTS] = { "/", "/mnt/vendor", "/mnt/data", "/mnt/mmc", "/mnt/sdcard" };
    for (int i = 0; i < ENG_MAX_MOUNTS; i++) {
        memset(&m[i], 0, sizeof m[i]);
        snprintf(m[i].mount, sizeof m[i].mount, "%s", want[i]);
        m[i].size_mb = m[i].free_mb = -1;
    }
    char path[512];
    snprintf(path, sizeof path, "%s/proc/mounts", root);
    FILE *f = fopen(path, "r");
    if (!f) return ENG_MAX_MOUNTS;
    char line[512];
    while (fgets(line, sizeof line, f)) {
        char dev[128], mnt[128], fs[32], opts[256];
        if (sscanf(line, "%127s %127s %31s %255s", dev, mnt, fs, opts) != 4) continue;
        for (int i = 0; i < ENG_MAX_MOUNTS; i++) {
            if (strcmp(mnt, want[i]) != 0) continue;
            m[i].found = 1;   /* the last line for a mount point wins, as in the kernel */
            snprintf(m[i].dev, sizeof m[i].dev, "%.39s", dev);
            snprintf(m[i].fstype, sizeof m[i].fstype, "%.15s", fs);
            m[i].ro = strncmp(opts, "ro", 2) == 0 && (opts[2] == ',' || opts[2] == '\0');
        }
    }
    fclose(f);
    for (int i = 0; i < ENG_MAX_MOUNTS; i++) {
        struct statvfs v;
        snprintf(path, sizeof path, "%s%s", root, want[i]);
        if (m[i].found && statvfs(path, &v) == 0) {
            m[i].size_mb = (long long)v.f_blocks * (long long)v.f_frsize / (1024 * 1024);
            m[i].free_mb = (long long)v.f_bavail * (long long)v.f_frsize / (1024 * 1024);
        }
    }
    return ENG_MAX_MOUNTS;
}

int engread_tail(const char *rel, char lines[][ENG_LOG_COLS], int max) {
    char path[512];
    snprintf(path, sizeof path, "%s%s", root, rel);
    FILE *f = fopen(path, "r");
    if (!f || max <= 0) { if (f) fclose(f); return 0; }
    /* Read only the last 16 KB: logs can be large. */
    if (fseek(f, 0, SEEK_END) == 0) {
        long end = ftell(f);
        if (end > 16384) fseek(f, end - 16384, SEEK_SET); else fseek(f, 0, SEEK_SET);
        if (end > 16384) { int ch; while ((ch = fgetc(f)) != EOF && ch != '\n') {} }   /* skip partial */
    }
    int n = 0, head = 0;   /* ring buffer of the last max lines */
    char buf[1024];
    while (fgets(buf, sizeof buf, f)) {
        size_t len = strcspn(buf, "\r\n");
        int full_line = buf[len] != '\0';
        buf[len] = '\0';
        for (char *p = buf; *p; p++) if ((unsigned char)*p < 32) *p = ' ';
        buf[ENG_LOG_COLS - 1] = '\0';
        memcpy(lines[head], buf, ENG_LOG_COLS);
        head = (head + 1) % max;
        if (n < max) n++;
        if (!full_line) { int ch; while ((ch = fgetc(f)) != EOF && ch != '\n') {} }   /* rest of a long line */
    }
    fclose(f);
    if (n == max && head != 0) {   /* rotate so the oldest comes first */
        char tmp[ENG_LOG_COLS];
        for (int r = 0; r < head; r++) {
            memcpy(tmp, lines[0], sizeof tmp);
            memmove(lines[0], lines[1], (size_t)(max - 1) * ENG_LOG_COLS);
            memcpy(lines[max - 1], tmp, sizeof tmp);
        }
    }
    return n;
}
