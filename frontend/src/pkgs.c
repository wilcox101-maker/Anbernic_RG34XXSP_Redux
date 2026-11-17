#define _POSIX_C_SOURCE 200809L
#include "pkgs.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

extern const unsigned char redux_extras_ini[];
extern const size_t redux_extras_ini_len;

#ifdef REDUX_PLATFORM_STOCK
#define PKGS_LOG "/mnt/data/redux/packages.log"
#else
#define PKGS_LOG "/tmp/redux.packages.log"
#endif
#define APT_FLAGS "--no-upgrade --no-install-recommends"

static char names[PKGS_MAX][40], descs[PKGS_MAX][80];
static int installed[PKGS_MAX];
static int n_pkgs = -1;
static const char *custom_ini;
static pkgs_run_fn run_fn;
static char log_path[256] = PKGS_LOG;

void pkgs_set_run(pkgs_run_fn fn) { run_fn = fn; }
void pkgs_set_log(const char *p) { snprintf(log_path, sizeof log_path, "%s", p ? p : PKGS_LOG); }
void pkgs_set_catalog(const char *ini) { custom_ini = ini; n_pkgs = -1; }

/* Package names as apt allows them, and nothing a shell would read specially. */
static int good_name(const char *s) {
    if (!s[0] || strlen(s) >= 40 || (!islower((unsigned char)s[0]) && !isdigit((unsigned char)s[0]))) return 0;
    for (; *s; s++)
        if (!(islower((unsigned char)*s) || isdigit((unsigned char)*s) || *s == '-' || *s == '+' || *s == '.')) return 0;
    return 1;
}

static void trim(char *s) {
    size_t n = strlen(s);
    while (n && isspace((unsigned char)s[n - 1])) s[--n] = '\0';
    size_t k = 0;
    while (s[k] && isspace((unsigned char)s[k])) k++;
    if (k) memmove(s, s + k, strlen(s + k) + 1);
}

static void load(void) {
    if (n_pkgs >= 0) return;
    n_pkgs = 0;
    const char *t = custom_ini;
    size_t len = t ? strlen(t) : redux_extras_ini_len;
    if (!t) t = (const char *)redux_extras_ini;
    char line[256];
    int cur = -1;
    for (size_t at = 0; at < len;) {
        size_t e = at;
        while (e < len && t[e] != '\n') e++;
        size_t L = e - at < sizeof line - 1 ? e - at : sizeof line - 1;
        memcpy(line, t + at, L);
        line[L] = '\0';
        at = e + 1;
        trim(line);
        if (!line[0] || line[0] == ';' || line[0] == '#') continue;
        if (line[0] == '[') {
            char *close = strchr(line, ']');
            cur = -1;
            if (!close) continue;
            *close = '\0';
            if (strcmp(line + 1, "stock") == 0 || !good_name(line + 1) || n_pkgs >= PKGS_MAX) continue;
            cur = n_pkgs++;
            snprintf(names[cur], sizeof names[0], "%.39s", line + 1);
            descs[cur][0] = '\0';
            installed[cur] = 0;
            continue;
        }
        if (cur >= 0 && strncmp(line, "desc", 4) == 0) {
            char *eq = strchr(line, '=');
            if (eq) { eq++; while (*eq == ' ') eq++; snprintf(descs[cur], sizeof descs[0], "%.79s", eq); }
        }
    }
}

int pkgs_count(void) { load(); return n_pkgs; }
const char *pkgs_name(int i) { load(); return i >= 0 && i < n_pkgs ? names[i] : ""; }
const char *pkgs_desc(int i) { load(); return i >= 0 && i < n_pkgs ? descs[i] : ""; }
int pkgs_installed(int i) { load(); return i >= 0 && i < n_pkgs ? installed[i] : 0; }

static int run(const char *cmd) {
    if (run_fn) return run_fn(cmd);
#ifdef REDUX_PLATFORM_STOCK
    int st = system(cmd);
    if (st == -1 || !WIFEXITED(st)) return -1;
    return WEXITSTATUS(st);
#else
    (void)cmd;
    return -1;   /* desktop: never touches apt */
#endif
}

void pkgs_refresh(void) {
    load();
    char cmd[256];
    for (int i = 0; i < n_pkgs; i++) {
        snprintf(cmd, sizeof cmd,
                 "dpkg-query -W -f='${Status}' %.39s 2>/dev/null | grep -q 'install ok installed'", names[i]);
        installed[i] = run(cmd) == 0;
    }
}

