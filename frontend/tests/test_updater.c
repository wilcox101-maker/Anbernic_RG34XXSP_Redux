/* Engineering > Update step 2: download, checksum, install, prune. Temp folders; the "release" is a
   local folder served by a fake fetch, so no network is used. Needs sha256sum (CI and the device). */
#define _DEFAULT_SOURCE
#include "updater.h"
#include "buildhist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int fails;
static void expect(int cond, const char *what) {
    printf("%s %s\n", cond ? "ok  " : "FAIL", what);
    if (!cond) fails++;
}

static char rel[300], redux[300];
static int net_down;

static void put(const char *path, const char *text) {
    FILE *f = fopen(path, "w");
    if (!f) { perror(path); exit(2); }
    fputs(text, f);
    fclose(f);
}

static int same(const char *path, const char *text) {
    char buf[256] = "";
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    size_t n = fread(buf, 1, sizeof buf - 1, f);
    fclose(f);
    buf[n] = '\0';
    return strcmp(buf, text) == 0;
}

static int exists(const char *path) { struct stat st; return stat(path, &st) == 0; }

/* Fake fetch: copies <rel>/<basename of url> to dest. */
static int fake_fetch(const char *url, const char *dest) {
    if (net_down) return -1;
    if (strncmp(url, UPDATER_URL "/", strlen(UPDATER_URL) + 1) != 0) return -1;   /* only this repo */
    char src[400], cmd[900];
    snprintf(src, sizeof src, "%s/%s", rel, strrchr(url, '/') + 1);
    if (!exists(src)) return -1;
    snprintf(cmd, sizeof cmd, "cp '%s' '%s'", src, dest);
    return system(cmd) == 0 ? 0 : -1;
}

static void make_release(const char *sha, const char *bin) {
    char p[400], cmd[700];
    snprintf(p, sizeof p, "%s/dmenu.bin", rel); put(p, bin);
    snprintf(p, sizeof p, "%s/BUILD.txt", rel);
    char line[32]; snprintf(line, sizeof line, "%s\n", sha); put(p, line);
    snprintf(cmd, sizeof cmd, "cd '%s' && sha256sum dmenu.bin BUILD.txt > SHA256SUMS.txt", rel);
    if (system(cmd) != 0) exit(2);
}

