/* Heat guard decision logic (pure). See heatguard.h. */
#include "heatguard.h"

static uint32_t mark(uint32_t now_ms) { return now_ms + 1; }   /* 0 means "not started" */

hg_action_t hg_step(hg_state_t *s, uint32_t now_ms, int temp_mc) {
    if (temp_mc < 0) return HG_NONE;

    /* Power off: >= 90 C held for 10 s. Sent once. */
    if (temp_mc >= HG_OFF_AT_MC) {
        if (!s->off_since) s->off_since = mark(now_ms);
        else if (!s->off_sent && now_ms + 1 - s->off_since >= HG_OFF_AFTER_MS) {
            s->off_sent = 1;
            return HG_POWEROFF;
        }
    } else {
        s->off_since = 0;
    }

    if (s->capped) {
        if (temp_mc < HG_UNCAP_BELOW_MC) {
            s->capped = 0;
            s->cap_since = 0;
            return HG_UNCAP;
        }
        return HG_NONE;
    }
    if (temp_mc >= HG_CAP_AT_MC) {
        if (!s->cap_since) s->cap_since = mark(now_ms);
        else if (now_ms + 1 - s->cap_since >= HG_CAP_AFTER_MS) {
            s->capped = 1;
            return HG_CAP;
        }
    } else {
        s->cap_since = 0;
    }
    return HG_NONE;
}
