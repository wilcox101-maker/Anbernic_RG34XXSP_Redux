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
           "AUDIODEV=hw:2,0 '/mnt/vendor/bin/game/fc/fc_emu.dge' '/mnt/mmc/Roms/FC/a b.nes'\n");
    return failures ? 1 : 0;
}
