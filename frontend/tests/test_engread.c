/* Engineering Mode read layer (F5 step 1). Fixtures under /tmp only; nothing under /sys is touched. */
#include "engread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int fails;
static void expect(int cond, const char *what) {
    printf("%s %s\n", cond ? "ok  " : "FAIL", what);
    if (!cond) fails++;
}

static void mkdir_p(const char *path) {
    char tmp[512];
    snprintf(tmp, sizeof tmp, "%s", path);
    for (char *p = tmp + 1; *p; p++)
        if (*p == '/') { *p = '\0'; mkdir(tmp, 0755); *p = '/'; }
    mkdir(tmp, 0755);
}

static void put(const char *base, const char *rel, const char *text) {
    char path[512];
    snprintf(path, sizeof path, "%s%s", base, rel);
    char dir[512];
    snprintf(dir, sizeof dir, "%s", path);
    char *slash = strrchr(dir, '/');
    if (slash) { *slash = '\0'; mkdir_p(dir); }
    FILE *f = fopen(path, "w");
    if (!f) { perror(path); return; }
    fputs(text, f);
    fclose(f);
}

int main(void) {
    const char *base = "/tmp/redux-engread";
    const char *cf = "/sys/devices/system/cpu/cpu0/cpufreq/";
    char rel[256];

    /* Parser: kHz to MHz, sorted, duplicates dropped, junk skipped. */
    int mhz[ENG_MAX_OPPS];
    int n = engread_parse_khz_list("1512000 480000 600000 480000 x 1008000\n", mhz, ENG_MAX_OPPS);
    expect(n == 4 && mhz[0] == 480 && mhz[1] == 600 && mhz[2] == 1008 && mhz[3] == 1512,
           "kHz list parsed, sorted, deduplicated");
    expect(engread_parse_khz_list("", mhz, ENG_MAX_OPPS) == 0, "empty list gives 0");
    expect(engread_parse_khz_list("1 2000 3000 4000", mhz, 2) == 2, "list capped at max");

    /* CPU, the stock table as read on Dale's unit (480-1512 MHz). */
    snprintf(rel, sizeof rel, "%sscaling_governor", cf);            put(base, rel, "schedutil\n");
    snprintf(rel, sizeof rel, "%sscaling_min_freq", cf);            put(base, rel, "480000\n");
    snprintf(rel, sizeof rel, "%sscaling_max_freq", cf);            put(base, rel, "1512000\n");
    snprintf(rel, sizeof rel, "%sscaling_available_frequencies", cf);
    put(base, rel, "480000 600000 792000 1008000 1200000 1296000 1416000 1512000 \n");
    snprintf(rel, sizeof rel, "%sscaling_available_governors", cf);
    put(base, rel, "interactive conservative ondemand userspace powersave performance schedutil \n");
    engread_set_root(base);
    eng_cpu_t c;
    expect(engread_cpu(&c) == 0, "cpu read ok");
    expect(strcmp(c.governor, "schedutil") == 0, "governor trimmed");
    expect(c.min_mhz == 480 && c.max_mhz == 1512, "min/max in MHz");
    expect(c.n_opps == 8 && c.opps_mhz[0] == 480 && c.opps_mhz[7] == 1512, "8 OPPs from the fixture");
    expect(c.n_govs == 7 && strcmp(c.govs[6], "schedutil") == 0, "7 governors in the kernel's order");

    /* Thermal: two zones, the second without trips; a gap in numbering is skipped. */
    put(base, "/sys/class/thermal/thermal_zone0/type", "cpu_thermal_zone\n");
    put(base, "/sys/class/thermal/thermal_zone0/temp", "41250\n");
    put(base, "/sys/class/thermal/thermal_zone0/trip_point_0_type", "passive\n");
    put(base, "/sys/class/thermal/thermal_zone0/trip_point_0_temp", "85000\n");
    put(base, "/sys/class/thermal/thermal_zone0/trip_point_1_type", "critical\n");
    put(base, "/sys/class/thermal/thermal_zone0/trip_point_1_temp", "110000\n");
    put(base, "/sys/class/thermal/thermal_zone2/type", "gpu_thermal_zone\n");
    put(base, "/sys/class/thermal/thermal_zone2/temp", "bad\n");
    eng_zone_t z[ENG_MAX_ZONES];
    n = engread_thermal(z, ENG_MAX_ZONES);
    expect(n == 2, "two zones found across a numbering gap");
    expect(strcmp(z[0].type, "cpu_thermal_zone") == 0 && z[0].temp_x10 == 412, "zone0 41.2 C");
    expect(z[0].n_trips == 2 && z[0].trips[1].temp_x10 == 1100
           && strcmp(z[0].trips[1].type, "critical") == 0, "zone0 trips read");
    expect(strcmp(z[1].type, "gpu_thermal_zone") == 0 && z[1].temp_x10 == -1 && z[1].n_trips == 0,
           "unreadable temp is -1, no trips");
    expect(engread_thermal(z, 1) == 1, "zone count capped at max");

    /* GPU devfreq in Hz, as on Dale's unit (min/max read 0). */
    put(base, "/sys/class/devfreq/gpu/governor", "simple_ondemand\n");
    put(base, "/sys/class/devfreq/gpu/available_frequencies", "420000000 456000000 504000000 552000000 600000000 648000000\n");
    put(base, "/sys/class/devfreq/gpu/cur_freq", "420000000\n");
    put(base, "/sys/class/devfreq/gpu/min_freq", "0\n");
    put(base, "/sys/class/devfreq/gpu/max_freq", "0\n");
    eng_gpu_t g;
    expect(engread_gpu(&g) == 0 && g.present && strcmp(g.governor, "simple_ondemand") == 0, "gpu present");
    expect(g.n_freqs == 6 && g.freqs_mhz[0] == 420 && g.freqs_mhz[5] == 648 && g.cur_mhz == 420, "gpu speeds in MHz");
    expect(g.min_mhz == 0 && g.max_mhz == 0, "gpu min/max 0 as read");

    /* Battery extras. */
    put(base, "/sys/class/power_supply/axp2202-battery/time_to_empty_now", "15120\n");
    put(base, "/sys/class/power_supply/axp2202-battery/health", "Good\n");
    eng_batt_t b;
    engread_battery(&b);
    expect(b.time_to_empty_s == 15120 && b.time_to_full_s == -1 && strcmp(b.health, "Good") == 0, "battery extras");

    /* Mounts: two entries for /mnt/mmc, the last wins; ro detected; a missing one stays not found. */
    put(base, "/proc/mounts", "/dev/mmcblk0p5 / ext4 rw,relatime 0 0\n/dev/mmcblk0p7 /mnt/data ext4 rw 0 0\n"
                              "/dev/mmcblk0p1 /mnt/mmc vfat rw 0 0\n/dev/mmcblk0p1 /mnt/mmc vfat ro,noatime 0 0\n");
    eng_mount_t m[ENG_MAX_MOUNTS];
    engread_mounts(m);
    expect(m[0].found && strcmp(m[0].fstype, "ext4") == 0 && !m[0].ro, "/ found rw");
    expect(m[3].found && m[3].ro && strcmp(m[3].dev, "/dev/mmcblk0p1") == 0, "/mnt/mmc ro from the last line");
    expect(!m[1].found && !m[4].found && m[1].size_mb == -1, "unlisted mounts not found");
    expect(m[0].size_mb >= 0, "statvfs of the fixture root gives a size");

    /* Tail: last 3 of 5 lines, oldest first; control characters blanked; long lines cut. */
    put(base, "/log.txt", "one\ntwo\nthree\nfo\tur\nfive\n");
    char lines[3][ENG_LOG_COLS];
    n = engread_tail("/log.txt", lines, 3);
    expect(n == 3 && strcmp(lines[0], "three") == 0 && strcmp(lines[1], "fo ur") == 0 && strcmp(lines[2], "five") == 0,
           "tail keeps the last 3 in order");
    char longl[400];
    memset(longl, 'x', sizeof longl - 2); longl[sizeof longl - 2] = '\n'; longl[sizeof longl - 1] = '\0';
    put(base, "/long.txt", longl);
    n = engread_tail("/long.txt", lines, 3);
    expect(n == 1 && strlen(lines[0]) == ENG_LOG_COLS - 1, "long line cut to the column limit");
    expect(engread_tail("/nope.txt", lines, 3) == 0, "missing log gives 0 lines");

    /* Missing everything: no crash, -1s and empty strings. */
    engread_set_root("/tmp/redux-engread-missing");
    expect(engread_cpu(&c) == -1 && c.governor[0] == '\0' && c.min_mhz == -1 && c.n_opps == 0,
           "missing cpufreq gives -1 and empty values");
    expect(engread_thermal(z, ENG_MAX_ZONES) == 0, "missing thermal gives 0 zones");
    expect(engread_gpu(&g) == -1 && !g.present, "missing gpu not present");

    printf("%s\n", fails ? "FAILED" : "all passed");
    return fails ? 1 : 0;
}
