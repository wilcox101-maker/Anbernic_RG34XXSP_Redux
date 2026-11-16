---
from: dale
to: all
event: phase-gate
phase: F4
status: closed (by dale, 2026-11-16)
refs: [notes/2026-11-16-dale-hw-result-f4.md, notes/2026-11-16-dale-decision-f4-plan.md, issue #185, PR #209, PR #210, PR #213, PR #215, PR #216]
---

# F4 signed off

Dale signed off F4 on 2026-11-16 at 15:09 (recorded by Claude from Dale's chat answer "A" to
"SIGN F4 now and move Wi-Fi off to F6"). Device-checked on dev `6d87953`
([hw-result](2026-11-16-dale-hw-result-f4.md)): lid in the menu (#209), governor restore (#210), menu
dimming (#213), lid watcher beside RetroArch (#215), the cache crash fix (#216), and the exit test:
50 lid cycles mid-game, no crash, governor `schedutil` after.

- The "no lost save" part of the exit test: in-game saves and save states work; save-before-sleep
  timing waived by Dale (14:59).
- Moved to F6: Wi-Fi off unless needed (from the 2026-11-10 power decision), a Power & Lid settings
  page for the dim/lid defaults.
- Not run: lid during Restart/Power Off from the Redux menu, ROM copied from the PC after restart,
  start-up time with the cache.

Next phase: **F5 Engineering Mode** (inside the stock OPP tables; Expert Mode is not part of Beta).
