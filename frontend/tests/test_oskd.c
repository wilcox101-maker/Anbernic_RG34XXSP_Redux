/* Global on-screen keyboard service (src/oskd.c) against a temp folder and a fake runner: no device. */
#define _POSIX_C_SOURCE 200809L
#include "oskd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int fails;
static void expect(int ok, const char *what) { printf("%s %s\n", ok ? "ok  " : "FAIL", what); if (!ok) fails++; }

static char cmds[8][700];
static int ncmd, run_rc;
static int fake_run(const char *c) { if (ncmd < 8) snprintf(cmds[ncmd++], sizeof cmds[0], "%s", c); return run_rc; }

int main(void) {
    char dir[] = "/tmp/redux-oskd-test-XXXXXX";
    if (!mkdtemp(dir)) { perror("mkdtemp"); return 1; }
    char setting[300], program[300];
    snprintf(setting, sizeof setting, "%s/osk", dir);
    snprintf(program, sizeof program, "%s/redux-osk", dir);
    oskd_set_paths(setting, program);
    oskd_set_run(fake_run);

    expect(!oskd_enabled(), "no setting file: off (default Off until the probe passes)");
    expect(oskd_at_start() == 0 && ncmd == 0, "start while off: nothing run");
    struct stat st;
    expect(stat(program, &st) == 0 && (size_t)st.st_size == redux_osk_bin_len && (st.st_mode & 0111),
           "the program is written out, complete and executable");

    expect(oskd_set(1) == 0 && oskd_enabled(), "On: saved");
    expect(ncmd == 1 && strstr(cmds[0], "setsid ") && strstr(cmds[0], program) && strstr(cmds[0], " &"),
           "On: started in its own session, in the background");
    ncmd = 0;
    expect(oskd_at_start() == 1 && ncmd == 1 && strstr(cmds[0], "setsid "), "start while on: started (a running copy keeps going)");

    ncmd = 0;
    expect(oskd_set(0) == 0 && !oskd_enabled(), "Off: saved");
    expect(ncmd == 1 && strstr(cmds[0], "kill") && strstr(cmds[0], "--off"), "Off: stopped and its layer cleared");

    ncmd = 0; run_rc = -1;
    expect(oskd_set(1) == -2 && oskd_enabled(), "start failure: -2, setting still saved");
    run_rc = 0;

    char bad[300];
    snprintf(bad, sizeof bad, "%s/missing-dir/osk", dir);
    oskd_set_paths(bad, program);
    expect(oskd_set(1) == -1, "unwritable setting: -1");

    oskd_set_paths(setting, program);
    remove(setting); remove(program);
    char nw[320]; snprintf(nw, sizeof nw, "%s.new", setting); remove(nw);
    rmdir(dir);
    printf("%s\n", fails ? "test_oskd: FAILED" : "test_oskd: all passed");
    return fails ? 1 : 0;
}
