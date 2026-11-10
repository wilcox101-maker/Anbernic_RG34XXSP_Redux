# Kernel strategy

The existing H700 firmware projects split into two camps.

| Approach | Projects | Kernel | GPU | Trade-off |
|---|---|---|---|---|
| Reuse the stock BSP kernel binary | muOS, Knulli, StockOS-Mod | 4.9.170 extracted from Anbernic images | Arm's closed Mali driver | Everything works on day one, but the kernel is frozen. No kernel source matches this build, so compiled-in options (HZ, zram, drivers) can't change |
| Build mainline from source | ROCKNIX, postmarketOS | Linux 6.12 → 7.x | Panfrost (open, Mesa) | Maintainable and fixable. Upstream already has RG35XX DTs (Ryan Walklin) and u-boot (Chris Morgan); ROCKNIX supports the RG34XX SP |

## Our plan: staged

**Phase 1, stock kernel (baseline).** Patch only what's reachable without kernel source: the DTB
(input polling) and the initramfs (governor, mount options). This gives a measurable "before
vs after" for button latency, boot time and battery life.

**Phase 2, mainline kernel (target).**
- Start from ROCKNIX's H700 kernel patches and kernel config, plus its RG34XX-SP device tree.
- Benefits we already know of from upstream:
  - Lid as a real `SW_LID` `gpio-keys` switch on PE7
  - Interrupt-driven `gpio-keys` for the gamepad (no 20 ms poll)
  - Standard gamepad event codes
  - Kernel config under our control (zram, HZ, schedutil default)
- Risks to evaluate: Panfrost vs. closed Mali performance on G31, DRAM type for mainline u-boot
  (ROCKNIX ships separate DDR3/DDR4 H700 images), and AXP2202 fuel-gauge parity.

## Where the kernel source is

| Need | Source | Notes |
|---|---|---|
| Stock 4.9.170 BSP (exact) | **none** | Anbernic hasn't published its H700 kernel or u-boot source. Knulli, muOS and our Phase 1 reuse the stock binaries |
| Stock-family BSP, for reading how a stock driver behaves | Orange Pi `github.com/orangepi-xunlong/linux-orangepi`, branch `orange-pi-4.9-sun50iw9` (H616, same `sun50iw9` family, also the base of openEuler's Allwinner kernel) | Not Anbernic's build: no `hallkey` / `nds_pwrkey` AXP2202 extras, no panel driver. Don't try to rebuild the stock kernel from it |
| Mainline kernel (Phase 2 base) | kernel.org (`git.kernel.org`, stable tree) | Already has the H700 Anbernic device trees (`arch/arm64/boot/dts/allwinner/sun50i-h700-anbernic-*`, by Ryan Walklin and Chris Morgan). The RG35XX-SP tree has the lid as a `gpio-keys` switch on PE7 |
| RG34XX-SP on mainline (patches, DT, panel) | ROCKNIX `github.com/ROCKNIX/distribution` (H700 project) | Added RG34XX-SP and a generic H700 panel driver in release 20250517. Ships separate DDR3 and DDR4 H700 images since 20260701 |
| Second reference | postmarketOS port `anbernic-rgxxxx` (wiki: "Anbernic H700 Handheld") | Kernel 7.1.3, lists the RG34XX SP. Supports the LPDDR4 boards only, not the 2026 LPDDR3 revision |

Plan: a kernel.org stable tree + ROCKNIX's H700 patches and RG34XX-SP device tree, with
postmarketOS as a cross-check. Anything Anbernic-specific (hall sensor, panel ID, AXP2202 sysfs
extras) is reimplemented on mainline, not ported from the BSP.

## First Phase 2 task

Diff ROCKNIX's RG34XX-SP device tree against `reference/stock-202505/board.dts` node by node:
panel timings, regulators, OPPs, thermal, audio routing. Anything stock does that mainline doesn't
becomes the Phase 2 work list.
