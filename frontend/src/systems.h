#ifndef REDUX_SYSTEMS_H
#define REDUX_SYSTEMS_H

/* Per-system launch table. Maps a system name to a RetroArch core or a .dge path. */

typedef struct {
    const char *name;          /* folder / display name */
    launch_target_t target;
    const char *core_or_path;  /* RetroArch core or .dge binary */
} system_entry_t;

/* Lookup by system name. Returns NULL if unknown. */
const system_entry_t *systems_lookup(const char *name);

#endif
