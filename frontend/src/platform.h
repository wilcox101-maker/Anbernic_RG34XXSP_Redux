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

/* Engineering read-outs, F5 step 5. Read-only. */
int  platform_gpu_info(eng_gpu_t *g);                  /* 0 if the GPU devfreq node exists */
void platform_battery_extra(eng_batt_t *b);
void platform_mounts(eng_mount_t *m);                  /* ENG_MAX_MOUNTS entries */
/* which: 0 redux.log, 1 eng-writes.log, 2 dmesg. Last up to max lines, oldest first. */
int  platform_log_tail(int which, char lines[][ENG_LOG_COLS], int max);

/* fsck -n on one device (read-only, never repairs; output to /tmp/redux-fsck.txt). Exit code:
   0 clean, 4 problems found, 8 could not check, 127 fsck missing. */
int  platform_fsck(const char *dev);

/* LED and rumble test (F5 step 5c). 0 ok. Always leaves the green LED on and the rumble off. */
int  platform_led_rumble_test(void);

/* Raw stick values for the Input tab: out[0..3] = left X, left Y, right X, right Y. */
void platform_sticks(int out[4]);

/* Engineering > Update (step 1, read-only): build history, newest first; the last known working build. */
#include "buildhist.h"
int  platform_build_history(buildhist_entry_t *e, int max);
int  platform_last_good(buildhist_entry_t *out);    /* 0 found, -1 none */
/* Step 2: download the newest dev build (updater_download codes: 0 new, 1 same, -1 network, -2 checksum,
   -3 bad data) and install a kept build (0 ok; the caller reboots). Security item: Dale merges. */
int  platform_update_download(const char *current, char *sha, size_t n);
int  platform_update_install(const char *sha, const char *current, const char *how);
/* Step 3: 1 if builds/<sha>/dmenu.bin is kept on the device (roll-back possible). */
int  platform_update_have(const char *sha);

/* 1 while the heat guard has the CPU capped (F5 step 4). */
int  platform_hot(void);

/* Clock as "HH:MM". Real local time unless a fixed value was set (tests). */
void platform_clock(char *buf, int buflen);
void platform_set_fixed_clock(const char *hhmm);

#endif
