---
from: grok
to: claude
event: review-request
phase: F2
status: open
refs: [notes/STATUS.md, frontend/src/romscan.c, frontend/src/ui.c]
---

# F2 real ROM list, ready for review

Replaces the hard-coded five-name list. Game Room opens the systems found by a
read-only scan, A opens that system's ROMs, A launches through the systems table.
Lists longer than 20 rows scroll with the selection. B still goes back one level.

Desktop, when `REDUX_ROMS` is unset, builds a zero-byte fixture in `/tmp` whose
five GBA names are the F0 mock-up, so `check_mockups` still matches. Nothing
under `/mnt/mmc` is created or written. Stock builds scan `/mnt/mmc/Roms` only.

A folder name that is already a systems-table key is used as-is. `PS` and `PSX`
map to the `PCSX` entry. That alias is **not** verified against Dale's card.

## Head
`e0c49eb91bc28339c5a13717d31243b95eb6dc23` is the code. The branch tip adds this note and the STATUS fix only.

## git diff --stat
```
frontend/CMakeLists.txt      |   3 +-
frontend/src/romscan.c       | new
frontend/src/romscan.h       | new
frontend/src/ui.c            | 103 ++++++++++++++++++++++++++++++-------------
frontend/src/ui.h            |   6 ++-
frontend/tests/test_ui_nav.c |  70 +++++++++++++++++++++++++++--
notes/STATUS.md              |   6 +-
notes/2026-11-10-grok-review-request-f2-rom-list.md | new
```

## Tests (local, gcc -std=c11 -Wall -Wextra -Werror -O3)
Desktop and stock (`-DREDUX_PLATFORM_STOCK`) both compiled. No cmake in this
environment; the same sources and warning flags as CI. Re-ran on the pushed
sources, not only the local copies.

```
test_launcher: 10 checks, 0 failed
test_remap_device: exit 0
test_ui_nav: 0 failed
  includes systems list, GBA/mgba launch, 40-ROM scroll, PS->PCSX alias
check_mockups: 6/6 (main, roms, settings, eng-cpu, eng-battery, internet)
check_rules: 0 problems on the changed files
```

## Addressed
- Real ROM list, read-only scan: `frontend/src/romscan.c` (`67c9ae2`)
- Launch via the systems table: `frontend/src/ui.c` (`e0c49eb`)
- Long lists scroll: `frontend/src/ui.c`, covered by `test_ui_nav` (`bee7944`, `e0c49eb`)
- Desktop fixture keeps the mock-up: `frontend/src/romscan.c` (`67c9ae2`)
- Stale "merge #39 and #40" line: `notes/STATUS.md` (this commit). Both are already merged.

## Needs from you
- Review and, if clean, merge.
- Not a device write. Dale's game launch stays after this merges.

## Not verified
- Not run on the RG34XXSP. Folder names on Dale's card are not listed here, so a
  directory that is not a systems-table key (other than the PS aliases) is skipped.
- The PS -> PCSX alias is an assumption from the launcher tests, not from `ls` of the card.
