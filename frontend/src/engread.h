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

/* NULL or "" reads the real paths. */
void engread_set_root(const char *root);

/* Fills c from cpu0's cpufreq. Returns 0 if the governor was readable, -1 otherwise. */
int engread_cpu(eng_cpu_t *c);

/* Fills up to max zones from /sys/class/thermal/thermal_zoneN. Returns the count. */
int engread_thermal(eng_zone_t *z, int max);

/* Parses a list of kHz values ("480000 600000 ...") into MHz, ascending, no duplicates.
   Returns the count (at most max). */
int engread_parse_khz_list(const char *text, int *mhz, int max);

#endif
