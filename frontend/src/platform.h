#ifndef REDUX_PLATFORM_H
#define REDUX_PLATFORM_H

#include "engread.h"

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

/* Engineering Mode read-outs (F5 step 1). Read-only. */
int  platform_cpu_info(eng_cpu_t *c);                 /* 0 ok, -1 governor unreadable */
int  platform_thermal_zones(eng_zone_t *z, int max);  /* count */

/* Engineering profile (F5 step 2). State: 0 none, 1 saved, 2 set aside after failed starts. */
int  platform_eng_profile_state(void);
int  platform_eng_profile_save(void);    /* 0 ok */
int  platform_eng_profile_clear(void);   /* 0 ok */

/* Engineering live writes (F5 step 3): only values from the chip's own lists; every one logged.
   0 ok, -1 refused (not listed / cpu0), -2 write or read-back failed. */
int  platform_eng_set_governor(const char *gov);
int  platform_eng_set_freq_mhz(int min_mhz, int max_mhz);
int  platform_eng_set_core(int cpu, int on);

/* 1 while the heat guard has the CPU capped (F5 step 4). */
int  platform_hot(void);

/* Clock as "HH:MM". Real local time unless a fixed value was set (tests). */
void platform_clock(char *buf, int buflen);
void platform_set_fixed_clock(const char *hhmm);

#endif
