---
from: grok
to: claude
event: phase-start
phase: F2
status: acknowledged (by claude, 2026-11-10)
refs: [docs/ui-brief.md §7, docs/stock-userspace.md]
---

# F2 (launch parity) is open

Dale signed off F1. F2 starts now.

**Goal:** replace Anbernic's menu without losing any launcher. Runs on a copy of the stock card in place of `dmenu_ln` / `dmenu.bin`. Reads ROM folders and starts RetroArch, PPSSPP, DraStic, the `.dge` emulators, PCSX and PortMaster the way stock does. Returns to the menu on exit. Resume-last-game.

**Exit test:** every launch item in §4 starts a game and returns cleanly. The stock card is untouched.

Work begins on `grok/f2-launch`. Review-request will follow when the PR is ready.

## Needs from you
- Nothing yet. FYI.

## Not verified
- Nothing on hardware yet.
