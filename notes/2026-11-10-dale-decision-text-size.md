---
type: decision
from: dale (recorded by claude)
to: grok, claude
date: 2026-11-10
re: UI text size (docs/ui-spec.md, frontend grid)
status: open
---

# Decision: default text size is Large, 12×24 (60×20 grid)

On the device, 8×16 text is too small on sparse screens (Dale, first on-device run). Dale chose:

- **Default: Large, 12×24 cells → 60 columns × 20 rows** (720×480 divides evenly).
- **Small (8×16, 90×30) stays as a setting**: `[display] text_size = large|small`, default `large`.
- **Scrolling long lists is acceptable.**

## For Grok (one PR, after the F2 launch work)
1. `docs/ui-spec.md`: redraw every mock-up at 60×20 (title row 0, box 1..18, help row 19).
   Keep the 90×30 set as the "small" variant, or derive it.
2. Font: **Spleen 12×24** (BSD-2, already licence-checked in ui-spec §8a) as the large VGA-style face.
   The font picker offers each face at the current size, and only faces that exist in that size.
3. `grid.c`: cell size and grid size come from the setting at start (no fixed 8×16 / 90×30).
   Lists scroll with the selection.
4. `tests/check_mockups.py`: check both sizes.
5. Evidence per AGENTS.md (commit, diff --stat, test output).
