---
type: decision
from: dale
to: all
date: 2026-11-14
re: tearing in stock SimpleTerminal and File Manager (notes/2026-11-14-claude-decision-vsync-stock-apps.md)
status: closed
---

# Dale decision: a vsync library for the stock apps that tear (option B)

Dale, 2026-11-14 08:34 ET, in the owning chat: **"B"**.

## What it means
- Redux preloads a small library (`frontend/vsync/redux_vsync.c`) only when it launches SimpleTerminal
  (Terminal, Task Monitor and the five Tools apps) and File Manager. It waits for the next vertical
  blank (`FBIO_WAITFORVSYNC` on `/dev/fb0`) before each frame those apps show.
- The library is built into `dmenu.bin` and written to `/tmp/redux-vsync.so` (RAM) before each such
  launch, so deploying stays one file. If it can't be written, the app starts without it.
- If the framebuffer can't wait for vblank, the library switches itself off on the first frame and
  every call passes straight through: the apps behave exactly as before. It prints `[vsync] on` or
  `[vsync] off: <reason>` once.
- Fallback stays **C**: if it doesn't help or adds lag on the device, remove the preload and close the item.

## Not verified on device
- Whether `/dev/fb0` answers `FBIO_WAITFORVSYNC` (decides on/off).
- Whether the tearing is gone, and whether typing in SimpleTerminal feels slower.
- Whether stock SDL calls the wrapped functions at all (if it draws another way, nothing changes).
