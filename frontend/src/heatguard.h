#ifndef REDUX_HEATGUARD_H
#define REDUX_HEATGUARD_H
#include <stddef.h>
#include <stdint.h>

/* Heat guard (F5 step 4, notes/2026-11-16-claude-decision-f5-heat-guard.md, Dale A 16:12).
   There is no battery sensor, so it watches the CPU thermal zone:
   - >= 75 C for 30 s: warn and cap the CPU max at 1200 MHz (in the chip's list) until below 65 C;
   - >= 90 C for 10 s: RetroArch SAVE_FILES, sync, power off.
   Charging is never touched. The critical trip point (115 C) is never changed.
   The decision logic below is pure (tests feed it times and temperatures). The program
   frontend/heatguard/redux_heatguard.c runs it once a second, in the menu and during games. */

#define HG_CAP_AT_MC        75000
#define HG_CAP_AFTER_MS     30000
#define HG_UNCAP_BELOW_MC   65000
#define HG_OFF_AT_MC        90000
#define HG_OFF_AFTER_MS     10000
#define HG_CAP_KHZ          1200000
#define HG_FLAG             "/tmp/redux-hot"   /* exists while capped; the menu shows HOT */

typedef enum { HG_NONE = 0, HG_CAP, HG_UNCAP, HG_POWEROFF } hg_action_t;

typedef struct {
    int capped;
    int off_sent;
    uint32_t cap_since;   /* 0 = not above the cap limit; else first time seen + 1 */
    uint32_t off_since;
} hg_state_t;

/* One reading. temp_mc < 0 means unreadable: nothing changes. Returns what to do now. */
hg_action_t hg_step(hg_state_t *s, uint32_t now_ms, int temp_mc);

/* Program embedded in dmenu.bin, written to /tmp at start and started once per boot. */
int heatguard_install(void);
const char *heatguard_path(void);
void heatguard_set_path(const char *path);
/* Starts it in its own session; a second copy exits at once (lock file). 0 if started. */
int heatguard_start(void);

extern const unsigned char redux_heatguard_bin[];
extern const size_t redux_heatguard_bin_len;

#endif
