#ifndef PLATFORM_H
#define PLATFORM_H

/* Platform layer. Desktop provides fake values.
   F2/F7 replace this module; UI never touches /sys. */

int platform_battery_percent(void);
int platform_battery_charging(void);
int platform_cpu_temp_c(void);
void platform_clock(char *buf, int buflen); /* "HH:MM" */

#endif
