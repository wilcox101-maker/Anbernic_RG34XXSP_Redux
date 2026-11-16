/* redux-heatguard (F5 step 4, notes/2026-11-16-claude-decision-f5-heat-guard.md, Dale A).
   Started once per boot by Redux (a lock file stops a second copy) and runs in its own session, so
   it keeps watching in the menu and during every game or app. Once a second it reads the CPU
   thermal zone and runs hg_step():
   - cap: CPU max to the highest listed speed <= 1200 MHz (and min down to it if needed), through the
     Engineering write layer (validated against the chip's list, read back, logged in eng-writes.log).
     While capped, a max raised by someone else (govrestore after a game, Engineering) is capped again.
   - uncap: the min/max seen before the cap come back.
   - power off: RetroArch SAVE_FILES on UDP 55355 (ignored if no game runs), sync, poweroff.
   It never touches charging, thermal trip points or anything on the cards.
   Usage: redux-heatguard (no arguments). */
#define _DEFAULT_SOURCE
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include "engprofile.h"
#include "heatguard.h"

#define CPUFREQ "/sys/devices/system/cpu/cpu0/cpufreq/"
#define LOCK    "/tmp/redux-heatguard.lock"

static int read_text(const char *path, char *buf, size_t n) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    int ok = fgets(buf, (int)n, f) != NULL;
    fclose(f);
    if (!ok) return -1;
    buf[strcspn(buf, "\r\n")] = '\0';
    return 0;
}

static int read_int(const char *path) {
    char buf[32];
    return read_text(path, buf, sizeof buf) == 0 ? atoi(buf) : -1;
}

/* The zone whose type is cpu_thermal_zone; zone0 if none says so (Dale's unit: zone0). */
static int find_cpu_zone(void) {
    for (int z = 0; z < 16; z++) {
        char path[96], type[48];
        snprintf(path, sizeof path, "/sys/class/thermal/thermal_zone%d/type", z);
        if (read_text(path, type, sizeof type) == 0 && strncmp(type, "cpu", 3) == 0) return z;
    }
    return 0;
}

/* Highest listed speed at or below limit_khz; -1 if the list is unreadable. */
static int cap_khz(int limit_khz) {
    char buf[512];
    if (read_text(CPUFREQ "scaling_available_frequencies", buf, sizeof buf) != 0) return -1;
    int best = -1;
    for (char *t = strtok(buf, " "); t; t = strtok(NULL, " ")) {
        int v = atoi(t);
        if (v > 0 && v <= limit_khz && v > best) best = v;
    }
    return best;
}

static uint32_t now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000u + ts.tv_nsec / 1000000u);
}

static void save_files(void) {
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) return;
    struct sockaddr_in a;
    memset(&a, 0, sizeof a);
    a.sin_family = AF_INET;
    a.sin_port = htons(55355);
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    static const char cmd[] = "SAVE_FILES";
    sendto(s, cmd, sizeof cmd - 1, 0, (struct sockaddr *)&a, sizeof a);
    close(s);
}

static void flag(const char *text) {
    FILE *f = fopen(HG_FLAG, "w");
    if (f) { fputs(text, f); fclose(f); }
}

int main(void) {
    int lock = open(LOCK, O_CREAT | O_RDWR, 0644);
    if (lock < 0 || flock(lock, LOCK_EX | LOCK_NB) != 0) return 0;   /* already running */
    remove(HG_FLAG);

    char zone_path[96];
    snprintf(zone_path, sizeof zone_path, "/sys/class/thermal/thermal_zone%d/temp", find_cpu_zone());
    fprintf(stderr, "[heatguard] watching %s: cap at %d C for %d s, power off at %d C for %d s\n",
            zone_path, HG_CAP_AT_MC / 1000, HG_CAP_AFTER_MS / 1000, HG_OFF_AT_MC / 1000, HG_OFF_AFTER_MS / 1000);

    hg_state_t s;
    memset(&s, 0, sizeof s);
    int saved_min = -1, saved_max = -1, cap = -1;
    for (;;) {
        int t = read_int(zone_path);
        hg_action_t a = hg_step(&s, now_ms(), t);
        if (a == HG_CAP) {
            saved_min = read_int(CPUFREQ "scaling_min_freq");
            saved_max = read_int(CPUFREQ "scaling_max_freq");
            cap = cap_khz(HG_CAP_KHZ);
            fprintf(stderr, "[heatguard] CPU %d.%d C for %d s: capping max at %d kHz (was %d)\n",
                    t / 1000, (t % 1000) / 100, HG_CAP_AFTER_MS / 1000, cap, saved_max);
            flag("capped\n");
        }
        if (s.capped && cap > 0) {
            int mn = read_int(CPUFREQ "scaling_min_freq"), mx = read_int(CPUFREQ "scaling_max_freq");
            if (mx > cap) eng_write_freq_limits(mn > cap ? cap : mn, cap);
        }
        if (a == HG_UNCAP) {
            fprintf(stderr, "[heatguard] CPU %d.%d C: cap lifted, back to %d-%d kHz\n",
                    t / 1000, (t % 1000) / 100, saved_min, saved_max);
            if (saved_min > 0 && saved_max > 0) eng_write_freq_limits(saved_min, saved_max);
            remove(HG_FLAG);
        }
        if (a == HG_POWEROFF) {
            fprintf(stderr, "[heatguard] CPU %d.%d C for %d s: saving and powering off\n",
                    t / 1000, (t % 1000) / 100, HG_OFF_AFTER_MS / 1000);
            flag("poweroff\n");
            save_files();
            sleep(2);
            sync();
            execl("/bin/sh", "sh", "-c", "sync && poweroff", (char *)NULL);
            perror("[heatguard] poweroff");
        }
        sleep(1);
    }
}
