#ifndef REDUX_ENGPROFILE_H
#define REDUX_ENGPROFILE_H

/* Engineering Mode safety net (F5 step 2, notes/2026-11-16-claude-decision-f5-plan.md).
   - Every write goes through eng_write_*: the value must be in the chip's own list (so it is always
     inside the stock OPP table), the old and new values are logged, and the result is read back.
   - Live changes are not kept. "Save as profile" stores the current CPU policy; the profile is
     applied at the first Redux start of each boot.
   - Two failed starts in a row (Redux started with the profile but never reached the menu) set the
     profile aside as eng-profile.off, so the next boot runs stock settings.
   Files live in /mnt/data/redux (p7), written as temp file + fsync + rename. Never on p1. */

#define ENG_DATA_DIR  "/mnt/data/redux"
#define ENG_CPUFREQ   "/sys/devices/system/cpu/cpu0/cpufreq"
#define ENG_BOOT_FLAG "/tmp/redux-eng-started"     /* RAM: first Redux start of this boot */
#define ENG_MAX_FAILED_STARTS 2
#define ENG_LOG_MAX_BYTES 65536                    /* then the log rolls to eng-writes.log.1 */

/* Tests point these at temp folders. NULL keeps the default. */
void engprofile_set_paths(const char *cpufreq_dir, const char *data_dir, const char *boot_flag);
/* Tests: folder holding cpu1/online ... (default /sys/devices/system/cpu). */
void engprofile_set_cpu_root(const char *cpu_root);

typedef enum {
    ENG_PROFILE_NONE = 0,     /* no profile saved */
    ENG_PROFILE_APPLIED,      /* applied at this start */
    ENG_PROFILE_LATER_START,  /* not the first start this boot: nothing to do */
    ENG_PROFILE_SET_ASIDE,    /* two failed starts: moved to eng-profile.off, stock settings kept */
    ENG_PROFILE_INVALID       /* unreadable or outside the chip's lists: not applied */
} eng_profile_result_t;

/* Run once at Redux start, before govrestore. Prints one line for redux.log when it acts. */
eng_profile_result_t engprofile_at_start(void);

/* Call once the main menu has been drawn: clears the failed-start count. */
void engprofile_menu_reached(void);

/* Live writes (no persistence). 0 ok; -1 refused (not in the chip's list); -2 write or read-back failed. */
int eng_write_governor(const char *gov);
int eng_write_freq_limits(int min_khz, int max_khz);
/* cpu 1-3 only: cpu0 always stays online. */
int eng_write_core_online(int cpu, int on);

/* LED and rumble test (F5 step 5c; values from Dale's device, 2026-11-17): red LED 1 s, green
   power LED off 1 s, green back on, rumble 0.5 s. Always ends with the green LED on and the rumble
   off, even if a write fails. Logged. 0 ok, -2 a write failed. Tests: engprofile_set_batt_dir(). */
int eng_led_rumble_test(void);
void engprofile_set_batt_dir(const char *dir);
/* Tests: no sleeping. */
void engprofile_set_no_sleep(int on);

/* Save the current CPU policy as the profile. 0 ok. */
int engprofile_save(void);
/* Remove the profile (and any set-aside one). Stock settings from the next boot. 0 ok. */
int engprofile_clear(void);

typedef enum { ENG_STATE_NONE = 0, ENG_STATE_SAVED, ENG_STATE_SET_ASIDE } eng_state_t;
eng_state_t engprofile_state(void);

#endif