/* Exit codes inside the commands: 2 = apt-get update failed (network), 3 = refused, else failed. */
int pkgs_install_cmd(const char *pkg, char *out, size_t n) {
    if (!good_name(pkg)) return -1;
    snprintf(out, n,
             "( export LC_ALL=C DEBIAN_FRONTEND=noninteractive; echo \"=== install %s $(date)\"; "
             "apt-get update -qq || exit 2; "
             "up=$(apt-get -s install " APT_FLAGS " %s | grep \"^Inst [^ ]* \\[\"); "
             "if [ -n \"$up\" ]; then echo \"REFUSED, this would upgrade installed packages:\"; echo \"$up\"; exit 3; fi; "
             "apt-get install -y " APT_FLAGS " %s ) >> '%s' 2>&1",
             pkg, pkg, pkg, log_path);
    return 0;
}

int pkgs_remove_cmd(const char *pkg, char *out, size_t n) {
    if (!good_name(pkg)) return -1;
    snprintf(out, n,
             "( export LC_ALL=C DEBIAN_FRONTEND=noninteractive; echo \"=== remove %s $(date)\"; "
             "other=$(apt-get -s remove %s | grep \"^Remv \" | grep -v \"^Remv %s[ :]\"); "
             "if [ -n \"$other\" ]; then echo \"REFUSED, this would also remove:\"; echo \"$other\"; exit 3; fi; "
             "apt-get remove -y %s ) >> '%s' 2>&1",
             pkg, pkg, pkg, pkg, log_path);
    return 0;
}

int pkgs_write_script(int i, int remove_it, const char *path) {
    load();
    if (i < 0 || i >= n_pkgs) return -1;
    const char *pkg = names[i];
    FILE *f = fopen(path, "w");
    if (!f) return -1;
    fprintf(f, "#!/bin/bash\n# Written by Redux for Settings > System > Packages; runs in Redux Terminal.\n"
               "export LC_ALL=C DEBIAN_FRONTEND=noninteractive\n"
               "{\n");
    if (remove_it)
        fprintf(f, "  echo \"=== remove %s $(date)\"\n"
                   "  other=$(apt-get -s remove %s | grep \"^Remv \" | grep -v \"^Remv %s[ :]\")\n"
                   "  if [ -n \"$other\" ]; then echo \"REFUSED, this would also remove:\"; echo \"$other\"; exit 3; fi\n"
                   "  apt-get remove -y %s\n", pkg, pkg, pkg, pkg);
    else
        fprintf(f, "  echo \"=== install %s $(date)\"\n"
                   "  echo \"Reading the package lists...\"\n"
                   "  apt-get update -qq || exit 2\n"
                   "  up=$(apt-get -s install " APT_FLAGS " %s | grep \"^Inst [^ ]* \\[\")\n"
                   "  if [ -n \"$up\" ]; then echo \"REFUSED, this would upgrade installed packages:\"; echo \"$up\"; exit 3; fi\n"
                   "  apt-get install -y " APT_FLAGS " %s\n", pkg, pkg, pkg);
    fprintf(f, "} 2>&1 | tee -a '%s'\n"
               "rc=${PIPESTATUS[0]}\n"
               "echo\n"
               "case $rc in\n"
               "  0) echo \"%s %s: done.\";;\n"
               "  2) echo \"No package list: is Wi-Fi connected to the internet?\";;\n"
               "  3) echo \"Refused: it would change other packages. Nothing done.\";;\n"
               "  *) echo \"apt failed (exit $rc). Log: %s\";;\n"
               "esac\n"
               "exit $rc\n", log_path, remove_it ? "Removed" : "Installed", pkg, log_path);
    if (fclose(f) != 0) return -1;
    return 0;
}

static int act(int i, int remove_it) {
    load();
    if (i < 0 || i >= n_pkgs) return PKGS_E_BAD;
    char cmd[1400];
    if ((remove_it ? pkgs_remove_cmd : pkgs_install_cmd)(names[i], cmd, sizeof cmd) != 0) return PKGS_E_BAD;
    int rc = run(cmd);
    pkgs_refresh();
    if (rc == 0) return PKGS_OK;
    if (rc == 2) return PKGS_E_NET;
    if (rc == 3) return PKGS_E_REFUSED;
    return PKGS_E_FAIL;
}

int pkgs_install(int i) { return act(i, 0); }
int pkgs_remove(int i)  { return act(i, 1); }

const char *pkgs_message(int rc) {
    switch (rc) {
    case PKGS_OK:        return "Done.";
    case PKGS_E_NET:     return "No package list: is Wi-Fi connected to the internet?";
    case PKGS_E_REFUSED: return "Refused: it would change other packages. Nothing done. See packages.log.";
    case PKGS_E_BAD:     return "Not in the catalog.";
    default:             return "apt failed. See /mnt/data/redux/packages.log.";
    }
}
