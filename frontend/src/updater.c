#define _DEFAULT_SOURCE
#include "updater.h"
#include "buildhist.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static char g_redux[256] = UPDATER_REDUX;
static updater_fetch_fn g_fetch;
static int g_logo;   /* the last install also replaced bootlogo.bmp */

int updater_logo_changed(void) { return g_logo; }

void updater_set_paths(const char *redux_dir) {
    snprintf(g_redux, sizeof g_redux, "%s", redux_dir ? redux_dir : UPDATER_REDUX);
    char b[300];
    snprintf(b, sizeof b, "%s/builds", g_redux);
    buildhist_set_dir(b);
}
void updater_set_fetch(updater_fetch_fn fn) { g_fetch = fn; }

static int hex_sha(const char *s) {
    size_t n = strlen(s);
    if (n < 7 || n > 15) return 0;
    for (size_t i = 0; i < n; i++)
        if (!((s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'f'))) return 0;
    return 1;
}

/* wget into dest. The URL is always UPDATER_URL + a fixed file name; nothing user-typed. */
static int fetch(const char *url, const char *dest) {
    if (g_fetch) return g_fetch(url, dest);
    char cmd[1024];
    snprintf(cmd, sizeof cmd, "wget -q -T 30 -O '%s' '%s'", dest, url);
    int st = system(cmd);
    return (st != -1 && WIFEXITED(st) && WEXITSTATUS(st) == 0) ? 0 : -1;
}

static int mkdirs(const char *a) {
    if (mkdir(a, 0755) != 0 && errno != EEXIST) return -1;
    return 0;
}

static void rm_dir(const char *dir) {
    DIR *d = opendir(dir);
    if (d) {
        struct dirent *e;
        char p[600];
        while ((e = readdir(d))) {
            if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
            snprintf(p, sizeof p, "%s/%s", dir, e->d_name);
            remove(p);
        }
        closedir(d);
    }
    rmdir(dir);
}

/* sha256 check of one file against SHA256SUMS.txt in the same folder (the release's own list). */
static int sum_ok(const char *dir, const char *name) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, "cd '%s' && grep ' %s$' SHA256SUMS.txt | sha256sum -c - >/dev/null 2>&1", dir, name);
    int st = system(cmd);
    return st != -1 && WIFEXITED(st) && WEXITSTATUS(st) == 0;
}

int updater_download(const char *current_sha, char *sha_out, size_t n) {
    char builds[300], inc[320], p[400], url[300];
    if (n) sha_out[0] = '\0';
    snprintf(builds, sizeof builds, "%s/builds", g_redux);
    snprintf(inc, sizeof inc, "%s/incoming", builds);
    if (mkdirs(builds) != 0) return -1;
    rm_dir(inc);
    if (mkdirs(inc) != 0) return -1;
    static const char *files[] = { "SHA256SUMS.txt", "BUILD.txt", "dmenu.bin" };
    for (int i = 0; i < 3; i++) {
        snprintf(url, sizeof url, "%s/%s", UPDATER_URL, files[i]);
        snprintf(p, sizeof p, "%s/%s", inc, files[i]);
        if (fetch(url, p) != 0) { rm_dir(inc); return -1; }
    }
    if (!sum_ok(inc, "BUILD.txt") || !sum_ok(inc, "dmenu.bin")) { rm_dir(inc); return -2; }
    /* The boot logo stamped with this build (Dale 2026-11-17 A). Optional: a release without one
       still installs; one that fails its checksum is refused like dmenu.bin. */
    snprintf(url, sizeof url, "%s/bootlogo.bmp", UPDATER_URL);
    snprintf(p, sizeof p, "%s/bootlogo.bmp", inc);
    if (fetch(url, p) != 0) remove(p);
    else if (!sum_ok(inc, "bootlogo.bmp")) { rm_dir(inc); return -2; }
    char sha[32] = "";
    snprintf(p, sizeof p, "%s/BUILD.txt", inc);
    FILE *f = fopen(p, "r");
    if (!f || !fgets(sha, sizeof sha, f)) { if (f) fclose(f); rm_dir(inc); return -3; }
    fclose(f);
    sha[strcspn(sha, " \r\n")] = '\0';
    if (!hex_sha(sha)) { rm_dir(inc); return -3; }
    snprintf(sha_out, n, "%s", sha);
    if (current_sha && strcmp(sha, current_sha) == 0) { rm_dir(inc); return 1; }
    char dst[360];
    snprintf(dst, sizeof dst, "%s/%.15s", builds, sha);
    rm_dir(dst);
    if (rename(inc, dst) != 0) { rm_dir(inc); return -1; }
    return 0;
}

int updater_have(const char *sha) {
    if (!sha || !hex_sha(sha)) return 0;
    char p[400];
    struct stat st;
    snprintf(p, sizeof p, "%s/builds/%s/dmenu.bin", g_redux, sha);
    return stat(p, &st) == 0 && st.st_size > 0;
}

