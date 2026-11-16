#define _POSIX_C_SOURCE 200809L
#include "govrestore.h"
#include <stdio.h>
#include <string.h>

static const char *g_dir = GOVRESTORE_DIR;
static const char *g_state = GOVRESTORE_STATE;

void govrestore_set_paths(const char *cpufreq_dir, const char *state_file) {
    g_dir = cpufreq_dir ? cpufreq_dir : GOVRESTORE_DIR;
    g_state = state_file ? state_file : GOVRESTORE_STATE;
}

typedef struct { char gov[32]; long min, max; } policy_t;

static int read_line(const char *name, char *buf, size_t len) {
    char path[256];
    snprintf(path, sizeof path, "%s/%s", g_dir, name);
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    int ok = fgets(buf, (int)len, f) != NULL;
    fclose(f);
    if (!ok) return -1;
    buf[strcspn(buf, "\r\n")] = '\0';
    return 0;
}

static int write_text(const char *name, const char *text) {
    char path[256];
    snprintf(path, sizeof path, "%s/%s", g_dir, name);
    FILE *f = fopen(path, "w");
    if (!f) return -1;
    int ok = fputs(text, f) >= 0;
    return (fclose(f) == 0 && ok) ? 0 : -1;
}

static int read_policy(policy_t *p) {
    char buf[64];
    if (read_line("scaling_governor", p->gov, sizeof p->gov) != 0 || !p->gov[0]) return -1;
    if (read_line("scaling_min_freq", buf, sizeof buf) != 0 || sscanf(buf, "%ld", &p->min) != 1) return -1;
    if (read_line("scaling_max_freq", buf, sizeof buf) != 0 || sscanf(buf, "%ld", &p->max) != 1) return -1;
    return 0;
}

/* word is one of the space-separated words in file `name` */
static int listed(const char *name, const char *word) {
    char buf[512];
    if (read_line(name, buf, sizeof buf) != 0) return 0;
    for (char *t = strtok(buf, " \t"); t; t = strtok(NULL, " \t"))
        if (strcmp(t, word) == 0) return 1;
    return 0;
}

static int freq_ok(long khz) {
    char w[24];
    snprintf(w, sizeof w, "%ld", khz);
    return listed("scaling_available_frequencies", w);
}

static int load_state(policy_t *p) {
    FILE *f = fopen(g_state, "r");
    if (!f) return -1;
    int n = fscanf(f, "%31s %ld %ld", p->gov, &p->min, &p->max);
    fclose(f);
    return n == 3 ? 0 : -2;
}

static int save_state(const policy_t *p) {
    FILE *f = fopen(g_state, "w");
    if (!f) return -1;
    fprintf(f, "%s %ld %ld\n", p->gov, p->min, p->max);
    return fclose(f) == 0 ? 0 : -1;
}

static int write_freq(const char *name, long khz) {
    char buf[24];
    snprintf(buf, sizeof buf, "%ld", khz);
    return write_text(name, buf);
}

gov_result_t govrestore_at_start(void) {
    policy_t now, want;
    if (read_policy(&now) != 0) {
        printf("[cpufreq] can't read %s; governor not checked\n", g_dir);
        return GOV_ERROR;
    }
    int st = load_state(&want);
    if (st == -1) {                                  /* first start this boot: baseline */
        if (save_state(&now) != 0) {
            printf("[cpufreq] can't save %s\n", g_state);
            return GOV_ERROR;
        }
        return GOV_SAVED;
    }
    if (st != 0 || !listed("scaling_available_governors", want.gov) ||
        !freq_ok(want.min) || !freq_ok(want.max) || want.min > want.max) {
        printf("[cpufreq] saved state not valid; left as is (%s %ld-%ld)\n", now.gov, now.min, now.max);
        return GOV_ERROR;
    }
    if (strcmp(now.gov, want.gov) == 0 && now.min == want.min && now.max == want.max)
        return GOV_SAME;

    int err = 0;
    if (strcmp(now.gov, want.gov) != 0) err |= write_text("scaling_governor", want.gov);
    /* max, min, max: works whichever way the window has to move */
    err |= write_freq("scaling_max_freq", want.max);
    err |= write_freq("scaling_min_freq", want.min);
    err |= write_freq("scaling_max_freq", want.max);

    policy_t after;
    int same = read_policy(&after) == 0 && strcmp(after.gov, want.gov) == 0 &&
               after.min == want.min && after.max == want.max;
    printf("[cpufreq] app left %s %ld-%ld; restored %s %ld-%ld: %s\n",
           now.gov, now.min, now.max, want.gov, want.min, want.max,
           (same && !err) ? "ok" : "FAILED");
    return (same && !err) ? GOV_RESTORED : GOV_ERROR;
}

int govrestore_rebaseline(void) {
    policy_t now;
    if (read_policy(&now) != 0) return -1;
    return save_state(&now);
}
