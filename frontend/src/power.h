#ifndef REDUX_POWER_H
#define REDUX_POWER_H

/* Restart and Power Off. sync first, then reboot or poweroff.
   Desktop builds do not run the commands unless a test hook is set.
   Never writes to /mnt/mmc. */

#define POWER_REBOOT_CMD   "sync && reboot"
#define POWER_POWEROFF_CMD "sync && poweroff"

typedef int (*power_run_fn)(const char *command);

/* Tests replace the exec. NULL restores the real path (stock builds only). */
void power_set_run(power_run_fn fn);

/* 0 if the command was started. -1 otherwise.
   Desktop builds return -1 unless a test hook is set. They do not reboot. */
int power_reboot(void);
int power_poweroff(void);

/* The command the last call asked to run, or "" if none. For tests. */
const char *power_last_command(void);

#endif
