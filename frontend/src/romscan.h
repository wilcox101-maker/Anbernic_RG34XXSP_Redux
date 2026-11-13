#ifndef REDUX_ROMSCAN_H
#define REDUX_ROMSCAN_H

/* Read-only scan of the ROM roots (stock: TF1 /mnt/mmc/Roms and TF2 /mnt/sdcard/Roms). Never creates, edits or
   deletes anything under that root. Desktop builds with no REDUX_ROMS set a
   zero-byte fixture under /tmp so the F0 ROM mock-up still matches. */

/* ROM roots on the device: TF1 (Roms partition) and TF2 (second card, if inserted).
   Both are read only. A missing card or folder is skipped. */
#define ROMSCAN_TF1 "/mnt/mmc/Roms"
#define ROMSCAN_TF2 "/mnt/sdcard/Roms"
#define ROMSCAN_ROOTS_MAX 4

/* Load one root now. romscan_init picks the root; tests call this directly. */
void romscan_load(const char *root);

/* Load several roots; the same system on two cards becomes one merged, sorted list. */
void romscan_load_roots(const char *const *roots, int nroots);

/* Load REDUX_ROMS (one root or several separated by ':'), else TF1 + TF2 on the
   device, else the desktop fixture. Safe to call again. */
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
