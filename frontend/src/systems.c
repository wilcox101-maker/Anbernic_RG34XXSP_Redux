#include "systems.h"
#include "launcher.h"
#include <string.h>

/* Paths confirmed on device 2026-11-10. 2026-11-11: the .dge launch failed on
   the device for SMS, PCE, SFC and WS (exit to the stock "Exit Game" screen),
   so those systems use RetroArch cores that are installed in
   /mnt/vendor/deep/retro/cores. PS and NEOGEO are the stock folder names. */
static const system_entry_t systems[] = {
    { "GBA",  LAUNCH_RETROARCH, "mgba_libretro.so" },
    { "GB",   LAUNCH_RETROARCH, "gambatte_libretro.so" },
    { "GBC",  LAUNCH_RETROARCH, "gambatte_libretro.so" },
    { "NES",  LAUNCH_RETROARCH, "fceumm_libretro.so" },
    { "SNES", LAUNCH_RETROARCH, "snes9x_libretro.so" },
    { "MD",   LAUNCH_RETROARCH, "genesis_plus_gx_libretro.so" },
    { "PS1",  LAUNCH_RETROARCH, "pcsx_rearmed_libretro.so" },
    { "NEOGEO", LAUNCH_RETROARCH, "fbneo_libretro.so" },
    { "FBNEO", LAUNCH_RETROARCH, "fbneo_libretro.so" },
    { "CPS1", LAUNCH_RETROARCH, "fbalpha2012_cps1_libretro.so" },
    { "CPS2", LAUNCH_RETROARCH, "fbalpha2012_cps2_libretro.so" },
    { "CPS3", LAUNCH_RETROARCH, "fbalpha2012_cps3_libretro.so" },
    { "PSP",  LAUNCH_PPSSPP,    NULL },
    { "NDS",  LAUNCH_DRASTIC,   NULL },
    { "FC",   LAUNCH_RETROARCH, "fceumm_libretro.so" },
    { "SFC",  LAUNCH_RETROARCH, "snes9x_libretro.so" },
    { "GBA_DGE", LAUNCH_DGE,    "/mnt/vendor/bin/game/gba/gba_emu.dge" },
    { "GBC_DGE", LAUNCH_DGE,    "/mnt/vendor/bin/game/gbc/gbcgb_emu.dge" },
    { "MD_DGE",  LAUNCH_DGE,    "/mnt/vendor/bin/game/md/md_emu.dge" },
    { "SMS",  LAUNCH_RETROARCH, "picodrive_libretro.so" },
    { "GG",   LAUNCH_RETROARCH, "picodrive_libretro.so" },
    { "PCE",  LAUNCH_RETROARCH, "mednafen_pce_fast_libretro.so" },
    { "PCECD", LAUNCH_RETROARCH, "mednafen_pce_fast_libretro.so" },
    { "WS",   LAUNCH_RETROARCH, "mednafen_wswan_libretro.so" },
    { "NGP",  LAUNCH_RETROARCH, "mednafen_ngp_libretro.so" },
    { "PCSX", LAUNCH_PCSX,      NULL },
    { NULL,    0,                NULL }
};

const system_entry_t *systems_lookup(const char *name) {
    for (const system_entry_t *s = systems; s->name; s++)
        if (strcmp(s->name, name) == 0)
            return s;
    return NULL;
}
