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

## Options
- **A. Recommended: one opt-in switch, default Off.** Settings > System gets "Run scripts from the
  card: Off/On" (3 s hold to turn on, like other confirms). When On, Game Room shows a **Ports**
  system listing `Roms/PORTS/*.sh`, launched with `cd` into the folder and `bash`. Card `APPS`
  scripts stay out of the App Center for now (the downloader is a network/update-path item).
- B. Ask every time: no setting; each port launch shows a hold-to-confirm "Runs a script from the
  card" screen. Safer, slower every launch.
- C. Skip card scripts: list only ports we can map to a stock RetroArch core ourselves (Cannonball,
  Dinothawr...), never running the card's `.sh`. Few ports work this way.

Not verified on device: everything above comes from the image and the stock SD copy.
