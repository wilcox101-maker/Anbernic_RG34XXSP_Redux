/* Packages page: the embedded catalog and the apt command text. Nothing runs apt: a fake run
   records the commands. */
#include "pkgs.h"
#include <stdio.h>
#include <string.h>

static int fails;
static void expect(int c, const char *w) { printf("%s %s\n", c ? "ok  " : "FAIL", w); if (!c) fails++; }

static char last[2048];
static int next_rc, dpkg_installed_mc;
static int fake_run(const char *cmd) {
    snprintf(last, sizeof last, "%s", cmd);
    if (strncmp(cmd, "dpkg-query", 10) == 0) return (dpkg_installed_mc && strstr(cmd, " mc ")) ? 0 : 1;
    return next_rc;
}

int main(void) {
    pkgs_set_run(fake_run);
    pkgs_set_log("/tmp/redux-test-packages.log");

    /* The real embedded tools/extras.ini. */
    int n = pkgs_count(), mc = -1, stock = 0;
    for (int i = 0; i < n; i++) {
        if (!strcmp(pkgs_name(i), "mc")) mc = i;
        if (!strcmp(pkgs_name(i), "stock")) stock = 1;
    }
    expect(n >= 5 && !strcmp(pkgs_name(0), "htop"), "embedded catalog lists htop first");
    expect(mc >= 0 && strstr(pkgs_desc(mc), "Midnight") != NULL, "mc is in the catalog with its description");
    expect(!stock, "the [stock] section is never offered");

    char cmd[2048];
    expect(pkgs_install_cmd("htop", cmd, sizeof cmd) == 0, "install command for htop");
    expect(strstr(cmd, "apt-get update -qq || exit 2") && strstr(cmd, "apt-get -s install --no-upgrade --no-install-recommends htop")
           && strstr(cmd, "exit 3") && strstr(cmd, "apt-get install -y --no-upgrade --no-install-recommends htop"),
           "install: update, simulate, refuse on change, then install without upgrades");
    expect(strstr(cmd, "upgrade ") == NULL || strstr(cmd, "apt-get upgrade") == NULL, "never apt-get upgrade");
    expect(pkgs_remove_cmd("mc", cmd, sizeof cmd) == 0 && strstr(cmd, "apt-get -s remove mc")
           && strstr(cmd, "exit 3") && strstr(cmd, "apt-get remove -y mc"), "remove: simulate, refuse if others go");
    expect(pkgs_install_cmd("x;rm -rf /", cmd, sizeof cmd) != 0 && pkgs_install_cmd("", cmd, sizeof cmd) != 0
           && pkgs_install_cmd("Bad", cmd, sizeof cmd) != 0, "names a shell would read specially are refused");

    /* Installed state and results. */
    dpkg_installed_mc = 1;
    pkgs_refresh();
    expect(pkgs_installed(mc) && !pkgs_installed(0), "dpkg-query decides installed");
    next_rc = 0;  expect(pkgs_install(0) == PKGS_OK && strstr(last, "dpkg-query"), "install ok, then state refreshed");
    next_rc = 2;  expect(pkgs_install(0) == PKGS_E_NET, "update failed: no network");
    next_rc = 3;  expect(pkgs_remove(mc) == PKGS_E_REFUSED, "refused: something else would change");
    next_rc = 100; expect(pkgs_install(0) == PKGS_E_FAIL, "apt error");
    expect(pkgs_install(99) == PKGS_E_BAD, "out of range: not in the catalog");

    /* A custom catalog: bad names are skipped. */
    pkgs_set_catalog("[ok-pkg]\ndesc = one\n[stock]\nbinaries = x\n[Bad;Name]\ndesc = no\n[two]\n");
    expect(pkgs_count() == 2 && !strcmp(pkgs_name(1), "two") && !strcmp(pkgs_desc(0), "one"), "custom catalog parsed, bad names skipped");

    printf("%s\n", fails ? "FAILED" : "all passed");
    return fails ? 1 : 0;
}
