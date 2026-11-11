#ifndef REDUX_STOCKMENU_H
#define REDUX_STOCKMENU_H

/* Restore the stock launcher. The command is docs/dale-scripts.md S7:
     cp /mnt/vendor/ctrl/dmenu_ln.bak /mnt/vendor/ctrl/dmenu_ln && sync && reboot
   Refuses when dmenu_ln.bak is missing, same as deploy_frontend.py --restore.
   S7 copies (the .bak stays). deploy --restore moves it; this menu follows S7.
   Writes /mnt/vendor/ctrl/dmenu_ln. Dale-only. Not run on the device yet. */

#define STOCK_RESTORE_CMD \
    "cp /mnt/vendor/ctrl/dmenu_ln.bak /mnt/vendor/ctrl/dmenu_ln && sync && reboot"

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
