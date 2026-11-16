#ifndef REDUX_GOVRESTORE_H
#define REDUX_GOVRESTORE_H

/* CPU governor restore (F4 step 2, notes/2026-11-16-dale-decision-f4-plan.md).
   Redux restarts after every game or app (dmenu_ln runs /tmp/.next, then run.sh again), so the
   check lives at Redux start, which covers every launch type, including the video player's exec:
   - first start this boot (no state file in /tmp): save the governor and min/max frequency;
   - every later start: write those saved values back where an app left them different.
   It only writes values it read from this boot's sysfs, and only ones listed as available, so it
   can never set anything outside the stock OPP table (AGENTS.md hard rule). */

#define GOVRESTORE_DIR   "/sys/devices/system/cpu/cpu0/cpufreq"   /* one policy for all 4 cores */
#define GOVRESTORE_STATE "/tmp/redux-cpufreq"                      /* RAM: gone at the next boot */

/* Tests point these at a temp folder. NULL keeps the default. */
void govrestore_set_paths(const char *cpufreq_dir, const char *state_file);

typedef enum { GOV_SAVED = 0, GOV_SAME, GOV_RESTORED, GOV_ERROR } gov_result_t;

/* Run once at Redux start. Prints one line for redux.log when it restores or fails. */
gov_result_t govrestore_at_start(void);

/* Make the current policy the one restored after every app (F5: after "Save as profile").
   0 ok. */
int govrestore_rebaseline(void);

#endif
