#include "platform.h"
#include <stdio.h>
#include <time.h>

int platform_battery_percent(void) { return 87; }
int platform_battery_charging(void) { return 1; }
int platform_cpu_temp_c(void) { return 54; }

void platform_clock(char *buf, int buflen) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    snprintf(buf, buflen, "%02d:%02d", tm->tm_hour, tm->tm_min);
}
