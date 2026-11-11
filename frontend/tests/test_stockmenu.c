/* Back-to-stock command. Does not touch the device. Exit 0 = pass. */
#include "stockmenu.h"
#include <stdio.h>
#include <string.h>

static int fails;
static void expect(int cond, const char *what) {
    printf("%s %s\n", cond ? "ok  " : "FAIL", what);
    if (!cond) fails++;
}

static int calls;
static char cmd_ok;
static int run(const char *command) {
    calls++;
    cmd_ok = strcmp(command, STOCK_RESTORE_CMD) == 0;
    return 0;
}

int main(void) {
    expect(strcmp(STOCK_RESTORE_CMD,
                  "cp /mnt/vendor/ctrl/dmenu_ln.bak /mnt/vendor/ctrl/dmenu_ln && sync && reboot") == 0,
           "command is dale-scripts S7");
    stock_set_bak(0);
    stock_set_run(run);
    expect(stock_restore() == -2, "missing dmenu_ln.bak is refused");
    expect(calls == 0, "refusal does not run the command");
    stock_set_bak(1);
    expect(stock_restore() == 0, "backup present runs the hook");
    expect(calls == 1 && cmd_ok == 1, "the hook received the S7 command");
    printf("%d failed\n", fails);
    return fails ? 1 : 0;
}
