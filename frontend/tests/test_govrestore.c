/* Governor restore against a fake cpufreq folder (F4 step 2). No device. Exit 0 = pass. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "govrestore.h"

#define DIR_ "test_govrestore.d"
#define STATE "test_govrestore.state"
static int fails;
static void expect(int ok, const char *what) { printf("%s %s\n", ok ? "ok  " : "FAIL", what); if (!ok) fails++; }

static void put(const char *name, const char *text) {
    char p[128]; snprintf(p, sizeof p, DIR_ "/%s", name);
    FILE *f = fopen(p, "w"); fputs(text, f); fclose(f);
}
static void get(const char *name, char *buf, int len) {
    char p[128]; snprintf(p, sizeof p, DIR_ "/%s", name);
    FILE *f = fopen(p, "r"); buf[0] = 0; if (f) { if (!fgets(buf, len, f)) buf[0] = 0; fclose(f); }
    buf[strcspn(buf, "\n")] = 0;
}
static void policy(const char *gov, const char *min, const char *max) {
    put("scaling_governor", gov); put("scaling_min_freq", min); put("scaling_max_freq", max);
}

int main(void) {
    mkdir(DIR_, 0755);
    remove(STATE);
    put("scaling_available_governors", "ondemand userspace powersave performance schedutil\n");
    put("scaling_available_frequencies", "480000 600000 792000 1008000 1200000 1296000 1416000 1512000 \n");
    policy("schedutil\n", "480000\n", "1512000\n");
    govrestore_set_paths(DIR_, STATE);

    expect(govrestore_at_start() == GOV_SAVED, "first start saves the baseline");
    expect(govrestore_at_start() == GOV_SAME, "nothing changed: no write");

    policy("performance\n", "1416000\n", "1512000\n");             /* an app pinned it high */
    expect(govrestore_at_start() == GOV_RESTORED, "app changed governor and min: restored");
    char b[64];
    get("scaling_governor", b, sizeof b); expect(strcmp(b, "schedutil") == 0, "governor back to schedutil");
    get("scaling_min_freq", b, sizeof b); expect(strcmp(b, "480000") == 0, "min back to 480000");
    get("scaling_max_freq", b, sizeof b); expect(strcmp(b, "1512000") == 0, "max back to 1512000");

    policy("schedutil\n", "480000\n", "600000\n");                  /* an app capped it low */
    expect(govrestore_at_start() == GOV_RESTORED, "app lowered max: restored");
    get("scaling_max_freq", b, sizeof b); expect(strcmp(b, "1512000") == 0, "max back up");

    /* A tampered state outside the table is never written. */
    FILE *f = fopen(STATE, "w"); fputs("schedutil 480000 1800000\n", f); fclose(f);
    policy("schedutil\n", "480000\n", "1008000\n");
    expect(govrestore_at_start() == GOV_ERROR, "state above the OPP table: refused");
    get("scaling_max_freq", b, sizeof b); expect(strcmp(b, "1008000") == 0, "nothing written");
    f = fopen(STATE, "w"); fputs("turbo 480000 1512000\n", f); fclose(f);
    expect(govrestore_at_start() == GOV_ERROR, "unknown governor: refused");
    f = fopen(STATE, "w"); fputs("garbage\n", f); fclose(f);
    expect(govrestore_at_start() == GOV_ERROR, "unreadable state: refused");

    /* No cpufreq folder (desktop): error, nothing saved. */
    remove(STATE);
    govrestore_set_paths("no_such_dir", STATE);
    expect(govrestore_at_start() == GOV_ERROR, "no cpufreq: error");
    f = fopen(STATE, "r"); expect(f == NULL, "no state saved"); if (f) fclose(f);

    remove(STATE);
    const char *files[] = { "scaling_available_governors", "scaling_available_frequencies",
                            "scaling_governor", "scaling_min_freq", "scaling_max_freq" };
    for (unsigned i = 0; i < sizeof files / sizeof *files; i++) {
        char p[128]; snprintf(p, sizeof p, DIR_ "/%s", files[i]); remove(p);
    }
    rmdir(DIR_);
    printf("%d failed\n", fails);
    return fails ? 1 : 0;
}
