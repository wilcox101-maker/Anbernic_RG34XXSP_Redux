---
type: decision
from: claude
to: dale
date: 2026-11-16
re: F5 step 4 heat guard limits (Q1 A in notes/2026-11-16-claude-decision-f5-plan.md); data from the 10 min heat run
status: open (answered A 16:12; closes when step 4 is device-checked)
---

# Decision: heat-guard limits on the SoC temperature

**Why it needs you:** Q1 A says the guard uses the SoC temperature and you choose the limits after a
measurement. Data: [hw-result](2026-11-16-dale-hw-result-f5-steps-1-2.md), 10 min full CPU load on the
charger: CPU zone 53.5 → 64.9 °C, still rising about 1 °C/min. Stock DTS trips on the CPU zone: passive
60 °C and 70 °C (stock throttles to hold about 70), critical 115 °C (never changed).

Constraints found: no battery sensor, no charge on/off switch (only `constant_charge_current`, untested),
so "stop charging" is not available without a risky PMIC write.

## Options
- **A (recommended): warn, cap, then shut down.** Always on, in the menu and beside games (a small watcher,
  like the lid watcher). CPU zone **≥ 75 °C for 30 s** (stock's throttling failed to hold 70): warning in
  `redux.log` and on screen in the menu, CPU max capped to **1200 MHz** (in the chip's list) until it is
  **below 65 °C**, then the previous max comes back. **≥ 90 °C for 10 s**: RetroArch `SAVE_FILES` if a game
  runs, `sync`, power off. Charging is not touched. Limits live in one header; revisited after the 30 min
  exit test.
- **B: warn only.** Same 75 °C warning and log line; no cap, no shutdown. Stock's trips do the rest.
- **C: build the guard, pick numbers after the exit test.** Code and tests now with A's numbers as
  placeholders; the guard stays log-only until you choose after the 30 min run.

## Answer
Dale, 16:12: **A.** Warn and cap CPU max at 1200 MHz at ≥ 75 °C for 30 s (back below 65 °C);
save, sync and power off at ≥ 90 °C for 10 s; in the menu and beside games; charging untouched. Built in
F5 step 4; limits revisited after the 30 min exit test.
