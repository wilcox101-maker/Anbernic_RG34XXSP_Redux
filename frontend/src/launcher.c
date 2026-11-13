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

int launcher_start_app(const char *bin, const char *env, const char *args) {
    const char *slash = bin ? strrchr(bin, '/') : NULL;
    if (!slash || slash == bin || !slash[1]) return -1;
    char dir[512];
    size_t n = (size_t)(slash - bin);
    if (n >= sizeof dir) return -1;
    memcpy(dir, bin, n);
    dir[n] = '\0';
    FILE *f = fopen(next_path, "w");
    if (!f) return -1;
    fprintf(f, "cd ");
    quote(f, dir);
    fprintf(f, " && ");
    if (env && env[0]) fprintf(f, "%s ", env);
    fprintf(f, "./");
    quote(f, slash + 1);
    if (args && args[0]) fprintf(f, " %s", args);
    fputc('\n', f);
    fclose(f);
    return 0;
}

int launcher_start(launch_target_t target, const char *core, const char *rom_path) {
    FILE *f = fopen(next_path, "w");
    if (!f) return -1;

    /* dmenu.bin prefixes launches with AUDIODEV=hw:2,0.
       A shell variable prefix applies only to the command right after it,
       so for ELF targets it goes after the cd, directly in front of the emulator. */
    const char *audiodev = "AUDIODEV=hw:2,0 ";

    if (target == LAUNCH_SCRIPT) {
        /* Same as stock dmenu.bin for PORTS: from the script's own folder, with bash. */
        const char *slash = core ? strrchr(core, '/') : NULL;
        if (!slash || slash == core || !slash[1]) { fclose(f); return -1; }
        char dir[512];
        size_t n = (size_t)(slash - core);
        if (n >= sizeof dir) { fclose(f); return -1; }
        memcpy(dir, core, n);
        dir[n] = '\0';
        fprintf(f, "cd ");
        quote(f, dir);
        fprintf(f, " && %s/bin/bash ./", audiodev);
        quote(f, slash + 1);
        fputc('\n', f);
    } else if (target == LAUNCH_DGE) {
        /* core is the .dge binary path. Stock dmenu.bin runs a .dge as
           cd "<dir>" && ./<name>.dge (strings, 2026-11-13), and the .dge files load
           their resources relative to that folder, so do the same. */
        if (!core) { fclose(f); return -1; }
        const char *slash = strrchr(core, '/');
        if (!slash || slash == core || !slash[1]) { fclose(f); return -1; }
        char dir[512];
        size_t n = (size_t)(slash - core);
        if (n >= sizeof dir) { fclose(f); return -1; }
        memcpy(dir, core, n);
        dir[n] = '\0';
        fprintf(f, "cd ");
        quote(f, dir);
        fprintf(f, " && %s./", audiodev);
        quote(f, slash + 1);
        if (rom_path) { fputc(' ', f); quote(f, rom_path); }
        fputc('\n', f);
    } else {
        const char *script = script_for(target);
        if (!script) { fclose(f); return -1; }

        if (!elf_dir_for(target)) fputs(audiodev, f);

        /* The stock launch scripts are bash (RA_launch.sh uses [[ ]] and declare -A);
           plain sh fails on them at once. bash also runs any POSIX sh script. */
        if (target == LAUNCH_RETROARCH) {
            if (!core) { fclose(f); return -1; }
            fprintf(f, "/bin/bash %s ", script);
            quote(f, core);
            fputc(' ', f);
            /* RA_launch.sh: $1 core, $2 ROM, $3 "auto".
               Without a ROM, "auto" would land in $2 and be taken as the ROM. */
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
            fprintf(f, "/bin/bash %s ", script);
            if (rom_path) quote(f, rom_path);
            fputc('\n', f);
        }
    }

    fclose(f);
    return 0;
}
