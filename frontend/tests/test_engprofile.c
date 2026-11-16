/* Engineering safety net (F5 step 2) against temp folders. Nothing under /sys or /mnt is touched. */
#include "engprofile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int fails;
static void expect(int cond, const char *what) {
    printf("%s %s\n", cond ? "ok  " : "FAIL", what);
    if (!cond) fails++;
}

static char cpu[256], data[256], flag[256], cpus[256];

static void put(const char *dir, const char *name, const char *text) {
    char p[512];
    snprintf(p, sizeof p, "%s/%s", dir, name);
    FILE *f = fopen(p, "w");
    if (!f) { perror(p); exit(2); }
    fputs(text, f);
    fclose(f);
}

static void get(const char *dir, const char *name, char *buf, size_t n) {
    char p[512];
    snprintf(p, sizeof p, "%s/%s", dir, name);
    buf[0] = '\0';
    FILE *f = fopen(p, "r");
    if (!f) return;
    if (!fgets(buf, (int)n, f)) buf[0] = '\0';
    fclose(f);
    buf[strcspn(buf, "\n")] = '\0';
}

static int exists(const char *dir, const char *name) {
    char p[512];
    struct stat st;
    snprintf(p, sizeof p, "%s/%s", dir, name);
    return stat(p, &st) == 0;
}

static void stock_cpu(void) {
    put(cpu, "scaling_available_governors", "interactive conservative ondemand userspace powersave performance schedutil\n");
    put(cpu, "scaling_available_frequencies", "480000 600000 792000 1008000 1200000 1296000 1416000 1512000 \n");
    put(cpu, "scaling_governor", "schedutil\n");
    put(cpu, "scaling_min_freq", "480000\n");
    put(cpu, "scaling_max_freq", "1512000\n");
}

static void cores_on(void) {
    char d[300];
    for (int c = 1; c <= 3; c++) {
        snprintf(d, sizeof d, "%s/cpu%d", cpus, c);
        mkdir(d, 0755);
        put(d, "online", "1\n");
    }
}

static int core(int c) {
    char d[300], buf[8];
    snprintf(d, sizeof d, "%s/cpu%d", cpus, c);
    get(d, "online", buf, sizeof buf);
    return buf[0] == '1';
}

static void new_boot(void) { remove(flag); }

static int log_has(const char *needle) {
    char p[512], line[256];
    snprintf(p, sizeof p, "%s/eng-writes.log", data);
    FILE *f = fopen(p, "r");
    int found = 0;
    if (!f) return 0;
    while (fgets(line, sizeof line, f)) if (strstr(line, needle)) found = 1;
    fclose(f);
    return found;
}

