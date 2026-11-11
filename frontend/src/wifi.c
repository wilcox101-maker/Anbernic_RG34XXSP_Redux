#define _POSIX_C_SOURCE 200809L
#include "wifi.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static wifi_ap_t g_ap[WIFI_MAX];
static int g_count;
static int g_truncated;
static wifi_spawn_fn g_spawn;

static void copy_trunc(char *dst, size_t n, const char *src) {
    size_t i = 0;
    if (n == 0) return;
    while (src[i] && i + 1 < n) { dst[i] = src[i]; i++; }
    dst[i] = '\0';
}

static void wipe(char *p, size_t n) {
    volatile char *v = p;
    while (n--) *v++ = 0;
}

/* Empty SECURITY field only. "--" / "none" are not assumed. */
static int security_is_open(const char *sec) {
    return sec[0] == '\0';
}

int wifi_parse_line(const char *line, wifi_ap_t *ap) {
    char field[3][WIFI_SSID_MAX];
    int fi = 0;
    size_t n = 0;
    memset(field, 0, sizeof field);
    memset(ap, 0, sizeof *ap);
    if (!line) return -1;
    for (const char *p = line; *p && *p != '\n' && *p != '\r'; p++) {
        if (*p == '\\' && p[1]) {
            p++;
            if (fi < 3 && n + 1 < sizeof field[0]) field[fi][n++] = *p;
            continue;
        }
        if (*p == ':' && fi < 2) {
            field[fi][n] = '\0';
            fi++;
            n = 0;
            continue;
        }
        if (fi < 3 && n + 1 < sizeof field[0]) field[fi][n++] = *p;
    }
    if (fi < 3) field[fi][n] = '\0';
    if (fi < 2 || !field[0][0]) return -1;   /* need SSID:SIGNAL:SECURITY; skip hidden */
    copy_trunc(ap->ssid, sizeof ap->ssid, field[0]);
    copy_trunc(ap->security, sizeof ap->security, field[2]);
    ap->signal = atoi(field[1]);
    ap->open = security_is_open(field[2]);
    return 0;
}

static int default_spawn(char *const argv[], char *out, size_t out_n, int *status) {
    int fd[2];
    if (pipe(fd) != 0) return -1;
    pid_t pid = fork();
    if (pid < 0) { close(fd[0]); close(fd[1]); return -1; }
    if (pid == 0) {
        int devnull = open("/dev/null", O_WRONLY);
        dup2(fd[1], STDOUT_FILENO);
        if (devnull >= 0) dup2(devnull, STDERR_FILENO);
        close(fd[0]); close(fd[1]);
        if (devnull >= 0) close(devnull);
        setenv("LC_ALL", "C", 1);   /* nmcli follows the locale; stock is zh */
        setenv("LANG", "C", 1);
        execv(argv[0], argv);
        _exit(127);
    }
    close(fd[1]);
    size_t got = 0;
    if (out && out_n) out[0] = '\0';
    while (out && got + 1 < out_n) {
        ssize_t r = read(fd[0], out + got, out_n - 1 - got);
        if (r <= 0) break;
        got += (size_t)r;
    }
    if (out) out[got] = '\0';
    close(fd[0]);
    int st = 0;
    if (waitpid(pid, &st, 0) < 0) return -1;
    if (status) *status = WIFEXITED(st) ? WEXITSTATUS(st) : 1;
    return 0;
}

void wifi_set_spawn(wifi_spawn_fn fn) { g_spawn = fn; }

static int run(char *const argv[], char *out, size_t n, int *status) {
    wifi_spawn_fn fn = g_spawn ? g_spawn : default_spawn;
    return fn(argv, out, n, status);
}

int wifi_scan(void) {
    g_count = 0;
    g_truncated = 0;
    char out[16384];
    int status = 1;
    char *argv[] = {
        "/usr/bin/nmcli", "-t", "-f", "SSID,SIGNAL,SECURITY", "dev", "wifi", "list", NULL
    };
    if (run(argv, out, sizeof out, &status) != 0 || status != 0) return -1;
    if (strlen(out) + 1 >= sizeof out) g_truncated = 1;
    char *line = out;
    while (*line) {
        char *nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        wifi_ap_t ap;
        if (wifi_parse_line(line, &ap) == 0) {
            int found = -1;
            for (int i = 0; i < g_count; i++)
                if (strcmp(g_ap[i].ssid, ap.ssid) == 0) { found = i; break; }
            if (found >= 0) {
                if (ap.signal > g_ap[found].signal) g_ap[found] = ap;
            } else if (g_count < WIFI_MAX) {
                g_ap[g_count++] = ap;
            } else {
                g_truncated = 1;
            }
        }
        if (!nl) break;
        line = nl + 1;
    }
    return g_count;
}

int wifi_count(void) { return g_count; }
int wifi_truncated(void) { return g_truncated; }
const wifi_ap_t *wifi_ap(int i) {
    return (i >= 0 && i < g_count) ? &g_ap[i] : NULL;
}

int wifi_connect(const char *ssid, const char *password) {
    if (!ssid || !ssid[0]) return -1;
    char ssid_buf[WIFI_SSID_MAX];
    char pass_buf[128];
    copy_trunc(ssid_buf, sizeof ssid_buf, ssid);
    int status = 1;
    int rc;
    if (password && password[0]) {
        copy_trunc(pass_buf, sizeof pass_buf, password);
        char *argv[] = {
            "/usr/bin/nmcli", "dev", "wifi", "connect", ssid_buf,
            "password", pass_buf, "ifname", "wlan0", NULL
        };
        rc = run(argv, NULL, 0, &status);
        wipe(pass_buf, sizeof pass_buf);
    } else {
        char *argv[] = {
            "/usr/bin/nmcli", "dev", "wifi", "connect", ssid_buf, "ifname", "wlan0", NULL
        };
        rc = run(argv, NULL, 0, &status);
    }
    return (rc == 0 && status == 0) ? 0 : -1;
}
