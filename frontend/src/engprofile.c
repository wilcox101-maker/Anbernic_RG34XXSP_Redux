#define _POSIX_C_SOURCE 200809L
#include "engprofile.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define PATH_N 256
static char g_cpu[PATH_N] = ENG_CPUFREQ;
static char g_data[PATH_N] = ENG_DATA_DIR;
static char g_flag[PATH_N] = ENG_BOOT_FLAG;
static char g_cpuroot[PATH_N] = "/sys/devices/system/cpu";

void engprofile_set_cpu_root(const char *cpu_root) {
    snprintf(g_cpuroot, sizeof g_cpuroot, "%s", cpu_root ? cpu_root : "/sys/devices/system/cpu");
}

void engprofile_set_paths(const char *cpufreq_dir, const char *data_dir, const char *boot_flag) {
    snprintf(g_cpu, sizeof g_cpu, "%s", cpufreq_dir ? cpufreq_dir : ENG_CPUFREQ);
    snprintf(g_data, sizeof g_data, "%s", data_dir ? data_dir : ENG_DATA_DIR);
    snprintf(g_flag, sizeof g_flag, "%s", boot_flag ? boot_flag : ENG_BOOT_FLAG);
}

static void data_path(char *out, size_t n, const char *name) {
    snprintf(out, n, "%s/%s", g_data, name);
}

/* ---- small file helpers ---- */

static int read_line_at(const char *path, char *buf, size_t len) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    int ok = fgets(buf, (int)len, f) != NULL;
    fclose(f);
    if (!ok) return -1;
    buf[strcspn(buf, "\r\n")] = '\0';
    return 0;
}

static int cpu_read(const char *name, char *buf, size_t len) {
    char path[PATH_N * 2];
    snprintf(path, sizeof path, "%s/%s", g_cpu, name);
    return read_line_at(path, buf, len);
}

static int cpu_write(const char *name, const char *text) {
    char path[PATH_N * 2];
    snprintf(path, sizeof path, "%s/%s", g_cpu, name);
    FILE *f = fopen(path, "w");
    if (!f) return -1;
    int ok = fputs(text, f) >= 0;
    return (fclose(f) == 0 && ok) ? 0 : -1;
}

static int ensure_dir(void) {
    if (mkdir(g_data, 0755) != 0 && errno != EEXIST) return -1;
    return 0;
}

/* temp file + fsync + rename: a crash leaves the old file or the new one, never half of one. */
static int write_atomic(const char *name, const char *text) {
    char path[PATH_N * 2], tmp[PATH_N * 2 + 8];
    if (ensure_dir() != 0) return -1;
    data_path(path, sizeof path, name);
    snprintf(tmp, sizeof tmp, "%s.tmp", path);
    int fd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return -1;
    size_t len = strlen(text);
    int ok = write(fd, text, len) == (ssize_t)len && fsync(fd) == 0;
    if (close(fd) != 0) ok = 0;
    if (!ok || rename(tmp, path) != 0) { remove(tmp); return -1; }
    return 0;
}

static int data_exists(const char *name) {
    char path[PATH_N * 2];
    struct stat st;
    data_path(path, sizeof path, name);
    return stat(path, &st) == 0;
}

static void data_remove(const char *name) {
    char path[PATH_N * 2];
    data_path(path, sizeof path, name);
    remove(path);
}

/* ---- write log ---- */

static void log_write(const char *what, const char *old_v, const char *new_v, const char *result) {
    char path[PATH_N * 2], old_path[PATH_N * 2 + 4];
    if (ensure_dir() != 0) return;
    data_path(path, sizeof path, "eng-writes.log");
    struct stat st;
    if (stat(path, &st) == 0 && st.st_size > ENG_LOG_MAX_BYTES) {
        snprintf(old_path, sizeof old_path, "%s.1", path);
        rename(path, old_path);
    }
    FILE *f = fopen(path, "a");
    if (!f) return;
    char when[32] = "?";
    time_t t = time(NULL);
    struct tm tmv;
    if (localtime_r(&t, &tmv)) strftime(when, sizeof when, "%Y-%m-%d %H:%M:%S", &tmv);
    fprintf(f, "%s %s %s -> %s %s\n", when, what, old_v, new_v, result);
    fclose(f);
}

/* ---- validation against the chip's own lists ---- */

static int listed(const char *name, const char *word) {
    char buf[512];
    if (cpu_read(name, buf, sizeof buf) != 0) return 0;
    for (char *save = NULL, *t = strtok_r(buf, " \t", &save); t; t = strtok_r(NULL, " \t", &save))
        if (strcmp(t, word) == 0) return 1;
    return 0;
}

