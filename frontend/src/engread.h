#ifndef REDUX_ENGREAD_H
#define REDUX_ENGREAD_H

/* Engineering Mode read layer (F5 step 1). Reads /sys only; never writes.
   A test can point engread_set_root() at a fixture directory. */

#define ENG_MAX_OPPS   16
#define ENG_MAX_ZONES  12
#define ENG_MAX_TRIPS   6
#define ENG_NAME       24

typedef struct {
    char governor[ENG_NAME];     /* "" if unreadable */
    int  min_mhz, max_mhz;       /* current scaling limits; -1 if unreadable */
    int  opps_mhz[ENG_MAX_OPPS]; /* the chip's own list, ascending */
    int  n_opps;                 /* 0 if the list is unreadable */
    char govs[8][ENG_NAME];      /* scaling_available_governors, in the kernel's order */
    int  n_govs;
} eng_cpu_t;

typedef struct {
    char type[ENG_NAME];
    int  temp_x10;               /* tenths of a degree; -1 if unreadable */
} eng_trip_t;

typedef struct {
    char type[ENG_NAME];
    int  temp_x10;               /* tenths of a degree; -1 if unreadable */
    int  n_trips;
    eng_trip_t trips[ENG_MAX_TRIPS];
} eng_zone_t;

typedef struct {
    int  present;                /* 0: no /sys/class/devfreq/gpu */
    char governor[ENG_NAME];
    int  cur_mhz, min_mhz, max_mhz;   /* -1 if unreadable; min/max read 0 on the stock kernel */
    int  freqs_mhz[ENG_MAX_OPPS];
    int  n_freqs;
} eng_gpu_t;

typedef struct {
    int  time_to_empty_s, time_to_full_s;   /* PMIC estimates; -1 if unreadable */
    char health[ENG_NAME];
} eng_batt_t;

#define ENG_MAX_MOUNTS 5
typedef struct {
    char mount[24], dev[40], fstype[16];
    int  found, ro;
    long long size_mb, free_mb;  /* -1 if unknown */
} eng_mount_t;

#define ENG_LOG_COLS 120
/* NULL or "" reads the real paths. */
void engread_set_root(const char *root);

/* Fills c from cpu0's cpufreq. Returns 0 if the governor was readable, -1 otherwise. */
int engread_cpu(eng_cpu_t *c);

/* Fills up to max zones from /sys/class/thermal/thermal_zoneN. Returns the count. */
int engread_thermal(eng_zone_t *z, int max);

/* /sys/class/devfreq/gpu (Hz in sysfs). Returns 0 if present. */
int engread_gpu(eng_gpu_t *g);

/* axp2202-battery extras. */
void engread_battery(eng_batt_t *b);

/* The five Redux-relevant mounts (/, /mnt/vendor, /mnt/data, /mnt/mmc, /mnt/sdcard) from /proc/mounts,
   with size and free space (statvfs). Returns ENG_MAX_MOUNTS. */
int engread_mounts(eng_mount_t *m);

/* Last up to max lines of a file under the root, oldest first, each cut to ENG_LOG_COLS-1. Count. */
int engread_tail(const char *path, char lines[][ENG_LOG_COLS], int max);

/* Parses a list of kHz values ("480000 600000 ...") into MHz, ascending, no duplicates.
   Returns the count (at most max). */
int engread_parse_khz_list(const char *text, int *mhz, int max);

#endif
