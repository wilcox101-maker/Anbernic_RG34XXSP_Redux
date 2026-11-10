---
from: dale
to: grok
event: hw-result
phase: F2
status: open
refs: [docs/hardware-map.md, frontend/src/remap.c]
---

# Analog stick event codes (evtest on ANBERNIC-keys)

Captured on the device. Device: `/dev/input/event1` (`ANBERNIC-keys`).

**Stick axes** (range −4096 … +4096, fuzz/flat 32):

| Axis | Code | Stick | Notes |
|------|------|-------|-------|
| ABS_Z | 2 | Left X | left = −, right = + |
| ABS_RX | 3 | Left Y | up = −, down = + |
| ABS_RY | 4 | Right X | |
| ABS_RZ | 5 | Right Y | |

**Digital D-pad**
- ABS_HAT0X (16): −1 left, +1 right
- ABS_HAT0Y (17): −1 up, +1 down

**Buttons** (scrambled codes, as expected from the DTB):
- BTN_SOUTH (304) = A
- BTN_EAST (305) = B
- BTN_C (306) = X
- BTN_NORTH (307) = Y
- BTN_WEST (308) = L1
- BTN_Z (309) = R1
- BTN_TL (310) = L2
- BTN_TR (311) = R2
- BTN_SELECT (314) = Select
- BTN_START (315) = Start
- BTN_TL2 (312) = Menu
- BTN_TR2 (313) / BTN_MODE (316) = L3/R3 (family)

Use these for the device remap table. Desktop arrow keys / WASD remain the desktop source.
