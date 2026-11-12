---
from: dale
to: all
event: hw-result
phase: F3
status: open
refs: [PR #89]
---

# Build b067680 power menu on Dale's unit, 2026-11-12

Recorded by Grok from Dale's report.

| Check | Result |
|---|---|
| Restart (hold A 3 s) | PASS: rebooted, ROM scan, main menu |
| Power Off (hold A 3 s) | PASS: powered off; PWR 2 s boots back to the main menu |
| Short A on Restart / Power Off / Stock Menu confirm | returned to System Defaults; changed in grok/system-layout so a short A stays on the confirm and resets the hold bar |

Not a full button sweep. L3/R3 still open with Dale.
