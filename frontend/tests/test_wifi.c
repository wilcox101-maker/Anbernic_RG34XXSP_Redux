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
static const char *connect_body;
static char saw_ip_query;

static const char *profile_body;   /* NULL = no profiles; else NAME list */
static const char *verify_body;    /* NULL = on Home, in the real `nmcli -t device show` format */
static char saw_modify;
static char saw_up;
static char modify_password_ok;

static int spawn(char *const argv[], char *out, size_t out_n, int *status) {
    if (argv[1] && strcmp(argv[1], "con") == 0 && argv[2] && strcmp(argv[2], "modify") == 0) {
        saw_modify = 1;
        modify_password_ok = 0;
        for (int i = 0; argv[i]; i++)
            if (strcmp(argv[i], "wifi-sec.psk") == 0 && argv[i + 1]
                && expect_pw && strcmp(argv[i + 1], expect_pw) == 0)
                modify_password_ok = 1;
        if (status) *status = 0;
        if (out && out_n) out[0] = '\0';
        return 0;
    }
    if (argv[1] && strcmp(argv[1], "con") == 0 && argv[2] && strcmp(argv[2], "up") == 0) {
        saw_up = 1;
        if (status) *status = spawn_status;
        if (out && out_n) snprintf(out, out_n, "%s", connect_body ? connect_body : "");
        return 0;
    }
    if (argv[3] && strcmp(argv[3], "GENERAL.STATE,GENERAL.CONNECTION") == 0) {
        if (status) *status = 0;
        if (out && out_n)
            snprintf(out, out_n, "%s", verify_body ? verify_body : "GENERAL.STATE:100 (connected)\nGENERAL.CONNECTION:Home\n");
        return 0;
    }
    if (argv[3] && strcmp(argv[3], "NAME") == 0) {
        if (status) *status = 0;
        if (out && out_n) snprintf(out, out_n, "%s", profile_body ? profile_body : "");
        return 0;
    }
    if (argv[3] && strcmp(argv[3], "IP4.ADDRESS") == 0) {
        saw_ip_query = 1;
        if (status) *status = 0;
        if (out && out_n) snprintf(out, out_n, "IP4.ADDRESS[1]:10.0.0.8/24\n");
        return 0;
    }
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
    if (out && out_n) snprintf(out, out_n, "%s", connect_body ? connect_body : "");
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
    expect(saw_ip_query == 1, "a successful connect asks nmcli for the address");
    expect(strcmp(wifi_last_ip(), "10.0.0.8") == 0, "the address drops the prefix length");
    expect(wifi_last_reason()[0] == '\0', "success has no failure reason");

    expect_pw = NULL;
    expect(wifi_connect("Home", NULL) == 0, "open connect returns 0");
    expect(connect_has_password == 0 && connect_password_ok == 1,
           "no password argument when the password is empty");

    spawn_status = 1;
    connect_body = "Error: Secrets were required, but not provided.\n";
    expect(wifi_connect("Home", "") == -1, "non-zero nmcli status is a failure");
    expect(strcmp(wifi_last_reason(), "Secrets were required, but not provided.") == 0,
           "failure keeps nmcli's reason");
    expect(wifi_last_ip()[0] == '\0', "failure has no address");
    connect_body = "Error: bad s3cret-test password\n";
    expect(wifi_connect("Home", "s3cret-test") == -1, "a reason that echoes the password is refused");
    expect(strcmp(wifi_last_reason(), "failed") == 0, "the password is not kept in the reason");
    connect_body = NULL;
    expect(wifi_scan() == -1, "non-zero scan status is a failure");
    spawn_status = 0;

    char big[2048];
    int pos = 0;
    for (int i = 0; i < WIFI_MAX + 1; i++)
        pos += snprintf(big + pos, sizeof big - (size_t)pos, "N%d:10:WPA2\n", i);
    scan_body = big;
    expect(wifi_scan() == WIFI_MAX, "scan stops at WIFI_MAX");
    expect(wifi_truncated() == 1, "the extra network sets truncated");

    /* nmcli exits 0 but prints Error: treat as failure, do not read IP. */
    spawn_status = 0;
    connect_body = "Error: Connection activation failed: (7) Secrets were required, but not provided.\n";
    saw_ip_query = 0;
    expect(wifi_connect("Home", NULL) == -1, "exit 0 with Error text is a failure");
    expect(saw_ip_query == 0, "Error text skips the address query");
    expect(strstr(wifi_last_reason(), "Secrets were required") != NULL,
           "Error text becomes the reason");

    /* verify fails (wrong network or not 100): failure even if connect printed nothing. */
    connect_body = "";
    verify_body = "GENERAL.STATE:30 (disconnected)\nGENERAL.CONNECTION:\n";
    expect(wifi_connect("Home", NULL) == -1, "STATE not 100 is a failure");
    expect(strcmp(wifi_last_reason(), "not connected") == 0, "verify failure reason");
    verify_body = "GENERAL.STATE:100 (connected)\nGENERAL.CONNECTION:Other\n";
    expect(wifi_connect("Home", NULL) == -1, "connected to a different SSID is a failure");
    /* The old "100:Home" shape (no field names) is not what the device prints: not a success. */
    verify_body = "100:Home\n";
    expect(wifi_connect("Home", NULL) == -1, "unlabelled 100:Home is not taken as connected");
    verify_body = NULL;

    /* saved profile + password: modify then up, then verify. */
    profile_body = "DLN_Guest\nHome\nDLN_TeamWireless\n";
    expect_pw = "s3cret-test";
    saw_modify = 0; saw_up = 0; saw_ip_query = 0;
    connect_body = "";
    expect(wifi_connect("Home", expect_pw) == 0, "saved profile uses modify + up");
    expect(saw_modify == 1 && modify_password_ok == 1, "modify sets the typed password");
    expect(saw_up == 1, "modify is followed by con up");
    expect(saw_ip_query == 1, "verified profile connect reads the address");
    expect(strcmp(wifi_last_ip(), "10.0.0.8") == 0, "profile connect keeps the address");
    profile_body = NULL;

    printf("%d failed\n", fails);
    return fails ? 1 : 0;
}
