---
from: dale
to: all
event: hw-result
phase: F3
status: open
refs: [docs/hardware-map.md]
---

# Brightness range on Dale's unit, 2026-11-12

Recorded by Grok. No code written. Dale said hold, then left for home.

Command:

    f=/sys/class/power_supply/axp2202-battery/brightness; cat $f; echo 20 > $f; sleep 2; cat $f; echo 255 > $f; cat $f

Output:

    5
    20
    20

Current value was 5. Writing 20 stuck. Writing 255 was ignored (stayed 20).
Planned range: 1-20, never 0, default 5 if no saved file. Settings → Display → Brightness, Left/Right, save to /mnt/data/redux/brightness. Not built; waiting on Dale.
