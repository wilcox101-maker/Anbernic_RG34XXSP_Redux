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
    /* MAME: stock has a MAME folder and xmame.dge, but .dge launches fail, so use the
       stock mame2003_plus core (listed on the device 2026-11-12). It wants 0.78-era romsets. */
    { "MAME", LAUNCH_RETROARCH, "mame2003_plus_libretro.so" },
    /* Folders on Dale's card, cores from the device's core list (2026-11-12). Empty
       folders are skipped by the scanner, so mapping them costs nothing. */
    { "A2600",      LAUNCH_RETROARCH, "stella2014_libretro.so" },
    { "A5200",      LAUNCH_RETROARCH, "a5200_libretro.so" },
    { "A7800",      LAUNCH_RETROARCH, "prosystem_libretro.so" },
    { "A800",       LAUNCH_RETROARCH, "atari800_libretro.so" },
    { "LYNX",       LAUNCH_RETROARCH, "handy_libretro.so" },
    { "ATARIST",    LAUNCH_RETROARCH, "hatari_libretro.so" },
    { "AMIGA",      LAUNCH_RETROARCH, "puae_libretro.so" },
    { "C64",        LAUNCH_RETROARCH, "vice_x64_libretro.so" },
    { "VIC20",      LAUNCH_RETROARCH, "vice_xvic_libretro.so" },
    { "MSX",        LAUNCH_RETROARCH, "bluemsx_libretro.so" },
    { "DOS",        LAUNCH_RETROARCH, "dosbox_pure_libretro.so" },
    { "SCUMMVM",    LAUNCH_RETROARCH, "scummvm_libretro.so" },
    { "EASYRPG",    LAUNCH_RETROARCH, "easyrpg_libretro.so" },
    { "ONS",        LAUNCH_RETROARCH, "onscripter_libretro.so" },
    { "FDS",        LAUNCH_RETROARCH, "fceumm_libretro.so" },
    { "GW",         LAUNCH_RETROARCH, "gw_libretro.so" },
    { "POKE",       LAUNCH_RETROARCH, "pokemini_libretro.so" },
    { "VB",         LAUNCH_RETROARCH, "mednafen_vb_libretro.so" },
    { "PICO",       LAUNCH_RETROARCH, "fake08_libretro.so" },
    { "SEGA32X",    LAUNCH_RETROARCH, "picodrive_libretro.so" },
    { "MDCD",       LAUNCH_RETROARCH, "genesis_plus_gx_libretro.so" },
    { "SATURN",     LAUNCH_RETROARCH, "yabasanshiro_libretro.so" },
    { "N64",        LAUNCH_RETROARCH, "mupen64plus_next_libretro.so" },
    { "DREAMCAST",  LAUNCH_RETROARCH, "flycast_libretro.so" },
    { "NAOMI",      LAUNCH_RETROARCH, "flycast_libretro.so" },
    { "ATOMISWAVE", LAUNCH_RETROARCH, "flycast_libretro.so" },
    { "NEOCD",      LAUNCH_RETROARCH, "neocd_libretro.so" },
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
