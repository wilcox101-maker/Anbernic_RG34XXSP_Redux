---
type: hw-result
from: dale (recorded by claude)
to: all
date: 2026-11-17
re: Engineering > Update install + run.sh fallback on ec758ae / ffd8076 (#250-#253, #259, #264)
status: open
---

# Device session 11:35-12:22 (Dale; builds installed from the Update tab)

From Dale's `redux.log` and `builds/history` (12:22):

| # | Check | Result |
|---|---|---|
| 1 | Update tab installs a newer build | PASS: `ec758ae 11:35 update`, `ffd8076 12:09 update`, both reached the menu |
| 2 | History lists the builds | PASS: 1038c32, ec758ae, ffd8076 kept in `builds/` |
| 7 | run.sh fallback after 3 quick failures | PASS: three `exit 1` starts 30 s apart (12:20:04-12:21:04), then "back to the last known working build ffd8076", next start ran ffd8076 (history `autoback`, menu reached) |

Notes:
- The first test-7 command failed harmlessly: writing over a running `dmenu.bin` gives "Text file busy" and the `&&` chain stopped. Corrected command writes `dmenu.bin.bad` and renames it.
- Dale saw the Redux boot screen for about 90 s during the fallback: the stock launcher waits about 30 s between restarts, so 3 failures take about 1.5 minutes with nothing on screen.
- Not yet reported: 3 (roll back from the history), 4-6 (Settings > UI colours and sizes).
