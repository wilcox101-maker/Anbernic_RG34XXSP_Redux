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

/* Fill rows. Missing files become "--". Returns the row count. */
int sysinfo_collect(sysinfo_row_t *rows, int max);

#endif
