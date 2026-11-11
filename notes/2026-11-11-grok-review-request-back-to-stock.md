---
from: grok
to: dale
event: review-request
phase: F2
status: open
refs: [PR inbox #41, docs/dale-scripts.md, frontend/src/stockmenu.c]
---

# DALE-ONLY: Back to stock menu. Do not merge.

Inbox #41: restore `dmenu_ln` from `dmenu_ln.bak` behind a confirmation prompt.
This writes `/mnt/vendor/ctrl/dmenu_ln`, so it stays a draft until Dale runs it.

The row is the last line of the Wi-Fi screen (Main → Network), so the locked
main-menu mock-up does not change and the row is still there when the scan
finds nothing. A opens a confirm screen. A again runs dale-scripts S7:

```
cp /mnt/vendor/ctrl/dmenu_ln.bak /mnt/vendor/ctrl/dmenu_ln && sync && reboot
```

S7 copies, so the `.bak` stays. `deploy_frontend.py --restore` moves the bak
instead; this menu follows S7, and it refuses when the bak is missing, same
refusal as that script. A desktop build never execs it. B cancels.

Stacked on `grok/wifi-setup`. Retarget this PR to `main` after the Wi-Fi PR merges.

## Head
`f3e096ab0e95c69ae0b685542c4df1e4e3636cc6` is the code. The branch tip adds this note only.

## git diff --stat
Against the Wi-Fi branch (`3f80039..f3e096a`), code commit only:

```
 .github/workflows/ci.yml        |  3 +++
 frontend/CMakeLists.txt         |  6 ++++-
 frontend/src/stockmenu.c        | 33 +++++++++++++++++++++++
 frontend/src/stockmenu.h        | 28 +++++++++++++++++++
 frontend/src/ui.c               | 59 +++++++++++++++++++++++++++++++++++------
 frontend/src/ui.h               |  3 ++-
 frontend/tests/test_stockmenu.c | 33 +++++++++++++++++++++++
 frontend/tests/test_ui_nav.c    | 30 +++++++++++++++++++++
 notes/STATUS.md                 |  8 +++---
 9 files changed, 189 insertions(+), 14 deletions(-)
```

## Tests (local, gcc -std=c11 -Wall -Wextra -Werror -O3)
```
test_stockmenu: 0 failed
  command is dale-scripts S7
  missing dmenu_ln.bak is refused and does not run
test_ui_nav: 0 failed
  last Wi-Fi row opens the confirm; B does not run S7
  missing backup does not run S7; A with a backup runs it
check_mockups: six screens still match
check_rules: 0 problems on the touched files
```

## Addressed
- Confirm, then S7: `frontend/src/ui.c`, `frontend/src/stockmenu.c` (`f3e096a`)
- Refusal when the bak is missing: `frontend/tests/test_stockmenu.c` (`f3e096a`)

## Not verified
- Not run on the device. Not verified that rebooting from inside the menu
  returns to the stock launcher. S7 is the command Dale already uses over SSH.

## Needs from you
- Dale: do not merge until you have tried it on the device, with `dmenu_ln.bak` present.
