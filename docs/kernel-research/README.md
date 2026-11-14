# Kernel research

Research for Phase 2 (mainline kernel) and the parallel Ubuntu userland track. **Desk research
only: nothing here has run on Dale's device** unless a line says so. Snapshot: 2026-11-13, by the
Redux Manager chat (read-only), for Dale.

| File | What it holds |
|---|---|
| [mainline-status.md](mainline-status.md) | Each RG34XXSP block (display, PMIC, input, lid, GPU, audio, Wi-Fi, suspend, DRAM) in stock 4.9, mainline Linux and ROCKNIX |
| [rocknix-h700.md](rocknix-h700.md) | ROCKNIX's H700 build in detail: kernel, patches, device trees, u-boot, suspend, kernel config |
| [ubuntu-track.md](ubuntu-track.md) | Plan for an Ubuntu 24.04 userland in parallel with Beta (plan only, Dale reviewing) |
| [upstream-references.md](upstream-references.md) | Every link Dale and the Manager collected, with a summary and a relevance rating |
| [community-builds.md](community-builds.md) | Other people's mainline builds for H700 handhelds (Debian 13 on the RG35XX-H; an Armbian fork) |
| [stock-kernel-ghidra.md](stock-kernel-ghidra.md) | Function map of the V1.0.6 stock kernel from Ghidra (backlight, panel, lid, sticks, OPP) |
| [open-questions.md](open-questions.md) | What to verify next, on the desk and on the device |

**Canonical source for Phase 2 desk work: the [H700 Kernel Research](https://github.com/wilcox101-maker/Allwinner-H700-Custom-Kernel-Research) repo (KR),**
checked at `c77891f` (2026-11-14): patches, device trees, build scripts, test kits, and the task list
`docs/master-task-list.md` (K-IDs). Redux keeps the summaries here and links to KR by path and commit;
KR keeps the patches. KR is GPL-2.0, like Redux. Nothing in KR has booted on an RG34XX-SP either.

Related: [`../kernel-strategy.md`](../kernel-strategy.md) (the strategy, still valid),
[`../hardware-map.md`](../hardware-map.md), [`../stock-boot-chain.md`](../stock-boot-chain.md),
[`../ram-system-design.md`](../ram-system-design.md), [`../field-reports.md`](../field-reports.md).

## Rules for this folder
- Text only. No kernel, u-boot, panel-firmware or Mali binaries, and no copies of other projects'
  patches: link to them by name and commit (AGENTS.md hard rules).
- Every claim names its source and date. Versions move fast: re-check before acting on one.
- "Upstream" means torvalds/linux master, checked at a named commit.

## Headlines (2026-11-13)
1. **ROCKNIX already runs the RG34XXSP on a mainline-based 7.2 kernel**, with two device trees for
   the SP (`rg34xx-sp` and `rg34xx-sp-v2-panel`) and separate LPDDR3 / LPDDR4 u-boot builds. That
   suggests the revised boards (field-reports.md) are now handled there. Not checked on our device.
2. **Upstream Linux (7.4-rc2) still can't drive the screen** of any H700 handheld: the DE33 mixer
   driver is in, but the H616 display device-tree nodes, the H616 PWM (backlight) driver and the
   generic SPI-panel driver are not. ROCKNIX carries those as patches.
3. **Upstream has no RG34XX-SP device tree.** ROCKNIX's is 23 lines on top of upstream's
   `rg35xx-sp.dts`: a different panel compatible and a 4-channel analog mux with inverted right stick.
4. **Real suspend-to-RAM exists for the H700** (an SRAM stub run by TF-A, DRAM self-refresh, GPL),
   packaged by ROCKNIX. That is the missing piece for F4 power and lid on mainline.
5. **Ubuntu's own kernels don't help directly.** 24.04's GA kernel is 6.8 (before H700 support);
   its 24.04.5 HWE kernel is 7.0, which has the RG35XX trees but none of the display patches and no
   RG34XX-SP tree. Any Ubuntu userland here runs on our kernel.
6. **A full Debian 13 system already runs on an H700 handheld with no vendor code**
   (GregoryPletnev/rg35xxh-debian13, RG35XX-H, Linux 7.1.2 + ROCKNIX patches). It's the closest template for
   Phase 2 and the Ubuntu track, and it found an RTL8821CS Wi-Fi crash on mainline (same chip as ours).
