/* SSH / SMB switches: system.cfg key rewrite and the service commands (no device, no systemctl). */
#define _DEFAULT_SOURCE
#include "netsvc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fails;
static void expect(int c, const char *w) { printf("%s %s\n", c ? "ok  " : "FAIL", w); if (!c) fails++; }
static char cmds[1024];
static int fail_run;
static int fake_run(const char *c) { strcat(cmds, c); strcat(cmds, ";"); return fail_run ? -1 : 0; }

static int file_is(const char *p, const char *want) {
    char buf[1024] = ""; FILE *f = fopen(p, "r"); if (!f) return 0;
    size_t n = fread(buf, 1, sizeof buf - 1, f); fclose(f); buf[n] = 0;
    if (strcmp(buf, want) != 0) { printf("  got: [%s]\n", buf); return 0; }
    return 1;
}

int main(void) {
    char dir[] = "/tmp/redux-netsvc-XXXXXX";
    if (!mkdtemp(dir)) return 2;
    char p[300]; snprintf(p, sizeof p, "%s/system.cfg", dir);
    FILE *f = fopen(p, "w");
    fputs("global.theme=1\nglobal.ssh=1\nglobal.samba=0\n# comment\n", f); fclose(f);
    netsvc_set_path(p);
    netsvc_set_run(fake_run);

    expect(netsvc_get(NETSVC_SSH) == 1 && netsvc_get(NETSVC_SMB) == 0, "reads ssh=1 samba=0");
    expect(netsvc_set(NETSVC_SMB, 1) == 0, "SMB on");
    expect(file_is(p, "global.theme=1\nglobal.ssh=1\nglobal.samba=1\n# comment\n"), "only the samba line changes");
    expect(strcmp(cmds, "systemctl start smbd;systemctl start nmbd;") == 0, "starts smbd and nmbd, like stock");
    cmds[0] = 0;
    expect(netsvc_set(NETSVC_SSH, 0) == 0 && netsvc_get(NETSVC_SSH) == 0, "SSH off");
    expect(strcmp(cmds, "systemctl stop ssh;") == 0, "stops ssh");
    fail_run = 1; cmds[0] = 0;
    expect(netsvc_set(NETSVC_SSH, 1) == -2 && netsvc_get(NETSVC_SSH) == 1, "service failure: -2, key still saved");
    fail_run = 0;
    /* key missing: appended */
    f = fopen(p, "w"); fputs("global.theme=1\n", f); fclose(f);
    expect(netsvc_get(NETSVC_SSH) == 0, "missing key reads off");
    expect(netsvc_set(NETSVC_SSH, 1) == 0 && file_is(p, "global.theme=1\nglobal.ssh=1\n"), "missing key appended");
    /* no file: nothing run */
    netsvc_set_path("/tmp/redux-netsvc-none/system.cfg"); cmds[0] = 0;
    expect(netsvc_set(NETSVC_SSH, 1) == -1 && cmds[0] == 0, "no system.cfg: -1, nothing run");
    char tmp[320]; snprintf(tmp, sizeof tmp, "%s.redux-new", p);
    expect(fopen(tmp, "r") == NULL, "no temp file left");
    printf("%s\n", fails ? "FAILED" : "all passed");
    return fails ? 1 : 0;
}
