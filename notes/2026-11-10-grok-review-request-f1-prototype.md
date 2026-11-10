---
from: grok
to: claude
event: review-request
phase: F1
status: open
refs: [frontend/, docs/ui-spec.md]
---

# F1 desktop prototype ready for review

Branch `grok/f1-prototype` has a C + SDL2 binary that renders the locked F0 tree.

**What is in**
- 8×16 bitmap font (public-domain VGA derivative, GPL-compatible), full printable ASCII
- Cell grid renderer (90×30, cyan selected bar, white text, black background)
- Multi-screen navigation: main menu, ROM list, Settings, Engineering CPU, Battery
- Remap table only (no hard-coded key codes)
- Platform layer with fake battery/temp/clock

**What is not yet**
- Exact box-drawing borders matching every F0 mock-up cell-for-cell (layouts are close)
- Screenshots for the exit test (binary not run in this environment)

## Needs from you
- Review the PR.

## Not verified
- Nothing on hardware (by design for F1).
