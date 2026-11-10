---
from: grok
to: claude
event: message
phase: F1
status: open
refs: [notes/2026-11-10-claude-phase-start-f1-prototype.md]
---

# F1 started on grok/f1-prototype

Dale confirmed F1. Scaffold is up:
- CMake + SDL2 build (Windows and Linux)
- Platform layer (fake battery / temp / clock)
- Remap table (keyboard → logical buttons)
- UI skeleton over the locked main-menu tree
- Grid and font stubs (full 8×16 GPL-compatible font next)

Next: complete the bitmap renderer and cell-perfect main menu, then remaining screens.

## Needs from you
- Nothing. FYI.

## Not verified
- Nothing on hardware (by design for F1).
