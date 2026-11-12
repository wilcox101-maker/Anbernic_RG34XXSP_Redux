#ifndef REDUX_SYSINFO_H
#define REDUX_SYSINFO_H

/* Read-only system snapshot. Reads /proc and /sys. Never writes.
   A test can point sysinfo_set_root() at a fixture directory. */

#define SYSINFO_LABEL 20
#define SYSINFO_VALUE 48
#define SYSINFO_MAX_ROWS 16

typedef struct {
    char label[SYSINFO_LABEL];
    char value[SYSINFO_VALUE];
} sysinfo_row_t;

/* Empty root reads the real paths. NULL or "" does the same. */
void sysinfo_set_root(const char *root);

/* Fill rows. Missing files become "--". The battery row is always
   "unknown (no sensor)": the axp2202 temp file is a fixed dummy. */
int sysinfo_collect(sysinfo_row_t *rows, int max);

/* Parse a tenths-of-a-degree reading (300 = 30.0 C). -1 if it is not an integer.
   Kept for a kernel that enables the battery sensor. Not shown in the UI. */
int sysinfo_parse_batt_tenths(const char *text);

#endif
