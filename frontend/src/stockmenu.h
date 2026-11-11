#ifndef REDUX_STOCKMENU_H
#define REDUX_STOCKMENU_H

/* Restore the stock launcher. The command is docs/dale-scripts.md S7.
   Copy to a temp file on the same filesystem, then rename over dmenu_ln.
   The running script is not rewritten in place. Refuses when dmenu_ln.bak
   is missing. The .bak stays. Writes /mnt/vendor/ctrl/dmenu_ln.
   Dale-only. Not run on the device yet. */

#define STOCK_RESTORE_CMD \
    "cp /mnt/vendor/ctrl/dmenu_ln.bak /mnt/vendor/ctrl/dmenu_ln.new && sync && mv -f /mnt/vendor/ctrl/dmenu_ln.new /mnt/vendor/ctrl/dmenu_ln && sync && reboot"

typedef int (*stock_run_fn)(const char *command);

/* Tests replace the exec. NULL restores the real path (stock builds only). */
void stock_set_run(stock_run_fn fn);

/* -1: stat the real backup. 0 or 1: force the answer (tests). */
void stock_set_bak(int present);

/* 1 if dmenu_ln.bak is present. */
int stock_bak_present(void);

/* 0 if the S7 command was started. -2 if the backup is missing. -1 otherwise.
   Desktop builds return -1 unless a test hook is set. They do not reboot. */
int stock_restore(void);

#endif
