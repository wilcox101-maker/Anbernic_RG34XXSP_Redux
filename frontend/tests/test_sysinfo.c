/* System Tools data layer. Fixtures only; nothing is written under /mnt or /sys. */
#include "sysinfo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int fails;
static void expect(int cond, const char *what) {
    printf("%s %s\n", cond ? "ok  " : "FAIL", what);
    if (!cond) fails++;
}

static void put(const char *path, const char *text) {
    FILE *f = fopen(path, "w");
    if (!f) { perror(path); return; }
    fputs(text, f);
    fclose(f);
}

static void mkdir_p(const char *path) { mkdir(path, 0755); }

static const char *value_of(const sysinfo_row_t *rows, int n, const char *label) {
    for (int i = 0; i < n; i++)
        if (strcmp(rows[i].label, label) == 0) return rows[i].value;
    return NULL;
}

int main(void) {
    const char *base = "/tmp/redux-sysinfo";
    mkdir_p(base);
    mkdir_p("/tmp/redux-sysinfo/sys");
    mkdir_p("/tmp/redux-sysinfo/sys/class");
    mkdir_p("/tmp/redux-sysinfo/sys/class/power_supply");
    mkdir_p("/tmp/redux-sysinfo/sys/class/power_supply/axp2202-battery");
    mkdir_p("/tmp/redux-sysinfo/sys/class/thermal");
    mkdir_p("/tmp/redux-sysinfo/sys/devices");
    mkdir_p("/tmp/redux-sysinfo/sys/devices/system");
    mkdir_p("/tmp/redux-sysinfo/sys/devices/system/cpu");
    mkdir_p("/tmp/redux-sysinfo/sys/devices/system/cpu/cpu0");
    mkdir_p("/tmp/redux-sysinfo/sys/devices/system/cpu/cpu0/cpufreq");
    mkdir_p("/tmp/redux-sysinfo/proc");
    for (int z = 0; z < 5; z++) {
        char p[128];
        snprintf(p, sizeof p, "/tmp/redux-sysinfo/sys/class/thermal/thermal_zone%d", z);
        mkdir_p(p);
    }
    put("/tmp/redux-sysinfo/sys/class/power_supply/axp2202-battery/temp", "300\n");
    put("/tmp/redux-sysinfo/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "schedutil\n");
    put("/tmp/redux-sysinfo/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", "1008000\n");
    put("/tmp/redux-sysinfo/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq", "480000\n");
    put("/tmp/redux-sysinfo/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", "1512000\n");
    put("/tmp/redux-sysinfo/sys/class/thermal/thermal_zone0/type", "cpu_thermal_zone\n");
    put("/tmp/redux-sysinfo/sys/class/thermal/thermal_zone0/temp", "41000\n");
    put("/tmp/redux-sysinfo/sys/class/thermal/thermal_zone1/type", "gpu_thermal_zone\n");
    put("/tmp/redux-sysinfo/sys/class/thermal/thermal_zone1/temp", "42000\n");
    put("/tmp/redux-sysinfo/sys/class/thermal/thermal_zone2/type", "ve_thermal_zone\n");
    put("/tmp/redux-sysinfo/sys/class/thermal/thermal_zone2/temp", "39000\n");
    put("/tmp/redux-sysinfo/sys/class/thermal/thermal_zone3/type", "ddr_thermal_zone\n");
    put("/tmp/redux-sysinfo/sys/class/thermal/thermal_zone3/temp", "40000\n");
    put("/tmp/redux-sysinfo/sys/class/thermal/thermal_zone4/type", "axp2202-battery\n");
    put("/tmp/redux-sysinfo/sys/class/thermal/thermal_zone4/temp", "30000\n");
    put("/tmp/redux-sysinfo/proc/meminfo", "MemTotal:        2026496 kB\nMemAvailable:    1048576 kB\n");
    put("/tmp/redux-sysinfo/proc/mounts",
        "/dev/mmcblk0p5 / ext4 rw 0 0\n"
        "/dev/mmcblk0p6 /mnt/vendor ext4 rw 0 0\n"
        "/dev/mmcblk0p7 /mnt/data ext4 rw 0 0\n"
        "/dev/mmcblk0p1 /mnt/mmc vfat rw 0 0\n"
        "/dev/mmcblk1p1 /mnt/sdcard vfat rw 0 0\n"
        "tmpfs /tmp tmpfs rw 0 0\n");

    sysinfo_set_root(base);
    sysinfo_row_t rows[SYSINFO_MAX_ROWS];
    int n = sysinfo_collect(rows, SYSINFO_MAX_ROWS);
    expect(n == 14, "fourteen rows");
    expect(sysinfo_parse_batt_tenths("300\n") == 300, "300 is 30.0 C in tenths");
    expect(sysinfo_parse_batt_tenths("nope") == -1, "a non-integer temp does not parse");
    expect(value_of(rows, n, "Battery") && strcmp(value_of(rows, n, "Battery"), "unknown (no sensor)") == 0,
           "battery temp is not shown; the sensor is a dummy");
    expect(value_of(rows, n, "CPU governor") && strcmp(value_of(rows, n, "CPU governor"), "schedutil") == 0,
           "governor");
    expect(value_of(rows, n, "CPU speed") && strcmp(value_of(rows, n, "CPU speed"), "1008 MHz") == 0,
           "current CPU speed");
    expect(value_of(rows, n, "CPU range") && strcmp(value_of(rows, n, "CPU range"), "480-1512 MHz") == 0,
           "OPP range");
    expect(value_of(rows, n, "CPU") && strcmp(value_of(rows, n, "CPU"), "41.0 C") == 0,
           "cpu zone is millidegrees, not the battery zone");
    expect(value_of(rows, n, "GPU") && strcmp(value_of(rows, n, "GPU"), "42.0 C") == 0, "gpu zone");
    expect(value_of(rows, n, "VE") && strcmp(value_of(rows, n, "VE"), "39.0 C") == 0, "ve zone");
    expect(value_of(rows, n, "DDR") && strcmp(value_of(rows, n, "DDR"), "40.0 C") == 0, "ddr zone");
    expect(value_of(rows, n, "RAM") && strcmp(value_of(rows, n, "RAM"), "1979 MB") == 0, "RAM in MB");
    expect(value_of(rows, n, "/") && strcmp(value_of(rows, n, "/"), "ext4") == 0, "root is ext4");
    expect(value_of(rows, n, "/mnt/vendor") && strcmp(value_of(rows, n, "/mnt/vendor"), "ext4") == 0,
           "vendor is ext4");
    expect(value_of(rows, n, "/mnt/data") && strcmp(value_of(rows, n, "/mnt/data"), "ext4") == 0,
           "data is ext4");
    expect(value_of(rows, n, "/mnt/mmc") && strcmp(value_of(rows, n, "/mnt/mmc"), "vfat") == 0, "TF1 is vfat");
    expect(value_of(rows, n, "/mnt/sdcard") && strcmp(value_of(rows, n, "/mnt/sdcard"), "vfat") == 0,
           "TF2 is vfat");

    sysinfo_set_root("/tmp/redux-sysinfo-missing");
    n = sysinfo_collect(rows, SYSINFO_MAX_ROWS);
    expect(value_of(rows, n, "CPU governor") && strcmp(value_of(rows, n, "CPU governor"), "--") == 0,
           "a missing file is --");
    expect(value_of(rows, n, "/mnt/sdcard") && strcmp(value_of(rows, n, "/mnt/sdcard"), "--") == 0,
           "an unmounted card is --");
    sysinfo_set_root(NULL);

    printf("%d failed\n", fails);
    return fails ? 1 : 0;
}
