#include "systems.h"
#include "launcher.h"
#include <string.h>

/* Paths confirmed on device 2026-11-10. */
static const system_entry_t systems[] = {
    { "GBA",  LAUNCH_RETROARCH, "mgba_libretro.so" },
    { "GB",   LAUNCH_RETROARCH, "gambatte_libretro.so" },
    { "GBC",  LAUNCH_RETROARCH, "gambatte_libretro.so" },
    { "NES",  LAUNCH_RETROARCH, "fceumm_libretro.so" },
    { "SNES", LAUNCH_RETROARCH, "snes9x_libretro.so" },
    { "MD",   LAUNCH_RETROARCH, "genesis_plus_gx_libretro.so" },
    { "PS1",  LAUNCH_RETROARCH, "pcsx_rearmed_libretro.so" },
    { "PSP",  LAUNCH_PPSSPP,    NULL },
    { "NDS",  LAUNCH_DRASTIC,   NULL },
    { "FC",   LAUNCH_DGE,       "/mnt/vendor/bin/game/fc/fc_emu.dge" },
    { "SFC",  LAUNCH_DGE,       "/mnt/vendor/bin/game/sfc/sfc_emu.dge" },
    { "GBA_DGE", LAUNCH_DGE,    "/mnt/vendor/bin/game/gba/gba_emu.dge" },
    { "GBC_DGE", LAUNCH_DGE,    "/mnt/vendor/bin/game/gbc/gbcgb_emu.dge" },
    { "MD_DGE",  LAUNCH_DGE,    "/mnt/vendor/bin/game/md/md_emu.dge" },
    { "SMS",  LAUNCH_DGE,       "/mnt/vendor/bin/game/sms/smsgg_emu.dge" },
    { "PCE",  LAUNCH_DGE,       "/mnt/vendor/bin/game/temper/pce.dge" },
    { "WS",   LAUNCH_DGE,       "/mnt/vendor/bin/game/wswan/swanemu.dge" },
    { "NGP",  LAUNCH_DGE,       "/mnt/vendor/bin/game/ngpc/ngp.dge" },
    { "PCSX", LAUNCH_PCSX,      NULL },
    { NULL,    0,                NULL }
};

const system_entry_t *systems_lookup(const char *name) {
    for (const system_entry_t *s = systems; s->name; s++)
        if (strcmp(s->name, name) == 0)
            return s;
    return NULL;
}
