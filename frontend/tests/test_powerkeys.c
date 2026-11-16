/* Power key and lid logic on event0 (F4 step 1). No device. Exit 0 = pass. */
#include <stdio.h>
#include "powerkeys.h"
#include "remap_device.h"

#define KEY_POWER_CODE 116
static int fails;
static void expect(int ok, const char *what) { printf("%s %s\n", ok ? "ok  " : "FAIL", what); if (!ok) fails++; }

int main(void) {
    long long t = 100000;
    powerkeys_init(t);
    t += POWERKEYS_BOOT_IGNORE_MS;

    /* Lid close in the menu: the lid path, not the power-key path. */
    expect(powerkeys_event(DEV_KEY_LID, 1, t) == PK_NONE, "lid close is not a power press");
    expect(powerkeys_due(t) == PK_NONE, "lid not due before debounce");
    expect(powerkeys_wait_ms(t) == POWERKEYS_LID_DEBOUNCE_MS, "wait = debounce left");
    expect(powerkeys_event(DEV_KEY_LID, 2, t + 100) == PK_NONE, "auto-repeat ignored");
    expect(powerkeys_due(t + POWERKEYS_LID_DEBOUNCE_MS) == PK_LID_CLOSED, "lid due after debounce");
    expect(powerkeys_due(t + 5000) == PK_NONE, "lid acts once per close");
    expect(powerkeys_wait_ms(t + 5000) == -1, "no wait limit after acting");

    /* Wake: queued events were drained; a still-closed lid only repeats. */
    t += 10000;
    powerkeys_resumed(t);
    expect(powerkeys_event(DEV_KEY_LID, 2, t + 10) == PK_NONE, "repeat after wake: nothing");
    expect(powerkeys_due(t + 2000) == PK_NONE, "repeat after wake: no second sleep");
    expect(powerkeys_event(KEY_POWER_CODE, 1, t + 100) == PK_NONE, "power quiet just after wake");
    expect(powerkeys_event(KEY_POWER_CODE, 1, t + POWERKEYS_QUIET_MS) == PK_POWER, "power works after quiet");

    /* Lid open sequence never counts as a power press. */
    expect(powerkeys_event(DEV_KEY_LID, 0, t + 3000) == PK_NONE, "lid open: nothing");
    expect(powerkeys_event(DEV_KEY_LID_TAP, 1, t + 3010) == PK_NONE, "open tap (KEY_DELETE) down ignored");
    expect(powerkeys_event(DEV_KEY_LID_TAP, 0, t + 3020) == PK_NONE, "open tap up ignored");

    /* Bounce: close then open inside the debounce. */
    t += 5000;
    powerkeys_event(DEV_KEY_LID, 1, t);
    powerkeys_event(DEV_KEY_LID, 0, t + 50);
    expect(powerkeys_due(t + 1000) == PK_NONE, "bounce under debounce: no sleep");

    /* Power key: press only; release and repeat do nothing. */
    expect(powerkeys_event(KEY_POWER_CODE, 0, t + 2000) == PK_NONE, "power release: nothing");
    expect(powerkeys_event(KEY_POWER_CODE, 2, t + 2000) == PK_NONE, "power repeat: nothing");

    /* Closed at boot (stuck sensor): ignored until it opens and closes again. */
    t = 500000;
    powerkeys_init(t);
    powerkeys_event(DEV_KEY_LID, 1, t + 100);
    expect(powerkeys_due(t + 60000) == PK_NONE, "lid closed at boot: no sleep");
    powerkeys_event(DEV_KEY_LID, 0, t + 61000);
    powerkeys_event(DEV_KEY_LID, 1, t + 62000);
    expect(powerkeys_due(t + 62000 + POWERKEYS_LID_DEBOUNCE_MS) == PK_LID_CLOSED, "after boot: re-close sleeps");

    /* Shutdown: nothing acts. */
    powerkeys_init(t);
    t += POWERKEYS_BOOT_IGNORE_MS;
    powerkeys_event(DEV_KEY_LID, 1, t);
    powerkeys_shutdown(1);
    expect(powerkeys_due(t + 1000) == PK_NONE, "shutdown: lid ignored");
    expect(powerkeys_event(KEY_POWER_CODE, 1, t + 1000) == PK_NONE, "shutdown: power ignored");
    powerkeys_shutdown(0);
    expect(powerkeys_event(KEY_POWER_CODE, 1, t + 2000) == PK_POWER, "failed shutdown: power back");

    printf("%d failed\n", fails);
    return fails ? 1 : 0;
}
