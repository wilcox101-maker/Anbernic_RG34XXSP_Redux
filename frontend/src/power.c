#define _POSIX_C_SOURCE 200809L
#include "power.h"
#include "powerkeys.h"
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static power_run_fn g_run;
static char g_last[64];

void power_set_run(power_run_fn fn) { g_run = fn; }
const char *power_last_command(void) { return g_last; }

static int run_cmd(const char *cmd) {
    if (strlen(cmd) + 1 > sizeof g_last) return -1;
    snprintf(g_last, sizeof g_last, "%s", cmd);
    if (g_run) return g_run(cmd);
#ifdef REDUX_PLATFORM_STOCK
    pid_t pid = fork();
    if (pid < 0) return -1;
    if (pid == 0) {
        execlp("sh", "sh", "-c", cmd, (char *)NULL);
        _exit(127);
    }
    int st = 0;
    if (waitpid(pid, &st, 0) < 0) return -1;
    return (WIFEXITED(st) && WEXITSTATUS(st) == 0) ? 0 : -1;
#else
    return -1;   /* a desktop build must not reboot the machine it is tested on */
#endif
}

/* From here on the lid and power key are ignored (a lid closed during shutdown left the
   device half-on on stock, docs/field-reports.md). */
static int shut(const char *cmd) {
    powerkeys_shutdown(1);
    int r = run_cmd(cmd);
    if (r != 0) powerkeys_shutdown(0);   /* it failed and Redux is still running: keys back */
    return r;
}
int power_reboot(void)   { return shut(POWER_REBOOT_CMD); }
int power_poweroff(void) { return shut(POWER_POWEROFF_CMD); }
