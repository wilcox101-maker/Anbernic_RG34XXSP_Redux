#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/* Desktop fakes: the same numbers the F0 mock-ups show. */
int platform_battery_percent(void)    { return 87; }
int platform_battery_charging(void)   { return 1; }
int platform_battery_mv(void)         { return 3920; }
int platform_battery_ma(void)         { return -420; }
int platform_battery_temp_known(void) { return 0; }
int platform_wifi_connected(void)     { return 1; }
int platform_cpu_temp_c(void)         { return 54; }
int platform_cpu_cur_mhz(int cpu)     { (void)cpu; return 1008; }
int platform_cpu_online(int cpu)      { (void)cpu; return 1; }

static char fixed_clock[6];

void platform_set_fixed_clock(const char *hhmm) {
    snprintf(fixed_clock, sizeof fixed_clock, "%s", hhmm ? hhmm : "");
}

void platform_clock(char *buf, int buflen) {
    if (fixed_clock[0]) { snprintf(buf, (size_t)buflen, "%s", fixed_clock); return; }
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    snprintf(buf, (size_t)buflen, "%02d:%02d", tm->tm_hour, tm->tm_min);
}
