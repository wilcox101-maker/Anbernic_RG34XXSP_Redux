#ifndef REDUX_PORTS_H
#define REDUX_PORTS_H

/* Ports, option B part 1 (notes/2026-11-13-dale-decision-ports.md): ports Redux knows how to
   start itself, as a stock RetroArch core plus a data file under Roms/PORTS/. No script from
   the card runs. Rows come from the 11 stock ports (each stock .sh is `retroarch -L core data`). */

typedef struct {
    const char *name;   /* shown in the list */
    const char *core;   /* stock core in /mnt/vendor/deep/retro/cores */
    const char *data;   /* path under the PORTS folder */
} port_t;

int ports_count(void);
const port_t *ports_get(int i);

#endif
