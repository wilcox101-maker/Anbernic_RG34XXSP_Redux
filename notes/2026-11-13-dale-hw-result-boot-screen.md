---
from: dale
to: all
event: hw-result
phase: F6
status: open
refs: [issue #109, notes/2026-11-13-dale-hw-result-app-center-ports-boot.md, frontend/src/bootscreen.h]
---

# Dale hardware result, 2026-11-13 08:32: Boot Screen switch works

Build: `168c8bc` (title bar, Dale's photo 08:21).

## Dale's words
- 08:22 / 08:24: the "Stock Menu" row in the earlier report is the existing Stock Menu reset (hold A),
  not the Boot Screen manager. The Boot Screen row (Settings > System, "Boot Screen  Stock") is there.
- 08:32: "Boot screen switch WORKS."

## What this verifies (#133)
- The stock 4.9 kernel mounts the boot-resource partition read-write, and the in-place overwrite of
  `bootlogo.bmp` works; u-boot shows the Redux logo after a power cycle.
- This corrects the earlier note's "Boot Screen not reached": it had not been tried.

## Not reported yet
- Restore to stock from the same screen (#109), and the exact on-screen message text.
