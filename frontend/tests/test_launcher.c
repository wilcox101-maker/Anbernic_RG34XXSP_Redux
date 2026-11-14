/* Desktop test for the F2 launcher: checks the exact /tmp/.next line per target.
   Built by CMake as `test_launcher`; run it, exit code 0 = pass. */
#include <stdio.h>
#include <string.h>
#include "launcher.h"

static int failures;

static void expect(const char *label, int target, const char *core, const char *rom, const char *want) {
    const char *path = "test_launcher_next.txt";
    launcher_set_next_path(path);
    int rc = launcher_start((launch_target_t)target, core, rom);
    char got[512] = "";
    FILE *f = fopen(path, "r");
    if (f) { if (!fgets(got, sizeof got, f)) got[0] = '\0'; fclose(f); }
    remove(path);
    int ok = want ? (rc == 0 && strcmp(got, want) == 0) : (rc == -1);
    printf("%s %s\n", ok ? "ok  " : "FAIL", label);
    if (!ok) { printf("  want: %s  got:  %s (rc=%d)\n", want ? want : "(rc=-1)\n", got, rc); failures++; }
}

int main(void) {
    expect("retroarch + rom", LAUNCH_RETROARCH, "gpsp_libretro.so", "/mnt/mmc/Roms/GBA/a.gba",
           "AUDIODEV=hw:2,0 /bin/bash /mnt/mod/ctrl/RA_launch.sh 'gpsp_libretro.so' '/mnt/mmc/Roms/GBA/a.gba' auto\n");
    expect("retroarch, no rom", LAUNCH_RETROARCH, "gpsp_libretro.so", NULL,
           "AUDIODEV=hw:2,0 /bin/bash /mnt/mod/ctrl/RA_launch.sh 'gpsp_libretro.so' \n");
    expect("retroarch needs a core", LAUNCH_RETROARCH, NULL, "/x.gba", NULL);
    expect("rom name with ' $ ` is literal", LAUNCH_RETROARCH, "gpsp_libretro.so", "/r/It's $HOME `id`.gba",
           "AUDIODEV=hw:2,0 /bin/bash /mnt/mod/ctrl/RA_launch.sh 'gpsp_libretro.so' '/r/It'\\''s $HOME `id`.gba' auto\n");
    expect("ppsspp runs from its folder", LAUNCH_PPSSPP, NULL, "/mnt/mmc/Roms/PSP/a b.iso",
           "cd '/mnt/vendor/deep/ppsspp' && AUDIODEV=hw:2,0 ./PPSSPPSDL '/mnt/mmc/Roms/PSP/a b.iso'\n");
    expect("pcsx runs from its folder", LAUNCH_PCSX, NULL, "/mnt/mmc/Roms/PS/c.chd",
           "cd '/mnt/vendor/bin/game/pcsx_rearmed' && AUDIODEV=hw:2,0 ./pcsx '/mnt/mmc/Roms/PS/c.chd'\n");
    expect("drastic script", LAUNCH_DRASTIC, NULL, "/mnt/mmc/Roms/NDS/d.nds",
           "AUDIODEV=hw:2,0 /bin/bash /mnt/vendor/deep/drastic-modify/launch.sh '/mnt/mmc/Roms/NDS/d.nds'\n");
    expect(".dge needs a binary", LAUNCH_DGE, NULL, "/x", NULL);
    expect(".dge binary + rom", LAUNCH_DGE, "/mnt/vendor/bin/game/fc/fc_emu.dge", "/mnt/mmc/Roms/FC/a b.nes",
           "cd '/mnt/vendor/bin/game/fc' && AUDIODEV=hw:2,0 ./'fc_emu.dge' '/mnt/mmc/Roms/FC/a b.nes'\n");
    expect(".dge app, no rom", LAUNCH_DGE, "/mnt/vendor/bin/music/ap.dge", NULL,
           "cd '/mnt/vendor/bin/music' && AUDIODEV=hw:2,0 ./'ap.dge'\n");
    expect(".dge bare name rejected", LAUNCH_DGE, "ap.dge", NULL, NULL);
    expect("openbor: seed script, then the engine with the pak", LAUNCH_OPENBOR, NULL, "/mnt/mmc/Roms/OPENBOR/Final Fight LNS.pak",
           "/bin/bash /mnt/vendor/deep/openBOR/scripts/openbor.sh '/mnt/mmc/Roms/OPENBOR/Final Fight LNS.pak'"
           " >/tmp/redux-app.out 2>&1; cd /mnt/vendor/deep/openBOR && ./OpenBOR.dge '/mnt/mmc/Roms/OPENBOR/Final Fight LNS.pak'"
           " >>/tmp/redux-app.out 2>&1; rc=$?; { echo \"[openbor] exit=$rc\"; tail -c 1200 /tmp/redux-app.out; echo; }"
           " >>/mnt/vendor/redux/redux.log 2>/dev/null; rm -f /tmp/redux-app.out\n");
    expect("openbor needs a pak", LAUNCH_OPENBOR, NULL, NULL, NULL);
    expect("card script from its folder", LAUNCH_SCRIPT, "/mnt/mmc/Roms/PORTS/It's.sh", NULL,
           "cd '/mnt/mmc/Roms/PORTS' && AUDIODEV=hw:2,0 /bin/bash ./'It'\\''s.sh'\n");
    expect("script needs a path", LAUNCH_SCRIPT, NULL, NULL, NULL);
    {
        const char *path = "test_launcher_app.txt";
        launcher_set_next_path(path);
        int rc = launcher_start_app("/mnt/mod/ctrl/SimpleTerminal", "HOME=/root", "-e top");
        char got[256] = "";
        FILE *f = fopen(path, "r");
        if (f) { if (!fgets(got, sizeof got, f)) got[0] = '\0'; fclose(f); }
        remove(path);
        int ok = rc == 0 && strcmp(got, "cd '/mnt/mod/ctrl' && HOME=/root ./'SimpleTerminal' -e top\n") == 0;
        printf("%s app from its folder with env and args\n", ok ? "ok  " : "FAIL");
        if (!ok) failures++;
        ok = launcher_start_app("SimpleTerminal", NULL, NULL) == -1;
        printf("%s app needs a folder\n", ok ? "ok  " : "FAIL");
        if (!ok) failures++;
    }
    {
        const char *path = "test_launcher_video.txt";
        launcher_set_next_path(path);
        int rc = launcher_start_video("/mnt/vendor/bin/video/vp.dge");
        char got[1024] = "";
        FILE *f = fopen(path, "r");
        if (f) { if (!fgets(got, sizeof got, f)) got[0] = '\0'; fclose(f); }
        remove(path);
        const char *want = "cd /mnt/vendor/bin/video && exec /bin/bash -c '"
            "L=/mnt/vendor/redux/redux.log; T=/tmp/redux-vp.out; "
            "while :; do rm -f /tmp/.next; AUDIODEV=hw:2,0 ./vp.dge >$T 2>&1; rc=$?; "
            "{ echo \"[video] player exit=$rc next=$(head -c 300 /tmp/.next 2>/dev/null) ini=$(head -c 200 /tmp/vpRun.ini 2>/dev/null)\"; "
            "tail -c 600 $T; echo; } >>$L 2>/dev/null; "
            "grep -q FFplay.sh /tmp/.next 2>/dev/null || break; "
            "mv -f /tmp/.next /tmp/.vpnext; sh /tmp/.vpnext >$T 2>&1; rc=$?; "
            "{ echo \"[video] playback exit=$rc\"; tail -c 600 $T; echo; } >>$L 2>/dev/null; done; "
            "rm -f /tmp/vpRun.ini /tmp/.vpnext /tmp/.next $T'\n";
        int ok = rc == 0 && strcmp(got, want) == 0;
        printf("%s video player loop\n", ok ? "ok  " : "FAIL");
        if (!ok) { printf("  want: %s  got:  %s", want, got); failures++; }
        ok = launcher_start_video("/mnt/x/it's.dge") == -1 && launcher_start_video("vp.dge") == -1
             && launcher_start_video(NULL) == -1;
        printf("%s video player refuses quotes and bare names\n", ok ? "ok  " : "FAIL");
        if (!ok) failures++;
    }
    return failures ? 1 : 0;
}
