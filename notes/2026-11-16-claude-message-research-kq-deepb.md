---
type: message
from: claude
to: all
date: 2026-11-16
re: MGR for DALE relay (15:11), research packages kernel-q and deep-b landed; checked against F4 as built
status: closed (landed in this PR; no conflict with F4)
---

# Research packages kernel-q and deep-b

Relay: MGR for DALE 15:11 (Dale yes). Files from Dale's `redux-work\Claude Files` on DLN-DALEW2.

## Landed
- **kernel-q** (`redux-kernel-research-1115q.zip`, replaces package n): `docs/kernel-research/stock-kernel-ghidra.md`
  and `open-questions.md`. Sleep is PSCI `SYSTEM_SUSPEND` to BL31, no SCP (AR100) firmware; Wi-Fi is not a wake
  source; `boot_mode`, `display_id`, `openbor_volume` described; findings with no public source tagged [N].
- **deep-b** (desk only, [D][N]): at every stock boot `loadapp.sh` runs `mkfs.ext4` on p7 (UDISK, `/mnt/data`) and
  `mkfs.vfat` on the Roms partition (p8 if it exists, else p1) when `blkid` finds no filesystem or the mount does
  not show in `/proc/mounts`. No prompt, no log. Already in [stock-userspace.md](../docs/stock-userspace.md)
  ("can format the ROM card"); new here: the p8 case and the `/proc/mounts` check.

## Checked against F4 as built
- **No conflict.** F4 changes no partition, label or filesystem. Lid sleep stays stock's (`pwr_new.sh`), so the
  BL31 suspend path is unchanged. Redux's lid watcher sends `SAVE_FILES` and runs `sync` before stock suspends,
  so p1 is clean if the battery dies while asleep. Wi-Fi not waking the device does not affect F4.
- **Rule for later work (deep-b):** Redux must never leave p1, p7 or p8 unmountable on a card stock may boot.
  - F5 profile and write log on `/mnt/data` (p7): temp file + `rename`, as `romscan.c` and `vsync.c` already do.
  - F5 storage check stays `fsck -n` only (Q3 A). Any future repair, installer B or update path must keep layout,
    labels and filesystem types, and never stop half-written; those PRs are Dale's anyway (AGENTS.md).
