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

    /* Missing everything: no crash, -1s and empty strings. */
    engread_set_root("/tmp/redux-engread-missing");
    expect(engread_cpu(&c) == -1 && c.governor[0] == '\0' && c.min_mhz == -1 && c.n_opps == 0,
           "missing cpufreq gives -1 and empty values");
    expect(engread_thermal(z, ENG_MAX_ZONES) == 0, "missing thermal gives 0 zones");

    printf("%s\n", fails ? "FAILED" : "all passed");
    return fails ? 1 : 0;
}
