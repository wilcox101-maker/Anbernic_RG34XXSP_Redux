/* Stock platform: reads real sysfs on the RG34XXSP. UI never touches /sys. */
#include "platform.h"
#include "engprofile.h"
#include "govrestore.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>

static int read_int(const char *path, int fallback) {
    FILE *f = fopen(path, "r");
    if (!f) return fallback;
    int v = fallback;
    if (fscanf(f, "%d", &v) != 1) v = fallback;
    fclose(f);
    return v;
}

int platform_battery_percent(void) {
    return read_int("/sys/class/power_supply/axp2202-battery/capacity", 0);
}
int platform_battery_charging(void) {
    char buf[32] = {0};
    FILE *f = fopen("/sys/class/power_supply/axp2202-battery/status", "r");
    if (f) { if (!fgets(buf, sizeof buf, f)) buf[0] = 0; fclose(f); }
    return strstr(buf, "Charging") != NULL;
}
int platform_battery_mv(void) {
    return read_int("/sys/class/power_supply/axp2202-battery/voltage_now", 0) / 1000;
}
int platform_battery_ma(void) {
    return read_int("/sys/class/power_supply/axp2202-battery/current_now", 0) / 1000;
}
int platform_battery_ma_known(void) {
    /* Stock axp2202-battery has no current_now (confirmed on device 2026-11-10). */
    FILE *f = fopen("/sys/class/power_supply/axp2202-battery/current_now", "r");
    if (!f) return 0;
    fclose(f);
    return 1;
}
int platform_battery_temp_known(void) {
    /* Unverified whether the thermistor works. Return 0 until confirmed. */
    return 0;
}
int platform_wifi_connected(void) {
    return read_int("/sys/class/net/wlan0/carrier", 0);
}
int platform_cpu_temp_c(void) {
    return read_int("/sys/class/thermal/thermal_zone0/temp", 0) / 1000;
}
int platform_cpu_cur_mhz(int cpu) {
    char path[128];
    snprintf(path, sizeof path, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", cpu);
    return read_int(path, 0) / 1000;
}
int platform_cpu_online(int cpu) {
    char path[128];
    snprintf(path, sizeof path, "/sys/devices/system/cpu/cpu%d/online", cpu);
    return read_int(path, 1);
}
int platform_cpu_info(eng_cpu_t *c) { return engread_cpu(c); }
int platform_thermal_zones(eng_zone_t *z, int max) { return engread_thermal(z, max); }
int platform_eng_profile_state(void) { return (int)engprofile_state(); }
int platform_eng_profile_save(void) {
    int rc = engprofile_save();
    if (rc == 0) govrestore_rebaseline();   /* games then return to the saved policy */
    return rc;
}
int platform_eng_profile_clear(void) { return engprofile_clear(); }
int platform_eng_set_governor(const char *gov) { return eng_write_governor(gov); }
int platform_eng_set_freq_mhz(int min_mhz, int max_mhz) {
    return eng_write_freq_limits(min_mhz * 1000, max_mhz * 1000);
}
int platform_eng_set_core(int cpu, int on) { return eng_write_core_online(cpu, on); }
int platform_gpu_info(eng_gpu_t *g) { return engread_gpu(g); }
void platform_battery_extra(eng_batt_t *b) { engread_battery(b); }
void platform_mounts(eng_mount_t *m) { engread_mounts(m); }
int platform_log_tail(int which, char lines[][ENG_LOG_COLS], int max) {
    if (which == 0) return engread_tail("/mnt/vendor/redux/redux.log", lines, max);
    if (which == 1) return engread_tail("/mnt/data/redux/eng-writes.log", lines, max);
    /* dmesg: the kernel ring buffer, read only. Its output goes to /tmp (RAM), never the cards. */
    if (system("dmesg > /tmp/redux-dmesg.txt 2>&1") == -1) return 0;
    return engread_tail("/tmp/redux-dmesg.txt", lines, max);
}
int platform_fsck(const char *dev) {
    /* Only /dev/ block devices; the name goes into a shell command. */
    if (!dev || strncmp(dev, "/dev/", 5) != 0 || strspn(dev, "/abcdefghijklmnopqrstuvwxyz0123456789") != strlen(dev))
        return 8;
    char cmd[200];
    snprintf(cmd, sizeof cmd, "echo '== fsck -n %s' >> /tmp/redux-fsck.txt; fsck -n %s >> /tmp/redux-fsck.txt 2>&1", dev, dev);
    int st = system(cmd);
    if (st == -1 || !WIFEXITED(st)) return 8;
    return WEXITSTATUS(st);
}
int platform_led_rumble_test(void) { return eng_led_rumble_test(); }
int platform_hot(void) {
    FILE *f = fopen("/tmp/redux-hot", "r");   /* HG_FLAG */
    if (!f) return 0;
    fclose(f);
    return 1;
}
void platform_clock(char *buf, int buflen) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    if (!tm) { snprintf(buf, buflen, "--:--"); return; }
    snprintf(buf, buflen, "%02d:%02d", tm->tm_hour, tm->tm_min);
}
void platform_set_fixed_clock(const char *hhmm) { (void)hhmm; }
