#ifndef REDUX_ROMSCAN_H
#define REDUX_ROMSCAN_H

/* Read-only scan of a ROM root (stock: /mnt/mmc/Roms). Never creates, edits or
   deletes anything under that root. Desktop builds with no REDUX_ROMS set a
   zero-byte fixture under /tmp so the F0 ROM mock-up still matches. */

/* Load one root now. romscan_init picks the root; tests call this directly. */
void romscan_load(const char *root);

/* Load REDUX_ROMS, or the stock root, or the desktop fixture. Safe to call again. */
void romscan_init(void);

int romscan_systems(void);
/* Folder key for systems_lookup (may differ from the directory name via an alias). */
const char *romscan_key(int sys);
/* Title used on the ROM screen. GBA is "Game Boy Advance" so the mock-up matches. */
const char *romscan_title(int sys);
int romscan_index_of(const char *key);   /* 0 if missing */

int romscan_roms(int sys);
const char *romscan_rom_name(int sys, int rom);   /* file name without the extension */
const char *romscan_rom_path(int sys, int rom);   /* full path, for launcher_start */

/* How many rows a list draws before it scrolls. Shared with the UI. */
#define ROM_LIST_WINDOW 20

#endif
