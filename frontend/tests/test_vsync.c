/* test_vsync: the embedded vsync library is the built one; installing it writes it and gives the
   LD_PRELOAD line; loaded, it passes frames through and switches itself off when the framebuffer
   can't wait for vblank (a plain file here). Exit code 0 = pass. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "vsync.h"

static int failures;
static void expect(int ok, const char *label) {
    printf("%s %s\n", ok ? "ok  " : "FAIL", label);
    if (!ok) failures++;
}

int main(void) {
    struct stat st;
    expect(stat(VSYNC_SO, &st) == 0 && (size_t)st.st_size == redux_vsync_so_len,
           "embedded library has the built library's size");

    char env[300];
    vsync_set_path("/tmp/redux-vsync-test.so");
    expect(vsync_install(env, sizeof env) == 0 && strcmp(env, "LD_PRELOAD=/tmp/redux-vsync-test.so") == 0,
           "install writes it and returns the LD_PRELOAD line");
    expect(stat("/tmp/redux-vsync-test.so", &st) == 0 && (size_t)st.st_size == redux_vsync_so_len,
           "installed file is complete");

    FILE *f = fopen("/tmp/redux-vsync-fb", "w");
    if (f) fclose(f);
    setenv("REDUX_VSYNC_FB", "/tmp/redux-vsync-fb", 1);   /* not a framebuffer: ioctl fails */

    /* As on the device: the app is started with the library in LD_PRELOAD. */
    char out[128] = "";
    FILE *p = popen("LD_PRELOAD=/tmp/redux-vsync-test.so " VSYNC_APP " 2>/dev/null", "r");
    if (p) { if (!fgets(out, sizeof out, p)) out[0] = '\0'; pclose(p); }
    expect(strcmp(out, "presents=3 state=-1\n") == 0,
           "preloaded: every frame reaches SDL; no vblank wait on this file, so it switched off");
    p = popen(VSYNC_APP, "r");
    out[0] = '\0';
    if (p) { if (!fgets(out, sizeof out, p)) out[0] = '\0'; pclose(p); }
    expect(strcmp(out, "presents=3 state=99\n") == 0, "without the preload the app runs unchanged");
    remove("/tmp/redux-vsync-test.so");
    remove("/tmp/redux-vsync-fb");
    printf("%d failed\n", failures);
    return failures != 0;
}
