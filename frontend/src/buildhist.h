#ifndef REDUX_BUILDHIST_H
#define REDUX_BUILDHIST_H
#include <time.h>

/* Build history for Engineering > Update (notes/2026-11-17-dale-decision-eng-update.md; security
   item, Dale merges). One file, /mnt/vendor/redux/builds/history, newest line last:
       <sha> <YYYY-MM-DD> <HH:MM> <menu|nomenu> <how>
   how: "manual" (wget, installer) for now; "update" and "rollback" come with steps 2-3.
   Written with temp file + fsync + rename; at most BUILDHIST_MAX lines kept. Never on the cards. */

#define BUILDHIST_DIR "/mnt/vendor/redux/builds"
#define BUILDHIST_MAX 10

typedef struct {
    char sha[16];
    char when[20];   /* "YYYY-MM-DD HH:MM" */
    int  menu;       /* 1: this build reached the main menu */
    char how[12];
} buildhist_entry_t;

/* Tests: another folder. NULL = default. */
void buildhist_set_dir(const char *dir);

/* Up to max entries, newest first. Returns the count. */
int buildhist_load(buildhist_entry_t *e, int max);

/* Called once the main menu is drawn: the running build is marked as working. If the newest entry is
   another build (installed by hand), a new entry "manual" is added. "dev" (desktop) builds are skipped.
   0 ok. */
int buildhist_menu_reached(const char *sha, time_t now);

/* The newest entry that reached the menu: its sha into out (empty if none). 0 found, -1 none. */
int buildhist_last_good(buildhist_entry_t *out);

#endif
