#include "launcher.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Stock launch scripts live under /mnt/vendor and /mnt/mod. We only write the
   next-command file; we never format or modify the card. */

static const char *script_for(launch_target_t t) {
    switch (t) {
    case LAUNCH_RETROARCH: return "/mnt/mod/ctrl/RA_launch.sh";
    case LAUNCH_PPSSPP:    return "/mnt/vendor/bin/ppsspp/PPSSPPSDL";
    case LAUNCH_DRASTIC:   return "/mnt/vendor/deep/drastic-modify/launch.sh";
    case LAUNCH_PCSX:      return "/mnt/vendor/bin/pcsx";
    case LAUNCH_PORTMASTER:return "/roms/ports/PortMaster/PortMaster.sh";
    default:               return NULL;
    }
}

int launcher_start(launch_target_t target, const char *rom_path) {
    const char *script = script_for(target);
    if (!script) return -1;

    FILE *f = fopen("/tmp/.next", "w");
    if (!f) return -1;
    if (rom_path)
        fprintf(f, "sh %s \"%s\"\n", script, rom_path);
    else
        fprintf(f, "sh %s\n", script);
    fclose(f);

    /* Exit so loadapp.sh picks up /tmp/.next and runs it. */
    exit(0);
}

int launcher_resume_last(void) {
    FILE *f = fopen("/mnt/mod/.last", "r");
    if (!f) return 0;
    char cmd[512];
    if (!fgets(cmd, sizeof cmd, f)) { fclose(f); return 0; }
    fclose(f);

    FILE *next = fopen("/tmp/.next", "w");
    if (!next) return -1;
    fputs(cmd, next);
    fclose(next);
    exit(0);
}
