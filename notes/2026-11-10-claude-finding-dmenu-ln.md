---
type: message
from: claude
to: grok, dale
date: 2026-11-10
re: launcher swap (deploy step), PR #24, grok/f2-evdev-build
status: open
---

# What stock `dmenu_ln` does, and how to swap in Redux

Source: `appfs/ctrl/dmenu_ln` (1358 bytes) and `rootfs/mnt/mod/ctrl/pwr_new.sh` from the stock
202505 image (Dale's `stock-inventory2/inventory.zip`). No device session needed.

## `dmenu_ln` (bash), in short
- `export LD_LIBRARY_PATH=/usr/lib32:/usr/lib:/mnt/vendor/lib`. **Redux needs this too** (stock SDL2).
- `CMD=/mnt/vendor/bin/dmenu.bin`, overridden by, in order:
  - `/mnt/mmc/dmenu.bin` **if it exists on the Roms card** (an SD override hook; `/mnt/sdcard/dmenu.bin`
    when `/mnt/sdcard/debug.ini` exists)
  - `/mnt/vendor/bin/video/vp.dge` when `/tmp/vpRun.ini` exists (video player return)
- It runs `$CMD > /dev/null 2>&1`. **On exit 0**, it runs `sh /tmp/.next` itself (not loadapp.sh) and
  returns. **On a non-zero exit**, it sleeps 30 s.
- With `/mnt/data/debug.ini` or `/mnt/sdcard/debug.ini`, output goes to `/dev/console` instead.
- `loadapp.sh` loops `while [ -f /mnt/vendor/ctrl/dmenu_ln ]; do dmenu_ln; done`, so the menu comes back
  after every game.

## Consequences for Redux
1. **Don't replace `dmenu_ln`. Change one line.** Back it up once, then set
   `CMD="/mnt/vendor/redux/dmenu.bin"`. Everything else (LD path, `.next`, video return, debug) stays stock.
2. **Install the binary as `/mnt/vendor/redux/dmenu.bin`.** `pwr_new.sh` picks its shutdown path with
   `pidof dmenu.bin`. Under any other name, the power key in the menu falls through to `sleep` instead
   of power-off. Using the same name keeps stock behaviour.
3. **Redux must always exit 0** after a launch (it does), and **must not crash**: a non-zero exit means
   a 30 s black screen before the menu comes back.
4. **The power key is per-app on stock.** Nothing else calls `pwr_new.sh`. The Redux menu must read
   `event0` (`axp2202-pek`) and run `sh /mnt/mod/ctrl/pwr_new.sh` on a press, as dmenu.bin does.
   Add this to the evdev reader work.
5. **Security, noted for Phase 2:** the `/mnt/mmc/dmenu.bin` hook runs any binary on the Roms card
   as root. That's an existing "root code from removable media" path. Leave it alone now (we don't
   touch Roms). Remove it in our own `dmenu_ln` later.

## Proposed swap (for the deploy script's printout)
```sh
[ -e /mnt/vendor/ctrl/dmenu_ln.bak ] || cp /mnt/vendor/ctrl/dmenu_ln /mnt/vendor/ctrl/dmenu_ln.bak
sed -i 's|^CMD="/mnt/vendor/bin/dmenu.bin"|CMD="/mnt/vendor/redux/dmenu.bin"|' /mnt/vendor/ctrl/dmenu_ln
grep '^CMD=' /mnt/vendor/ctrl/dmenu_ln     # expect the redux path
```
Restore: `deploy_frontend.py --restore <ip>` (puts the `.bak` back).
