---
from: claude
to: grok
event: phase-start
phase: F2
status: open
refs: [docs/ui-brief.md §7, docs/stock-userspace.md, docs/hardware-map.md, frontend/]
---

# F2 (launch parity on stock) is open. Yours to build

**Goal** (brief §7): the frontend replaces Anbernic's menu on a **copy** of the stock card, and every
stock launcher still works: RetroArch, PPSSPP, DraStic, the `.dge` emulators, PCSX, PortMaster. Each
returns to our menu on exit, and resume-last-game works.

**Stock facts you'll need** (`docs/stock-userspace.md`):
- The launch loop is `appfs/ctrl/loadapp.sh` → `ctrl/dmenu_ln` → `bin/dmenu.bin`, then `sh /tmp/.next`
  (the chosen app). Hooking in at `dmenu_ln` is the least invasive point: our binary writes
  `/tmp/.next` and exits, exactly like `dmenu.bin`.
- RetroArch runs via `/mnt/mod/ctrl/RA_launch.sh <core> <rom>`, DraStic via `ctrl/setNDS.sh`, and the
  `.dge` binaries directly. Copy how `dmenu.bin` builds `.next`. Its strings dump is in the inventory
  zip on Dale's PC (`strings/appfs/bin/dmenu.bin.txt`). Ask in a note if you need lines from it.
- Resume-last: shutdown in a game writes `/mnt/mod/.last`, and `autostart` resumes it.
- The stock rootfs is Ubuntu 22.04 aarch64 (glibc). Build against it, e.g. an aarch64 cross toolchain
  plus the stock `libSDL2`. Which SDL video driver works there (stock apps use
  `SDL_VIDEODRIVER=anbernic`) is the first thing to find out.

**Platform + input**
- `platform_stock.c` reads sysfs (`axp2202-battery/capacity`, `status`, `hallkey`, thermal zones). The
  desktop `platform.c` stays for development.
- Device remap table: the stock scrambled codes from `docs/hardware-map.md` → logical buttons. The
  analog sticks need `evtest` on the device first, so ask Dale via an `hw-result` request.

**Rules for F2 specifically**
- Work only on a copy of the stock card. Never touch the Roms partition. Never format anything.
- An install/deploy script that writes to the card is in the "never merged without Dale" list. Keep
  it in its own PR, with a rollback step (restore stock `dmenu_ln`).
- Mark everything not yet run on the device as *unverified*.

**Exit test:** every launch item starts a game and returns cleanly; resume-last works; the stock card
is untouched. Dale runs this on the device; write a checklist like `phase1-stock-tweaks/DEPLOY.md`.

## Needs from you
- `grok/f2-*` branches, `review-request` notes to claude. A `decision` note to Dale for anything that
  changes what he sees or loses. A `phase-gate` note when the exit test passes.

## Not verified
- The stock SDL2 driver situation and the stick event codes. Both need the device.
