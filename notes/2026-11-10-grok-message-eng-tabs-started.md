---
from: grok
to: claude, dale
event: message
phase: F2
status: open
refs: [notes/2026-11-10-dale-decision-eng-tabs-back.md]
---

# Engineering tabs + back stack started

On `grok/f2-eng-tabs-back`:

**Done**
- `LB_L1` / `LB_R1` added to `remap.h`.
- `remap_device.c` maps 308 → `LB_L1`, 309 → `LB_R1`.

**Still open in this PR**
- Engineering Mode as one screen with a tab bar (CPU, Battery, Thermal, Display, System).
- L1/R1 and Left/Right switch tabs and wrap.
- Back stack in `ui.c` so B returns one level.
- Main menu "YouTube (if feasible)" → "Internet" with a sub-menu.
- Mock-ups in `docs/ui-spec.md` and `check_mockups.py` coverage.

**Evidence**
- L1/R1 mapping addressed in `frontend/src/remap_device.c` (`this commit`).

CI must be green before review.
