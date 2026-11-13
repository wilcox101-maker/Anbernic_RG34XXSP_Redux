#ifndef REDUX_LAUNCHER_H
#define REDUX_LAUNCHER_H

/* F2 launcher: write /tmp/.next and return. main() exits so loadapp.sh runs it.
   Never formats a partition. Paths only. */

typedef enum {
    LAUNCH_RETROARCH,
    LAUNCH_PPSSPP,
    LAUNCH_DRASTIC,
    LAUNCH_DGE,      /* standalone .dge: core = binary path, rom optional */
    LAUNCH_PCSX,
    LAUNCH_PORTMASTER,
    LAUNCH_SCRIPT,   /* a .sh from the card, only with the Card Scripts switch On: core = script path */
    LAUNCH_COUNT
} launch_target_t;

/* Set the path written instead of /tmp/.next (for desktop tests). */
void launcher_set_next_path(const char *path);

/* Write the launch line and return 0 on success, -1 on error.
   core is used only for LAUNCH_RETROARCH (e.g. "gpsp_libretro.so").
   Does not call exit(). */
int launcher_start(launch_target_t target, const char *core, const char *rom_path);

/* A program on internal storage, run from its own folder: cd '<dir>' && <env> ./'<name>' <args>.
   env and args are fixed strings from Redux's own tables, written as given (not quoted);
   either may be NULL. Returns 0 or -1, like launcher_start. */
int launcher_start_app(const char *bin, const char *env, const char *args);

#endif