int main(void) {
    char base[] = "/tmp/redux-engprofile-XXXXXX";
    if (!mkdtemp(base)) { perror("mkdtemp"); return 2; }
    snprintf(cpu, sizeof cpu, "%s/cpufreq", base);
    snprintf(data, sizeof data, "%s/data", base);   /* created on first write */
    snprintf(flag, sizeof flag, "%s/started", base);
    snprintf(cpus, sizeof cpus, "%s/cpus", base);
    mkdir(cpu, 0755);
    mkdir(cpus, 0755);
    engprofile_set_paths(cpu, data, flag);
    engprofile_set_cpu_root(cpus);
    cores_on();
    stock_cpu();
    char buf[128];

    /* No profile: nothing applied, nothing written to data. */
    expect(engprofile_at_start() == ENG_PROFILE_NONE, "no profile: none");
    expect(engprofile_at_start() == ENG_PROFILE_LATER_START, "second start of the same boot does nothing");
    expect(engprofile_state() == ENG_STATE_NONE, "state none");

    /* Live writes are limited to the chip's lists. */
    expect(eng_write_governor("performance") == 0, "listed governor written");
    get(cpu, "scaling_governor", buf, sizeof buf);
    expect(strcmp(buf, "performance") == 0, "governor read back");
    expect(eng_write_governor("turbo") == -1, "unlisted governor refused");
    expect(eng_write_freq_limits(600000, 1200000) == 0, "listed min/max written");
    get(cpu, "scaling_max_freq", buf, sizeof buf);
    expect(strcmp(buf, "1200000") == 0, "max read back");
    expect(eng_write_freq_limits(480000, 1608000) == -1, "1608 MHz (above the stock table) refused");
    expect(eng_write_freq_limits(1200000, 600000) == -1, "min above max refused");
    get(cpu, "scaling_max_freq", buf, sizeof buf);
    expect(strcmp(buf, "1200000") == 0, "refused write left the value alone");
    expect(log_has("cpu.governor schedutil -> performance ok"), "log: governor old -> new");
    expect(log_has("1608000 refused"), "log: refused write");

    /* Save, then a new boot applies it. */
    expect(engprofile_save() == 0, "profile saved");
    get(data, "eng-profile", buf, sizeof buf);
    expect(strcmp(buf, "cpu performance 600000 1200000 cores f") == 0, "profile contents");
    expect(!exists(data, "eng-profile.tmp"), "no temp file left");
    expect(engprofile_state() == ENG_STATE_SAVED, "state saved");
    stock_cpu();
    new_boot();
    expect(engprofile_at_start() == ENG_PROFILE_APPLIED, "new boot applies the profile");
    get(cpu, "scaling_governor", buf, sizeof buf);
    expect(strcmp(buf, "performance") == 0, "governor applied");
    get(cpu, "scaling_min_freq", buf, sizeof buf);
    expect(strcmp(buf, "600000") == 0, "min applied");
    get(data, "eng-starts", buf, sizeof buf);
    expect(strcmp(buf, "1") == 0, "start counted before the menu");
    engprofile_menu_reached();
    expect(!exists(data, "eng-starts"), "menu reached clears the count");

    /* Two failed starts: the third boot sets the profile aside and keeps stock settings. */
    stock_cpu();
    new_boot();
    expect(engprofile_at_start() == ENG_PROFILE_APPLIED, "failed start 1 (no menu)");
    new_boot();
    expect(engprofile_at_start() == ENG_PROFILE_APPLIED, "failed start 2 (no menu)");
    stock_cpu();
    new_boot();
    expect(engprofile_at_start() == ENG_PROFILE_SET_ASIDE, "third boot sets the profile aside");
    get(cpu, "scaling_governor", buf, sizeof buf);
    expect(strcmp(buf, "schedutil") == 0, "stock settings kept");
    expect(engprofile_state() == ENG_STATE_SET_ASIDE && exists(data, "eng-profile.off"), "profile kept as .off");
    new_boot();
    expect(engprofile_at_start() == ENG_PROFILE_NONE, "next boot: no profile applied");
    expect(log_has("set aside two failed starts"), "log: set aside");

    /* A profile outside the chip's lists is never applied (e.g. hand-edited). */
    put(data, "eng-profile", "cpu performance 480000 1608000\n");
    new_boot();
    expect(engprofile_at_start() == ENG_PROFILE_INVALID, "out-of-table profile refused");
    get(cpu, "scaling_max_freq", buf, sizeof buf);
    expect(strcmp(buf, "1512000") == 0, "max untouched");
    put(data, "eng-profile", "garbage\n");
    new_boot();
    expect(engprofile_at_start() == ENG_PROFILE_INVALID, "unreadable profile refused");

    /* Cores: cpu0 never goes offline; 1-3 can, and the profile keeps the mask. */
    expect(eng_write_core_online(0, 0) == -1, "cpu0 offline refused");
    expect(eng_write_core_online(4, 0) == -1, "cpu4 (no such core) refused");
    expect(eng_write_core_online(2, 0) == 0 && !core(2), "cpu2 offline written");
    expect(log_has("cpu2.online 1 -> 0 ok"), "log: core change");
    engprofile_clear();
    expect(engprofile_save() == 0, "profile with cpu2 offline saved");
    get(data, "eng-profile", buf, sizeof buf);
    expect(strstr(buf, " cores b") != NULL, "mask b (cpu0, 1, 3)");
    cores_on();
    stock_cpu();
    new_boot();
    expect(engprofile_at_start() == ENG_PROFILE_APPLIED && !core(2) && core(1) && core(3),
           "new boot takes cpu2 offline again");
    engprofile_menu_reached();
    put(data, "eng-profile", "cpu schedutil 480000 1512000\n");
    new_boot();
    expect(engprofile_at_start() == ENG_PROFILE_APPLIED && core(2), "older profile (no mask) brings all cores online");
    put(data, "eng-profile", "cpu schedutil 480000 1512000 cores 0\n");
    cores_on();
    new_boot();
    expect(engprofile_at_start() == ENG_PROFILE_APPLIED && !core(1) && !core(2) && !core(3),
           "mask 0 is read as cpu0 only (cpu0 can't go offline)");
    engprofile_menu_reached();

    /* Clear removes saved and set-aside profiles. */
    put(data, "eng-profile.off", "cpu schedutil 480000 1512000\n");
    expect(engprofile_clear() == 0 && engprofile_state() == ENG_STATE_NONE, "clear removes both");

    printf("%s\n", fails ? "FAILED" : "all passed");
    return fails ? 1 : 0;
}
