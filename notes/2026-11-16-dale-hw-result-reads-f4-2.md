---
type: hw-result
from: dale (recorded by claude)
to: claude
date: 2026-11-16
re: F4 step 4 (in-game lid) and #185 Wolfenstein 3D; follows 2026-11-16-dale-hw-result-reads-f4.md
status: open
---

# Device reads, 13:28-13:39 (read-only, over SSH, a RetroArch game running)

- RetroArch **1.20.0**; `GET_STATUS` answers `PLAYING <system>,<game>,crc32=…` over UDP 55355.
- **`SAVE_FILES` works**: sent during Super Mario World (Snes9x), no reply (normal), and
  `/mnt/sdcard/saves_RA/Roms/SFC/Snes9x/Super Mario World (USA).srm` was rewritten within 1 s.
- Real save folders are on the **cards**: `/mnt/mmc/saves_RA/…` (TF1 = p1) and `/mnt/sdcard/saves_RA/…`
  (TF2), plus a few `.srm` next to ROMs. `RA_launch.sh` runs `${RABIN} -c ${RACONFIG}`, a different
  config from `/.config/retroarch/retroarch.cfg` (that file's `saves` folder is empty).
- `/.config/retroarch/config/remaps` exists and is empty.
- **Wolfenstein 3D is a GBA ROM** (`Roms/GBA/Wolfenstein 3D (USA, Europe).gba`): the GBA has no X/Y,
  so "Y and X do nothing" (#185 finding 3) is expected, not a bug.
