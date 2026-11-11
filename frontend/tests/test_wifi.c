/* Wi-Fi scan parsing and the nmcli argv. No radio, no SDL.
   The password is compared in memory and never printed. Exit 0 = pass. */
#include "wifi.h"
#include <stdio.h>
#include <string.h>

static int fails;
static void expect(int cond, const char *what) {
    printf("%s %s\n", cond ? "ok  " : "FAIL", what);
    if (!cond) fails++;
}

static char scan_argv_ok;
static char connect_ssid_ok;
static char connect_ifname_ok;
static char connect_has_password;
static char connect_password_ok;
static const char *expect_pw;
static int spawn_status;
static const char *scan_body;

static int spawn(char *const argv[], char *out, size_t out_n, int *status) {
    if (status) *status = spawn_status;
    if (argv[1] && strcmp(argv[1], "-t") == 0) {
        static const char *const want[] = {
            "/usr/bin/nmcli", "-t", "-f", "SSID,SIGNAL,SECURITY", "dev", "wifi", "list", NULL
        };
        scan_argv_ok = 1;
        for (int i = 0; want[i]; i++)
            if (!argv[i] || strcmp(argv[i], want[i]) != 0) scan_argv_ok = 0;
        if (argv[7] != NULL) scan_argv_ok = 0;
        if (out && out_n) snprintf(out, out_n, "%s", scan_body ? scan_body : "");
        return 0;
    }
    connect_ssid_ok = argv[0] && strcmp(argv[0], "/usr/bin/nmcli") == 0
        && argv[1] && strcmp(argv[1], "dev") == 0
        && argv[2] && strcmp(argv[2], "wifi") == 0
        && argv[3] && strcmp(argv[3], "connect") == 0
        && argv[4] && strcmp(argv[4], "Home") == 0;
    connect_has_password = 0;
    connect_password_ok = 0;
    connect_ifname_ok = 0;
    for (int i = 0; argv[i]; i++) {
        if (strcmp(argv[i], "password") == 0 && argv[i + 1]) {
            connect_has_password = 1;
            if (expect_pw && strcmp(argv[i + 1], expect_pw) == 0) connect_password_ok = 1;
        }
        if (strcmp(argv[i], "ifname") == 0 && argv[i + 1] && strcmp(argv[i + 1], "wlan0") == 0)
            connect_ifname_ok = 1;
    }
    if (!expect_pw) connect_password_ok = !connect_has_password;
    if (out && out_n) out[0] = '\0';
    return 0;
}

int main(void) {
    wifi_ap_t ap;
    expect(wifi_parse_line("Home:40:WPA2", &ap) == 0, "parse a secured line");
    expect(strcmp(ap.ssid, "Home") == 0 && ap.signal == 40 && strcmp(ap.security, "WPA2") == 0,
           "SSID, signal, security");
    expect(ap.open == 0, "non-empty security is not treated as open");
    expect(wifi_parse_line("Cafe:90:", &ap) == 0 && ap.open == 1, "empty security is open");
    expect(wifi_parse_line("a\\:b:10:WPA1", &ap) == 0 && strcmp(ap.ssid, "a:b") == 0,
           "escaped colon stays in the SSID");
    expect(wifi_parse_line(":5:WPA2", &ap) == -1, "empty SSID is skipped");
    expect(wifi_parse_line("Open--:20:--", &ap) == 0 && ap.open == 0,
           "a '--' security field is not assumed open");

    wifi_set_spawn(spawn);
    scan_body =
        "Home:40:WPA2\n"
        "Cafe:90:\n"
        "Home:70:WPA2\n"
        "a\\:b:10:WPA1\n"
        ":5:WPA2\n"
        "Open--:20:--\n";
    expect(wifi_scan() == 4, "scan keeps four networks");
    expect(scan_argv_ok, "scan argv is nmcli -t -f SSID,SIGNAL,SECURITY dev wifi list");
    expect(strcmp(wifi_ap(0)->ssid, "Home") == 0 && wifi_ap(0)->signal == 70,
           "duplicate SSID keeps the stronger signal");
    expect(strcmp(wifi_ap(1)->ssid, "Cafe") == 0 && wifi_ap(1)->open == 1, "open network kept");
    expect(strcmp(wifi_ap(2)->ssid, "a:b") == 0, "escaped SSID kept");
    expect(wifi_ap(3)->open == 0 && strcmp(wifi_ap(3)->security, "--") == 0,
           "'--' is kept as security text");

    expect_pw = "s3cret-test";
    expect(wifi_connect("Home", expect_pw) == 0, "connect returns 0 when nmcli does");
    expect(connect_ssid_ok, "connect argv starts nmcli dev wifi connect SSID");
    expect(connect_ifname_ok, "connect names ifname wlan0");
    expect(connect_has_password == 1 && connect_password_ok == 1,
           "password argument matches, and is not printed");

    expect_pw = NULL;
    expect(wifi_connect("Home", NULL) == 0, "open connect returns 0");
    expect(connect_has_password == 0 && connect_password_ok == 1,
           "no password argument when the password is empty");

    spawn_status = 1;
    expect(wifi_connect("Home", "") == -1, "non-zero nmcli status is a failure");
    expect(wifi_scan() == -1, "non-zero scan status is a failure");
    spawn_status = 0;

    char big[2048];
    int pos = 0;
    for (int i = 0; i < WIFI_MAX + 1; i++)
        pos += snprintf(big + pos, sizeof big - (size_t)pos, "N%d:10:WPA2\n", i);
    scan_body = big;
    expect(wifi_scan() == WIFI_MAX, "scan stops at WIFI_MAX");
    expect(wifi_truncated() == 1, "the extra network sets truncated");

    printf("%d failed\n", fails);
    return fails ? 1 : 0;
}
