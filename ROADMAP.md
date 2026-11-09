# Roadmap

## Phase 0: Dissect stock ✅
- [x] Partition layout and boot chain ([docs/stock-boot-chain.md](docs/stock-boot-chain.md))
- [x] Extract kernel, initramfs, DTB, kernel config, u-boot env
- [x] Hardware map: buttons, lid, display, PMIC, thermal, CPU OPPs ([docs/hardware-map.md](docs/hardware-map.md))
- [x] Round-trip tooling: `bootimg.py` rebuilds stock `boot.img` byte-for-byte
- [ ] Inventory rootfs/appfs: what Anbernic's launcher actually runs, and which kernel modules load

## Phase 1: Stock-kernel baseline
Changes only the `boot` partition. Flash a copy of the stock card, then `dd` the new `boot.img` to p4.
- [ ] DTB: `gpio-keys-polled` poll-interval 20 ms → 5 ms, then test whether 1–2 ms is feasible
- [ ] initramfs: set the `schedutil` governor before `switch_root`
- [ ] initramfs: skip the per-boot full `e2fsck`; only check when the fs is flagged as not clean
- [ ] initramfs: mount rootfs with `barrier=1`
- [ ] Measurements, stock vs. patched: boot time (power-on → menu), idle battery drain per hour,
      sustained-load temperature and clock, button latency (high-speed phone video)
- [ ] Find UART pads for a serial console

## Phase 2: Mainline + purpose-built OS
- [ ] Diff ROCKNIX RG34XX-SP DTS against the stock tree (see [docs/kernel-strategy.md](docs/kernel-strategy.md))
- [ ] Build environment (WSL2): cross toolchain, kernel, mainline u-boot, DRAM type confirmed
- [ ] Boot mainline kernel with a minimal BusyBox initramfs and a serial console
- [ ] Display, input, audio, PMIC/battery, Wi-Fi bring-up checklist
- [ ] Read-only squashfs system loaded into RAM; ext4 data partition with barriers; watchdog
- [ ] Lid: `SW_LID` → sync → suspend → resume to the same frame
- [ ] Standard gamepad event codes; integer-scaled 3× output; audio/video sync to the panel's real refresh
- [ ] Frontend + RetroArch, tuned for 720×480

## Non-goals
- Overclocking or undervolting past the stock OPP table
- Bundling ROMs or copyrighted BIOS files
