---
from: claude
to: all
event: message
phase: Phase 1
status: open
refs: [docs/field-reports.md, docs/hardware-map.md, docs/stock-boot-chain.md, reference/stock-202505/]
---

# Stock V1.0.6 (2026-05-26) vs our 2025 reference: the boot image now knows about the revised boards

Dale has Anbernic's `RG34XXSP-V1.0.6-EN16GB-260526.IMG`. I compared its boot partition, env and
initramfs with `rg34xxsp-fw-sd_202505` (read-only, on Dale's PC; nothing extracted is committed).

## Same as 2025
- GPT layout: the same 7 partitions with the same names and sizes (Roms is 2 GiB in this 16 GB image and grows on a bigger card).
- u-boot env: identical to `reference/stock-202505/env.txt`.
- initramfs `/init`: identical to `reference/stock-202505/initramfs-init.sh`.
- Kernel: still 4.9.170, rebuilt `#9 ... Tue May 26 15:49:58 CST 2026`. The config differs by one line:
  `CONFIG_MMC_PARANOID_SD_INIT=y` is new.
- Panel timings are unchanged (720×480, same `lcd_ht`/`lcd_vt`/`lcd_dclk_freq`).

## Changed in the DTB (everything else is identical once sorted)
1. **DRAM selection by board straps.** `dram_select_para`: `select_mode` 0 → 1, with four strap GPIOs
   `select_gpio0..3` = PH6, PH7, PH8, PH10 (2025 had only PH9). Parameter sets `dram_para14` and
   `dram_para15` changed from DDR3 @ 792 MHz to **type 7 (LPDDR3) and type 8 (LPDDR4) @ 672 MHz**
   (Allwinner type codes). This matches the field reports of revised boards with different, smaller RAM.
2. **New panel driver.** `lcd_driver_name` `fog_fj035fhd05_v1` → `rg34xxsp_v1`. The kernel has both
   drivers built in. Whether `rg34xxsp_v1` detects the panel at runtime is not known yet.
3. **TF1 slot (`sdmmc@04020000`, sdc0):** card-detect GPIO (PF6) removed, `non-removable` added,
   `status` disabled → okay. TF1 hot-plug detection is gone in V1.0.6.
4. **Battery capacity:** `pmu_battery_cap` 3000 → 3300 mAh.

## What it means for us
- **A Redux `boot.img` built from the 2025 DTB must never go on a revised board.** It names the old
  panel driver and only has DDR3 DRAM sets. This turns the existing field-reports rule into something
  concrete.
- V1.0.6 looks like Anbernic's single image for both boards. If it boots on Dale's original 2 GB unit,
  basing Phase 1 on the V1.0.6 `boot.img` instead of the 2025 one would cover both boards. That's a
  decision for Dale once it's tested. **Don't flash V1.0.6 until Dale chooses to.**
- The strap GPIOs (PH6/7/8/10) are a candidate runtime board-ID for Redux, alongside MemTotal.

## Not verified
- Nothing here has run on a device. The type 7/8 meaning comes from Allwinner's code convention, and
  "LPDDR3 board" is inferred, not measured.
- boot0 and u-boot weren't compared: the 2025 image's first 36 MiB isn't on Dale's PC (only V1.0.6's).
- appfs/rootfs (launcher, RetroArch) weren't compared yet.
