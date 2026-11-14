# Stock 4.9 kernel in Ghidra: function map

Desk research, 2026-11-14. Dale's Ghidra 12.2 project on DLN-DALEW2
(`REA\BootImage\ghidra-project\RG34XXSP-V106-kernel`). Input: the kernel `Image` from the **V1.0.6**
stock `boot.img` (`Linux version 4.9.170 … #9 SMP PREEMPT Tue May 26 15:49:58 CST 2026`), loaded as raw
AARCH64 at `0xffffff8008080000` (the kernel's link address; `CONFIG_ARM64_VA_BITS=39`).
Names below come from the kernel's own `__func__` strings (script `NameFunctionsFromFuncStrings.java`,
2,850 renames; 2,109 named of 36,849 functions in the export). Addresses are for **this build only**.
No Anbernic code is reproduced here; names and addresses only.

## Where to look, by Redux topic

| Topic | Functions (address) | Why |
|---|---|---|
| **Backlight** (writing 255 is ignored; real max between 20 and 254) | `disp_lcd_set_bright` (…8405ae0), `disp_lcd_bright_get_adjust_value` (…8405a6c), `disp_lcd_bright_curve_init` (…8403a80), `disp_lcd_get_bright` (…8405c60), `disp_lcd_set_bright_dimming` (…8406254), `disp_sys_pwm_config` (…84010e8) | The display driver maps a brightness value through a curve to PWM duty. The clamp is likely in `set_bright` or the curve. The `axp2202-battery/brightness` attribute (hardware-map.md) probably calls into these: follow `axp_sysfs_init` (named) |
| Panel `rg34xxsp_v1` | `disp_lcd_init` (…840400c, 4 KB), `bsp_disp_lcd_set_panel_funs` (…8402d94), `disp_lcd_set_panel_funs` (…8403e20), `disp_lcd_set_open_func` / `set_close_func`, `disp_lcd_pin_cfg`, `disp_lcd_gpio_*`, `disp_lcd_power_enable/disable`, `disp_lcd_pwm_enable/disable` | Panel drivers register through `set_panel_funs`; the `rg34xxsp_v1` string (…902c580) is data in that table. Its open/close sequence is the SPI init a mainline panel driver needs |
| Boot logo hand-off | `Fb_copy_boot_fb` (named) | How the U-Boot logo stays on screen into Linux |
| **Lid** | `hall_switch_isr` (…85c988c), `hall_press_work_func` (…85ca86c), `hall_release_work_func` (…85c98dc) | Interrupt plus work queue, next to the power-key code: confirms the stock lid is not a standard `SW_LID` switch |
| Sticks | `sunxi_gpadc_probe` (…85c0ddc), `sunxi_gpadc_ch_select` / `ch_deselect` / `channel_id` / `sample_rate_set`, `adc_joystick_open` (…85c1360), `joypad_store_adc_cal` (…85c14c0) | Mux channel switching and **stick calibration** (`joypad_store_adc_cal` is a sysfs store), useful for the mainline `adc-joystick` setup |
| Buttons | the `gpio-keys-polled` strings (…8c67e77; "does not support wakeup") | Polled buttons can't wake the device |
| Power chip | `axp2202_battery_probe` (…8649220), `axp_sysfs_init`, `set_my_work_lowpwr_led` (…86484d0) | Battery, charger, USB power, LEDs. Many other `axp2202_*` names exist only as log strings |
| Rumble | `sunxi_motor_startup` (…85cd108); "unable to request moto pwm" in the battery driver | A motor driver exists in the family kernel; whether the SP has a motor is open |
| CPU speed limits | `dev_pm_opp_of_cpumask_add_table_by_soc_bin` (…84bec50), `dev_pm_opp_of_get_sharing_cpus_by_soc_bin` (…84bea98), `sunxi_cpu_clocks_init` | The **bin-aware OPP table**: how stock picks voltages per chip. Compare before trusting any mainline OPP table |
| Thermal | `__cpufreq_cooling_register` (…86504dc), `cpufreq_cooling_get_level` | Throttling hooks; trips are in the DT (60/70/115 °C) |
| Headphone jack | `snd_sunxi_jack_gpio_init`, `snd_sunxi_jack_register`, `snd_sunxi_pa_pin_init` | Jack detection and amp enable on stock (mainline was still adding jack detection) |
| Suspend | `disp_suspend` / `disp_resume`, `sunxi_pwm_suspend` / `resume`, `cpufreq_suspend` / `resume`, `wakeup_reason_init`, `disp_register_standby_func` | Stock standby path, for F4 comparison |

