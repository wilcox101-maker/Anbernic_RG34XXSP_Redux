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
- [ ] Diff ROCKNIX RG34XX-SP DTS against the stock tree (see [docs/kernel-strategy.md](docs/kernel-strategy.md))
- [ ] Build environment (WSL2): cross toolchain, kernel, mainline u-boot, DRAM type confirmed
- [ ] Boot mainline kernel with a minimal BusyBox initramfs and a serial console
- [ ] Display, input, audio, PMIC/battery, Wi-Fi bring-up checklist
- [ ] Read-only squashfs system loaded into RAM; ext4 data partition with barriers; watchdog (design: [docs/ram-system-design.md](docs/ram-system-design.md))
- [ ] Lid: `SW_LID` with lid-open as a wake source → flush saves → sync → suspend → resume to the same frame
- [ ] Standard gamepad event codes; integer-scaled 3× output; audio/video sync to the panel's real refresh
- [ ] Frontend + RetroArch, tuned for 720×480

## Frontend: phases F0–F8
Runs alongside the kernel phases. The full plan, UI rules and exit tests are in [docs/ui-brief.md](docs/ui-brief.md).
- [x] F0: UI spec and settings schema ([docs/ui-spec.md](docs/ui-spec.md)), signed off 2026-11-10. Battery-temperature check on the device still open
- [x] F1: desktop prototype (C + SDL2, `frontend/`), signed off 2026-11-10. Exit test: `frontend/tests/check_mockups.py`
- [x] F2: launch parity on a copy of the stock card, signed off 2026-11-12
- [ ] F3 includes the UI font picker: vga / spleen / unifont / thin / chunky ([docs/ui-spec.md §8a](docs/ui-spec.md))
- [ ] F3 settings parity · F4 power and lid · F5 Engineering Mode · F6 features stock left out · F7 mainline port · F8 release

## Release milestones
Two targets. The phases above say *how* the work is done; these say *when it ships*. Each box is ticked
only after it has been checked on the device.

### Beta (v0.5): Redux on the stock kernel
The full frontend on the stock card with the Phase 1 kernel tweaks, installable by someone other than
Dale. No mainline kernel is needed.
- [x] F0–F2: spec, desktop prototype, launch parity on stock
- [ ] MAME launches through a stock RetroArch core (stock's `xmame.dge` path can't be used: stock .dge launches fail)
- [ ] F3 settings parity, including from the queue: Wi-Fi Connecting/result screens, BIOS-style UI with
      colour presets, TF2 scan/hot-plug, Restart/Power Off with a 3 s hold
- [ ] F4 power and lid: its exit test (50 lid cycles mid-game, no lost save, governor restored)
- [ ] F5 Engineering Mode inside the stock OPP tables (Expert Mode is **not** part of Beta)
- [ ] F6 features stock left out, including from the queue: System Tools (read-only), App Center,
      Ports/PortMaster, Terminal/htop
- [ ] Phase 1 measurements, stock vs. Redux: boot time, idle drain, sustained-load temperature, button latency
- [ ] Stock-card install: one explicit, opt-in installer, a documented way back to stock, the 640×480 boot logo
- [ ] User docs that match what's on screen; README status updated to Beta
- **Exit test:** a fresh copy of the stock card plus the installer boots to Redux. F3–F6 checklists pass on
  the device. One week of daily use with no lost save and no unplanned trip back to stock.

### 1.0: purpose-built OS on mainline
- [ ] Phase 2 complete (mainline kernel, RAM-resident system, `SW_LID` wake)
- [ ] F7: F2–F6 checklists pass again on mainline with no UI code changes outside the hardware layer
- [ ] F8: install/update path for the new OS, measurements against stock and Beta, user docs
- **Exit test:** a clean card boots mainline Redux to the menu, and every Beta checklist still passes

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
Rough, effort-weighted, as of 2026-11-12: **Beta ≈ 35%**, **1.0 ≈ 15%**. Phase 2 is the biggest
unknown: it exists only as a design and nothing has booted yet. Update this line when a phase is signed off.

## Non-goals
- Overclocking or undervolting past the stock OPP table **by default** (it is possible only through the opt-in Expert Mode; see notes/2026-11-12-dale-decision-expert-mode.md)
- Bundling ROMs or copyrighted BIOS files
