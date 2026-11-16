# Stock 4.9 kernel in Ghidra: function map

Desk research, 2026-11-14. Dale's Ghidra 12.2 project on DLN-DALEW2
(`REA\BootImage\ghidra-project\RG34XXSP-V106-kernel`). Input: the kernel `Image` from the **V1.0.6**
stock `boot.img` (`Linux version 4.9.170 … #9 SMP PREEMPT Tue May 26 15:49:58 CST 2026`), loaded as raw
AARCH64 at `0xffffff8008080000` (the kernel's link address; `CONFIG_ARM64_VA_BITS=39`).
**Names come from the kernel's own kallsyms table** (2026-11-15, Ghidra plan G0.2; see "Kallsyms" below):
all 36,849 functions in the project are named. Earlier names guessed from `__func__` strings were wrong for
480 functions; this page now uses the kallsyms names. Addresses are for **this build only**.
No Anbernic code is reproduced here; names, addresses, constants and behaviour only.
Tags: **[G]** static analysis of this kernel (Ghidra 12.2; kallsyms; relocation table), **[D]** device tree,
strings or build config, **[V]** device-verified (only with a Dale hw-result note), **[N]** no public source we know
of (list in the H700 repo `docs/ghidra-plan.md`). Nothing here is [V] yet.

## Where to look, by Redux topic

| Topic | Functions (address) | Why |
|---|---|---|
| **Backlight** (resolved, see "Brightness") | `disp_lcd_set_bright` (…8405ae0), `disp_lcd_bright_get_adjust_value` (…8405a6c), `disp_lcd_bright_curve_init` (…8403a80), `disp_lcd_get_bright` (…8405c60), `disp_lcd_set_bright_dimming` (…8406254), `disp_sys_pwm_config` (…84010e8) | [G] The control is `/dev/disp` ioctl `0x102` → `disp_lcd_set_bright`, 0–255, no curve. `axp2202-battery/brightness` is only a 0–100 storage slot (`store_brightness`/`show_brightness`, value in `brightLevl`); nothing in the kernel applies it |
| Panel `rg34xxsp_v1` | `disp_lcd_init` (…840400c, 4 KB), `bsp_disp_lcd_set_panel_funs` (…8402d94), `disp_lcd_set_panel_funs` (…8403e20), `disp_lcd_set_open_func` / `set_close_func`, `disp_lcd_pin_cfg` (…8404fb0), `disp_lcd_gpio_init`, `disp_lcd_power_enable/disable`, `disp_lcd_pwm_enable/disable` | Panel drivers register through `set_panel_funs`. [G][N] The `rg34xxsp_v1` name (…902c580) is the start of the panel table entry `fog_fj035fhd05_v1_panel`: the stock panel driver's source name is `fog_fj035fhd05_v1`, registered as `rg34xxsp_v1` (the last of 17 entries in the built-in panel table, which starts with `default_panel` at …902bf80, entries 0x60 apart). Its open/close sequence is the SPI init a mainline panel driver needs |
| Boot logo hand-off | `Fb_copy_boot_fb.isra.6` (…83fcfcc) | How the U-Boot logo stays on screen into Linux |
| **Lid** | `hall_switch_isr` (…85c988c), `hall_press_work_func` (…85ca86c), `hall_release_work_func` (…85c98dc) | Interrupt plus work queue, next to the power-key code: confirms the stock lid is not a standard `SW_LID` switch |
| Sticks | `sunxi_gpadc_probe` (…85c0ddc), `sunxi_gpadc_channel_id` / `sample_rate_set` / `ctrl_set`; the joypad code lives **inside the stock `gpio-keys-polled` driver**: `gpio_keys_polled_probe`, `gpio_keys_polled_open` (…85c1360, earlier mislabelled `adc_joystick_open`), `adc_joystick_read.isra.1` (…85c1280), `joypad_store_adc_cal` (…85c14c0) / `joypad_show_adc_cal` (…85c143c) | Mux channel switching and **stick calibration**: sysfs `adc_cal` (see "Stick calibration"), useful for the mainline `adc-joystick` setup |
| Buttons | the `gpio-keys-polled` strings (…8c67e77; "does not support wakeup") | Polled buttons can't wake the device |
| Power chip | `axp2202_battery_probe` (…8649220), `axp20x_device_probe` (…850e96c, earlier `axp_sysfs_init`), `allen_my_led_poll` (…86484d0, earlier `set_my_work_lowpwr_led`), `axp2202_bat_suspend`, `axp2202_usb_suspend`; battery sysfs attributes in `allen_power_attrs` (`brightness`, `os_sleep`, …) | Battery, charger, USB power, LEDs |
| Rumble | `sunxi_motor_startup` (…85cd108); "unable to request moto pwm" in the battery driver | A motor driver exists in the family kernel; whether the SP has a motor is open |
| CPU speed limits | `dev_pm_opp_of_cpumask_add_table_by_soc_bin` (…84bec50), `dev_pm_opp_of_get_sharing_cpus_by_soc_bin` (…84bea98), `sunxi_cpu_clocks_init` | The **bin-aware OPP table**: how stock picks voltages per chip. Compare before trusting any mainline OPP table |
| Thermal | `__cpufreq_cooling_register` (…86504dc), `cpufreq_cooling_get_level` | Throttling hooks; trips are in the DT (60/70/115 °C) |
| Headphone jack | `snd_sunxi_jack_gpio_init`, `snd_sunxi_jack_register`, `snd_sunxi_pa_pin_init` | Jack detection and amp enable on stock (mainline was still adding jack detection) |
| Suspend | `psci_system_suspend_enter`, `arisc_set_wakeup_source`, `axp2101_powerkey_suspend`, `disp_suspend` / `disp_resume`, `sunxi_pwm_suspend`, `cpufreq_suspend` | See "Sleep path" (G1), for F4/F5 |

## Kallsyms: every function named (G0.1, G0.2; 2026-11-15) [G]
- The table was read with a stdlib reader (H700 research repo `tools/kallsyms_49.py`, host test
  `tests/test_kallsyms_49.py`): **90,760 symbols**, 41,939 of them code. In this build every table is padded
  to 256 bytes, and `kallsyms_relative_base` reads 0 in the Image because the kernel is relocatable (a boot-time
  fixup fills it); the base is `_text` = `0xffffff8008080000`. That is why the first attempt found nothing.
- Cross-check (Dale's decision, 2026-11-15): `vmlinux-to-elf` 1.4.0 finds the same tables and base and gives
  the **identical** symbol list (90,759 address+name pairs, 0 differences). Its ELF is
  `REA\BootImage\kallsyms\Image-v106-kallsyms.elf` (new file; the stock `Image` is unchanged).
- Against Ghidra's 36,849 functions: **36,271 start exactly at a kallsyms code symbol**; the other 578 are pieces
  Ghidra split off inside a known function and are named `<function>_part_<offset>`. **36,849 of 36,849 named.**
- Checks: `printk` is at …815a2b0 and `axp2202_battery_probe` at …8649220, as found by hand. 17 named functions
  from this page were checked against their own `__func__` string in Ghidra: 17 of 17 reference it.
  (Three more names on the old list, `adc_joystick_open`, `axp_sysfs_init`, `set_my_work_lowpwr_led`, were
  wrong guesses: kallsyms gives the names now in the table above.) Of 2,109 functions named earlier, 1,593 agree
  with kallsyms, 480 disagree (the old script took a nearby string), 36 were pieces.
- Applying it to the project: Ghidra script `ReduxApplyKallsyms.java` with `kallsyms-v106.tsv` (both in
  `REA\BootImage\kallsyms`; tested on a copy of the project with Ghidra 12.2 headless). It renames …815a2b0 to
  `printk` (G0.1), names every function, adds 45,594 data labels and logs every earlier name it replaced.
  The full symbol list stays in `REA` (derived from the Anbernic binary), not in this repo.
- Still not in the kernel: Mali (`mali_kbase.ko`) and RTL8821CS (`8821cs.ko`) are modules.

## Stock V1.0.6 device tree: LCD node (`dtb.dts` from the same `boot.img`)
`lcd_driver_name = "rg34xxsp_v1"`, 720×480 (`lcd_x 0x2D0`, `lcd_y 0x1E0`), 150 × 94 mm (as written in the
DT), parallel RGB (`lcd_if 0`, `lcd_hv_if 0`), `lcd_dclk_freq` 26 MHz, `lcd_ht` 820, `lcd_vt` 536
(about 59.2 Hz), `hbp` 46, `hspw` 2, `vbp` 20, `vspw` 5, `lcd_hv_clk_phase 1`, `lcd_hv_sync_polarity 3`,
`lcd_frm 1` (dithering). Two LCD GPIOs on port I, pins 9 and 10 (`lcd_gpio_0/1`).
Backlight: `lcd_pwm_ch 0`, `lcd_pwm_freq` 50 kHz, `lcd_pwm_pol 1`, `lcd_pwm_max_limit` **255**,
`lcd_backlight` default **50**, `lcd_bright_curve_en 0` (no curve).

So the display driver itself allows the full 0–255 range with no curve.

## Brightness: resolved (Ghidra, 2026-11-14) [G][N]
- `store_brightness` (…86467f8), the write handler of `/sys/class/power_supply/axp2202-battery/brightness`:
  parses the text as decimal (`simple_strtoul(buf, NULL, 10)`), takes a lock, and **stores the value in a
  global (…90e1944) only if it is below 101**; otherwise it logs `"--store_brightness: value = %ld err"`
  and keeps the old value. It always returns `count`, so the writer sees no error. Range: **0–100**.
- That global (`brightLevl`, kallsyms) has exactly two references: the write in `store_brightness` and one
  read inside `show_brightness` (…8646880). **Nothing in the kernel applies it to the
  backlight.** The file is a storage slot for userspace (the stock UI), not a control.
- The real backlight path is `/dev/disp` ioctl `DISP_LCD_SET_BRIGHTNESS` (0x102) → the LCD device's
  `set_bright` pointer (stored at offset 0x238 by the LCD setup code) → `disp_lcd_set_bright`, which clamps
  to 0–255 and sets the PWM duty to about (value+1)/256 of the 50 kHz period. Redux already uses this path
  (`frontend/src/brightness.c`, levels 1–20 mapped to 16–255).
- This explains the 2026-11-12 measurement ("writing 20 sticks, 255 is ignored"): 255 > 100 is rejected by
  `store_brightness`, and the value never moved the backlight anyway.

## Lid: how stock reports it (Ghidra, 2026-11-14) [G][N]
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

## Stick calibration (G0.3, traced 2026-11-15) [G][N]
- `joypad_store_adc_cal` (…85c14c0) is a sysfs write handler: writing any non-zero number re-reads every
  stick channel (`adc_joystick_read`, …85c1280) and stores the result as that axis's new reference (offset
  +0x10 of a 0x28-byte per-channel record). In effect **"write 1 to recentre the sticks with them at rest"**.
- [N] The file is **`adc_cal`** (mode 0644; read handler `joypad_show_adc_cal`), attribute `dev_attr_adc_cal`, in
  the group `joypad_attr_group` together with **`amux_debug`**. `gpio_keys_polled_probe` creates the group on
  its own platform device: the stock **`gpio-keys-polled`** driver, DT node `/soc@03000000/gpio_keys`
  (label `ANBERNIC-keys`) [D]. So the file sits in that device's sysfs folder; the exact folder name (4.9 names
  DT devices without `reg` from the node path) is for the device check.
