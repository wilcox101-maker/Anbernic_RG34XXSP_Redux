---
from: dale
to: grok, claude
event: hw-result
phase: F2
status: open
refs: [docs/hardware-map.md, frontend/src/systems.c, frontend/src/platform_stock.c]
---

# Hardware session results (2026-11-10)

Captured over SSH on the stock RG34XXSP.

## Analog sticks and buttons
Device: `/dev/input/event1` (`ANBERNIC-keys`)

Stick axes (range −4096…+4096, fuzz/flat 32):
- Left X: ABS_Z (2)
- Left Y: ABS_RX (3)
- Right X: ABS_RY (4)
- Right Y: ABS_RZ (5)

D-pad:
- ABS_HAT0X (16): −1 left, +1 right
- ABS_HAT0Y (17): −1 up, +1 down

Buttons (scrambled):
- BTN_SOUTH (304) = A
- BTN_EAST (305) = B
- BTN_C (306) = X
- BTN_NORTH (307) = Y
- BTN_WEST (308) = L1
- BTN_Z (309) = R1
- BTN_TL (310) = L2
- BTN_TR (311) = R2
- BTN_SELECT (314) = Select
- BTN_START (315) = Start
- BTN_TL2 (312) = Menu
- BTN_TR2 (313) / BTN_MODE (316) = L3/R3 family

## Battery temperature
`/sys/class/power_supply/axp2202-battery/temp` stayed at **300** under:
- 120 s full CPU load (CPU temp 39.8 °C → 58.6 °C)
- ~5 min charging at 5 V / 3 A (capacity 29 % → 32 %)

**Conclusion:** treat as a dummy value. UI shows "battery temp: unknown". Heat guard falls back to SoC temperature only.

## Hall sensor
`/sys/class/power_supply/axp2202-battery/hallkey`:
- Lid open: 1
- Lid closed: 0

## Governor
`performance` (stock default).

## Battery other
- voltage_now: 3935000 (3.935 V)
- capacity: 35 % (later reading)
- current_now: does not exist
- Other attributes present: hallkey, moto, work_led, brightness, etc.

## .dge emulators (confirmed paths)
```
/mnt/vendor/bin/game/fc/fc_emu.dge
/mnt/vendor/bin/game/sfc/sfc_emu.dge
/mnt/vendor/bin/game/gba/gba_emu.dge
/mnt/vendor/bin/game/gbc/gbcgb_emu.dge
/mnt/vendor/bin/game/md/md_emu.dge
/mnt/vendor/bin/game/sms/smsgg_emu.dge
/mnt/vendor/bin/game/temper/pce.dge
/mnt/vendor/bin/game/wswan/swanemu.dge
/mnt/vendor/bin/game/ngpc/ngp.dge
/mnt/vendor/bin/game/pcsx_rearmed/pcsx
```
(plus FBA variants and xmame.dge)

## RetroArch cores
Present under `/mnt/vendor/deep/retro/cores/` (long list).

## system.cfg
```
global.ssh=1
global.samba=0
power.key=2
power.lock=0
```

## Input devices
- event0 = axp2202-pek
- event1 = ANBERNIC-keys (js0)
- event2 = dierct-keys-polled

## Mounts
- p5 rootfs: 6.9 G, 61 % used
- p6 appfs (/mnt/vendor): 3.9 G, 58 % used
- p7 UDISK (/mnt/data): 2.4 G, nearly empty
- p1 Roms (/mnt/mmc): 44 G, 70 % used, vfat
