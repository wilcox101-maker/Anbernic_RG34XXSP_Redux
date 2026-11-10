---
from: claude
to: grok
event: phase-start
phase: F1
status: open
refs: [docs/ui-brief.md §7, docs/ui-spec.md, docs/hardware-map.md, notes/2026-11-10-dale-phase-gate-f0-signed-off.md]
---

# F1 (desktop prototype) is open. Frontend is yours

F0 is signed off. Per `docs/ui-brief.md` §7, F1 proves the look and the navigation on a PC.

**Deliverables**
- C + SDL2, one binary: bitmap-font grid renderer (8×16, 90 × 30 at 720×480), the three-colour theme
  with the accent configurable, menu navigation over the locked tree with fake data.
- Input only through a remap table (keyboard → logical buttons on desktop; the device table comes
  later). No hard-coded key codes in UI code.
- A small `platform` layer from day one (brief §7 "hardware-layer rule"): even on desktop, battery,
  temps and clock come through it, with fake values, so F2/F7 only swap that layer.
- The font must be GPL-compatible. Name it and its licence in the PR.
- Builds on Windows (Dale's PC) as well as Linux, e.g. CMake + SDL2, with the steps in a README.

**Exit test** (from the brief)
- Every F0 mock-up is reproduced cell-for-cell (a screenshot per screen in the PR).
- Each input redraws within one frame.
- Controls work through the remap table only.

## Needs from you
- Work on `grok/f1-prototype`, open a PR, then leave a `review-request` to claude and a `phase-gate`
  to dale when the exit test passes.
- Code goes under a new top-level folder (suggest `frontend/`). Say in the PR what you couldn't run.

## Not verified
- Nothing on hardware in F1 by design.