/* Copy src to dst with fsync; dst is replaced only by the caller's rename. */
static int copy_file(const char *src, const char *dst, mode_t mode) {
    int in = open(src, O_RDONLY);
    if (in < 0) return -1;
    int out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, mode);
    if (out < 0) { close(in); return -1; }
    char buf[65536];
    ssize_t r;
    int ok = 1;
    while ((r = read(in, buf, sizeof buf)) > 0)
        if (write(out, buf, (size_t)r) != r) { ok = 0; break; }
    if (r < 0) ok = 0;
    if (ok && fsync(out) != 0) ok = 0;
    close(in);
    if (close(out) != 0) ok = 0;
    if (!ok) remove(dst);
    return ok ? 0 : -1;
}

/* Keep: the new build, the running one, the last known working one, and the newest UPDATER_KEEP
   distinct builds in the history. Every other builds/<sha>/ folder is removed. */
static void prune(const char *keep1, const char *keep2) {
    buildhist_entry_t h[BUILDHIST_MAX], g;
    int nh = buildhist_load(h, BUILDHIST_MAX);
    const char *keep[3 + UPDATER_KEEP];
    int nk = 0;
    if (keep1) keep[nk++] = keep1;
    if (keep2) keep[nk++] = keep2;
    if (buildhist_last_good(&g) == 0) keep[nk++] = g.sha;
    int distinct = 0;
    for (int k = 0; k < nh && distinct < UPDATER_KEEP; k++) {
        int seen = 0;
        for (int j = 0; j < k; j++) if (!strcmp(h[j].sha, h[k].sha)) seen = 1;
        if (!seen) { keep[nk++] = h[k].sha; distinct++; }
    }
    char builds[300];
    snprintf(builds, sizeof builds, "%s/builds", g_redux);
    DIR *d = opendir(builds);
    if (!d) return;
    struct dirent *e;
    char names[32][16];
    int nn = 0;
    while ((e = readdir(d)) && nn < 32)
        if (hex_sha(e->d_name)) snprintf(names[nn++], sizeof names[0], "%.15s", e->d_name);
    closedir(d);
    for (int i = 0; i < nn; i++) {
        int k = 0;
        for (int j = 0; j < nk; j++) if (!strcmp(names[i], keep[j])) k = 1;
        if (!k) {
            char p[400];
            snprintf(p, sizeof p, "%.300s/%.15s", builds, names[i]);
            rm_dir(p);
        }
    }
}

int updater_install(const char *sha, const char *current_sha, const char *how) {
    if (!updater_have(sha)) return -1;
    char src[400], cur[400], keepdir[400], keep[420], tmp[400], dst[400];
    snprintf(cur, sizeof cur, "%s/dmenu.bin", g_redux);
    /* Keep the running build for roll-back, once. */
    if (current_sha && hex_sha(current_sha) && !updater_have(current_sha)) {
        snprintf(keepdir, sizeof keepdir, "%s/builds/%s", g_redux, current_sha);
        mkdirs(keepdir);
        snprintf(keep, sizeof keep, "%s/dmenu.bin", keepdir);
        copy_file(cur, keep, 0755);
        char curlogo[400];
        snprintf(curlogo, sizeof curlogo, "%s/bootlogo.bmp", g_redux);
        snprintf(keep, sizeof keep, "%s/bootlogo.bmp", keepdir);
        if (access(curlogo, R_OK) == 0) copy_file(curlogo, keep, 0644);
    }
    snprintf(src, sizeof src, "%s/builds/%s/dmenu.bin", g_redux, sha);
    snprintf(tmp, sizeof tmp, "%s/dmenu.bin.new", g_redux);
    snprintf(dst, sizeof dst, "%s/dmenu.bin", g_redux);
    if (copy_file(src, tmp, 0755) != 0) return -2;
    if (chmod(tmp, 0755) != 0 || rename(tmp, dst) != 0) { remove(tmp); return -2; }
    /* That build's boot logo becomes the Redux logo (/mnt/vendor/redux/bootlogo.bmp). Writing it
       to the boot partition is the caller's step (bootscreen_install, only if Redux's logo is the
       one installed). A failure here keeps the old logo file; the new dmenu.bin stays. */
    char lsrc[400], ltmp[400], ldst[400];
    snprintf(lsrc, sizeof lsrc, "%s/builds/%s/bootlogo.bmp", g_redux, sha);
    snprintf(ltmp, sizeof ltmp, "%s/bootlogo.bmp.new", g_redux);
    snprintf(ldst, sizeof ldst, "%s/bootlogo.bmp", g_redux);
    g_logo = 0;
    if (access(lsrc, R_OK) == 0) {
        if (copy_file(lsrc, ltmp, 0644) == 0 && rename(ltmp, ldst) == 0) g_logo = 1;
        else remove(ltmp);
    }
    sync();
    buildhist_installed(sha, time(NULL), how);
    prune(sha, current_sha);
    return 0;
}

const char *updater_message(int rc) {
    switch (rc) {
    case 0:  return "Downloaded and checked.";
    case 1:  return "Already running the newest dev build.";
    case -1: return "Download failed. Is Wi-Fi connected?";
    case -2: return "Checksum mismatch: nothing kept, nothing installed.";
    default: return "The release data looks wrong: nothing kept.";
    }
}