static int freq_listed(int khz) {
    char w[24];
    snprintf(w, sizeof w, "%d", khz);
    return listed("scaling_available_frequencies", w);
}

static int read_int(const char *name, int *v) {
    char buf[32];
    if (cpu_read(name, buf, sizeof buf) != 0) return -1;
    return sscanf(buf, "%d", v) == 1 ? 0 : -1;
}

/* ---- live writes ---- */

int eng_write_governor(const char *gov) {
    char old_v[32] = "?", now[32] = "";
    cpu_read("scaling_governor", old_v, sizeof old_v);
    if (!gov || !gov[0] || strlen(gov) >= sizeof now || !listed("scaling_available_governors", gov)) {
        log_write("cpu.governor", old_v, gov ? gov : "(null)", "refused");
        return -1;
    }
    int err = cpu_write("scaling_governor", gov);
    int ok = !err && cpu_read("scaling_governor", now, sizeof now) == 0 && strcmp(now, gov) == 0;
    log_write("cpu.governor", old_v, gov, ok ? "ok" : "FAILED");
    return ok ? 0 : -2;
}

int eng_write_freq_limits(int min_khz, int max_khz) {
    int old_min = -1, old_max = -1;
    read_int("scaling_min_freq", &old_min);
    read_int("scaling_max_freq", &old_max);
    char old_v[48], new_v[48];
    snprintf(old_v, sizeof old_v, "%d-%d", old_min, old_max);
    snprintf(new_v, sizeof new_v, "%d-%d", min_khz, max_khz);
    if (min_khz > max_khz || !freq_listed(min_khz) || !freq_listed(max_khz)) {
        log_write("cpu.freq_khz", old_v, new_v, "refused");
        return -1;
    }
    char a[24], b[24];
    snprintf(a, sizeof a, "%d", max_khz);
    snprintf(b, sizeof b, "%d", min_khz);
    /* max, min, max: works whichever way the window has to move */
    int err = cpu_write("scaling_max_freq", a);
    err |= cpu_write("scaling_min_freq", b);
    err |= cpu_write("scaling_max_freq", a);
    int now_min = -1, now_max = -1;
    read_int("scaling_min_freq", &now_min);
    read_int("scaling_max_freq", &now_max);
    int ok = !err && now_min == min_khz && now_max == max_khz;
    log_write("cpu.freq_khz", old_v, new_v, ok ? "ok" : "FAILED");
    return ok ? 0 : -2;
}

static int core_path(char *out, size_t n, int cpu) {
    return snprintf(out, n, "%s/cpu%d/online", g_cpuroot, cpu) < (int)n ? 0 : -1;
}

static int core_online(int cpu) {
    char path[PATH_N * 2], buf[8];
    if (cpu == 0) return 1;
    if (core_path(path, sizeof path, cpu) != 0 || read_line_at(path, buf, sizeof buf) != 0) return -1;
    return buf[0] == '1';
}

int eng_write_core_online(int cpu, int on) {
    char what[24], old_v[8], new_v[8];
    snprintf(what, sizeof what, "cpu%d.online", cpu);
    int was = (cpu >= 1 && cpu <= 3) ? core_online(cpu) : -1;
    snprintf(old_v, sizeof old_v, "%d", was);
    snprintf(new_v, sizeof new_v, "%d", on ? 1 : 0);
    if (cpu < 1 || cpu > 3 || was < 0) {           /* cpu0 never goes offline */
        log_write(what, old_v, new_v, "refused");
        return -1;
    }
    char path[PATH_N * 2];
    core_path(path, sizeof path, cpu);
    FILE *f = fopen(path, "w");
    int err = !f;
    if (f) { err = fputs(on ? "1" : "0", f) < 0; if (fclose(f) != 0) err = 1; }
    int ok = !err && core_online(cpu) == (on ? 1 : 0);
    log_write(what, old_v, new_v, ok ? "ok" : "FAILED");
    return ok ? 0 : -2;
}

/* ---- profile ---- */

typedef struct { char gov[32]; int min, max, cores; } profile_t;   /* cores: bit n = cpu n online */

static int load_profile(const char *name, profile_t *p) {
    char path[PATH_N * 2], buf[128];
    data_path(path, sizeof path, name);
    if (read_line_at(path, buf, sizeof buf) != 0) return -1;
    p->cores = 0xF;                                  /* older profiles: all cores online */
    int n = sscanf(buf, "cpu %31s %d %d cores %x", p->gov, &p->min, &p->max, &p->cores);
    if (n < 3) return -2;
    p->cores = (p->cores & 0xF) | 1;                 /* cpu0 is always online */
    return 0;
}

