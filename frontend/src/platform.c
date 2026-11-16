#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/* Desktop fakes: the same numbers the F0 mock-ups show. */
int platform_battery_percent(void)    { return 87; }
int platform_battery_charging(void)   { return 1; }
int platform_battery_mv(void)         { return 3920; }
int platform_battery_ma(void)         { return -420; }
int platform_battery_ma_known(void)   { return 1; }
int platform_battery_temp_known(void) { return 0; }
int platform_wifi_connected(void)     { return 1; }
int platform_cpu_temp_c(void)         { return 54; }
int platform_cpu_cur_mhz(int cpu)     { (void)cpu; return 1008; }
int platform_cpu_online(int cpu)      { (void)cpu; return 1; }

/* Engineering fakes: the CPU mock-up's values (docs/ui-spec.md mock-up 4). */
int platform_cpu_info(eng_cpu_t *c) {
    static const int opps[] = { 480, 720, 936, 1008, 1104, 1200, 1320, 1416, 1512 };   /* Dale's unit */
    memset(c, 0, sizeof *c);
    snprintf(c->governor, sizeof c->governor, "schedutil");
    c->min_mhz = 480;
    c->max_mhz = 1512;
    c->n_opps = (int)(sizeof opps / sizeof opps[0]);
    memcpy(c->opps_mhz, opps, sizeof opps);
    return 0;
}

int platform_thermal_zones(eng_zone_t *z, int max) {
    static const struct { const char *type; int x10; } fake[] = {
        { "cpu_thermal_zone", 540 }, { "gpu_thermal_zone", 520 },
        { "ve_thermal_zone", 510 }, { "ddr_thermal_zone", 500 },
    };
    int n = 0;
    for (int i = 0; i < (int)(sizeof fake / sizeof fake[0]) && n < max; i++, n++) {
        memset(&z[n], 0, sizeof z[n]);
        snprintf(z[n].type, sizeof z[n].type, "%s", fake[i].type);
        z[n].temp_x10 = fake[i].x10;
        z[n].n_trips = 2;
        snprintf(z[n].trips[0].type, sizeof z[n].trips[0].type, "passive");
        z[n].trips[0].temp_x10 = 850;
        snprintf(z[n].trips[1].type, sizeof z[n].trips[1].type, "critical");
        z[n].trips[1].temp_x10 = 1100;
    }
    return n;
}

static int fake_profile;
int platform_eng_profile_state(void) { return fake_profile; }
int platform_eng_profile_save(void)  { fake_profile = 1; return 0; }
int platform_eng_profile_clear(void) { fake_profile = 0; return 0; }

static char fixed_clock[6];

void platform_set_fixed_clock(const char *hhmm) {
    snprintf(fixed_clock, sizeof fixed_clock, "%s", hhmm ? hhmm : "");
}

void platform_clock(char *buf, int buflen) {
    if (fixed_clock[0]) { snprintf(buf, (size_t)buflen, "%s", fixed_clock); return; }
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    snprintf(buf, (size_t)buflen, "%02d:%02d", tm->tm_hour, tm->tm_min);
}
