#ifndef REDUX_PLATFORM_H
#define REDUX_PLATFORM_H

/* Platform layer: the only place that knows where values come from.
 * This desktop build returns fake values matching docs/ui-spec.md.
 * F2 (stock) and F7 (mainline) replace platform.c; UI code never touches /sys. */

int  platform_battery_percent(void);
int  platform_battery_charging(void);
int  platform_battery_mv(void);          /* millivolts */
int  platform_battery_ma(void);          /* negative = discharging */
int  platform_battery_ma_known(void);    /* 0 if the PMIC exposes no current (stock: no current_now) */
int  platform_battery_temp_known(void);  /* 0 until the thermistor is verified */
int  platform_wifi_connected(void);
int  platform_cpu_temp_c(void);
int  platform_cpu_cur_mhz(int cpu);
int  platform_cpu_online(int cpu);

/* Clock as "HH:MM". Real local time unless a fixed value was set (tests). */
void platform_clock(char *buf, int buflen);
void platform_set_fixed_clock(const char *hhmm);

#endif
