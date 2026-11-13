#include "ports.h"

static const port_t ports[] = {
    { "Cannonball",                     "cannonball_libretro.so", "Cannonball/epr-10187.88" },
    { "Cave Story",                     "nxengine_libretro.so",   "Cave_Story/Doukutsu.exe" },
    { "Dinothawr",                      "dinothawr_libretro.so",  "Dinothawr/dinothawr.game" },
    { "Quake",                          "tyrquake_libretro.so",   "id1/PAK0.PAK" },
    { "Quake Dopa",                     "tyrquake_libretro.so",   "dopa/pak0.pak" },
    { "Quake Hipnotic",                 "tyrquake_libretro.so",   "hipnotic/pak0.pak" },
    { "Quake Rogue",                    "tyrquake_libretro.so",   "rogue/pak0.pak" },
    { "Rick Dangerous",                 "xrick_libretro.so",      "Rick_Dangerous/data.zip" },
    { "Super 3D Noah's Ark",            "ecwolf_libretro.so",     "Super_3D_Noahs_Ark/n3d.ecwolf" },
    { "Wolfenstein 3D",                 "ecwolf_libretro.so",     "Wolfenstein_3D/WL6.ecwolf" },
    { "Wolfenstein Spear of Destiny",   "ecwolf_libretro.so",     "wsd/SOD.ecwolf" },
};

int ports_count(void) { return (int)(sizeof ports / sizeof ports[0]); }

const port_t *ports_get(int i) {
    return (i >= 0 && i < ports_count()) ? &ports[i] : 0;
}
