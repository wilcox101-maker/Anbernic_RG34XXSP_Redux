# Mainline status by hardware block

Columns: **Stock** = Anbernic BSP 4.9.170 (`reference/stock-202505`). **Upstream** = torvalds/linux
master at `97d73c8` (2026-11-13, v7.4-rc2). **ROCKNIX** = ROCKNIX/distribution main at `4fc79d7`
(2026-11-13), H700 device, kernel 7.2. Patch numbers refer to
`projects/ROCKNIX/devices/H700/patches/linux/`.

| Block | Stock 4.9 | Upstream 7.4-rc2 | ROCKNIX 7.2 | Redux takeaway |
|---|---|---|---|---|
| Board device tree | one family DT for all H700 models | RG35XX 2024, Plus, H (6.10) and **SP** (added later; version not checked). Nothing else. **No RG34XX-SP** | 15 H700 DTs incl. `rg34xx-sp` and `rg34xx-sp-v2-panel` (include upstream `rg35xx-sp.dts`) | Start from ROCKNIX's 23-line SP overlay; panel and stick mux are the only deltas they found |
| Display engine (DE33 mixer) | BSP disp2 | driver merged (`sun8i_mixer.c`: `sun50i-h616-de33-mixer-0`) | same plus TCON fix (0003) | In upstream code, not yet wired up |
| LCD timing controller + display DT nodes | BSP | **not in `sun50i-h616.dtsi`** | patch 0002 (21-patch display series from Jernej Skrabec et al., 2025-10) and 0155 | Must carry patches until the DT series merges (posted as v2, 2025-07) |
| Panel (720×480 RGB, SPI init) | `fog_fj035fhd05_v1` (2025) / `rg34xxsp_v1` (V1.0.6) | no generic SPI panel driver (only `panel-newvision-nv3052c`) | generic `panel-mipi-dpi-spi` (0110, kikuchan v2 2025-02-26) + per-panel init files `anbernic,rg34xx-sp-panel.panel` and `-v2-panel.panel` | Two panel variants exist. Matches field-reports: **identify the panel before driving it** |
| Backlight PWM | PWM0 via AXP attribute | **no H616 PWM driver** (`pwm-sun4i.c` only; series v4 by Bootlin 2026-03, v8 by James Hilliard 2026-08) | patch 0008 (H616 PWM v8, James Hilliard 2026-08-04, `pwm-sun8i.c`; not in upstream at 7.4-rc2) + 0010 pwm-backlight | Patch needed; real `/sys/class/backlight` once in |
| PMIC AXP717 (stock calls it AXP2202) | `axp2202` BSP driver | regulators, ADC, battery and USB supply in upstream DT (`x-powers,axp717*`) | same, battery name patch (0124) | Fuel gauge parity still to test; stock's battery temp is a dummy anyway |
| Power key | `axp2202-pek` | `axp20x-pek` | + don't report the wake press (0157) | Take 0157 |
| Lid (hall, PE7) | inside the PMIC key driver | `gpio-keys` **SW_LID** in `rg35xx-sp.dts` | inherited | The F4 lid model is upstream already |
| Buttons | `gpio-keys-polled`, 20 ms | `gpio-keys` (interrupts) in the 2024 DT | ROCKNIX joypad driver (0140) | Upstream path gives IRQ buttons; ROCKNIX's driver is out of tree |
| Analog sticks | GPADC via BSP mux driver | `adc-joystick` over `adc_mux` (in `rg35xx-h.dts`) | joypad driver, `amux-count = <4>`, right stick inverted (SP DT) | Our two sticks need the 4-channel mux and the inversion |
| GPU Mali-G31 | closed `mali_kbase` + libmali | Panfrost, `gpu` node in `h616.dtsi` | Panfrost, GPU OPPs (0007) | GLES 3.1 per ROCKNIX wiki; performance vs. blob untested by us |
| CPU OPPs | bin-aware table, 1296–1512 MHz top by bin (stock-kernel-ghidra.md) | `sun50i-h616-cpu-opp.dtsi`: H616 table, **lower top voltages than stock H700** | 0200 tweaks upstream's table; 480 MHz suspend OPP (0210) | **Decided (option 2, notes/2026-11-14-dale-decision-mainline-cpu-table.md):** use ROCKNIX's table; 1512 MHz only via Expert Mode; voltages accepted after a stress test on Dale's unit |
| Thermal | trips 60/70/115 °C | `ths` + cpu/gpu thermal zones | inherited | Check trip values against stock (hard rule: critical trip unchanged) |
| Audio codec | sun50iw9 codec | sun4i-codec in 2024 DT | codec suspend + amp supply (0163); HDMI sound (0204); alsa-ucm fix for upstream H616 files (release 20261101) | Upstream codec works; take 0163 for suspend |
| Wi-Fi RTL8821CS (SDIO) | out-of-tree `8821cs.ko` | rtw88 SDIO (Martin Blumenstingl, merged 2023) | `CONFIG_RTW88_8821CS=y`, firmware `rtw88/rtw8821c_fw.bin`, powersave off by udev | Standard driver + linux-firmware |
| Bluetooth | Realtek BT, `rtl_btlpm.ko` | btrtl | firmware `rtl_bt/rtl8821cs_*` | Standard |
| USB OTG | BSP | musb | OTG fixes (0040 revert, 0151, 0152, 0160) | Take if we use OTG |
| SD cards | BSP | sunxi-mmc | second card kept powered (0159), H616 eMMC clock fix (0158) | 0159 relevant (TF2) |
| LEDs | `work_led` / `low_pwr_led` | — | RGB LED patch (0153) | Low priority |
| Rumble | family pin PH2 (not confirmed on the SP) | — | force-feedback (0150) | Confirm the SP has a motor |
| Suspend to RAM | BSP standby | PSCI only, no H616 DRAM-resume path | **SRAM suspend stub** (`h700-suspend-stub`, TF-A `SYSTEM_SUSPEND`, DRAM self-refresh), display pipeline gated (0161), LCD pads released (0162) | Key enabler for F4; ROCKNIX's wiki still says "fake suspend", so check which one the 20261101 build uses |
| Boot loader | BSP boot0 + u-boot | u-boot sunxi has RG35XX-2024 (Chris Morgan series, Aug 2024; sunxi pull for 2025.01), H616 LPDDR4; LPDDR3 needs care | u-boot **v2026.01**, two builds: `anbernic_rg35xx_h700_lpddr3_defconfig` (672 MHz) and `..._lpddr4_defconfig`; patches: LPDDR3 TPR6 byte fix, AXP717 charge-only boot | Board revision = DRAM type at build time. One image per DRAM type, or detect in SPL |
| DRAM sizes | 2 GB original; 512 MB / 1 GB LPDDR3 revised (field reports) | — | LPDDR3 and LPDDR4 images | Matches the revised boards; verify on Dale's unit (`dmesg`, `/proc/meminfo`) |

