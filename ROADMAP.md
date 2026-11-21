# Roadmap

## Phase 0: Dissect stock ✅
- [x] Partition layout and boot chain ([docs/stock-boot-chain.md](docs/stock-boot-chain.md))
- [x] Extract kernel, initramfs, DTB, kernel config, u-boot env
- [x] Hardware map: buttons, lid, display, PMIC, thermal, CPU OPPs ([docs/hardware-map.md](docs/hardware-map.md))
- [x] Round-trip tooling: `bootimg.py` rebuilds stock `boot.img` byte-for-byte
- [ ] Identify hardware revisions: original 2 GB vs. "Metallic Blue" (512 MB/1 GB LPDDR3, new panel). See [docs/field-reports.md](docs/field-reports.md)
- [x] Inventory rootfs/appfs/UDISK: launcher chain, kernel modules, governor and power/lid handling ([docs/stock-userspace.md](docs/stock-userspace.md))
- [x] Re-run `stock_inventory.py` with string dumps and logs: callers of `cpu_setting.sh` / `pwr_new.sh` found, stock boot log reviewed

## Phase 1: Stock-kernel baseline
Changes only the `boot` partition. Flash a copy of the stock card, then `dd` the new `boot.img` to p4.
- [x] DTB: `gpio-keys-polled` poll-interval 20 ms → 5 ms (built; test 1–2 ms later)
- [x] initramfs: set the `schedutil` governor before `switch_root` (built; no boot script resets it, but the music/video players (via `cpu_setting.sh`) and DraStic's SDL2 can later. See [stock-userspace.md](docs/stock-userspace.md#cpu-governor))
- [x] ~~Skip the per-boot fsck~~ dropped: stock `e2fsck -y` (no `-f`) already skips a clean fs, and keeping the check is right for stability
- [x] initramfs: mount rootfs with `barrier=1` (built)
- [x] On-device boot test: boots to the menu in about 20 s, no issues seen (2026-11-10). Per [phase1-stock-tweaks/DEPLOY.md](phase1-stock-tweaks/DEPLOY.md)
- [ ] Measurements, stock vs. patched: boot time (power-on → menu), idle battery drain per hour,
      sustained-load temperature and clock, button latency (high-speed phone video)
- [ ] Find UART pads for a serial console

## Phase 2: Mainline + purpose-built OS
**Now ReduxOS** (Q12, 2026-11-21): the product is the [ReduxOS-Allwinner-H700](https://github.com/wilcox101-maker/ReduxOS-Allwinner-H700) repo; this section is the history. The desk work (device tree, build scripts, DRAM sets, test kits) lives in the
[H700 Kernel Research](https://github.com/wilcox101-maker/Allwinner-H700-Custom-Kernel-Research) repo (KR), checked at `c77891f`; Redux links to it rather than redoing
it. **Nothing in KR has booted on an RG34XX-SP.** Task IDs (K3, K4…) are KR's
`docs/master-task-list.md`.
- [ ] Device tree: desk-done in KR (patch 0006, the SP tree with stock DT values; ROCKNIX comparison in
      `docs/kernel-findings-2026-11-13.md`). Stick directions unconfirmed (K5). See also [docs/kernel-strategy.md](docs/kernel-strategy.md)
- [ ] Build environment: desk-done in KR (`scripts/workbench.sh`, `build-kernel.sh`, `build-uboot.sh`;
      DRAM sets in `docs/r3-v106-boot0-dram-sets.md`). DRAM type on Dale's unit still needs K3 (UART)
- [ ] Boot mainline kernel with a minimal initramfs and a serial console (KR kits: K3 UART pads, K4 FEL boot `docs/r2-fel-test-kit.md`)
- [ ] Display, input, audio, PMIC/battery, Wi-Fi bring-up checklist (KR K-tasks: K2, K9/K18, K14–K16, K19, K21)
- [ ] Read-only squashfs system loaded into RAM; ext4 data partition with barriers; watchdog (design: [docs/ram-system-design.md](docs/ram-system-design.md)). **Superseded in practice:** ReduxOS is built on Ubuntu 24.04 (ReduxOS repo); the Owner recommends recording that (P2, Dale) and carrying the RAM-tier ideas in as settings
- [ ] Lid: `SW_LID` with lid-open as a wake source → flush saves → sync → suspend → resume to the same frame
- [ ] Standard gamepad event codes; integer-scaled 3× output; audio/video sync to the panel's real refresh
- [ ] Frontend + RetroArch, tuned for 720×480

## Frontend: phases F0–F8
Runs alongside the kernel phases. The full plan, UI rules and exit tests are in [docs/ui-brief.md](docs/ui-brief.md).
- [x] F0: UI spec and settings schema ([docs/ui-spec.md](docs/ui-spec.md)), signed off 2026-11-10. Battery-temperature check on the device still open
- [x] F1: desktop prototype (C + SDL2, `frontend/`), signed off 2026-11-10. Exit test: `frontend/tests/check_mockups.py`
- [x] F2: launch parity on a copy of the stock card, signed off 2026-11-12
- [x] F3 includes the UI font picker: vga / spleen / unifont / thin / chunky ([docs/ui-spec.md §8a](docs/ui-spec.md))
- [x] F3 settings parity, signed off 2026-11-16 · [x] F4 power and lid, signed off 2026-11-16 · [x] F5 Engineering Mode, signed off 2026-11-17 · F6 features stock left out · F7 mainline port · F8 release

## Release milestones
Two targets. The phases above say *how* the work is done; these say *when it ships*. Each box is ticked
only after it has been checked on the device.

### Beta (v0.5): Redux on the stock kernel
The full frontend on the stock card with the Phase 1 kernel tweaks, installable by someone other than
Dale. No mainline kernel is needed.
- [x] F0–F2: spec, desktop prototype, launch parity on stock
- [x] MAME launches through a stock RetroArch core (stock's `xmame.dge` path can't be used: stock .dge launches fail). Device-verified by Dale 2026-11-13 ([result](notes/2026-11-13-dale-hw-result-mame.md); build SHA not stated)
- [x] F3 settings parity (signed off 2026-11-16), including from the queue: Wi-Fi Connecting/result screens, BIOS-style UI with
      colour presets, TF2 scan/hot-plug, Restart/Power Off with a 3 s hold
- [x] F4 power and lid (signed off 2026-11-16): exit test 50 lid cycles mid-game, governor restored
- [x] F5 Engineering Mode inside the stock OPP tables (signed off 2026-11-17; heat-guard cap and 30 min run move to the Beta exit test; Expert Mode is **not** part of Beta)
- [ ] F6 features stock left out, including from the queue: System Tools (read-only), App Center,
      Ports/PortMaster, Terminal/htop
- [ ] Phase 1 measurements, stock vs. Redux: boot time, idle drain, sustained-load temperature, button latency
- [ ] Stock-card install: one explicit, opt-in installer, a documented way back to stock, the 640×480 boot logo
- [ ] User docs that match what's on screen; README status updated to Beta
- **Exit test:** a fresh copy of the stock card plus the installer boots to Redux. F3–F6 checklists pass on
  the device. One week of daily use with no lost save and no unplanned trip back to stock.

### 1.0: ReduxOS on mainline (one finish line, Q12 2026-11-21)
**ReduxOS 1.0 = the ReduxOS image + this frontend, with every Beta checklist passing on the mainline kernel, on the
device.** The Redux Owner leads it (Dale, Q12). The one-page roadmap with stages S1–S5 (image, first light,
bring-up, frontend port F7, release candidate F8) lives in the product repo:
[ReduxOS-Allwinner-H700 `docs/roadmap.md`](https://github.com/wilcox101-maker/ReduxOS-Allwinner-H700/blob/main/docs/roadmap.md).
- [ ] S1–S3: ReduxOS image, first light and hardware bring-up (replaces "Phase 2 complete")
- [ ] F7 (S4): F2–F6 checklists pass again on ReduxOS with no UI code changes outside the hardware layer
- [ ] F8 (S5): install/update/rollback, measurements against stock and Beta, user docs, signed images
- **Exit test:** a clean ReduxOS card boots to the Redux menu, and every Beta checklist still passes

### 1.x: requirements after 1.0
Required for the 1.x line ([decision](notes/2026-11-13-dale-decision-zram-v1x.md)). Starts only after every
current queue item and the 1.0 milestone are done. Nothing here goes into the stock-kernel (Beta) build.
- [ ] **ZRAM (compressed RAM swap) on mainline**
  - [ ] Kernel config: `ZRAM`, `ZSMALLOC`, `CRYPTO_LZ4` (plus `CRYPTO_ZSTD` to compare), built in
  - [ ] Measure first, on the 2 GB unit: lowest `MemAvailable` and any OOM kills in a heavy PSP (PPSSPP)
        and NDS (DraStic) game (the open item in [docs/ram-system-design.md](docs/ram-system-design.md))
  - [ ] Boards under 1.5 GiB (revised 512 MB / 1 GB): zram on by default, LZ4, about 50% of RAM, set up by the initramfs
  - [ ] 2 GB boards: default chosen from the measurements (off, a small LZ4 safety net, or a Settings > System toggle); a `decision` note for Dale
  - [ ] Never swap to the SD card or p7 (card wear); hibernation is not supported
  - [ ] Exit test on the device: zram active where expected (`/proc/swaps`), no audio or frame stutter in
        the heavy games above, no OOM kill during a forced low-memory run

### After 1.0 or alongside it (backlog, not gating either milestone)
Expert Mode (above-factory settings behind its unlock), PC link (browser file manager; ROM copy only in
[Xroot Mode], [decision](notes/2026-11-13-dale-decision-xroot-mode.md)), chiptune composer (LittleGPTracker port),
hardware video decode for media, in-game HUD overlay, Wi-Fi idle scanning, features.ini. Items can move
earlier once Beta is out, if they don't put its exit test at risk.

### Progress estimate
Rough, effort-weighted, as of 2026-11-17: **Beta ≈ 60%** (F3, F4 and F5 signed off; weights in "Status detail" below), **1.0 ≈ 20%** (research and design; nothing booted on mainline yet). Phase 2 is the biggest
unknown: it exists only as a design and nothing has booted yet. Update this line when a phase is signed off.

## Status detail (moved from the README)

*Updated 2026-11-17. **Device-checked** means Dale has run it on a real RG34XXSP and a result note in
[`notes/`](notes/) records it. Everything else is merged and passes CI but hasn't been confirmed on
hardware yet. Live task list: [`notes/STATUS.md`](notes/STATUS.md).*

| Milestone | What it is | Progress (rough) |
|---|---|---|
| **Beta (v0.5)** | Redux frontend on the stock kernel, installable by anyone | ≈ 60% |
| **1.0** | Purpose-built OS on a mainline kernel | ≈ 20% (research and design; nothing booted on mainline yet) |
| **1.x** | After 1.0: ZRAM on mainline | not started |

*How the Beta number is made: the Beta work is weighted by effort (F0-F2 25, F3 10, F4 10, F5 10, F6 15,
installer 10, Phase 1 measurements 5, user docs 5, exit test and releases 10). Done: F0-F5, installer A,
part of the docs. It was ≈ 40% on 2026-11-14 and ≈ 55% on 2026-11-17 before F5 was signed off.*

#### Latest (2026-11-16)

- **Device session #185** (dev `5d7dea6`): 27 of 30 checks passed, including the installer (check,
  install, upgrade, uninstall) and the start-failure guard ([result](notes/2026-11-16-dale-hw-result-185.md)).
- **F3 signed off:** five UI fonts and Reset All ([sign-off](notes/2026-11-16-dale-phase-gate-f3-signed-off.md)).
- **F4 signed off:** lid sleep in the menu and in games, governor restore after a game, the menu dims after
  60 s. Exit test: 50 lid cycles mid-game on `6d87953`, all slept and woke; save-before-sleep timing waived by
  Dale ([sign-off](notes/2026-11-16-dale-phase-gate-f4-signed-off.md)).
- **Game Room TF1/TF2 card picker**, device-checked with the cache crash fix (#216)
  ([result](notes/2026-11-16-dale-hw-result-f4.md)).
- **F5 Engineering Mode under way:** live read-outs and the safety net (logged writes, saved profile) are
  device-checked on `d334375` ([result](notes/2026-11-16-dale-hw-result-f5-steps-1-2.md)). CPU controls,
  the heat guard (caps the CPU at 75 °C, powers off at 90 °C) and the GPU, Storage and Logs tabs are built,
  not device-checked.
- **Research:** H700 project findings landed in Redux (#223): stock sleep is run by the BL31 firmware with
  no SCP firmware, Wi-Fi is not a wake source, and the stock `loadapp.sh` formats p1/p7/p8 at boot if they
  can't be mounted, so Redux must never leave them unmountable. Mainline work for 1.0 is in
  [`docs/kernel-research/`](docs/kernel-research/README.md) (desk research; nothing booted yet).

#### Completed
- **Phase 0, dissect stock:** partitions, boot chain, hardware map, stock userspace inventory.
- **Phase 1, stock-kernel baseline:** patched `boot.img` (5 ms input polling, `schedutil` governor,
  write barriers). Boots to the menu in about 20 s on the device (2026-11-10).
- **Frontend F0-F4 signed off:** UI spec, desktop prototype, launch parity with the stock menu, settings
  parity (F3), power and lid (F4).
- **Device-checked features:**
  - Game launching through stock RetroArch (PS1, Neo Geo, CPS, Saturn and others), PSP, **MAME** and
    **OpenBOR** (Contra on TF2, saves on TF1); back to the same game list after quitting. Another 27
    system folders are mapped; not every one has been run on the device
  - Game Room card picker (TF1/TF2); games on TF2 launch
  - Installer A from the PC (check, install, upgrade, uninstall to stock) and the start-failure guard
    (three quick failures fall back to the stock menu)
  - BIOS-style UI: colour presets (Amber kept across a reboot), 5 UI fonts, Reset All; brightness, volume
    and Screen Timeout kept
  - Lid sleep and wake in the menu and in games; governor restored after a game; menu dimming
  - Wi-Fi setup with a full on-screen keyboard
  - **Redux boot screen:** install, restore, and the SSH restore
  - Card Scripts: `.sh` ports on either card always listed and run when picked (#200)
  - Apps: Terminal, Task Monitor (htop), Music, Ebook Reader, five Tools apps; File Manager on Main; Apps
    resume where you left off. The Video Player plays through, but takes no buttons during playback (a
    Redux front-end is planned in F6)
  - System Tools, Restart/Power Off with a 3 s hold, restore the stock menu from Settings
  - `tools/extras.py`: optional Ubuntu extras (htop) from a checklist, one SSH line

#### In progress (merged, waiting for a device check)
- **F5 Engineering Mode:** CPU controls (governor, min/max, cores 1-3), the heat guard, GPU, Storage and
  Logs tabs, battery time left. Still to build: input viewer, LED and rumble test, a check-only filesystem
  check (`fsck -n`)
- Not yet run on the device: lid during Restart/Power Off from the menu, a ROM copied from the PC showing
  after restart, start-up time with the ROM cache
- Open device checks are listed in [`notes/STATUS.md`](notes/STATUS.md)

#### Next (to reach Beta)
- F5: finish and Dale's sign-off (exit test: 30 min at full load on the charger)
- F6: Wi-Fi off unless needed, a Power & Lid settings page, Redux front-ends for the stock Video Player,
  Music, Ebook Reader and File Manager
- Installer B: a TF2 package installed from the device
- Phase 1 measurements, stock vs. Redux: boot time, idle battery drain, temperature, button latency
- User docs; Beta exit test: one week of daily use with no lost save; tagged `v0.5-beta.N` releases

#### Future
- **1.0, mainline:** mainline kernel and u-boot, a read-only system that runs from RAM, lid wake,
  then the frontend checklists passed again on the new OS, and an install/update path
- **1.x:** ZRAM (compressed RAM swap) on mainline
- **Backlog:** Update option in Settings, DOS menu layout and a switch for it, a custom boot image picked
  on the device, Game Room Favorites, Expert Mode (above-factory settings behind an unlock), PC link
  (browser file manager), chiptune composer, vsync rebuilds of stock apps that tear, hardware video
  decode, in-game HUD

## Non-goals
- Overclocking or undervolting past the stock OPP table **by default** (it is possible only through the opt-in Expert Mode; see notes/2026-11-12-dale-decision-expert-mode.md)
- Bundling ROMs or copyrighted BIOS files