- Device check (optional): `find /sys/devices -name adc_cal`, then with both sticks at rest
  `echo 1 > <that path>` and `cat` it; the sticks should still centre in the menu.

## Correction (closed 2026-11-15): two functions carried the name `axp2202_battery_probe`
- `…8649220` is the real `axp2202_battery_probe`; `…815a2b0` is **`printk`** (kallsyms confirms both). Every
  "call to `axp2202_battery_probe`" in the 2026-11-14 decompiles (display, lid, OPP) is a log message.
  `ReduxApplyKallsyms.java` renames it in the project (G0.1).

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

## Sleep path (G1, 2026-11-15)
**Answer: stock sleep is real suspend-to-RAM handed to firmware** (PSCI `SYSTEM_SUSPEND` to the ATF/BL31), not
Allwinner "scene"/dynamic standby and not screen-off idle. Correction (Deep Research W3.4, 2026-11-15): the stock
BL31 offers `SYSTEM_SUSPEND` and runs standby itself; there is **no SCP (AR100) firmware**, and the kernel's
`arisc_*` calls are answered by BL31. Whether a real suspend works is still a device check (first line below).
- **Trigger** [D]: stock userspace writes `mem` to `/sys/power/state` (docs/stock-userspace.md). The kernel never
  sleeps on its own: `CONFIG_PM_WAKELOCKS=y` but no `PM_AUTOSLEEP` [D]; the lid only sends keys ("Lid" above).