int main(void) {
    char base[] = "/tmp/redux-updater-XXXXXX";
    if (!mkdtemp(base)) { perror("mkdtemp"); return 2; }
    snprintf(rel, sizeof rel, "%s/release", base);
    snprintf(redux, sizeof redux, "%s/redux", base);
    mkdir(rel, 0755); mkdir(redux, 0755);
    updater_set_paths(redux);
    updater_set_fetch(fake_fetch);
    char p[400], sha[16];

    snprintf(p, sizeof p, "%s/dmenu.bin", redux); put(p, "OLDBIN");
    buildhist_menu_reached("0000aaa", 1794900000);           /* the running build reached the menu */

    /* Download. */
    make_release("abc1234", "NEWBIN");
    net_down = 1;
    expect(updater_download("0000aaa", sha, sizeof sha) == -1, "no network: -1");
    net_down = 0;
    expect(updater_download("0000aaa", sha, sizeof sha) == 0 && strcmp(sha, "abc1234") == 0, "downloaded abc1234");
    snprintf(p, sizeof p, "%s/builds/abc1234/dmenu.bin", redux);
    expect(same(p, "NEWBIN") && updater_have("abc1234"), "kept in builds/abc1234/");
    snprintf(p, sizeof p, "%s/builds/incoming", redux);
    expect(!exists(p), "no incoming folder left");
    expect(updater_download("abc1234", sha, sizeof sha) == 1, "same as running: 1 (nothing kept twice)");

    /* Tampered binary: checksum mismatch, nothing kept. */
    make_release("bad0001", "GOODBIN");
    snprintf(p, sizeof p, "%s/dmenu.bin", rel); put(p, "EVILBIN");
    expect(updater_download("0000aaa", sha, sizeof sha) == -2 && !updater_have("bad0001"), "checksum mismatch: -2, not kept");
    /* A bad BUILD.txt. */
    make_release("not;hex", "X");
    expect(updater_download("0000aaa", sha, sizeof sha) == -3, "bad build id: -3");

    /* Install. */
    expect(updater_install("fff9999", "0000aaa", "update") == -1, "unknown build: -1");
    snprintf(p, sizeof p, "%s/dmenu.bin", redux);
    expect(same(p, "OLDBIN"), "running binary unchanged after a refused install");
    expect(updater_install("abc1234", "0000aaa", "update") == 0, "install abc1234");
    expect(same(p, "NEWBIN"), "dmenu.bin is the new build");
    snprintf(p, sizeof p, "%s/builds/0000aaa/dmenu.bin", redux);
    expect(same(p, "OLDBIN"), "the old build kept for roll-back");
    snprintf(p, sizeof p, "%s/dmenu.bin.new", redux);
    expect(!exists(p), "no dmenu.bin.new left");
    buildhist_entry_t h[BUILDHIST_MAX], g;
    int n = buildhist_load(h, BUILDHIST_MAX);
    expect(n == 2 && !strcmp(h[0].sha, "abc1234") && !h[0].menu && !strcmp(h[0].how, "update"), "history: abc1234 nomenu update");
    expect(buildhist_last_good(&g) == 0 && !strcmp(g.sha, "0000aaa"), "last known working is still the old build");

    expect(!updater_logo_changed(), "a release without a boot logo leaves bootlogo.bmp alone");

    /* Boot logo per build (Dale 2026-11-17 A): downloaded, checked, kept, installed beside dmenu.bin. */
    snprintf(p, sizeof p, "%s/bootlogo.bmp", redux); put(p, "LOGO-OLD");
    make_release("beef001", "LOGOBIN");
    {
        char q[400], cmd[700];
        snprintf(q, sizeof q, "%s/bootlogo.bmp", rel); put(q, "LOGO-beef001");
        snprintf(cmd, sizeof cmd, "cd '%s' && sha256sum dmenu.bin BUILD.txt bootlogo.bmp > SHA256SUMS.txt", rel);
        if (system(cmd) != 0) return 2;
    }
    expect(updater_download("abc1234", sha, sizeof sha) == 0, "release with a boot logo downloads");
    snprintf(p, sizeof p, "%s/builds/beef001/bootlogo.bmp", redux);
    expect(same(p, "LOGO-beef001"), "the build's logo is kept with it");
    expect(updater_install("beef001", "abc1234", "update") == 0 && updater_logo_changed(), "install reports a new logo");
    snprintf(p, sizeof p, "%s/bootlogo.bmp", redux);
    expect(same(p, "LOGO-beef001"), "bootlogo.bmp is that build's logo");
    snprintf(p, sizeof p, "%s/bootlogo.bmp.new", redux);
    expect(!exists(p), "no bootlogo.bmp.new left");
    snprintf(p, sizeof p, "%s/builds/abc1234/bootlogo.bmp", redux);
    expect(!exists(p) || same(p, "LOGO-OLD"), "the running build keeps its own logo only if it had none kept");
    expect(updater_install("abc1234", "beef001", "rollback") == 0, "roll back to abc1234");
    snprintf(p, sizeof p, "%s/bootlogo.bmp", redux);
    expect(same(p, "LOGO-beef001") && !updater_logo_changed(), "abc1234 had no logo: the current logo file stays");
    /* A tampered logo is refused like a tampered binary. */
    make_release("bad0002", "OKBIN");
    {
        char q[400], cmd[700];
        snprintf(q, sizeof q, "%s/bootlogo.bmp", rel); put(q, "LOGO-good");
        snprintf(cmd, sizeof cmd, "cd '%s' && sha256sum dmenu.bin BUILD.txt bootlogo.bmp > SHA256SUMS.txt", rel);
        if (system(cmd) != 0) return 2;
        put(q, "LOGO-evil");
    }
    expect(updater_download("abc1234", sha, sizeof sha) == -2 && !updater_have("bad0002"), "logo checksum mismatch: -2, not kept");
    {
        char q[400]; snprintf(q, sizeof q, "%s/bootlogo.bmp", rel); remove(q);
    }

    /* Prune: keep 3 newest distinct + current + new + last good. */
    const char *olds[] = { "1110001", "1110002", "1110003", "1110004" };
    for (int i = 0; i < 4; i++) {
        char d[400]; snprintf(d, sizeof d, "%s/builds/%s", redux, olds[i]); mkdir(d, 0755);
        snprintf(d, sizeof d, "%s/builds/%s/dmenu.bin", redux, olds[i]); put(d, "OLD");
        buildhist_installed(olds[i], 1794900100 + i, "update");
    }
    expect(updater_install("abc1234", "1110004", "rollback") == 0, "roll-back install of a kept build");
    expect(updater_have("abc1234") && updater_have("1110004") && updater_have("1110003") && updater_have("0000aaa"),
           "kept: new, running, newest in history, last known working");
    expect(!updater_have("1110001") && !updater_have("1110002"), "older builds removed");

    printf("%s\n", fails ? "FAILED" : "all passed");
    return fails ? 1 : 0;
}
