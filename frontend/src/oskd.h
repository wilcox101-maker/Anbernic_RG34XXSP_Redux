#ifndef REDUX_OSKD_H
#define REDUX_OSKD_H

#include <stddef.h>

/* The global on-screen keyboard service (Redux OSK plan D1/D2, Dale 2026-11-18): redux-osk is
   embedded in dmenu.bin (frontend/osk/redux_osk.c), written to /tmp at start and started when
   Settings > On-screen Keyboard is On. Default Off until the K24-K28 probe passes. The choice is
   Redux's own file (/mnt/data/redux/osk: "1" on; missing or "0" off). Nothing else on the device
   changes. */

int  oskd_enabled(void);
/* Save On/Off. On: start redux-osk now. Off: stop it and clear its layer. 0 ok, -1 could not
   save, -2 saved but starting or stopping failed. */
int  oskd_set(int on);
/* At menu start: write the program to /tmp and start it when enabled (a copy already running keeps
   going; redux-osk clears a layer left by a crash when it starts). 1 if started, 0 not, -1 error. */
int  oskd_at_start(void);
const char *oskd_path(void);

/* Tests: setting file and a runner instead of exec/system(). NULL = defaults. */
typedef int (*oskd_run_fn)(const char *command);
void oskd_set_paths(const char *setting, const char *program);
void oskd_set_run(oskd_run_fn fn);

extern const unsigned char redux_osk_bin[];
extern const size_t redux_osk_bin_len;

#endif
