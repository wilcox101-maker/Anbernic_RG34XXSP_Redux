#ifndef REDUX_WIFI_H
#define REDUX_WIFI_H

#include <stddef.h>

/* Wi-Fi setup through /usr/bin/nmcli (Claude, inbox #41, 2026-11-11).
   The password exists only as an argument to that one connect call and is
   wiped afterwards. Nothing in here prints it or writes it to a file.
   Profiles under /etc/NetworkManager/system-connections/ are not read. */

#define WIFI_MAX 32
#define WIFI_SSID_MAX 64

typedef struct {
    char ssid[WIFI_SSID_MAX];
    char security[32];
    int signal;
    /* 1 only when the SECURITY field is empty. Any other text is not treated
       as open. Which strings nmcli prints on this device is not verified. */
    int open;
} wifi_ap_t;

/* Replace the nmcli spawn. out receives stdout (may be NULL). status is the
   exit code. Tests use this so CI never needs a radio. NULL restores nmcli. */
typedef int (*wifi_spawn_fn)(char *const argv[], char *out, size_t out_n, int *status);
void wifi_set_spawn(wifi_spawn_fn fn);

/* Parse one `nmcli -t` line. Returns 0, or -1 if the line is skipped. */
int wifi_parse_line(const char *line, wifi_ap_t *ap);

/* `nmcli -t -f SSID,SIGNAL,SECURITY dev wifi list` with LC_ALL=C in the child.
   Returns the number of networks, or -1 if nmcli did not exit 0. */
int wifi_scan(void);
int wifi_count(void);
int wifi_truncated(void);
const wifi_ap_t *wifi_ap(int i);

/* Connect to ssid. A saved profile matching the SSID plus a typed password
   uses `nmcli con modify` + `nmcli con up`; otherwise `nmcli dev wifi connect`.
   nmcli can exit 0 on activation failure, so output containing "Error" is a
   failure, and success also requires device show to report STATE starting
   with 100 and the asked-for SSID. IP4.ADDRESS is read only after that.
   The password is never printed. Not verified on the device. */
int wifi_connect(const char *ssid, const char *password);
const char *wifi_last_ip(void);
const char *wifi_last_reason(void);

#endif
