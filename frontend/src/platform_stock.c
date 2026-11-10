/* Stock platform: reads real sysfs on the RG34XXSP. UI never touches /sys. */
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int read_int(const char *path, int fallback) {
    FILE *f = fopen(path, "r");
    if (!f) return fallback;
    int v = fallback;
    fscanf(f, "%d", &v);
    fclose(f);
    return v;
}

int platform_battery_percent(void) {
    return read_int("/sys/class/power_supply/axp2202-battery/capacity", 0);
}
int platform_battery_charging(void) {
    char buf[32] = {0};
    FILE *f = fopen("/sys/class/power_supply/axp2202-battery/status", "r");
    if (f) { fgets(buf, sizeof buf, f); fclose(f); }
    return strstr(buf, "Charging") != NULL;
}
int platform_battery_mv(void) {
    return read_int("/sys/class/power_supply/axp2202-battery/voltage_now", 0) / 1000;
}
int platform_battery_ma(void) {
    return read_int("/sys/class/power_supply/axp2202-battery/current_now", 0) / 1000;
}
int platform_battery_temp_known(void) {
    /* Unverified whether the thermistor works. Return 0 until confirmed. */
    return 0;
}
int platform_wifi_connected(void) {
    return read_int("/sys/class/net/wlan0/carrier", 0);
}
int platform_cpu_temp_c(void) {
    return read_int("/sys/class/thermal/thermal_zone0/temp", 0) / 1000;
}
int platform_cpu_cur_mhz(int cpu) {
    char path[128];
    snprintf(path, sizeof path, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", cpu);
    return read_int(path, 0) / 1000;
}
int platform_cpu_online(int cpu) {
    char path[128];
    snprintf(path, sizeof path, "/sys/devices/system/cpu/cpu%d/online", cpu);
    return read_int(path, 1);
}
void platform_clock(char *buf, int buflen) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    snprintf(buf, buflen, "%02d:%02d", tm->tm_hour, tm->tm_min);
}
void platform_set_fixed_clock(const char *hhmm) { (void)hhmm; }
