#ifndef REDUX_LIDWATCH_H
#define REDUX_LIDWATCH_H
#include <stddef.h>

/* redux-lidwatch (frontend/lidwatch) is built into dmenu.bin. At start Redux writes it to /tmp (RAM)
   and every RetroArch launch line runs it beside the game (F4 step 4). */

/* Write the program out (0755). Returns 0 on success. */
int lidwatch_install(void);

/* Path it is written to. Tests point at a temp path; NULL restores /tmp/redux-lidwatch. */
const char *lidwatch_path(void);
void lidwatch_set_path(const char *path);

extern const unsigned char redux_lidwatch_bin[];
extern const size_t redux_lidwatch_bin_len;

#endif