## Not found by name
- No Mali (`kbase`) functions: the GPU driver is a separate module in stock (`mali_kbase.ko`).
- No RTL8821CS driver: also a module (`8821cs.ko`). Only generic `sdio_*` and `hci_*`.
- Most of the 36,849 functions are still `FUN_…`. Full names need the kernel's kallsyms table
  (`CONFIG_KALLSYMS_ALL=y`, base-relative); not yet extracted (open question 14).

## Stock V1.0.6 device tree: LCD node (`dtb.dts` from the same `boot.img`)
`lcd_driver_name = "rg34xxsp_v1"`, 720×480 (`lcd_x 0x2D0`, `lcd_y 0x1E0`), 150 × 94 mm (as written in the
DT), parallel RGB (`lcd_if 0`, `lcd_hv_if 0`), `lcd_dclk_freq` 26 MHz, `lcd_ht` 820, `lcd_vt` 536
(about 59.2 Hz), `hbp` 46, `hspw` 2, `vbp` 20, `vspw` 5, `lcd_hv_clk_phase 1`, `lcd_hv_sync_polarity 3`,
`lcd_frm 1` (dithering). Two LCD GPIOs on port I, pins 9 and 10 (`lcd_gpio_0/1`).
Backlight: `lcd_pwm_ch 0`, `lcd_pwm_freq` 50 kHz, `lcd_pwm_pol 1`, `lcd_pwm_max_limit` **255**,
`lcd_backlight` default **50**, `lcd_bright_curve_en 0` (no curve).

So the display driver itself allows the full 0–255 range with no curve.

## Brightness: resolved (Ghidra, 2026-11-14)
- `store_brightness` (…86467f8), the write handler of `/sys/class/power_supply/axp2202-battery/brightness`:
  parses the text as decimal (`simple_strtoul(buf, NULL, 10)`), takes a lock, and **stores the value in a
  global (…90e1944) only if it is below 101**; otherwise it logs `"--store_brightness: value = %ld err"`
  and keeps the old value. It always returns `count`, so the writer sees no error. Range: **0–100**.
- That global has exactly two references: the write in `store_brightness` and one read at …86468b0, right
  after it (most likely the matching `show_brightness`). **Nothing in the kernel applies it to the
  backlight.** The file is a storage slot for userspace (the stock UI), not a control.
- The real backlight path is `/dev/disp` ioctl `DISP_LCD_SET_BRIGHTNESS` (0x102) → the LCD device's
  `set_bright` pointer (stored at offset 0x238 by the LCD setup code) → `disp_lcd_set_bright`, which clamps
  to 0–255 and sets the PWM duty to about (value+1)/256 of the 50 kHz period. Redux already uses this path
  (`frontend/src/brightness.c`, levels 1–20 mapped to 16–255).
- This explains the 2026-11-12 measurement ("writing 20 sticks, 255 is ignored"): 255 > 100 is rejected by
  `store_brightness`, and the value never moved the backlight anyway.

## Lid: how stock reports it (Ghidra, 2026-11-14)
- `hall_switch_isr` (…85c988c): the hall-sensor interrupt only logs and queues `hall_press_work_func`
  with a **2-jiffy delay (20 ms at `CONFIG_HZ=100`)**: that is the whole debounce.
