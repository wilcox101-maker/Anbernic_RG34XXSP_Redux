---
from: dale
to: all
event: phase-gate
phase: F5
status: closed (by dale, 2026-11-17)
refs: [notes/2026-11-16-claude-decision-f5-plan.md, notes/2026-11-16-claude-decision-f5-heat-guard.md, notes/2026-11-16-dale-hw-result-f5-steps-1-2.md, notes/2026-11-17-dale-hw-result-7ee31d1.md, notes/2026-11-17-dale-hw-result-2917a5c.md, notes/2026-11-17-dale-hw-result-070427e.md, issue #185, PR #221, PR #224, PR #227, PR #228, PR #229, PR #239, PR #240, PR #242]
---

# F5 signed off

Dale signed off F5 on 2026-11-17 at 10:28 (recorded by Claude from Dale's answer "Choice A" to
"A: sign F5 now"). Device-checked on `d334375`, `7ee31d1`, `2917a5c` and `070427e`: live read-outs on
every tab (CPU, GPU, Battery, Thermal, Display, Input, Storage, Logs, System), the safety net (validated
and logged writes, saved profile, two-failed-starts fallback), CPU controls inside the chip's own tables,
the heat guard running, the LED/rumble test, `fsck -n` check, and Input tab.

- **Not verified on the device:** the heat guard's cap (never reached 75 °C) and the 30 min full-load run;
  both moved to the Beta exit test (Dale, 2026-11-17 09:58).
- Follow-up agreed with the sign-off: live analog stick values on the Input tab (small PR).
- Next: STATUS item 5, Engineering > Update (repo build pulls, current, last known working, history);
  security item, Dale merges.
