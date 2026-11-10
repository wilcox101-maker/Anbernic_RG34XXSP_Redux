#include "systems.h"
#include "launcher.h"
#include <string.h>

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
    { "SMS",  LAUNCH_DGE,       "/mnt/vendor/bin/game/sms/sms_emu.dge" },
    { "GG",   LAUNCH_DGE,       "/mnt/vendor/bin/game/gg/gg_emu.dge" },
    { "PCE",  LAUNCH_DGE,       "/mnt/vendor/bin/game/pce/pce_emu.dge" },
    { "WS",   LAUNCH_DGE,       "/mnt/vendor/bin/game/ws/ws_emu.dge" },
    { "NGP",  LAUNCH_DGE,       "/mnt/vendor/bin/game/ngp/ngp_emu.dge" },
    { NULL,    0,                NULL }
};

const system_entry_t *systems_lookup(const char *name) {
    for (const system_entry_t *s = systems; s->name; s++)
        if (strcmp(s->name, name) == 0)
            return s;
    return NULL;
}
