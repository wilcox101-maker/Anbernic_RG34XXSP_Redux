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

## Buttons and sticks (confirmed on device, 2026-11-10)

Driver: `gpio-keys-polled`, label `ANBERNIC-keys`, device `/dev/input/event1`.

**Analog sticks** (range −4096…+4096, fuzz/flat 32):

| Stick | Axis | Code |
|---|---|---|
| Left X | ABS_Z | 2 |
| Left Y | ABS_RX | 3 |
| Right X | ABS_RY | 4 |
| Right Y | ABS_RZ | 5 |

**Digital D-pad**
- ABS_HAT0X (16): −1 left, +1 right
- ABS_HAT0Y (17): −1 up, +1 down

**Buttons** (scrambled codes):

| Button | Event code |
|---|---|
| A | BTN_SOUTH (304) |
| B | BTN_EAST (305) |
| X | BTN_C (306) |
| Y | BTN_NORTH (307) |
| L1 | BTN_WEST (308) |
| R1 | BTN_Z (309) |
| L2 | BTN_TL (310) |
| R2 | BTN_TR (311) |
| Select | BTN_SELECT (314) |
| Start | BTN_START (315) |
| Menu | BTN_TL2 (312) |
| L3/R3 (family) | BTN_TR2 (313) / BTN_MODE (316) |

## Lid (clamshell) — confirmed

`/sys/class/power_supply/axp2202-battery/hallkey`:
- Lid open: `1`
- Lid closed: `0`

## Battery temperature — dummy value

`/sys/class/power_supply/axp2202-battery/temp` stayed at **300** under:
- 120 s full CPU load (CPU temp rose 39.8 °C → 58.6 °C)
- ~5 min charging at 5 V / 3 A (capacity 29 % → 32 %)

Treat as a dummy. UI shows "battery temp: unknown". Heat guard falls back to SoC temperature only.
Matches the stock DTS setting `pmu_bat_temp_enable = <0x0>`.

## Other confirmed readings

- Governor: `performance` (stock default)
- voltage_now: 3935000 (3.935 V)
- capacity: 35 %
- `current_now` does not exist
- Input devices: event0 = axp2202-pek, event1 = ANBERNIC-keys (js0), event2 = dierct-keys-polled

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
- **Battery-temperature protection is disabled:** `pmu_bat_temp_enable = <0x0>`. Confirmed dummy
  on device (see above).

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
