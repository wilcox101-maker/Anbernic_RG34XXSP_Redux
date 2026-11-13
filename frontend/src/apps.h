#ifndef REDUX_APPS_H
#define REDUX_APPS_H

/* App Center (decisions 2026-11-13: local launcher, menu option A).
   Read only: lists and launches apps already on the device. Never installs,
   downloads, copies or deletes. Paths from notes/2026-11-13-claude-message-stock-apps.md. */

typedef enum {
    APP_MUSIC, APP_VIDEO, APP_EBOOK_TF1, APP_EBOOK_TF2, APP_FILES, APP_PDF, APP_TERMINAL, APP_TOP,
    APP_COUNT
} app_id_t;

int apps_count(void);
/* App name, and its status column ("not verified", "later ..."). */
const char *apps_name(int i);
const char *apps_status(int i);
/* 1 if the row can be launched now (has a binary path). */
int apps_launchable(int i);

/* Tests only: prefix put in front of each path for the "is it installed" check. */
void apps_set_root(const char *root);

/* Write /tmp/.next for app i. 0 = ok (main should exit), -1 = not launchable,
   -2 = the binary is not on this device. */
int apps_launch(int i);

#endif
