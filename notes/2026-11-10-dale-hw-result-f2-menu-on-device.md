---
type: hw-result
from: dale (recorded by claude)
to: grok, claude
date: 2026-11-10
re: F2 Redux menu on the device (builds from #33, #35, #39)
status: closed
---

# Redux menu runs on Dale's unit as the launcher

Installed as `/mnt/vendor/redux/dmenu.bin`, started by stock `dmenu_ln` (CMD line changed, `.bak` kept)
through a logging wrapper `/mnt/vendor/redux/run.sh` → `redux.log`. Unit: stock, 2 GB.

## Works (Dale, 2026-11-10)
- Menu replaces the boot logo by itself (start ≈ 10.3–10.8 s; with the 30 s EGL retry no boot failed since).
- One highlight (page-flip fix), D-pad, both sticks, A/B.
- Left/Right and L1/R1 move between the five Engineering tabs; the active tab is highlighted.
- B goes back one level from Engineering, Settings, Internet and the ROM list.
- Hold-A entry to Engineering; release cancels.
- Power key: short press sleeps/wakes; long hold no longer loops. `pwr_new.sh` runs under bash with no errors.
- Real readings: battery %, voltage, CHG, Wi-Fi, SoC temp, per-core MHz (480 MHz idle).

## Found and fixed today
Double highlight (#32), Left/Right dead from a `<linux/input.h>` macro clash (#33), `pwr_new.sh` run by
`sh` (#33), menu exit when Mali/EGL wasn't ready at 10.7 s → 30 s black wait (#33), boot logo staying
up (#33), power-key sleep/wake loop (#35), Release-only build failures (#31).

## Still open
- ROM list is the demo list (selecting a ROM returns to the menu): Grok's next PR.
- Battery "Full" status; CPU governor/min/max are fixed text; "Time left"/"Drain" are placeholders.
- Idle drain 100 → 87 % in ~30 min; Redux itself ~0 % CPU. Power management is decided (notes/…power-management.md).
- Not yet run on device: a game launched from Redux (needs the real ROM list).
