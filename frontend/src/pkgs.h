#ifndef REDUX_PKGS_H
#define REDUX_PKGS_H
#include <stddef.h>

/* Settings > System > Packages (Dale 2026-11-17 A): install or remove the Ubuntu packages listed in
   tools/extras.ini (embedded at build time), on the device, over Wi-Fi. Same safety as
   tools/extras.py: apt-get update, simulate, REFUSE if anything already installed would change
   version, never upgrade, --no-install-recommends. Remove: only a catalog package, and only if the
   simulation removes nothing else. Output goes to /mnt/data/redux/packages.log. Writes the system
   partition (packages); never the cards. Desktop builds run nothing. */

#define PKGS_MAX 32

/* Load the catalog (once). Tests pass their own ini text; NULL = the embedded tools/extras.ini. */
void pkgs_set_catalog(const char *ini_text);
int  pkgs_count(void);
const char *pkgs_name(int i);
const char *pkgs_desc(int i);

/* Apps row for a catalog package (Dale 2026-11-17 A): `app = <row name>` and `launch = <redux-term
   arguments>` in extras.ini; empty when the package has none. `binary` is the program the
   functional check runs; Apps uses it to see whether the package is installed. */
const char *pkgs_app(int i);
const char *pkgs_launch(int i);
const char *pkgs_binary(int i);

/* Installed state from dpkg-query, refreshed by pkgs_refresh (one command per package). */
void pkgs_refresh(void);
int  pkgs_installed(int i);

enum { PKGS_OK = 0, PKGS_E_NET = -1, PKGS_E_REFUSED = -2, PKGS_E_FAIL = -3, PKGS_E_BAD = -4 };
int pkgs_install(int i);
int pkgs_remove(int i);
const char *pkgs_message(int rc);

/* Tests: run(cmd) returns the shell exit status; log path. NULL = defaults. */
typedef int (*pkgs_run_fn)(const char *cmd);
void pkgs_set_run(pkgs_run_fn fn);
void pkgs_set_log(const char *path);

/* Install or remove in Redux Terminal (Dale 2026-11-17 A): write a bash script with the same safety
   steps whose output goes to the screen and to packages.log, for `redux-term --hold -e /bin/bash
   <path>`. Exit 0 done, 2 no package list, 3 refused, else apt failed. */
#define PKGS_SCRIPT "/tmp/redux-pkg.sh"
int pkgs_write_script(int i, int remove_it, const char *path);

/* The exact shell text used (tests check the safety steps). */
int pkgs_install_cmd(const char *pkg, char *out, size_t n);
int pkgs_remove_cmd(const char *pkg, char *out, size_t n);

#endif
