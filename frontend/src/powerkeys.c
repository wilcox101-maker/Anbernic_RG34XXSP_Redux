#include "powerkeys.h"
#include "remap_device.h"

static long long boot_until, quiet_until, closed_since;
static int lid_closed, lid_acted, shutting_down;

void powerkeys_init(long long now_ms) {
    boot_until = now_ms + POWERKEYS_BOOT_IGNORE_MS;
    quiet_until = 0;
    closed_since = 0;
    lid_closed = lid_acted = shutting_down = 0;
}

pk_action_t powerkeys_event(int code, int value, long long now_ms) {
    if (shutting_down) return PK_NONE;
    if (code == DEV_KEY_LID_TAP) return PK_NONE;           /* part of the lid-open sequence */
    if (code == DEV_KEY_LID) {
        if (value == 1 && !lid_closed) {                     /* close; repeats (2) change nothing */
            lid_closed = 1;
            lid_acted = now_ms < boot_until;   /* closed at boot: wait for open + close */
            closed_since = now_ms;
        } else if (value == 0) {                             /* open (also cancels a bounce) */
            lid_closed = 0;
            lid_acted = 0;
        }
        return PK_NONE;
    }
    if (value == 1 && now_ms >= quiet_until) return PK_POWER;
    return PK_NONE;
}

pk_action_t powerkeys_due(long long now_ms) {
    if (shutting_down || !lid_closed || lid_acted) return PK_NONE;
    if (now_ms - closed_since < POWERKEYS_LID_DEBOUNCE_MS) return PK_NONE;
    lid_acted = 1;
    return PK_LID_CLOSED;
}

int powerkeys_wait_ms(long long now_ms) {
    if (shutting_down || !lid_closed || lid_acted) return -1;
    long long at = closed_since + POWERKEYS_LID_DEBOUNCE_MS;
    return at <= now_ms ? 0 : (int)(at - now_ms);
}

void powerkeys_resumed(long long now_ms) {
    /* The queued lid-open events were thrown away with the rest, so don't wait for them.
       A lid still closed only auto-repeats (ignored), so it can't send the device back to sleep. */
    lid_closed = 0;
    lid_acted = 0;
    quiet_until = now_ms + POWERKEYS_QUIET_MS;
}

void powerkeys_shutdown(int on) { shutting_down = on; }
