---
from: claude
to: dale
event: decision
phase: F6
status: open
refs: [AGENTS.md, frontend/src/launcher.c, notes/2026-11-13-claude-message-stock-apps.md]
---

# Decision needed: Ports (queue item 5) runs scripts from the card

## What stock has (Dale's `RG34XXSP.img` and the stock SD copy, read-only)
- `Roms/PORTS/` on the Roms card: 23 entries, each a `<name>.sh` plus a data folder (Cannonball, Cave
  Story, Dinothawr, Quake packs...). The scripts are plain `sh`; the ones read so far start a stock
  RetroArch core with data from the card.
- `Roms/APPS/` on the card: the mod's tools as `.sh` (Bezel manager, Clock, Image browser, System
  settings/tools, Tiny Scraper, **"RixelHK Game Downloader"**). Stock `dmenu.bin` lists both folders.
- **No PortMaster** on the stock card. Redux's `LAUNCH_PORTMASTER` path (`/roms/ports/PortMaster`) is
  not on the device. Installing PortMaster means downloading (a Dale-gated security item, App Center
  option B).

## The rule it hits
AGENTS.md: "Never run code from removable media without an explicit user opt-in." Every port and
every card app is a script on the Roms card.

## Update 02:35: every stock port is a RetroArch core + a data file
All 11 scripts in Dale's stock `Roms/PORTS` do one thing: `retroarch -L <core> <data file>`. The six
cores (`cannonball`, `nxengine`, `dinothawr`, `tyrquake`, `xrick`, `ecwolf`) are in appfs
`deep/retro/cores`, and every data file named is present on the stock card. So Redux can launch all
of them the way it launches a ROM, **without running the card's scripts**.

| Port | Core | Data file (under `Roms/PORTS/`) |
|---|---|---|
| Cannonball | cannonball | `Cannonball/epr-10187.88` |
| Cave Story | nxengine | `Cave_Story/Doukutsu.exe` |
| Dinothawr | dinothawr | `Dinothawr/dinothawr.game` |
| Quake, Dopa, Hipnotic, Rogue | tyrquake | `id1/PAK0.PAK`, `dopa/`, `hipnotic/`, `rogue/` `pak0.pak` |
| Rick Dangerous | xrick | `Rick_Dangerous/data.zip` |
| Wolfenstein 3D, Spear of Destiny, Super 3D Noah's Ark | ecwolf | `Wolfenstein_3D/WL6.ecwolf`, `wsd/SOD.ecwolf`, `Super_3D_Noahs_Ark/n3d.ecwolf` |

## Options
- **A. Recommended: Ports as a known-ports table, no card scripts.** Game Room gets a **Ports**
  system built from the table above: each entry shows only if its data file exists, and launches
  through the existing RetroArch path (core + data file), like any ROM. Nothing from the card runs as
  code, so no opt-in is needed. New ports later = new table rows.
- B. A, plus a "Run scripts from the card" switch (Settings > System, default Off, 3 s hold) that
  also lists any other `Roms/PORTS/*.sh` (third-party ports, PortMaster if ever installed).
- C. Script switch only (the earlier option A): run the card's `.sh` files once switched on.

Not verified on device: everything above comes from the image and the stock SD copy.
