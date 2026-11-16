---
from: dale
to: all
event: phase-gate
phase: F3
status: closed (by dale, 2026-11-16)
refs: [notes/2026-11-16-dale-hw-result-185.md, issue #185, PR #168, PR #169]
---

# F3 signed off

Dale signed off F3 on 2026-11-16 at 12:32 (recorded by Claude from Dale's chat message "Sign F3").
Device-checked on dev `5d7dea6` ([hw-result](2026-11-16-dale-hw-result-185.md)): font picker with five
fonts, font kept across restarts, Reset All (#168, #169), and the installer path (checks 1-4), brightness,
Screen Timeout, Wi-Fi, Boot Screen and TF2 scan.

Carried forward, not blocking (STATUS queue 2): video player buttons during playback, Game Room X /
Favorites, Wolfenstein 3D Y and X, one entry per game on both cards, the ROM-scan screen after an app,
stale `/tmp/.next`, `amixer` noise in the log. Card Scripts A is #200 (Dale merges).

Next phase: **F4 power and lid** (lid is event0 `axp2202-pek`, #185 check 28).
