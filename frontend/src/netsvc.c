#define _POSIX_C_SOURCE 200809L
#include "netsvc.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static const char *const keys[NETSVC_COUNT] = { "global.ssh", "global.samba" };
static const char *const names[NETSVC_COUNT] = { "SSH", "SMB (Samba)" };
/* The same services stock's autostart switches with each key. */
static const char *const units[NETSVC_COUNT][3] = { { "ssh", NULL, NULL }, { "smbd", "nmbd", NULL } };

#ifdef REDUX_PLATFORM_STOCK
static const char *g_path = NETSVC_CFG;
#else
static const char *g_path = "/tmp/redux.system.cfg";
#endif
static netsvc_run_fn g_run;

void netsvc_set_path(const char *p) {
#ifdef REDUX_PLATFORM_STOCK
    g_path = p ? p : NETSVC_CFG;
#else
    g_path = p ? p : "/tmp/redux.system.cfg";
#endif
}
void netsvc_set_run(netsvc_run_fn fn) { g_run = fn; }

const char *netsvc_name(int which) { return which >= 0 && which < NETSVC_COUNT ? names[which] : ""; }

static int run(const char *cmd) {
    if (g_run) return g_run(cmd);
#ifdef REDUX_PLATFORM_STOCK
    int st = system(cmd);
    return (st != -1 && WIFEXITED(st) && WEXITSTATUS(st) == 0) ? 0 : -1;
#else
    fprintf(stderr, "[netsvc] desktop: not running \"%s\"\n", cmd);
    return 0;
#endif
}

/* "global.ssh=1" -> 1 when the line is exactly that key. */
static int line_key(const char *line, const char *key, int *val) {
    size_t k = strlen(key);
    if (strncmp(line, key, k) != 0) return 0;
    const char *p = line + k;
    while (*p == ' ' || *p == '\t') p++;
    if (*p != '=') return 0;
    p++;
    while (*p == ' ' || *p == '\t') p++;
    *val = (*p == '1');
    return 1;
}

int netsvc_get(int which) {
    if (which < 0 || which >= NETSVC_COUNT) return 0;
    FILE *f = fopen(g_path, "r");
    if (!f) return 0;
    char line[512];
    int v = 0, found = 0;
    while (fgets(line, sizeof line, f))
        if (line_key(line, keys[which], &v)) found = 1;
    fclose(f);
    return found && v;
}

/* Rewrite system.cfg with key=on; every other line byte for byte. */
static int write_key(int which, int on) {
    FILE *in = fopen(g_path, "r");
    if (!in) return -1;
    char tmp[600];
    snprintf(tmp, sizeof tmp, "%s.redux-new", g_path);
    FILE *out = fopen(tmp, "w");
    if (!out) { fclose(in); return -1; }
    char line[1024];
    int done = 0, v, ok = 1;
    while (fgets(line, sizeof line, in)) {
        if (!done && line_key(line, keys[which], &v)) {
            if (fprintf(out, "%s=%d\n", keys[which], on ? 1 : 0) < 0) ok = 0;
            done = 1;
        } else if (fputs(line, out) == EOF) ok = 0;
    }
    if (ferror(in)) ok = 0;
    fclose(in);
    if (!done && fprintf(out, "%s=%d\n", keys[which], on ? 1 : 0) < 0) ok = 0;
    if (fflush(out) != 0 || fsync(fileno(out)) != 0) ok = 0;
    if (fclose(out) != 0) ok = 0;
    if (!ok || rename(tmp, g_path) != 0) { remove(tmp); return -1; }
    return 0;
}

int netsvc_set(int which, int on) {
    if (which < 0 || which >= NETSVC_COUNT) return -1;
    if (write_key(which, on) != 0) return -1;
    int rc = 0;
    for (int i = 0; units[which][i]; i++) {
        char cmd[128];
        snprintf(cmd, sizeof cmd, "systemctl %s %s", on ? "start" : "stop", units[which][i]);
        if (run(cmd) != 0) rc = -2;
    }
    return rc;
}
