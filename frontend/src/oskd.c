/* See oskd.h. */
#define _POSIX_C_SOURCE 200809L
#include "oskd.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef REDUX_PLATFORM_STOCK
#define SETTING_PATH "/mnt/data/redux/osk"
#else
#define SETTING_PATH "/tmp/redux.osk"
#endif
#define PROGRAM_PATH "/tmp/redux-osk"
#define PID_PATH     "/tmp/redux-osk.pid"

static const char *g_setting = SETTING_PATH;
static const char *g_program = PROGRAM_PATH;
static oskd_run_fn g_run;

void oskd_set_paths(const char *setting, const char *program) {
    g_setting = setting ? setting : SETTING_PATH;
    g_program = program ? program : PROGRAM_PATH;
}
void oskd_set_run(oskd_run_fn fn) { g_run = fn; }
const char *oskd_path(void) { return g_program; }

static int run(const char *cmd) {
    if (g_run) return g_run(cmd);
#ifdef REDUX_PLATFORM_STOCK
    int st = system(cmd);
    return (st != -1 && WIFEXITED(st) && WEXITSTATUS(st) == 0) ? 0 : -1;
#else
    fprintf(stderr, "[oskd] desktop: not running \"%s\"\n", cmd);
    return 0;
#endif
}

int oskd_enabled(void) {
    FILE *f = fopen(g_setting, "r");
    if (!f) return 0;   /* missing: off (default Off until the probe passes, Dale D2 A) */
    int c = fgetc(f);
    fclose(f);
    return c == '1';
}

static int save(int on) {
    char tmp[300];
    snprintf(tmp, sizeof tmp, "%s.new", g_setting);
#ifdef REDUX_PLATFORM_STOCK
    if (mkdir("/mnt/data/redux", 0755) != 0 && errno != EEXIST) return -1;
#endif
    FILE *f = fopen(tmp, "w");
    if (!f) return -1;
    fputs(on ? "1\n" : "0\n", f);
    if (fclose(f) != 0 || rename(tmp, g_setting) != 0) { remove(tmp); return -1; }
    return 0;
}

static int install(void) {
    if (redux_osk_bin_len == 0) return -1;
    char tmp[300];
    snprintf(tmp, sizeof tmp, "%s.new", g_program);
    FILE *f = fopen(tmp, "wb");
    if (!f) return -1;
    size_t w = fwrite(redux_osk_bin, 1, redux_osk_bin_len, f);
    if (fclose(f) != 0 || w != redux_osk_bin_len) { remove(tmp); return -1; }
    if (chmod(tmp, 0755) != 0 || rename(tmp, g_program) != 0) { remove(tmp); return -1; }
    return 0;
}

/* The program detaches itself from Redux (setsid + background), so it outlives the menu while a
   game runs; a second copy exits at once (pid file check in redux-osk). */
static int start(void) {
    char cmd[400];
    snprintf(cmd, sizeof cmd, "setsid %s >/dev/null 2>&1 </dev/null &", g_program);
    return run(cmd);
}

static int stop(void) {
    char cmd[600];
    snprintf(cmd, sizeof cmd, "p=$(cat %s 2>/dev/null) && kill \"$p\" 2>/dev/null; sleep 0.2; %s --off >/dev/null 2>&1",
             PID_PATH, g_program);
    return run(cmd);
}

int oskd_set(int on) {
    if (save(on) != 0) return -1;
    if (on) return (install() == 0 && start() == 0) ? 0 : -2;
    return stop() == 0 ? 0 : -2;
}

int oskd_at_start(void) {
    if (install() != 0) return -1;
    if (!oskd_enabled()) return 0;
    return start() == 0 ? 1 : -1;
}
