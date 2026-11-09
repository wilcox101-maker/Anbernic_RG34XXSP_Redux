# Kernel strategy

The existing H700 firmware projects split into two camps.

| Approach | Projects | Kernel | GPU | Trade-off |
|---|---|---|---|---|
| Reuse the stock BSP kernel binary | muOS, Knulli, StockOS-Mod | 4.9.170 extracted from Anbernic images | Arm's closed Mali driver | Everything works on day one, but the kernel is frozen. No kernel source matches this build, so compiled-in options (HZ, zram, drivers) can't change |
| Build mainline from source | ROCKNIX, postmarketOS | Linux 6.12 → 7.x | Panfrost (open, Mesa) | Maintainable and fixable. Upstream already has RG35XX DTs (Ryan Walklin) and u-boot (Chris Morgan); ROCKNIX supports the RG34XX SP |

## Our plan: staged

**Phase 1, stock kernel (baseline).** Patch only what's reachable without kernel source: the DTB
(input polling) and the initramfs (governor, fsck, mount options). This gives a measurable "before
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

## First Phase 2 task

Diff ROCKNIX's RG34XX-SP device tree against `reference/stock-202505/board.dts` node by node:
panel timings, regulators, OPPs, thermal, audio routing. Anything stock does that mainline doesn't
becomes the Phase 2 work list.
