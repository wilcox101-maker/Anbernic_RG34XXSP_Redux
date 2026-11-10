#ifndef REDUX_LAUNCHER_H
#define REDUX_LAUNCHER_H

/* F2 launcher: start stock emulators the way dmenu.bin does and return on exit.
   All paths are relative to the stock card layout. Never formats a partition. */

typedef enum {
    LAUNCH_RETROARCH,
    LAUNCH_PPSSPP,
    LAUNCH_DRASTIC,
    LAUNCH_DGE,      /* standalone .dge emulators */
    LAUNCH_PCSX,
    LAUNCH_PORTMASTER,
    LAUNCH_COUNT
} launch_target_t;

/* Write the chosen app to /tmp/.next (stock convention) and exit so loadapp.sh runs it.
   On return the launcher is restarted. */
int launcher_start(launch_target_t target, const char *rom_path);

/* Resume the last game if /mnt/mod/.last exists. */
int launcher_resume_last(void);

#endif
