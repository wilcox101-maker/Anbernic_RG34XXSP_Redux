/* Ports, option B part 1: only known ports whose data file exists are listed, each with its
   own core; scripts and other files in PORTS are not. Run ./test_ports, exit 0 = pass. */
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "romscan.h"
#include "launcher.h"
#include "ports.h"

static int fails;
static void expect(int ok, const char *what) {
    printf("%s %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) fails++;
}
static void touch(const char *p) { FILE *f = fopen(p, "a"); if (f) fclose(f); }

int main(void) {
    const char *R = "/tmp/redux-ports-fixture";
    mkdir(R, 0755);
    mkdir("/tmp/redux-ports-fixture/GBA", 0755);
    touch("/tmp/redux-ports-fixture/GBA/a.gba");
    mkdir("/tmp/redux-ports-fixture/PORTS", 0755);
    mkdir("/tmp/redux-ports-fixture/PORTS/Cannonball", 0755);
    touch("/tmp/redux-ports-fixture/PORTS/Cannonball/epr-10187.88");
    mkdir("/tmp/redux-ports-fixture/PORTS/id1", 0755);
    touch("/tmp/redux-ports-fixture/PORTS/id1/PAK0.PAK");
    touch("/tmp/redux-ports-fixture/PORTS/Quake.sh");
    touch("/tmp/redux-ports-fixture/PORTS/Unknown Port.sh");

    expect(ports_count() == 11, "table has the 11 stock ports");
    romscan_load(R);
    int p = -1;
    for (int i = 0; i < romscan_systems(); i++)
        if (strcmp(romscan_key(i), "PORTS") == 0) p = i;
    expect(p >= 0, "PORTS folder becomes a system");
    expect(p >= 0 && strcmp(romscan_title(p), "Ports") == 0, "titled Ports");
    expect(p >= 0 && romscan_roms(p) == 2, "only the two known ports with data are listed");
    expect(p >= 0 && strcmp(romscan_rom_name(p, 0), "Cannonball") == 0
                  && strcmp(romscan_rom_name(p, 1), "Quake") == 0, "sorted by name");
    expect(p >= 0 && strcmp(romscan_rom_core(p, 1), "tyrquake_libretro.so") == 0, "Quake carries its own core");
    expect(p >= 0 && strcmp(romscan_rom_path(p, 1), "/tmp/redux-ports-fixture/PORTS/id1/PAK0.PAK") == 0,
           "path is the data file, not a script");
    int g = romscan_index_of("GBA");
    expect(romscan_rom_core(g, 0) == NULL, "ordinary ROMs have no own core");

    const char *next = "/tmp/redux-ports-next";
    launcher_set_next_path(next);
    char line[512] = "";
    if (p >= 0 && launcher_start(LAUNCH_RETROARCH, romscan_rom_core(p, 1), romscan_rom_path(p, 1)) == 0) {
        FILE *f = fopen(next, "r");
        if (f) { if (!fgets(line, sizeof line, f)) line[0] = '\0'; fclose(f); }
    }
    expect(strcmp(line, "AUDIODEV=hw:2,0 /bin/bash /mnt/mod/ctrl/RA_launch.sh 'tyrquake_libretro.so' "
                        "'/tmp/redux-ports-fixture/PORTS/id1/PAK0.PAK' auto\n") == 0,
           "launches through RA_launch.sh with the port's core and data file");
    remove(next);
    printf("%d failed\n", fails);
    return fails ? 1 : 0;
}
