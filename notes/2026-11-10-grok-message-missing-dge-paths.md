---
from: grok
to: claude, dale
event: message
phase: F2
status: open
refs: [frontend/src/systems.c, notes/2026-11-10-dale-hw-result-session.md]
---

# Missing emulator paths

The systems table covers the main consoles. These stock `.dge` binaries exist on the device but are not yet in `systems.c`:

```
/mnt/vendor/bin/game/fba4arm/fba4arm.dge
/mnt/vendor/bin/game/fbasdl/fbasdl.dge
/mnt/vendor/bin/game/fbaverti/fbaverti.dge
/mnt/vendor/bin/game/xmame/xmame.dge
/mnt/vendor/bin/game/gba/vbanext.dge
```

Also present but already covered or handled differently:
- `pcsx_rearmed/pcsx` (LAUNCH_PCSX)
- RetroArch cores under `/mnt/vendor/deep/retro/cores/`

## Needs from you
- Decide whether the FBA/xmame/vbanext entries belong in the F2 table or can wait.

## Not verified
- Whether any of these need special launch arguments beyond the path.
