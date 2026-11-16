/* Writes the embedded redux-heatguard to /tmp and starts it in its own session (F5 step 4). */
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "heatguard.h"

static const char *g_path;

void heatguard_set_path(const char *path) { g_path = path; }
const char *heatguard_path(void) { return g_path ? g_path : "/tmp/redux-heatguard"; }

int heatguard_install(void) {
    const char *path = heatguard_path();
    char tmp[256];
    snprintf(tmp, sizeof tmp, "%s.new", path);
    FILE *f = fopen(tmp, "wb");
    if (!f) return -1;
    size_t w = fwrite(redux_heatguard_bin, 1, redux_heatguard_bin_len, f);
    if (fclose(f) != 0 || w != redux_heatguard_bin_len) { remove(tmp); return -1; }
    if (chmod(tmp, 0755) != 0 || rename(tmp, path) != 0) { remove(tmp); return -1; }
    return 0;
}

int heatguard_start(void) {
    pid_t pid = fork();
    if (pid < 0) return -1;
    if (pid == 0) {
        /* Double fork: the guard is not Redux's child, so it outlives Redux and leaves no zombie. */
        if (setsid() < 0) _exit(1);
        if (fork() != 0) _exit(0);
        execl(heatguard_path(), heatguard_path(), (char *)NULL);
        _exit(127);
    }
    int st = 0;
    waitpid(pid, &st, 0);
    return (WIFEXITED(st) && WEXITSTATUS(st) == 0) ? 0 : -1;
}
