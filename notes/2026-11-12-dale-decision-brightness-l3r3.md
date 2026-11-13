---
from: dale
to: grok
event: decision
phase: F3
status: open
refs: [notes/2026-11-12-dale-hw-result-brightness-range.md, docs/hardware-map.md]
---

# Dale decisions, 2026-11-12 evening: brightness go, L3/R3 stay unmapped

Recorded by Claude from Dale's chat reply ("1. Confirmed ... 3. Leave mappable at this time").

## 1. Brightness: proceed (option A)

Grok's plan from the [brightness-range hw-result](2026-11-12-dale-hw-result-brightness-range.md), with one addition:

- Attribute: `/sys/class/power_supply/axp2202-battery/brightness`.
- Range **1–20, never 0**. Default 5 when no saved file. Settings → Display → Brightness, Left/Right,
  saved to `/mnt/data/redux/brightness`, re-applied at startup.
- **Addition:** the min, max and default live in config (one place, not literals scattered in the
  code), so a later builder can widen the range once the real upper limit is known. Writing 20
  stuck and 255 was ignored, so the true maximum is somewhere in between and is not yet measured.
  Do not probe above 20 in code.
- Same working style as volume: CHECKPOINT and "proceed?" to Dale at each step. Not verified on the
  device until Dale tests it.

## 2. L3 / R3 (codes 313 / 316): leave unmapped

They stay unmapped for now and remain available for mapping later (a future button-remap setting or
feature). Don't assign them a function without a new decision from Dale.
