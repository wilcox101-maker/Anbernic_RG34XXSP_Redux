#ifndef REDUX_UPDATER_H
#define REDUX_UPDATER_H
#include <stddef.h>

/* Engineering > Update, step 2 (notes/2026-11-17-dale-decision-eng-update.md; security item, Dale
   merges). Downloads the newest dev build of this repo only, over HTTPS, checks it against the
   release's SHA256SUMS.txt, keeps it in /mnt/vendor/redux/builds/<sha>/, and installs it as
   dmenu.bin.new + rename, with that build's bootlogo.bmp beside it (Dale 2026-11-17 A). Never
   touches the cards or run.sh; the boot partition is written by bootscreen_install, called by the UI. */

#define UPDATER_URL      "https://github.com/wilcox101-maker/Anbernic_RG34XXSP_Redux/releases/download/dev"
#define UPDATER_REDUX    "/mnt/vendor/redux"
#define UPDATER_KEEP     3      /* builds kept (Dale Q2 A) */

/* Tests: another Redux folder (builds/ goes inside it) and a fetch function instead of wget.
   fetch(url, dest) returns 0 when dest was written. NULL restores the defaults. */
typedef int (*updater_fetch_fn)(const char *url, const char *dest);
void updater_set_paths(const char *redux_dir);
void updater_set_fetch(updater_fetch_fn fn);

/* Download the newest dev build into builds/<sha>/. sha_out gets its short SHA.
   0 = downloaded and checked; 1 = it is the build already running (nothing kept);
   -1 = download failed (no network?); -2 = checksum mismatch (nothing kept); -3 = bad release data. */
int updater_download(const char *current_sha, char *sha_out, size_t n);

/* Install builds/<sha>/dmenu.bin as the Redux binary (dmenu.bin.new, fsync, rename), after keeping the
   running build in builds/<current>/ for roll-back. Records "<sha> ... nomenu <how>" in the history
   and keeps the newest UPDATER_KEEP builds (never the current or the last known working one).
   0 ok; -1 no such build; -2 copy or rename failed (the old dmenu.bin stays). The caller reboots. */
int updater_install(const char *sha, const char *current_sha, const char *how);

/* 1 if the last updater_install also replaced /mnt/vendor/redux/bootlogo.bmp with that build's logo
   (the release had one). The caller then rewrites the boot partition unless the user chose the stock
   logo (bootscreen_chose_stock). */
int updater_logo_changed(void);

/* 1 if builds/<sha>/dmenu.bin is kept. */
int updater_have(const char *sha);

/* One-line text for a result code of updater_download. */
const char *updater_message(int rc);

#endif
