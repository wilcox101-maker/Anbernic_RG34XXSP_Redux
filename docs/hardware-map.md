# RG34XXSP hardware map

Source: the device tree embedded in the stock `boot.img` (firmware `rg34xxsp-fw-sd_202505`),
decompiled to [`reference/stock-202505/board.dts`](../reference/stock-202505/board.dts).

> Anbernic ships **one device tree for the whole H700 family**. Some nodes describe hardware the
> SP doesn't have (L3/R3, the analog-stick mux, the rumble motor pin). Those are marked *family*
> below and need confirming on real hardware.

## SoC and memory

- **Allwinner H700**, which the tree identifies as `allwinner,h616` / `sun50iw9p1`
- 4× Cortex-A53. The CPU operating points are **bin-aware** (`opp-supported-hw`): each chip's
  fuses select the voltage set Allwinner validated for that silicon.
  - Ceiling: **1512 MHz** (best bin). Common steps: 480 / 600 / 792 / 1008 / 1200 / 1296 / 1416 / 1512 MHz.
- **Mali-G31** GPU, driven by Arm's closed kernel driver (the tree says `arm,mali-midgard`)
- **2 GB RAM.** The memory node reads `0x20000000` (512 MB) and the DRAM settings say
  `dram_type = 3`. Both are template values: boot0 trains the real DRAM and u-boot rewrites the node.
  Check RAM on the running system, not in the device tree.

## Thermal (stock limits, kept as-is)

| Trip | Temperature | Action |
|---|---|---|
| trip-point@0 | 60 °C | passive (power-allocator control starts) |
| trip-point@1 | 70 °C | passive (CPU/GPU cooling maps engage) |
| cpu_crit | 115 °C | critical shutdown |

## Buttons

Driver: `gpio-keys-polled`, label `ANBERNIC-keys`, **poll-interval 20 ms**. That alone adds up to a
frame of input lag. All buttons are active-low GPIOs.

| Button | Pin | Event | Code sent |
|---|---|---|---|
| D-pad Up / Down | PA6 / PE0 | EV_ABS | ABS_HAT0Y −1 / +1 |
| D-pad Left / Right | PA8 / PA9 | EV_ABS | ABS_HAT0X −1 / +1 |
| A | PA0 | EV_KEY | BTN_SOUTH |
| B | PA1 | EV_KEY | BTN_EAST |
| X | PA3 | EV_KEY | BTN_C |
| Y | PA2 | EV_KEY | BTN_NORTH |
| Start | PA4 | EV_KEY | BTN_TR ⚠ |
| Select | PA5 | EV_KEY | BTN_TL ⚠ |
| L1 | PA10 | EV_KEY | BTN_WEST ⚠ |
| L2 | PA11 | EV_KEY | BTN_SELECT ⚠ |
| R1 | PA12 | EV_KEY | BTN_Z ⚠ |
| R2 | PA7 | EV_KEY | BTN_START ⚠ |
| Menu | PE3 | EV_KEY | BTN_TL2 ⚠ |
| L3 / R3 *(family)* | PE8 / PE9 | EV_KEY | BTN_TR2 / BTN_MODE |

**Analog sticks: two, confirmed present on the RG34XXSP** (Dale, 2026-11-10). In the stock DT they have
no node of their own. The `ANBERNIC-keys` node carries an analog multiplexer, `amux-en-gpios` (PI8),
mux selects `A0_gpio` / `A1_gpio` (PI1 / PI2) and `adc-en-gpios` (PE11), read through the GPADC by
Anbernic's driver (stock also loads `gpio_keys_gpadc.ko` from the SD card if present). Their event
codes aren't in the DT, so check with `evtest` on the device. Mainline's RG35XX-H tree models the same
idea as `adc-joystick` over an `adc_mux` (`ABS_X/Y/RX/RY`), the model for Phase 2. L3 / R3 (stick
clicks) are then probably real too, not *family*. To confirm.

⚠ = Anbernic's scrambled codes. Every frontend needs a remap table. We'll emit standard Linux
gamepad codes instead. Regenerate this table with `python3 tools/dt_keys.py`.

Power and volume keys are on the AXP2202 power-key driver and the ADC (to be documented).

## Lid (clamshell)

- Hall sensor on **PE7**, declared as `hall_key` inside the **AXP2202 power-key node**
  (`x-powers,axp2101-pek`). The stock kernel treats the lid as a power-key event, not a standard lid
  switch.
- Mainline Linux's RG35XX-SP device tree exposes the same pin as a proper `gpio-keys`
  **SW_LID** switch, which is the model for Phase 2.

## Display

| Property | Value |
|---|---|
| Interface | parallel RGB (`lcd_if = 0`) |
| Resolution | 720 × 480 |
| Panel driver | `fog_fj035fhd05_v1` |
| Pixel clock | 26 MHz |
| H total / V total | 820 / 536 |
| **Refresh** | 26 MHz ÷ (820 × 536) ≈ **59.16 Hz** |
| Backlight | PWM0, 50 kHz, max 255 |

720×480 is exactly **3× GBA** (240×160) and 3× Game Boy vertically (144 lines). That makes integer
scaling the native mode. GBA runs at 59.73 Hz, so audio/video sync should target the panel's real
rate. The panel driver may override these timings at runtime (to verify on device).

## Power: AXP2202 PMIC (I²C 0x34)

- USB-C, VBUS detect on PH13, battery temperature table, fuel gauge
- Charge LEDs: `work_led` PI12, `low_pwr_led` PI11
- `moto_pwm_gpio` PH2 *(family, rumble)*
- Power key: long-press 1.5 s, forced-off 6 s; wakes on both edges
- **Battery-temperature protection is disabled:** `pmu_bat_temp_enable = <0x0>`. Charge and shutdown
  thresholds are defined (`pmu_bat_charge_ltf/htf`, `pmu_bat_shutdown_ltf/htf`) with a 16-point NTC
  table (`pmu_bat_temp_para1…16`), but nothing uses them. It's unverified whether the battery has a
  working thermistor at all. Check `/sys/class/power_supply/axp2202-battery/temp` on the device. With
  no PMIC protection, the OS has to watch battery heat itself (see [ui-brief.md](ui-brief.md))

## Audio

- Internal codec (sun50iw9), `LINEOUT` routed to one `SPK` path. Amp powered from `axp2202-cldo1`
  with ramp-up/down enabled.
- Headphone jack detect via `SND_SOC_SUNXI_JACK_GPIO`

## Wireless

- Wi-Fi: Realtek **RTL8821CS** on SDIO bus 1, powered from `axp2202-cldo4` (kernel module `8821cs.ko`)
- Bluetooth: Realtek, reset on PG19, low-power mode via `RTL_BT_LPM`

## Stock kernel config highlights

From [`reference/stock-202505/kernel.config`](../reference/stock-202505/kernel.config):

| Option | Value | Note |
|---|---|---|
| Version | 4.9.170, GCC 5.3.1 (Linaro 2016.05) | built 2025-04-12 |
| `CPU_FREQ_DEFAULT_GOV_PERFORMANCE` | y | pinned at full clock. **Phase 1 fix** (set at runtime) |
| `HZ` | 100 | coarse timer tick, needs a rebuild to change |
| `PREEMPT` | y | |
| `ZRAM` | not set | |
| `SCHEDUTIL`, `ONDEMAND`, `INTERACTIVE` governors | available | |
| `SQUASHFS` (+LZ4), `OVERLAY_FS`, `EXFAT_FS` | y | enough for a RAM-loaded squashfs system on the stock kernel |
| `CMA_SIZE_MBYTES` | 16 | overridden by `cma=64M` on the command line |
