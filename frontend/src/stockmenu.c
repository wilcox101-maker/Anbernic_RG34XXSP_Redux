#define _POSIX_C_SOURCE 200809L
#include "stockmenu.h"
#include <sys/wait.h>
#include <unistd.h>

static stock_run_fn g_run;
static int g_bak = -1;   /* -1: look at the real path */

void stock_set_run(stock_run_fn fn) { g_run = fn; }
void stock_set_bak(int present) { g_bak = present; }

int stock_bak_present(void) {
    if (g_bak >= 0) return g_bak ? 1 : 0;
    return access("/mnt/vendor/ctrl/dmenu_ln.bak", R_OK) == 0;
}

int stock_restore(void) {
    if (!stock_bak_present()) return -2;
    if (g_run) return g_run(STOCK_RESTORE_CMD);
#ifdef REDUX_PLATFORM_STOCK
    pid_t pid = fork();
    if (pid < 0) return -1;
    if (pid == 0) {
        execlp("sh", "sh", "-c", STOCK_RESTORE_CMD, (char *)NULL);
        _exit(127);
    }
    int st = 0;
    if (waitpid(pid, &st, 0) < 0) return -1;
    return (WIFEXITED(st) && WEXITSTATUS(st) == 0) ? 0 : -1;
#else
    return -1;   /* a desktop build must not reboot the machine it is tested on */
#endif
}
