#ifndef REDUX_VOLUME_H
#define REDUX_VOLUME_H

/* Persistent lineout volume. Steps 'lineout volume' 0-31 via amixer.
   Saved to /mnt/data/redux/volume on the device, /tmp/redux.volume on desktop.
   Never writes to /mnt/mmc. Desktop builds do not run amixer unless a test
   hook is set. */

#define VOLUME_MIN 0
#define VOLUME_MAX 31

typedef int (*volume_run_fn)(const char *command);

/* Tests replace the exec. NULL restores the real path (stock builds only). */
void volume_set_run(volume_run_fn fn);

/* Tests point at a temp file. NULL restores the build default. */
void volume_set_path(const char *path);

/* Current level, 0-31. Default 31 until a file is loaded. */
int volume_get(void);

/* Step by dir (+1 or -1), clamp, apply, save. Returns the new level. */
int volume_step(int dir);

/* Load the saved level if the file exists, then apply it. */
void volume_load(void);

/* Re-apply the current level (startup and just before a game). */
int volume_apply(void);

/* The command the last apply asked to run, or "". For tests. */
const char *volume_last_command(void);

#endif
