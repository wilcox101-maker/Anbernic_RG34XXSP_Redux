---
type: hw-result
from: dale (recorded by claude)
to: all
date: 2026-11-16
re: device session on d334375 (F5 step 1 read-outs #221, step 2 safety net #224, step 3 reads, Q1 heat run); #185
status: open
---

# Device session 15:45-16:06 (Dale; d334375 deployed with wget, checksum-checked)

| # | Check | Result |
|---|---|---|
| 1 | CPU tab vs `scaling_{governor,min_freq,max_freq,available_frequencies}` | PASS. Device: `schedutil`, 480000, 1512000, `480000 720000 936000 1008000 1104000 1200000 1320000 1416000 1512000` (9 steps) |
| 2-4 | Thermal zones and trips live; Display brightness; System rows | PASS ("A PASS", 15:47) |
| 5-6 | X saves; profile applied after restart | PASS. `eng-writes.log`: `15:47:40 profile.save - -> schedutil 480000-1512000 ok`, then at 15:48:17 `cpu.freq_khz ... ok` and `cpu.governor ... ok` |
| 7-8 | Governor same after a game; Y removes, next start stock | PASS ("B PASS", 15:54) |
| 9 | Reads for step 3 | See below |
| 10 | Full CPU load on the charger | 10 of 20 min used (Dale: "use 10 min for now"). See below |

Not tested: the two-failed-starts fallback (step 3 session).

## Step 3 reads (check 9)
- **GPU** `/sys/class/devfreq/gpu`: governor `simple_ondemand` (the only one); available 420, 456, 504, 552,
  600, 648 MHz; `cur_freq` 420 MHz; `min_freq` and `max_freq` both read **0**.
- **Battery attributes** (`axp2202-battery`): `boot_mode brightness capacity capacity_alert_min capacity_level
  charge_counter charge_full constant_charge_current device display_id energy_full_design health hallkey
  led_test lowpwr_led moto nds_esckey nds_pwrkey openbor_volume os_sleep otg_id power present serial_number
  spk_state status subsystem temp temp_alert_max temp_alert_min temp_ambient_alert_max temp_ambient_alert_min
  time_to_empty_now time_to_full_now type uevent voltage_now work_led workled_sleep`. No charge on/off switch.
- `/sys/class/leds/` is empty; `/sys/class/pwm/` has `pwmchip0`.

## Heat run (check 10), 15:56-16:05, 4 busy loops, on the charger
Columns: thermal zones 0-4 in m°C, then battery %. Zone order is the sysfs order (zone0 is the CPU zone).

| Time | zone0 | zone1 | zone2 | zone3 | zone4 | Bat % |
|---|---|---|---|---|---|---|
| 15:56:26 | 53522 | 45989 | 47690 | 44531 | 30000 | 56 |
| 15:58:57 | 56033 | 48338 | 49877 | 47690 | 30000 | 54 |
| 16:00:57 | 59192 | 51011 | 51659 | 50120 | 30000 | 56 |
| 16:02:58 | 61703 | 52874 | 54656 | 52631 | 30000 | 57 |
| 16:04:28 | 63080 | 54251 | 55628 | 53765 | 30000 | 59 |
| 16:05:58 | 64862 | 56762 | 58463 | 55709 | 30000 | 60 |

(Every 30 s in Dale's paste; one row a minute or two kept here.) CPU still rising about 1 °C/min at 10 min,
past the stock DTS first passive trip (60 °C; second passive 70 °C, critical 115 °C). zone4 is fixed at
30.0 °C (a dummy, like the battery file). The battery charged under full load (54 → 60 %).

## Follow-ups
- Docs listed 480/600/792/1008/1200/1296/1416/1512 MHz; Dale's unit has the 9-step list above. Corrected in
  this PR (hardware-map, ui-brief, desktop fake, mock-up). Redux code always reads the chip's own list.
- Heat-guard limits: [decision note](2026-11-16-claude-decision-f5-heat-guard.md).