int engprofile_save(void) {
    profile_t p;
    if (cpu_read("scaling_governor", p.gov, sizeof p.gov) != 0 ||
        read_int("scaling_min_freq", &p.min) != 0 || read_int("scaling_max_freq", &p.max) != 0) {
        log_write("profile", "-", "save", "FAILED (cpufreq unreadable)");
        return -1;
    }
    p.cores = 1;
    for (int c = 1; c <= 3; c++) if (core_online(c) == 1) p.cores |= 1 << c;
    char text[128];
    snprintf(text, sizeof text, "cpu %s %d %d cores %x\n", p.gov, p.min, p.max, p.cores);
    int rc = write_atomic("eng-profile", text);
    if (rc == 0) {
        data_remove("eng-profile.off");
        data_remove("eng-starts");
    }
    char new_v[96];
    snprintf(new_v, sizeof new_v, "%s %d-%d", p.gov, p.min, p.max);
    log_write("profile.save", "-", new_v, rc == 0 ? "ok" : "FAILED");
    return rc;
}

int engprofile_clear(void) {
    int had = data_exists("eng-profile") || data_exists("eng-profile.off");
    data_remove("eng-profile");
    data_remove("eng-profile.off");
    data_remove("eng-starts");
    log_write("profile.clear", had ? "saved" : "none", "none", "ok");
    return (data_exists("eng-profile") || data_exists("eng-profile.off")) ? -1 : 0;
}

eng_state_t engprofile_state(void) {
    if (data_exists("eng-profile")) return ENG_STATE_SAVED;
    if (data_exists("eng-profile.off")) return ENG_STATE_SET_ASIDE;
    return ENG_STATE_NONE;
}

static int read_starts(void) {
    char path[PATH_N * 2], buf[16];
    data_path(path, sizeof path, "eng-starts");
    if (read_line_at(path, buf, sizeof buf) != 0) return 0;
    int n = atoi(buf);
    return n < 0 ? 0 : n;
}

eng_profile_result_t engprofile_at_start(void) {
    struct stat st;
    if (stat(g_flag, &st) == 0) return ENG_PROFILE_LATER_START;
    FILE *f = fopen(g_flag, "w");                    /* mark the first start of this boot */
    if (f) fclose(f);

    if (!data_exists("eng-profile")) return ENG_PROFILE_NONE;

    int starts = read_starts();
    if (starts >= ENG_MAX_FAILED_STARTS) {
        char path[PATH_N * 2], off[PATH_N * 2 + 8];
        data_path(path, sizeof path, "eng-profile");
        data_path(off, sizeof off, "eng-profile.off");
        rename(path, off);
        data_remove("eng-starts");
        log_write("profile", "saved", "set aside", "two failed starts");
        printf("[eng] profile set aside after %d failed starts; stock settings\n", starts);
        return ENG_PROFILE_SET_ASIDE;
    }
    char cnt[16];
    snprintf(cnt, sizeof cnt, "%d\n", starts + 1);
    write_atomic("eng-starts", cnt);                 /* counted before applying: a hang still counts */

    profile_t p;
    if (load_profile("eng-profile", &p) != 0) {
        printf("[eng] profile unreadable; not applied\n");
        return ENG_PROFILE_INVALID;
    }
    if (!listed("scaling_available_governors", p.gov) || !freq_listed(p.min) || !freq_listed(p.max)
        || p.min > p.max) {
        printf("[eng] profile not in this chip's lists (%s %d-%d); not applied\n", p.gov, p.min, p.max);
        log_write("profile", "saved", "apply", "refused (not in the chip's lists)");
        return ENG_PROFILE_INVALID;
    }
    int err = eng_write_freq_limits(p.min, p.max) != 0;
    err |= eng_write_governor(p.gov) != 0;
    for (int c = 1; c <= 3; c++) {
        int want = (p.cores >> c) & 1;
        if (core_online(c) != want) err |= eng_write_core_online(c, want) != 0;
    }
    printf("[eng] profile applied: %s %d-%d kHz: %s\n", p.gov, p.min, p.max, err ? "FAILED" : "ok");
    return err ? ENG_PROFILE_INVALID : ENG_PROFILE_APPLIED;
}

void engprofile_menu_reached(void) {
    if (data_exists("eng-starts")) data_remove("eng-starts");
}
