#ifndef REDUX_TERM_H
#define REDUX_TERM_H
#include <stddef.h>

/* Redux Terminal (frontend/term/redux_term.c): embedded in dmenu.bin, written to /tmp at start,
   launched by Apps > Redux Terminal through /tmp/.next (Dale 2026-11-17 A). */
int term_install(void);
const char *term_path(void);
void term_set_path(const char *path);   /* tests */

extern const unsigned char redux_term_bin[];
extern const size_t redux_term_bin_len;

#endif
