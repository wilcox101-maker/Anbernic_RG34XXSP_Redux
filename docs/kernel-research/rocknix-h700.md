# ROCKNIX H700 build, read in detail

Source: ROCKNIX/distribution main at `4fc79d7` (2026-11-13), read-only sparse clone of
`projects/ROCKNIX/devices/H700`, `projects/ROCKNIX/packages/linux` and `packages/linux`.
ROCKNIX is GPL; nothing is copied into this repo, only described.

## Kernel
- **Linux 7.2** from kernel.org (`packages/linux/package.mk`: the `AMD64|H700|SM…` case,
  `PKG_VERSION="7.2"`). Patch dirs applied: `default mainline H700 rocknix-joypad` (+ `7.2`).
- Config `devices/H700/linux/linux.aarch64.conf` (7.2.0). Notable:
  `HZ=300`, `PREEMPT=y`, `CPU_FREQ_DEFAULT_GOV_PERFORMANCE=y` (same default as stock; Redux sets
  schedutil at runtime), `ZRAM=m` + `ZSMALLOC=m` + `CRYPTO_LZ4=y` (device option
  `ZRAM_SWAP_SIZE="0"`: off by default), `SQUASHFS=y` + `SQUASHFS_LZ4=y`, `SUSPEND=y`,
  `DRM_PANFROST=y`, `RTW88_8821CS=y`, `JOYSTICK_ADC=y`, `KEYBOARD_GPIO=y`, `PWM_SUN8I=y` (from patch 0008).
- Kernel command line: `quiet console=ttyS0,115200 console=tty0 systemd.debug_shell=ttyS0`. UART on
  ttyS0 is the debug console, as in the Phase 2 plan.
- Userland: Wayland, sway, EmulationStation; `GRAPHIC_DRIVERS="panfrost"`, `PREFER_GLES="yes"`;
  a voltage-based battery percentage daemon (`BATTERYPLUS_SUPPORT`).

## Device trees (`linux/dts/allwinner/`)
15 H700 boards: RG28XX, RG34XX, **RG34XX-SP**, **RG34XX-SP v2 panel**, RG35XX 2024 / H / Plus
(rev6-panel variants), RG35XX Pro, RG35XX-SP v2 panel, RG40XX-H / -V (and v2 panels), RG CubeXX,
plus `rg-sp`.

`sun50i-h700-anbernic-rg34xx-sp.dts` (Ryan Walklin, Chris Morgan, 2024) is 23 lines:
- `#include "sun50i-h700-anbernic-rg35xx-sp.dts"` (the upstream SP tree: lid as SW_LID etc.)
- `model = "Anbernic RG34XX-SP"`, `compatible = "anbernic,rg34xx-sp", "allwinner,sun50i-h700"`
- `&panel { compatible = "anbernic,rg34xx-sp-panel", "panel-mipi-dpi-spi"; }`
- `&joypad { invert-absrx; invert-absry; amux-count = <4>; }`

The `-v2-panel` file differs only in the panel compatible (`anbernic,rg34xx-sp-v2-panel`). The
panel init sequences live as firmware files in
`projects/ROCKNIX/packages/linux-firmware/kernel-firmware/extra-firmware/panels/`
(`anbernic,rg34xx-sp-panel.panel`, `anbernic,rg34xx-sp-v2-panel.panel`), loaded by the generic
`panel-mipi-dpi-spi` driver. How ROCKNIX picks v1 or v2 at boot was not traced (open question).

## H700 kernel patches (32 files, one disabled, `devices/H700/patches/linux/`)
Display: **0002 is a 21-patch display series** (Jernej Skrabec and others, from 2025-10-02: DE33 planes driver and new bindings, H616 TCON TV/LCD, HDMI PHY, H616 display pipeline DT, LCD/LVDS pins, RG35XX LCD output, HDMI connector, GPIO backlight) · 0003 sun8i_tcon_top fix · 0110 generic MIPI/DPI-SPI panel driver
(kikuchan v2, 2025-02-26) · 0111 RG35XX-2024 uses it · 0155 RGB connector as DSI · 0161 gate the
display pipeline across suspend · 0162 release LCD0 pads while the panel is off.
Backlight/PWM: 0008 H616 PWM controller v8 (James Hilliard, 2026-08-04; earlier versions by Richard Genoud, Bootlin; driver `pwm-sun8i.c`, plus the `h616.dtsi` PWM node) · 0010 PWM backlight.
Power: 0124 battery name · 0157 power key: don't report the wake press · 0200 H700 OPPs · 0210 480 MHz
as the suspend OPP · 0007 GPU OPPs.
Input: 0140 ROCKNIX joypad driver · 0150 force feedback.
Board: 0126 Chris Morgan "rg35xx add additional hardware support" (2024-10-18) · 0127 mmc1 on
RG35XX-2024 · 0144 RG35XX-H DT update · 0154 `rocknix-dt-id` · 0156 RG35XX Plus RTC · 0159 keep the
second SD card powered · 0153 RGB LEDs · 0901 LED colour fix.
Audio: 0163 codec suspend + amp supply · 0204 HDMI sound (0203 sunxi_v2 AHUB disabled).
USB/MMC: 0040 musb revert · 0151 OTG host mode · 0152 RG35XX-2024 OTG · 0160 USB PHY clock ·
0158 H616 eMMC clock data0 mask.

## Boot loader
- `u-boot-DDR3` and `u-boot-DDR4`: **u-boot v2026.01**, defconfigs
  `anbernic_rg35xx_h700_lpddr3_defconfig` (`SUNXI_DRAM_H616_LPDDR3`, `DRAM_CLK=672`) and
  `anbernic_rg35xx_h700_lpddr4_defconfig`. Different ODT/drive/TPR values per type.
- Patches on both: `0001 Update dram_sun50i_h616.c` (Philippe Simons, 2026-04-01: LPDDR3 reads
  TPR6 bits 16-23, not 8-15) and `0002 sunxi: axp717 charge-only boot`.
- The `u-boot` meta package installs `H700_DDR3_u-boot-sunxi-with-spl.bin` and
  `H700_DDR4_…` side by side; TF-A platform `sun50i_h616`, `bl31.bin`.
- Release images are split DDR3 / DDR4 (release 20261101).

## Suspend
`h700-suspend-stub` (MagnominusGit, original work by kailashrs, GPL-2.0-or-later): an SRAM A1
program run by TF-A for PSCI `SYSTEM_SUSPEND`. It puts DRAM in self-refresh, gates clocks, waits
for a wake interrupt, then re-inits DRAM keeping its contents, so Linux resumes the same session.
Built once per DRAM type (`suspend_stub_lpddr4.bin`, `suspend_stub_lpddr3.bin`); TF-A embeds both
and picks by the installed DRAM. Uses a 28-line resume patch to u-boot's DRAM driver at build time.
No power figures published.

## What Redux would take, and what not
- Take: the SP DT approach, panel-variant handling, PWM backlight, suspend stub + 0157/0161/0162/0163,
  u-boot per DRAM type, rtw88.
- Leave: the ROCKNIX joypad driver (out of tree) in favour of upstream `gpio-keys` + `adc-joystick`,
  unless the 4-channel mux can't be described upstream; the sway/ES userland.
- Check: their OPP table (0200) against stock's bin-aware table and our Expert Mode rule; thermal trips.
