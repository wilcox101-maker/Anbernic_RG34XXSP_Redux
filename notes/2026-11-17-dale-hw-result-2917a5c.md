---
type: hw-result
from: dale (recorded by claude)
to: all
date: 2026-11-17
re: device session on 2917a5c (F5 step 5b #239, cpu0 label, failed-start fallback, LED/rumble by hand); #185
status: open
---

# Device session 09:40-09:58 (Dale; 2917a5c deployed with wget, checksum-checked)

| # | Check | Result |
|---|---|---|
| 1 | Input tab | Works, but **R1, B and Menu stay lit** after use (they leave the tab before release), and **L3/R3 missing** (also L2/R2). Fixed in #240, not yet re-checked |
| 2 | Storage A: `fsck -n` | `/`, `/mnt/vendor`, `/mnt/data`: exit 0 (clean); `/mnt/mmc`, `/mnt/sdcard` (FAT): **exit 1**. #240 labels exit 1 "issues reported (nothing changed)"; cause (likely the FAT in-use flag while mounted) not yet confirmed from `/tmp/redux-fsck.txt` |
| 3 | cpu0 "online (always on)" | PASS |
| 4 | Two-failed-starts fallback (counter set to 2 by hand, reboot) | PASS |
| 5 | `work_led` 1, 2 s, 0 | The green power LED **went off and stayed off** until USB-C was plugged/unplugged: `work_led` 0 = power LED off, 1 = on |
| 6 | `lowpwr_led` 1, 2 s, 0 | Power LED **red** for 2 s, then solid green again |
| 7 | `moto` 1, 2 s, 0 | **Rumbles** for 2 s |
| 8 | F5 exit test, 30 min full load on the charger | **Suspended by Dale until the final Beta release** (09:58) |

## Follow-ups
- LED/rumble test in Engineering: always end with `work_led` 1 (never leave the power LED off).
- F5 exit test: moved to the Beta exit run (Dale 09:58). The heat guard cap has still not been triggered on the device.
