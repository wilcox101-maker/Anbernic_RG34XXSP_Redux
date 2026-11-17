/* See btchip.h. */
#define _POSIX_C_SOURCE 200809L
#include "btchip.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef REDUX_PLATFORM_STOCK
#define SETTING_PATH "/mnt/data/redux/bluetooth"
#else
#define SETTING_PATH "/tmp/redux.bluetooth"
#endif

static const char *g_setting = SETTING_PATH;
static const char *g_hci = BTCHIP_HCI;
static btchip_run_fn g_run;

void btchip_set_paths(const char *setting, const char *hci) {
    g_setting = setting ? setting : SETTING_PATH;
    g_hci = hci ? hci : BTCHIP_HCI;
}
void btchip_set_run(btchip_run_fn fn) { g_run = fn; }

static int run(const char *cmd) {
    if (g_run) return g_run(cmd);
#ifdef REDUX_PLATFORM_STOCK
    int st = system(cmd);
    return (st != -1 && WIFEXITED(st) && WEXITSTATUS(st) == 0) ? 0 : -1;
#else
    fprintf(stderr, "[btchip] desktop: not running \"%s\"\n", cmd);
    return 0;
#endif
}

static int hci_present(void) {
    struct stat st;
    return stat(g_hci, &st) == 0;
}

int btchip_enabled(void) {
    FILE *f = fopen(g_setting, "r");
    if (!f) return 1;   /* missing: on, as stock */
    int c = fgetc(f);
    fclose(f);
    return c != '0';
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

int btchip_set(int on) {
    if (save(on) != 0) return -1;
    if (on) {
        if (hci_present()) return 0;
        return run(BTCHIP_INIT " >/dev/null 2>&1") == 0 ? 0 : -2;
    }
    if (!hci_present()) return 0;
    return run("hciconfig hci0 down >/dev/null 2>&1") == 0 ? 0 : -2;
}

int btchip_at_start(void) {
    if (!btchip_enabled() || hci_present()) return 0;
    /* In the background: the script waits for the chip (seconds) and the menu must not. */
    run(BTCHIP_INIT " >/dev/null 2>&1 &");
    return 1;
}
