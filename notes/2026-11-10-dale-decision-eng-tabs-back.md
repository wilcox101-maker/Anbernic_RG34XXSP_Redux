---
type: decision
from: dale (recorded by claude)
to: grok, claude
date: 2026-11-10
re: Engineering Mode layout, B button (docs/ui-spec.md, frontend/src/ui.c)
status: open
---

# Decisions: BIOS tab bar in Engineering; B goes back one level

From Dale's on-device test: Engineering pages need BIOS-style tabs, and B on Battery went to the
main menu instead of the previous screen.

## 1. Engineering Mode is one screen with a tab bar (AMI/Phoenix style)
```
 [ENGINEERING]                              BAT 100% 15:36
┌──────────────────────────────────────────────────────┐
│ ▓CPU▓  Battery  Thermal  Display  System             │
│──────────────────────────────────────────────────────│
│  Governor:   schedutil                               │
│  ...                                                 │
└──────────────────────────────────────────────────────┘
 L1/R1:Tab  A:Edit  B:Exit  X:Save profile  Y:Reset
```
- Tabs: **CPU, Battery, Thermal, Display, System**. Only CPU and Battery have content now; the
  others say "Not yet" until their phase.
- **L1/R1 and Left/Right** switch tabs and wrap; the active tab is inverted in the accent colour.
- Needs L1/R1 as logical buttons: `remap_device.c` maps 308 (L1) and 309 (R1) to `LB_NONE` today.
  Add `LB_L1`/`LB_R1`, plus keyboard keys on desktop.

## 2. B goes back one level, everywhere
- A small back stack in `ui.c`: B returns to the screen you came from, not always Main.
- The tabs are one screen, so B on any Engineering tab exits to the main menu.
- Settings → a sub-page → B returns to Settings.

## For Grok
One PR, after the ROM list / Battery / CPU fixes and before or with the text-size change:
mock-ups in `docs/ui-spec.md`, `ui.c` (tabs + back stack), remap L1/R1, and `check_mockups.py`
covering the tab bar. Evidence per AGENTS.md.
