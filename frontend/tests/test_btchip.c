/* Bluetooth chip init against a temp setting file and a fake hci0 (no device). Exit 0 = pass. */
#include "btchip.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int fails;
static char last[256];
static int runs;
static void expect(int ok, const char *what) { printf("%s %s\n", ok ? "ok  " : "FAIL", what); if (!ok) fails++; }
static int fake_run(const char *cmd) { snprintf(last, sizeof last, "%s", cmd); runs++; return 0; }

int main(void) {
    const char *setting = "/tmp/redux-btchip-test", *hci = "/tmp/redux-btchip-hci0";
    remove(setting); rmdir(hci);
    btchip_set_paths(setting, hci);
    btchip_set_run(fake_run);

    expect(btchip_enabled() == 1, "no setting file: on, as stock");
    expect(btchip_at_start() == 1 && strstr(last, "setBluetooth.sh init") && strstr(last, " &"), "start with no hci0: init runs in the background");
    mkdir(hci, 0755);
    runs = 0;
    expect(btchip_at_start() == 0 && runs == 0, "start with hci0 present: nothing runs");
    expect(btchip_set(0) == 0 && strstr(last, "hciconfig hci0 down") && btchip_enabled() == 0, "Off: saved, radio taken down");
    rmdir(hci);
    runs = 0;
    expect(btchip_at_start() == 0 && runs == 0, "start when Off: no init");
    runs = 0; last[0] = 0;
    expect(btchip_set(1) == 0 && strstr(last, "setBluetooth.sh init") && !strstr(last, " &") && btchip_enabled() == 1, "On: saved, init run now (foreground)");
    mkdir(hci, 0755);
    runs = 0;
    expect(btchip_set(1) == 0 && runs == 0, "On with hci0 present: saved, nothing run");
    rmdir(hci);
    expect(btchip_set(0) == 0 && btchip_enabled() == 0, "Off with no hci0: saved, nothing to take down");
    btchip_set_paths("/tmp/redux-no-such-dir/x", NULL);
    expect(btchip_set(1) == -1, "unwritable setting: -1");
    remove(setting);
    printf("%s\n", fails ? "test_btchip: FAILED" : "test_btchip: all passed");
    return fails ? 1 : 0;
}
