---
type: message
from: readonly
to: claude
date: 2026-11-17
re: Bluetooth adapter missing under Redux; every script stock dmenu.bin names, checked against Redux (via the Manager, passed on by Dale 15:42)
status: open
---

# Read Only: Bluetooth bug and the stock-script audit

Relayed text (Dale 15:42), kept in full for the follow-ups it lists:

- **Bug:** under Redux, `bluetoothctl` says "No default controller available": no `hci0`, no
  `rtk_hciattach`; rfkill unblocked, bluetooth.service running. Stock dmenu.bin runs
  `/mnt/vendor/ctrl/setBluetooth.sh init` at boot (when Bluetooth is enabled); the script starts
  `rtk_hciattach -n -s 115200 ttyS1 rtk_h5` and brings `hci0` up, keeping its own lock at
  `/tmp/.init_bt`. Source: H700 repo `docs/stock-re-boot-and-userspace.md` lines 529, 631, 787.
  Fix: run it once at Redux start, in the background. Decision needed: always on or behind a
  switch (spec had `bluetooth_enabled = 0`). → Dale A 15:36: on by default + switch
  ([decision](2026-11-17-dale-decision-bluetooth-init.md), PR #286); spec default changed to 1.
- **Audit of every script stock dmenu.bin names** (Dale's `strings`): the only boot-time job
  missing was Bluetooth. Menu actions Redux lacks or should check:
  - `mmc_new.sh add` / `mmc.sh remove`: SD hot-plug; boot `add` runs (loadapp). A card inserted
    after boot is probably not mounted → check against the F3 "SD hot-plug" item.
  - `setNDS.sh run` (DraStic start, save folder): Redux runs `drastic-modify/launch.sh` directly
    → check whether `run` does setup that `launch.sh` skips.
  - `setSaturn.sh BIOS/HLE`, `setFlycast.sh`: check whether the Saturn/DC cores need them before
    a launch.
  - `setRA.sh easy/adv/rest/language`, `setPSP.sh language/recover`: stock settings Redux has no
    equivalent for (presets, reset, language, PPSSPP recovery).
  - `setHostapd.sh`: Wi-Fi hotspot screen (feature gap). `setSSH.sh`: Redux uses system.cfg.
    `cpu_setting.sh`: per-app governor; Redux manages the governor itself. `pdf/launch.sh`,
    PICO-8 `launch.sh`: PDF is "later"; PICO-8 not checked.
- Suggested next step: `ps -eo comm,args | sort` under stock and under Redux, then diff.
