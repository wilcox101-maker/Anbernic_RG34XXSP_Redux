#include "launcher.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *next_path = "/tmp/.next";

void launcher_set_next_path(const char *path) {
    next_path = path;
}

/* Quote a path for a shell single-quoted string: 'foo'bar' -> 'foo'\''bar' */
static void quote(FILE *f, const char *s) {
    fputc('\'', f);
    for (; *s; s++) {
        if (*s == '\'') {
            fputs("'\\''", f);
        } else {
            fputc(*s, f);
        }
    }
    fputc('\'', f);
}

/* ELF emulators: stock dmenu.bin runs them as ./NAME from their own folder
   (PPSSPP needs ./assets), so we cd there first. */
static const char *elf_dir_for(launch_target_t t) {
    switch (t) {
    case LAUNCH_PPSSPP: return "/mnt/vendor/deep/ppsspp";
    case LAUNCH_PCSX:   return "/mnt/vendor/bin/game/pcsx_rearmed";
    default:            return NULL;
    }
}

static const char *script_for(launch_target_t t) {
    switch (t) {
    case LAUNCH_RETROARCH:  return "/mnt/mod/ctrl/RA_launch.sh";
    case LAUNCH_PPSSPP:     return "/mnt/vendor/deep/ppsspp/PPSSPPSDL";
    case LAUNCH_DRASTIC:    return "/mnt/vendor/deep/drastic-modify/launch.sh";
    case LAUNCH_PCSX:       return "/mnt/vendor/bin/game/pcsx_rearmed/pcsx";
    case LAUNCH_PORTMASTER: return "/roms/ports/PortMaster/PortMaster.sh";
    default:                return NULL;
    }
}

int launcher_start(launch_target_t target, const char *core, const char *rom_path) {
    FILE *f = fopen(next_path, "w");
    if (!f) return -1;

    const char *audiodev = "AUDIODEV=hw:2,0 ";

    if (target == LAUNCH_DGE) {
        /* core is the .dge binary path. */
        if (!core) { fclose(f); return -1; }
        fputs(audiodev, f);
        fprintf(f, "%s ", core);
        if (rom_path) quote(f, rom_path);
        fputc('\n', f);
    } else {
        const char *script = script_for(target);
        if (!script) { fclose(f); return -1; }

        if (!elf_dir_for(target)) fputs(audiodev, f);

        if (target == LAUNCH_RETROARCH) {
            if (!core) { fclose(f); return -1; }
            fprintf(f, "sh %s ", script);
            quote(f, core);
            fputc(' ', f);
            if (rom_path) { quote(f, rom_path); fprintf(f, " auto"); }
            fputc('\n', f);
        } else if (target == LAUNCH_PPSSPP || target == LAUNCH_PCSX) {
            const char *dir = elf_dir_for(target);
            const char *name = strrchr(script, '/') + 1;
            fprintf(f, "cd ");
            quote(f, dir);
            fprintf(f, " && %s./%s ", audiodev, name);
            if (rom_path) quote(f, rom_path);
            fputc('\n', f);
        } else {
            fprintf(f, "sh %s ", script);
            if (rom_path) quote(f, rom_path);
            fputc('\n', f);
        }
    }

    fclose(f);
    return 0;
}
