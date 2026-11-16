#ifndef REDUX_POWERKEYS_H
#define REDUX_POWERKEYS_H

/* Power key and lid on event0, as plain logic (no device, testable on the desktop).
   Feed it every event0 EV_KEY event; it says when to run stock's power-key path or its
   lid/timeout path (pwr_new.sh / pwr_new.sh auto). F4 step 1, notes/2026-11-16-dale-decision-f4-plan.md. */

#define POWERKEYS_BOOT_IGNORE_MS 5000   /* a lid closing this soon after start is ignored until
                                           it opens and closes again (stuck sensor at boot) */
#define POWERKEYS_LID_DEBOUNCE_MS 300   /* lid must stay closed this long before it counts */
#define POWERKEYS_QUIET_MS       1500   /* after a sleep returns, ignore the power key this long */

typedef enum { PK_NONE = 0, PK_POWER, PK_LID_CLOSED } pk_action_t;

/* Start (or restart) the state; now_ms = monotonic milliseconds. */
void powerkeys_init(long long now_ms);

/* One event0 key event (value 0 up, 1 down, 2 auto-repeat). Returns PK_POWER for a power press. */
pk_action_t powerkeys_event(int code, int value, long long now_ms);

/* Call every loop: PK_LID_CLOSED once the lid has been closed long enough. */
pk_action_t powerkeys_due(long long now_ms);

/* Longest the caller may sleep before powerkeys_due() can change (-1 = no limit). */
int powerkeys_wait_ms(long long now_ms);

/* A sleep just returned (events were drained): the lid counts as open until it closes again,
   and the power key is quiet for POWERKEYS_QUIET_MS. */
void powerkeys_resumed(long long now_ms);

/* on = 1: Redux is rebooting or powering off, ignore everything. on = 0: that failed, resume. */
void powerkeys_shutdown(int on);

#endif
