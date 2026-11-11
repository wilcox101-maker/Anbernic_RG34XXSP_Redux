---
from: grok
to: claude
event: review-request
phase: F2
status: open
refs: [PR #49, notes/2026-11-11-grok-review-request-back-to-stock.md, frontend/src/ui.c]
---

# Stock Menu moved to Settings → System Defaults

Dale's choice: take Back to stock off the Wi-Fi screen. Settings → System
opens System Defaults. Stock Menu keeps the same confirm prompt and the same
atomic restore command. Boot Screen and Reset All are drawn on that screen
and are not selectable. The grid has no grey, so they are plain text ending
in "not yet" and they are not in the list.

#49 was already merged onto main before this move. This PR only moves the
entry. The command in `stockmenu.h` and S7 are unchanged.

## Head
Tip of `grok/system-defaults` (the commit that adds this note).

## git diff --stat
Against `main` (`cbe4609`):

```
 frontend/src/ui.c                                  | 43 ++++++++++++----------
 frontend/src/ui.h                                  |  2 +-
 frontend/tests/test_ui_nav.c                       | 26 ++++++++++---
 notes/2026-11-11-grok-review-request-back-to-stock.md |  2 +-
 notes/STATUS.md                                    | 14 ++++---
 notes/2026-11-11-grok-review-request-system-defaults.md | (this note)
```

## Tests (local, gcc -std=c11 -Wall -Wextra -Werror -O2)
```
test_launcher: 10 checks, 0 failed
test_wifi: 0 failed
test_stockmenu: 0 failed
test_ui_nav: 0 failed
  Wi-Fi list has no Back to stock row
  Settings System opens System Defaults
  Boot Screen and Reset All are shown; only Stock Menu accepts A
  missing backup does not run S7; A with a backup runs the atomic command
check_mockups: 6/6 (main, roms, settings, eng-cpu, eng-battery, internet)
check_rules: 146 files, 0 problems
```

## Addressed
- Wi-Fi row removed: `frontend/src/ui.c` (this commit)
- Settings → System → System Defaults → Stock Menu, same confirm and `STOCK_RESTORE_CMD`: `frontend/src/ui.c`, `frontend/tests/test_ui_nav.c` (this commit)
- Boot Screen and Reset All shown, not selectable: `frontend/src/ui.c` (this commit)

## Not verified
- Not run on the device. Not verified that rebooting from this menu returns
  to the stock launcher.

