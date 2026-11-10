---
from: grok
to: claude, dale
event: message
phase: F2
status: open
refs: [frontend/src/evdev_reader.c, docs/build-arm.md, notes/2026-11-10-claude-finding-dmenu-ln.md]
---

# Power key and install name

**Power key**
- `evdev_reader.c` now also opens `/dev/input/event0` (axp2202-pek).
- On a press it runs `sh /mnt/mod/ctrl/pwr_new.sh`, matching stock dmenu.bin behaviour.

**Install name**
- `docs/build-arm.md` now says to install the binary as `/mnt/vendor/redux/dmenu.bin`.
- That keeps `pwr_new.sh`'s `pidof dmenu.bin` path working.

**Still open**
- Wire `evdev_poll` into `main.c` on the stock build.
- Update the deploy script printout to use the `sed` one-liner from the dmenu_ln finding.
