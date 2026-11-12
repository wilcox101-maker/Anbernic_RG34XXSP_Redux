#include "sysinfo.h"
#include <stdio.h>
#include <string.h>

static char root_dir[256];

void sysinfo_set_root(const char *root) {
    if (!root || !root[0]) { root_dir[0] = '\0'; return; }
    snprintf(root_dir, sizeof root_dir, "%s", root);
}

static int read_text(const char *rel, char *buf, size_t n) {
    char path[512];
    if (root_dir[0]) snprintf(path, sizeof path, "%s%s", root_dir, rel);
    else snprintf(path, sizeof path, "%s", rel);
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    size_t got = fread(buf, 1, n - 1, f);
    fclose(f);
    buf[got] = '\0';
    return 0;
}

static void copy_fit(char *dst, size_t n, const char *src) {
    size_t i = 0;
    if (n == 0) return;
    if (!src) src = "";
    while (src[i] && i + 1 < n) { dst[i] = src[i]; i++; }
    dst[i] = '\0';
}

static void trim(char *s) {
    size_t n = strlen(s);
    while (n && (s[n - 1] == '\n' || s[n - 1] == '\r' || s[n - 1] == ' ')) s[--n] = '\0';
}

static void row(sysinfo_row_t *r, const char *label, const char *value) {
    copy_fit(r->label, sizeof r->label, label);
    copy_fit(r->value, sizeof r->value, value && value[0] ? value : "--");
}

static void fmt_x10(int x10, char *out, size_t n) {
    if (x10 < 0) snprintf(out, n, "--");
    else snprintf(out, n, "%d.%d C", x10 / 10, x10 % 10);
}

static int first_int(const char *text, int *out) {
    int sign = 1, v = 0, any = 0;
    if (!text) return -1;
    while (*text == ' ' || *text == '\t') text++;
    if (*text == '-') { sign = -1; text++; }
    while (*text >= '0' && *text <= '9') { any = 1; v = v * 10 + (*text - '0'); text++; }
    if (!any) return -1;
    *out = v * sign;
    return 0;
}

int sysinfo_parse_batt_tenths(const char *text) {
    int v = 0;
    if (first_int(text, &v) != 0) return -1;
    return v;
}

static int zone_index(const char *type) {
    if (!type || !type[0]) return -1;
    if (strstr(type, "battery")) return -1;
    if (strcmp(type, "cpu") == 0 || strncmp(type, "cpu", 3) == 0) return 0;
    if (strcmp(type, "gpu") == 0 || strncmp(type, "gpu", 3) == 0) return 1;
    /* Device names are cpu_thermal_zone, gpu_thermal_zone, ve_thermal_zone, ddr_thermal_zone
       (Dale's unit, 2026-11-12). Match the prefix so "ve" never matches e.g. "vendor". */
    if (strcmp(type, "ve") == 0 || strncmp(type, "ve_", 3) == 0) return 2;
    if (strncmp(type, "ddr", 3) == 0) return 3;
    return -1;
}

static void read_thermal(int therm_x10[4]) {
    for (int i = 0; i < 4; i++) therm_x10[i] = -1;
    for (int z = 0; z < 8; z++) {
        char rel[64], type[64], temp[32];
        snprintf(rel, sizeof rel, "/sys/class/thermal/thermal_zone%d/type", z);
        if (read_text(rel, type, sizeof type) != 0) continue;
        trim(type);
        int idx = zone_index(type);
        if (idx < 0 || therm_x10[idx] >= 0) continue;
        snprintf(rel, sizeof rel, "/sys/class/thermal/thermal_zone%d/temp", z);
        if (read_text(rel, temp, sizeof temp) != 0) continue;
        int milli = 0;
        if (first_int(temp, &milli) != 0) continue;
        therm_x10[idx] = milli / 100;   /* millidegrees -> tenths */
    }
}

static void read_mounts(char fstype[5][16]) {
    static const char *want[5] = { "/", "/mnt/vendor", "/mnt/data", "/mnt/mmc", "/mnt/sdcard" };
    for (int i = 0; i < 5; i++) fstype[i][0] = '\0';
    char text[4096];
    if (read_text("/proc/mounts", text, sizeof text) != 0) return;
    char *p = text;
    while (*p) {
        char *nl = strchr(p, '\n');
        if (nl) *nl = '\0';
        char dev[128], mnt[128], fs[32];
        if (sscanf(p, "%127s %127s %31s", dev, mnt, fs) == 3) {
            for (int i = 0; i < 5; i++)
                if (strcmp(mnt, want[i]) == 0 && !fstype[i][0])
                    copy_fit(fstype[i], sizeof fstype[i], fs);
        }
        if (!nl) break;
        p = nl + 1;
    }
}

static long mem_kb(const char *text, const char *key) {
    const char *p = strstr(text, key);
    if (!p) return -1;
    p += strlen(key);
    while (*p == ' ' || *p == ':') p++;
    int v = 0;
    if (first_int(p, &v) != 0) return -1;
    return v;
}

int sysinfo_collect(sysinfo_row_t *rows, int max) {
    if (!rows || max <= 0) return 0;
    char buf[256], value[SYSINFO_VALUE];
    int n = 0;
    /* axp2202 temp stays at 300. Do not present it as a reading. */
    if (n < max) row(&rows[n++], "Battery", "unknown (no sensor)");

    buf[0] = '\0';
    if (read_text("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", buf, sizeof buf) == 0)
        trim(buf);
    if (n < max) row(&rows[n++], "CPU governor", buf);

    int khz = -1;
    if (read_text("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", buf, sizeof buf) == 0)
        first_int(buf, &khz);
    if (khz < 0) snprintf(value, sizeof value, "--");
    else snprintf(value, sizeof value, "%d MHz", khz / 1000);
    if (n < max) row(&rows[n++], "CPU speed", value);

    int lo = -1, hi = -1;
    if (read_text("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq", buf, sizeof buf) == 0)
        first_int(buf, &lo);
    if (read_text("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", buf, sizeof buf) == 0)
        first_int(buf, &hi);
    if (lo < 0 || hi < 0) snprintf(value, sizeof value, "--");
    else snprintf(value, sizeof value, "%d-%d MHz", lo / 1000, hi / 1000);
    if (n < max) row(&rows[n++], "CPU range", value);

    int therm[4];
    read_thermal(therm);
    static const char *tname[4] = { "CPU", "GPU", "VE", "DDR" };
    for (int i = 0; i < 4; i++) {
        fmt_x10(therm[i], value, sizeof value);
        if (n < max) row(&rows[n++], tname[i], value);
    }

    long total = -1;
    if (read_text("/proc/meminfo", buf, sizeof buf) == 0) total = mem_kb(buf, "MemTotal");
    if (total < 0) snprintf(value, sizeof value, "--");
    else snprintf(value, sizeof value, "%ld MB", total / 1024);
    if (n < max) row(&rows[n++], "RAM", value);

    char fs[5][16];
    read_mounts(fs);
    static const char *mlabel[5] = { "/", "/mnt/vendor", "/mnt/data", "/mnt/mmc", "/mnt/sdcard" };
    for (int i = 0; i < 5 && n < max; i++) row(&rows[n++], mlabel[i], fs[i]);
    return n;
}
