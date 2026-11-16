/* Heat guard decision logic (F5 step 4): times and temperatures in, actions out. */
#include "heatguard.h"
#include <stdio.h>
#include <string.h>

static int fails;
static void expect(int cond, const char *what) {
    printf("%s %s\n", cond ? "ok  " : "FAIL", what);
    if (!cond) fails++;
}

/* Feed one reading a second from t0 for n seconds; returns the first non-NONE action and its time. */
static hg_action_t run(hg_state_t *s, uint32_t *t, int n, int mc, uint32_t *at) {
    hg_action_t first = HG_NONE;
    for (int i = 0; i < n; i++, *t += 1000) {
        hg_action_t a = hg_step(s, *t, mc);
        if (a != HG_NONE && first == HG_NONE) { first = a; if (at) *at = *t; }
    }
    return first;
}

int main(void) {
    hg_state_t s;
    uint32_t t = 0, at = 0;

    memset(&s, 0, sizeof s);
    expect(run(&s, &t, 600, 64900, NULL) == HG_NONE, "Dale's 10 min peak (64.9 C) does nothing");
    expect(run(&s, &t, 29, 76000, NULL) == HG_NONE, "75+ C for 29 s: not yet");
    expect(run(&s, &t, 2, 76000, &at) == HG_CAP && s.capped, "75+ C for 30 s caps");
    expect(run(&s, &t, 60, 70000, NULL) == HG_NONE && s.capped, "stays capped at 70 C (above 65)");
    expect(run(&s, &t, 1, 64900, NULL) == HG_UNCAP && !s.capped, "below 65 C lifts the cap");

    memset(&s, 0, sizeof s); t = 0;
    run(&s, &t, 20, 76000, NULL);
    run(&s, &t, 1, 74000, NULL);
    expect(run(&s, &t, 20, 76000, NULL) == HG_NONE, "a dip below 75 C restarts the 30 s count");

    memset(&s, 0, sizeof s); t = 0;
    expect(run(&s, &t, 5, -1, NULL) == HG_NONE, "unreadable temperature: nothing happens");

    memset(&s, 0, sizeof s); t = 0;
    hg_action_t a = run(&s, &t, 9, 91000, NULL);
    expect(a == HG_NONE, "90+ C for 9 s: no power off yet");
    expect(run(&s, &t, 2, 91000, &at) == HG_POWEROFF, "90+ C for 10 s powers off");
    expect(run(&s, &t, 30, 91000, NULL) != HG_POWEROFF, "power off is sent once");

    memset(&s, 0, sizeof s); t = 0;
    run(&s, &t, 8, 91000, NULL);
    run(&s, &t, 1, 89000, NULL);
    expect(run(&s, &t, 9, 91000, NULL) == HG_NONE, "a dip below 90 C restarts the 10 s count");

    memset(&s, 0, sizeof s); t = 0xFFFFF000u;   /* counter wrap near 49.7 days */
    expect(run(&s, &t, 31, 76000, NULL) == HG_CAP, "caps across a millisecond counter wrap");

    printf("%s\n", fails ? "FAILED" : "all passed");
    return fails ? 1 : 0;
}
