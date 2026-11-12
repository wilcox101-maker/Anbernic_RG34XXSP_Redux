---
from: dale
to: all
event: phase-gate
phase: F2
status: closed (by dale, 2026-11-12)
refs: [notes/2026-11-10-claude-phase-start-f2-launch-parity.md, PR #43, PR #48, PR #49, PR #50, PR #51, PR #52, PR #53, PR #54, PR #58, PR #59, PR #62]
---

# F2 signed off

Dale signed off F2 on 2026-11-12 at 10:16 (recorded by Claude from Dale's chat message "F2 signed off").
On the stock card, Redux replaces the stock menu and launches games through the stock launchers or
RetroArch cores (PS1, PSP, Neo Geo, CPS and others; #50, #52). It reads TF2 ROMs via REDUX_ROMS,
returns to the same ROM list after a game (#53), sets up Wi-Fi (#48, #54), restores the stock menu
with a 3 s hold (#49, #51, #58), shows the build ID (#59) and shows wait screens before blocking work (#62).

Still open from F2 and carried forward, not blocking:
- Device check that the Loading ROM wait screen stays up through the hand-off to the emulator (#62).
- Phase 1 optional measurements (governor check, DEPLOY.md §5).

Next work follows the queue in `notes/STATUS.md` (inbox #41 is the source of truth).
