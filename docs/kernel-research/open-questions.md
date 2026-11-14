# Open questions and next steps

## Desk (no device)
1. Which version added upstream `sun50i-h700-anbernic-rg35xx-sp.dts`? Partly answered: present in Armbian's 6.12
   archive, absent from 6.6/6.7, so between 6.10 and 6.12 (community-builds.md). Exact version still open.
2. Has the H616 display DT series or the H616 PWM driver been queued for 7.5 (sunxi / pwm trees)?
3. How does ROCKNIX choose `rg34xx-sp` vs `rg34xx-sp-v2-panel` at boot (u-boot script, `rocknix-dt-id`,
   panel probe)? This is the "identify the panel before driving it" rule from field-reports.md.
4. Does ROCKNIX 20261101 use the real suspend stub on H700, or still "fake suspend" (wiki)?
5. Partly answered (stock-kernel-ghidra.md, CPU section): stock H700 table per bin is recorded; upstream H616 table
   uses lower top voltages and different grade numbering; mapping still open. Thermal trips still to compare. Original: Diff ROCKNIX's H700 OPPs (patch 0200) and thermal zones against stock's bin-aware table and trips
   (hard rule: stock OPP limits by default, critical trip never changed).
6. Mostly answered (mainline-status.md, last section): yes, with the RG35XX-H tree's `adc-joystick` +
   `io-channel-mux` + `gpio-mux` on PI1/PI2, plus PI8/PE11 enables. Still to check: the enables' polarity, and
   whether there's an RTC at 0x51 on our `r_i2c`. Original question: can the SP's 4-channel stick mux with right-stick inversion be described with upstream
   `adc-joystick` + `io-channel-mux`, so we don't need the out-of-tree ROCKNIX joypad driver?
7. Update `docs/field-reports.md`: ROCKNIX now ships LPDDR3 u-boot and an RG34XX-SP v2-panel tree,
   so "Knulli and ROCKNIX don't boot" on revised boards may be out of date (unverified).
8. RTL8821CS RX-descriptor crash (rg35xxh-debian13 patch 0001, `skb_over_panic` in `ksdioirqd` on 7.1.2):
   upstream 7.4-rc2 has no length check and ROCKNIX carries no rtw88 patch. Carry a fix; consider sending
   it upstream (or check linux-wireless for a pending one).
9. Evaluate the hybrid DRAM-probe SPL (rg35xxh-debian13) against ROCKNIX's per-type builds: can one
   image serve original (LPDDR4) and revised (LPDDR3) RG34XXSP boards safely?
10. `tools/extras.py` runs `apt-get update` against the stock sources (TUNA mirror + ports.ubuntu.com). Should it
    warn when the mirror is slow, or offer to put ports.ubuntu.com first? (Owner's call; it touches the device.)
11. Check our stock image for the alpine-h700 offsets: SPL `eGON.BT0` at 8 KiB, U-Boot `sunxi-package` at
    16400 KiB (read-only, on Dale's `RG34XXSP.img`).
12. Which DTB does the stock 4.9 kernel actually use: the one in `boot.img` (stock-boot-chain.md) or the one in
    the U-Boot package (mporrato)? Compare both with `/sys/firmware/fdt` on a running device.
13. Backlight clamp: read `disp_lcd_set_bright`, `disp_lcd_bright_get_adjust_value` and the curve init in
    the V1.0.6 kernel (stock-kernel-ghidra.md) to find why values above ~20 are capped and what the true maximum is.
14. Extract the kernel's kallsyms table (base-relative, 4.9) to name all functions; either `vmlinux-to-elf` (needs
    Dale's OK to install) or a stdlib reader (first attempt didn't locate the table).
15. `docs/kernel-strategy.md`: its risk line "DRAM type for mainline u-boot" can now point at
   ROCKNIX's two defconfigs.

## Device (Dale, bench card only, never the stock card)
1. Which board is Dale's unit: DRAM type and size (`dmesg | grep -i dram`, `/proc/meminfo`), and which
   panel (stock V1.0.6 calls the new one `rg34xxsp_v1`)?
2. Boot ROCKNIX 20261101 (matching DDR image) on a spare card: screen, sticks, lid, suspend/resume,
   Wi-Fi, audio, battery reading. One hw-result note. This is the cheapest way to learn what mainline
   already does on this exact unit.
3. UART: find the pads (ROADMAP Phase 1 item); ROCKNIX's console is ttyS0 at 115200.

## Decisions for Dale
- U0 in [ubuntu-track.md](ubuntu-track.md): 24.04 or 26.04; role of Ubuntu in 1.0.
- Whether to base Phase 2 on ROCKNIX's kernel tree and patch set (fast) or on upstream plus only
  the patches we need (cleaner). Manager view: start from ROCKNIX's set, drop patches as they land
  upstream.
