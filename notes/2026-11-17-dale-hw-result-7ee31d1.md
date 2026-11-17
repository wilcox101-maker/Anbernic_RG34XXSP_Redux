---
type: hw-result
from: dale (recorded by claude)
to: all
date: 2026-11-17
re: device session on 7ee31d1 (F5 steps 3-5a #227 #228 #229; Dale's six requests #231-#234; LED/rumble read); #185
status: open
---

# Device session 08:50-09:15 (Dale; 7ee31d1 deployed with wget, checksum-checked)

| # | Check | Result |
|---|---|---|
| 1-4 | Apps sorted without Music/Video/File Manager, statuses; Engineering hold 3 s; full-screen Loading ROM / App; 19 colours, a VGA colour applies and survives restart | PASS ("A - PASS", 08:58). Dale's colour now 15 = VGA Light Red |
| 5 | CPU Max one step down and back | PASS (log: `480000-1512000 -> 480000-1416000 ok`, then back) |
| 6 | A on cpu1 offline, A again online | PASS on the second try (09:06: `cpu1.online 1 -> 0 ok`, `online` = 0). First try A landed on another row: Down skips cpu0 by design, Dale expected a highlight on cpu0 ("cpu0 does not ever highlight") |
| 7 | X with cpu2 offline, restart: still offline; Y, restart: all online | PASS |
| 8 | B while editing cancels, stays in Engineering | PASS |
| 9 | GPU, Battery time, Storage, Logs tabs | PASS |
| 10 | Heat guard: one process in the menu and in a game; `redux.log` watching line | PASS: `pidof` 2489 both times; `[heatguard] watching /sys/class/thermal/thermal_zone0/temp: cap at 75 C for 30 s, power off at 90 C for 10 s` (one line per boot) |
| 11 | LED/rumble read | `work_led`, `lowpwr_led`, `led_test`, `moto`, `workled_sleep` read **empty** (write-only); `constant_charge_current` = 1153. Stock `dmenu_ln` writes `work_led` only from `/mnt/mod/ctrl/configs/led_exit.cfg` / `led_launch.cfg`, which **don't exist** on Dale's unit. `moto` appears only inside emulator binaries |

Also seen: by hand, `echo 0 > /sys/devices/system/cpu/cpu1/online` reads 0 after 1 s (no automatic
hotplug brings it back); `/sys/devices/system/cpu/hotplug` exists.

Not run: the heat guard's cap/uncap (needs 75 °C; Dale's 10 min peak was 64.9 °C), the 30 min exit-test
run, the two-failed-starts fallback, Apps return to the sorted row, dark VGA accents' readability.

## Follow-ups
- CPU tab: label cpu0 "(always on)" so it is clear why the cursor skips it.
- LED and rumble: no stock values to copy; a by-hand write test (`work_led`, `moto`) goes on a later sheet
  before step 5's LED/rumble test is built.