## Upstream `sun50i-h700-anbernic-rg35xx-sp.dts` vs. the RG34XXSP

Read at torvalds/linux `97d73c8` (2026-11-13). Authors Ryan Walklin and Chris Morgan, 2024,
GPL-2.0-only OR BSD-2-Clause. It's short: it includes `rg35xx-plus.dts` (which includes
`rg35xx-2024.dts`) and adds two things.

| Piece | Upstream SP tree | RG34XXSP (`docs/hardware-map.md`) | Fit |
|---|---|---|---|
| Lid | `gpio-keys` `SW_LID` on **PE7**, active low, wake source on open (`EV_ACT_DEASSERTED`), `linux,can-disable` | Hall sensor on **PE7** (stock reports it through the AXP2202 power key) | Same pin. Use as is |
| External RTC | `nxp,pcf8563` at **0x51** on `r_i2c` | Not documented for our unit | **Unknown**: check `i2cdetect` on `r_i2c` or the stock DT. A missing chip only fails to probe |
| Wi-Fi / BT (from Plus) | RTL8821CS on `mmc1`, power sequence reset **PG18**, host-wake **PG15**, `cldo4`/`aldo4` supplies | RTL8821CS on SDIO 1, powered from `axp2202-cldo4` | Same family; check PG18/PG15 against the stock DT |
| Buttons (from 2024) | `gpio-keys`, interrupts | `gpio-keys-polled`, 20 ms | Same buttons; upstream is better (interrupts) |
| **Analog sticks** | **none** (the RG35XX-SP has no sticks) | **two sticks** through a 4-way analog mux | **Missing: must be added** |
| Display, backlight | none upstream (see table above) | 720×480 SPI-init panel, PWM backlight | ROCKNIX patches |

**The sticks can be described with upstream bindings** (answers most of open question 6). Upstream's
`rg35xx-h.dts` already has the block we need: `adc-joystick` over an `io-channel-mux` on `gpadc 0`,
driven by a `gpio-mux` on **PI1 / PI2**, which are exactly our stock mux-select pins. Differences for us:
- Our stock DT also has an analog-mux enable on **PI8** and an ADC enable on **PE11**; the H tree has
  neither. They can be driven high with fixed GPIO settings (e.g. `gpio-hog` or a fixed regulator).
- Axis inversion is done upstream by reversing `abs-range` (e.g. `<4096 0>`), which covers what
  ROCKNIX does with `invert-absrx` / `invert-absry` in its out-of-tree joypad driver.
- Stock reports the sticks as `ABS_Z/RX/RY/RZ`; the H tree uses `ABS_X/Y/RX/RY` (standard gamepad layout).
  Redux's input layer would have to map either.

So a mainline RG34XXSP tree is roughly: upstream SP tree + the H tree's stick block (+ PI8/PE11
enables) + ROCKNIX's panel compatible. Not built or tested.

