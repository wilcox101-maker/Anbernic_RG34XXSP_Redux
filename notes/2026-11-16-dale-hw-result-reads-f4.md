---
type: hw-result
from: dale (recorded by claude)
to: claude
date: 2026-11-16
re: #185 leftovers (video buttons, Wolfenstein 3D Y/X) and F4 step 4 (in-game lid, option A)
status: open
---

# Device reads, 13:22 (read-only, over SSH)

- **`/mnt/vendor/bin/video/FFplay.sh`**: runs `./ffplay -fs -autoexit -vf scale=640:-2 -i "$1"` from its own
  folder (`SDL_NOMOUSE=1 SDL_AUDIO_CHANNELS=2`, plus `AUDIODEV=hw:2,0` only when `$2` = `hdmi`), then
  `sync`. No input mapping in the script: ffplay only takes SDL keyboard keys, so the device buttons
  (event1 gamepad codes) never reach it. Fix belongs to the Redux video front-end (queue 4, F6).
- **`/mnt/vendor/deep/retro/config/remaps`**: does not exist.
- **`find / -xdev -name '*ecwolf*'`**: nothing. Wolfenstein 3D is not run by a core named ecwolf; the
  next read finds what runs it.
- **RetroArch config**: only `/.config/retroarch/retroarch.cfg`:
  `network_cmd_enable = "true"`, `network_cmd_port = "55355"`, `autosave_interval = "0"`,
  `savestate_auto_save = "true"`, `savefile_directory = "~/.config/retroarch/saves"`.

## What it means for F4 step 4 (option A)
- Redux can talk to RetroArch over UDP 55355 while a game runs.
- `autosave_interval = 0`: in-game saves (SRAM) are written only when the content closes. A lid
  sleep that ends in a power loss can lose them; that is what step 4 must cover.
- Saves go to `/.config/retroarch/saves` (system partition), not the ROM card (p1).
- Still unknown: RetroArch's version and which network commands it accepts (a save-files command
  exists only in some versions). Next read, during a game.
