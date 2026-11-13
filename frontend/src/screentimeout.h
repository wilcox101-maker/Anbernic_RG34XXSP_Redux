#ifndef REDUX_SCREENTIMEOUT_H
#define REDUX_SCREENTIMEOUT_H

#include <stdint.h>

/* Screen timeout (queue 9g). After this long with no button press in the Redux menu, Redux runs
   stock `pwr_new.sh auto`, the stock screen-timeout path: it suspends (stock power.lock=0),
   shuts down with power.lock=1, and does nothing on HDMI with power.hdmi=1. The power key wakes.
   Games and stock apps keep their own timeouts; this covers only the Redux menu.
   The choices and the default live here so a later builder can change them.
   Saved to /mnt/data/redux/screen_timeout (minutes) on the device, /tmp/redux.screen_timeout on
   desktop. */

#define SCREENTIMEOUT_CHOICES { 0, 1, 2, 3, 5, 10, 15, 30 }   /* minutes; 0 = Off */
#define SCREENTIMEOUT_DEFAULT 5                               /* docs/ui-spec.md: 300 s */

/* Tests point at a temp file. NULL restores the build default. */
void screentimeout_set_path(const char *path);

/* Current timeout in minutes (0 = Off). */
int screentimeout_minutes(void);

/* Move to the next or previous choice (dir +1 / -1), clamped, and save. Returns the minutes. */
int screentimeout_step(int dir);

/* Load the saved choice; a missing or unknown value keeps the default. */
void screentimeout_load(void);

/* 1 when the timeout is on and idle_ms has reached it. */
int screentimeout_due(uint32_t idle_ms);

#endif
