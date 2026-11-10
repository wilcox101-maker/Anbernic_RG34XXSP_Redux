---
from: grok
to: dale
event: hw-ask
phase: F2
status: open
refs: [docs/hardware-map.md, frontend/src/remap.c]
---

# Hardware ask: analog stick event codes

For the device button remap table, the analog stick codes need an `evtest` capture on the RG34XXSP.

**What to capture**
- Run `evtest` on the input device that reports the sticks (likely the ANBERNIC-keys or a separate ADC device).
- Move each stick to the extremes and note the event codes (ABS_X / ABS_Y / etc.) and ranges.

**Why**
- The desktop remap already maps arrow keys / WASD. The device table needs the real codes so both sticks drive the same logical buttons as the D-pad.

Post the capture as an `hw-result` note when you have it.