- `hall_press_work_func` (…85ca86c) reads the hall GPIO once. Raw 0 = closed (matches the active-low PE7
  in mainline's SP tree); it stores closed=1/open=0 in a global, then sends **key events, not a lid switch**:
  - lid **closed** → `KEY_DELETE` (111) down, `KEY_INSERT` up
  - lid **opened** → `KEY_INSERT` (110) down, `KEY_DELETE` up
  then queues `hall_release_work_func` **1 s later (100 jiffies)**, which releases both keys.
- The kernel itself does nothing else on lid close: no suspend, no screen off. Whatever happens is decided
  in userspace by whoever reads those keys (stock `dmenu.bin`).
- For Redux (F4): on stock, watch for `KEY_DELETE`/`KEY_INSERT` from the input device that carries them
  (check with `evtest` which `/dev/input/event*` it is), not `SW_LID`. A lid flapping faster than 1 s can
  produce overlapping presses. On mainline it becomes a proper `SW_LID` switch (upstream SP tree).

## Stick calibration
- `joypad_store_adc_cal` (…85c14c0) is a sysfs write handler: writing any non-zero number re-reads every
  stick channel (`FUN_…85c1280`) and stores the result as that axis's new reference (offset +0x10 of a
  0x28-byte per-channel record). In effect **"write 1 to recentre the sticks with them at rest"**.
  The sysfs file name and path aren't traced yet.

## Correction: two functions carried the name `axp2202_battery_probe`
- `…8649220` (3,612 bytes) **is** the real `axp2202_battery_probe` (it references its own `__func__`).
- `…815a2b0` (156 bytes, called from all over the kernel) is **`printk`** and was mislabelled during the
  manual renaming. Every "call to `axp2202_battery_probe`" in other decompiles (display, lid, OPP) is a log
  message. Rename `…815a2b0` to `printk`.

## CPU speed and voltage per chip grade (DT + Ghidra, 2026-11-14)
How stock picks: `dev_pm_opp_of_cpumask_add_table_by_soc_bin` (…84bec50) loops over the CPUs and calls
`FUN_…84bec14`, which picks the OPP table node for the chip's bin (`FUN_…84bea24`) and loads it (`_of_add_opp_table_v2`).
The bin comes from the chip's fuses: `opp_l_table` reads the 2-byte `speed` cell at offset 0 of the SID
(`speedbin_efuse = /sunxi-sid-ng@03006000/speed@00`). Each entry's `opp-supported-hw` bit says which bins may
use it; a second set of `-ab` entries carries per-grade voltages (`a1`, `b0`, `b1`, `c0`, `c1`, `c2`).

**The table is identical in V1.02 (2025-05, `reference/stock-202505`) and V1.0.6 (2026-05).** Top speed per bin:

| `opp-supported-hw` bit | Top speed | Voltage there |
|---|---|---|
| 0x01 | **1512 MHz** | 1120 mV |
| 0x02 | 1296 MHz | 1100 mV |
| 0x04 | 1344 MHz | 1120 mV |
| 0x08 | 1296 MHz | 1120 mV |
| 0x10 | 1416 MHz | 1150 mV |
| 0x20 | 1416 MHz | 1160 mV |

**Mainline is not the same table** (open question 5). Upstream `sun50i-h616-cpu-opp.dtsi` (Martin Botka 2023,
read at 7.4-rc2) is the H616 table with its own speed-grade numbering (`speed0`–`speed5`, read by the mainline
cpufreq-nvmem driver). At the top it uses **lower voltages than stock H700**: 1416 MHz at 1100 mV for bins 0x2d
(stock: 1150–1160 mV) and 1512 MHz at 1100 mV for speed1/speed3 (stock: 1120 mV, best bin only). ROCKNIX's
patch 0200 only drops 1032 MHz, opens 1320 MHz to one more grade and marks 1512 MHz as turbo.
How mainline's grade numbers map to stock's `opp-supported-hw` bits is **not established**.

Decided 2026-11-14 (notes/2026-11-14-dale-decision-mainline-cpu-table.md, option 2): on mainline Redux uses
ROCKNIX's H700 table, 1512 MHz only through Expert Mode, lower voltages accepted after a stress test on Dale's
unit. The bin mapping is still needed for that test; check the unit's grade on the device
(`/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies` on stock shows Dale's unit's bin).

