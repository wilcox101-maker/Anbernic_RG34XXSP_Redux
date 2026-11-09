# Roadmap

## Phase 0: Dissect stock ✅
- [x] Partition layout and boot chain ([docs/stock-boot-chain.md](docs/stock-boot-chain.md))
- [x] Extract kernel, initramfs, DTB, kernel config, u-boot env
- [x] Hardware map: buttons, lid, display, PMIC, thermal, CPU OPPs ([docs/hardware-map.md](docs/hardware-map.md))
- [x] Round-trip tooling: `bootimg.py` rebuilds stock `boot.img` byte-for-byte
- [ ] Identify hardware revisions: original 2 GB vs. "Metallic Blue" (512 MB/1 GB LPDDR3, new panel). See [docs/field-reports.md](docs/field-reports.md)
- [ ] Inventory rootfs/appfs: what Anbernic's launcher actually runs, and which kernel modules load

## Phase 1: Stock-kernel baseline
Changes only the `boot` partition. Flash a copy of the stock card, then `dd` the new `boot.img` to p4.
- [x] DTB: `gpio-keys-polled` poll-interval 20 ms → 5 ms (built; test 1–2 ms later)
- [x] initramfs: set the `schedutil` governor before `switch_root` (built; confirm stock userspace doesn't reset it)
- [x] ~~Skip the per-boot fsck~~ dropped: stock `e2fsck -y` (no `-f`) already skips a clean fs, and keeping the check is right for stability
- [x] initramfs: mount rootfs with `barrier=1` (built)
- [ ] On-device test per [phase1-stock-tweaks/DEPLOY.md](phase1-stock-tweaks/DEPLOY.md)
- [ ] Measurements, stock vs. patched: boot time (power-on → menu), idle battery drain per hour,
      sustained-load temperature and clock, button latency (high-speed phone video)
- [ ] Find UART pads for a serial console

## Phase 2: Mainline + purpose-built OS
- [ ] Diff ROCKNIX RG34XX-SP DTS against the stock tree (see [docs/kernel-strategy.md](docs/kernel-strategy.md))
- [ ] Build environment (WSL2): cross toolchain, kernel, mainline u-boot, DRAM type confirmed
- [ ] Boot mainline kernel with a minimal BusyBox initramfs and a serial console
- [ ] Display, input, audio, PMIC/battery, Wi-Fi bring-up checklist
- [ ] Read-only squashfs system loaded into RAM; ext4 data partition with barriers; watchdog
- [ ] Lid: `SW_LID` with lid-open as a wake source → flush saves → sync → suspend → resume to the same frame
- [ ] Standard gamepad event codes; integer-scaled 3× output; audio/video sync to the panel's real refresh
- [ ] Frontend + RetroArch, tuned for 720×480

## Non-goals
- Overclocking or undervolting past the stock OPP table
- Bundling ROMs or copyrighted BIOS files
