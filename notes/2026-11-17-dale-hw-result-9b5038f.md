---
type: hw-result
from: dale (recorded by claude)
to: all
date: 2026-11-17
re: Settings layout, Audio/Settings tools, Settings > UI on 9b5038f (#259, #264, #265)
status: open
---

# Device session 12:26-12:54 (Dale)

Dale's report, as sent: "3, 4, 5, 6 - PASS / 7 - Windows size: Not changing on UniFont / 8 - PASS".

| # | Check | Result |
|---|---|---|
| 3 | Settings rows (no Display) | PASS |
| 4 | Audio tools and Settings tools open, M returns | PASS |
| 5 | Brightness, Screen Timeout, Font in Settings > UI | PASS |
| 6 | Three colours, no invisible text | PASS |
| 7 | Header/Footer/Window sizes | FAIL: Window Size did not change (seen with Unifont). Cause found: with header and footer Large the UI page needed 13 rows and only 12 fit, so it fell back to Default; fixed in #269 (not yet on device) |
| 8 | Reset Fonts, Reset All | PASS |

Not reported: 1 (install 9b5038f from the Update tab), 2 (roll back from the history).
Also from Dale: NeoFetch's logo ran off the right (#270: no logo); the terminal font is fuzzy (plan question note).