- **Modes** [G]: `/sys/power/state` offers `freeze` (always, `SUSPEND_FREEZER`) and `mem` only if the firmware
  answers PSCI_FEATURES for `SYSTEM_SUSPEND` (`0xC400000E`): `psci_0_2_init` registers `psci_suspend_ops`
  (`valid` = `suspend_valid_only_mem`, `enter` = `psci_system_suspend_enter`) only then. No `standby`.
  **There is no `/sys/power/mem_sleep`** in 4.9 (added in 4.10; no symbol for it).
- **Call order for `mem`** [G]: core `pm_suspend` → devices' suspend callbacks (display, PWM, AXP, USB, MMC,
  Wi-Fi …) → `psci_system_suspend_enter` → `cpu_suspend` → SMC `0xC400000E` with the kernel resume address →
  BL31 powers the SoC down and wakes it on a registered source. The DRAM/power sequence itself is in
  firmware, not in this kernel. DT [D]: `psci` node `arm,psci-1.0`, `smc`; `pm_para/standby_mode = 1` (Allwinner
  "super standby"), but the kernel has no `standby_mode` string, so that value is read by firmware if at all.
- **Wake sources** [G]: every wake IRQ is passed to firmware with `arisc_set_wakeup_source` (answered by BL31, W3.4), from
  `sunxi_irq_set_wake` (the interrupt-domain code next to `sunxi_nmi_*`, likely the `sunxi_wakeupgen` layer: any
  interrupt marked as a wake source, such as the PMIC's; DT `pmu_irq_wakeup = 1` [D]) and `sunxi_pinctrl_irq_set_wake` (GPIO interrupts).
  - **Power key** [G][D]: the AXP power-key interrupts (`wakeup_rising`, `wakeup_falling` in the DT).
  - **Lid** [G][N]: `axp2101_powerkey_suspend` arms the hall-sensor IRQ as a wake source **if the power-key
    device may wake** (it also cancels pending lid key work). So on stock, opening the lid can wake.
  - **`os_sleep` switch** [G][N]: `/sys/class/power_supply/axp2202-battery/os_sleep` (mode 0644, in
    `allen_power_attrs`). Writing non-zero **turns power-key device wakeup off** (and with it the lid wake);
    0 turns it on. The USB host drivers (`sunxi_ehci_hcd_suspend`/`ohci`) also check it. It reads 0 after boot.
  - **Timer** [G]: `/sys/power/sunxi_debug/time_to_wakeup_ms` (0644): writing N asks the firmware to wake after N ms.
  - **RTC** [G][D]: `sunxi-rtc` is `wakeup-source` and calls `device_init_wakeup`, so `rtc0/wakealarm` should work.
  - **Not wake sources** [G][D]: the buttons and sticks (`gpio-keys-polled`, polled, "does not support wakeup");
    USB (`usb_wakeup_suspend = 0` on every USB node); **Wi-Fi** (`8821cs` is built without WoWLAN and the DT has no
    host-wake pin) [G][D].
- **Display and PWM hooks** [G]: `disp_suspend` turns off every enabled output (panel and backlight) through each
  device's own disable call, runs the devices' suspend calls, then the registered standby callbacks
  (`disp_suspend_cb`; the composer registers `hwc_suspend` through `disp_register_standby_func` in
  `composer_init`). `disp_resume` reverses it and re-enables through queued work. `sunxi_pwm_suspend` applies a
  disabled state to its channels and stops the PWM clock. `cpufreq_suspend` is the plain 4.9 core.
- **Idle, not sleep** [D]: CPU idle uses the PSCI `cpu-sleep-0` state (`arm,psci-suspend-param 0x10000`).
- **Debug** [G][D]: `/sys/kernel/wakeup_reasons/last_resume_reason`, `/sys/kernel/debug/wakeup_sources`
  (`DEBUG_FS=y`), `/sys/power/wake_lock`, `/sys/power/pm_test` (`PM_DEBUG=y`).

Device checks (optional, read-only on stock; for the Device test queue):
- `cat /sys/power/state` (expect `freeze mem`; only `freeze` would mean the firmware refuses `SYSTEM_SUSPEND`),
  `ls /sys/power/mem_sleep` (expect: no such file), `cat /sys/class/power_supply/axp2202-battery/os_sleep`.
- `cat /sys/kernel/debug/wakeup_sources` (if debugfs is mounted) and, after a sleep/wake,
  `cat /sys/kernel/wakeup_reasons/last_resume_reason`.
- Battery % before and after 30 min asleep with the lid closed; note whether opening the lid wakes it.

## Waves 2 and 3: what matters for Redux (2026-11-15)
Full records with data tables are in the H700 research repo (`wilcox101-maker/Allwinner-H700-Custom-Kernel-Research`,
`docs/ghidra-*.md`): G2 CPU grades, G4 battery gauge, G5/G6 audio and rumble, W3.1 kernel inventory, W3.2 modules,
W3.3 DT diff. Only what Redux on stock can use is repeated here.

**Vendor files on `/sys/class/power_supply/axp2202-battery/`** [G][N] (from W3.1; `os_sleep` and `brightness` above):
| File | Mode | What it does |
|---|---|---|
| `moto` | RW | Rumble: 0–100 % duty on PWM2/PH2, 2 kHz period (G6). The joypad input device also supports `FF_RUMBLE` (levels under 37 % become 0) |
| `hallkey` | R | Lid state |
| `spk_state` | R | Speaker amp state (the jack logic turns the amp off while headphones are in, G5) |
| `otg_id` | R | USB OTG id |
| `display_id` | R | 1 when the `/dev/disp` output switch selects HDMI (most likely), else 0 [N] |
| `boot_mode` | R | 1 after a charge-only boot (`bootreason=charger`), else 0 [N] |
| `work_led`, `lowpwr_led`, `led_test`, `workled_sleep` | RW | LED GPIOs (PI12 work, PI11 low power) |
| `openbor_volume`, `nds_esckey`, `nds_pwrkey` | RW | Storage slots for stock apps, no kernel effect: `openbor_volume` is the **shared volume level** read by nearly every stock emulator; `nds_pwrkey` by PPSSPP/RetroArch; `nds_esckey` by the NDS/ports control apps [N] |

Other facts for Redux on stock [G][D]:
- **Battery %** is the PMIC gauge value (as stock shows it); stock charges at 1472 mA to 4.10 V and warns at 15 % (G4).
- **Wi-Fi** (`8821cs` v5.5.1, loaded by `/etc/rc.local` with no parameters): power save is on by default
  (`rtw_power_mgnt = 2`, IPS 1, LPS 1). Only `8821cs` and `mali_kbase` are loaded at boot (W3.2).
- **Headphone jack** PI3 (200 ms debounce), speaker amp PI5 with a 100 ms on-delay (G5).
- **Buttons** [D][N]: the stock DT names PA3 "x" (code 306) and PA2 "y" (307), the reverse of Redux's stock remap table
  (306 = Y, 307 = X), which Redux uses on the device. Probably a DT label mix-up; one `evtest` press settles it.
- **CPU:** Dale's fuse value decides whether mainline would run his chip above stock's top step (G2; Phase 2).

Device checks (optional, on stock; all read-only except the `moto` line):
- `ls /sys/class/power_supply/axp2202-battery/` and `cat hallkey spk_state otg_id display_id boot_mode`.
- With headphones in and out: `cat …/spk_state`. Lid closed and open: `cat …/hallkey`.
- `echo 50 > …/moto`, then `echo 0 > …/moto` (writes a runtime value only): does the unit vibrate (answers K13)?
- `evtest` on `ANBERNIC-keys`: press X and Y, note 306/307.
