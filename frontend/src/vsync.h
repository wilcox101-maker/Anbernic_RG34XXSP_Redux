#ifndef REDUX_VSYNC_H
#define REDUX_VSYNC_H
#include <stddef.h>

/* The vsync library (frontend/vsync/redux_vsync.c) is built into dmenu.bin. Before launching an
   app that tears, Redux writes it to /tmp (RAM) and preloads it for that launch only. */

/* Write the library out; on success fill `env` with "LD_PRELOAD=<path>" and return 0. */
int vsync_install(char *env, size_t n);

/* Tests point at a temp path. NULL restores /tmp/redux-vsync.so. */
void vsync_set_path(const char *path);

extern const unsigned char redux_vsync_so[];
extern const size_t redux_vsync_so_len;

#endif
