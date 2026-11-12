---
from: dale
to: all
event: hw-result
phase: F3
status: open
refs: [PR #73, PR #74, PR #78, PR #81, PR #82, PR #83]
---

# Build 0afecd9 on Dale's unit (2 GB original board), 2026-11-12

Recorded by Claude from Dale's chat reports and photos.

| Check | Result |
|---|---|
| Title bar | `v0.1-dev - 0afecd9` (main) |
| Settings without Network (#78) | pass |
| UI → Colors (#81, #82): Amber applies, survives relaunch **and reboot** | pass |
| System Tools (#74) | pass: battery "unknown (no sensor)", schedutil, 480 MHz idle, 480–1512 MHz, CPU 37.7 C, GPU 37.6 C, RAM 1979 MB, all 5 mounts. VE and DDR showed `--`, fixed in #83 |
| BIOS system (Saturn) | pass: the RetroArch BIOS path works, which closes the `system_directory` question |
| Wi-Fi connect (#73) | **fail**, see below |

## Wi-Fi failure
Picking `DLN_Guest` showed "Connected", "IP: none", and the unit stayed on `DLN_TeamWireless`. Over SSH:
- `nmcli dev wifi connect "DLN_Guest" ifname wlan0` printed (zh locale) "Error: Connection activation
  failed: (7) Secrets were required, but not provided." and **exited 0**.
- Afterwards wlan0 was `disconnected`, and NM later went back to the previous network on its own.
- `nmcli -t -f IP4.ADDRESS device show wlan0` works: `IP4.ADDRESS[1]:172.16.3.178/24`, rc 0.
- Thermal zone types: `cpu_thermal_zone`, `gpu_thermal_zone`, `ve_thermal_zone`, `ddr_thermal_zone`, `axp2202-battery`.

So `wifi_connect()` can't trust nmcli's exit status: it must check the error text and verify wlan0's
connection afterwards. Redux showed `DLN_Guest` as open (A connected with no keyboard), but nmcli asked
for a secret. Why is still open: the scan's SECURITY field, or a saved profile.
Dale also reported that Start didn't connect; which screen he was on isn't known yet.
