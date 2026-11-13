---
from: dale
to: all
event: decision
phase: F6
status: open
refs: [ROADMAP.md, docs/ui-brief.md]
---

# Dale decision, 2026-11-13: App Center is a local launcher (option A)

Recorded by Claude from Dale's chat reply "Option A".

## Options that were put to Dale

- **A. Launcher for what is already on the device (chosen).** Lists stock apps and Redux's own
  tools (Terminal, later the chiptune composer and the ASCII art maker) and launches them the same
  way games launch. Never downloads anything.
- B. A plus downloading apps over Wi-Fi. Rejected for now: it goes through the update path, a
  security item, and is much more work.
- C. Skip it for now and do Terminal/htop first.

## What this means for the build

- Read only: the App Center lists and launches. It does not install, download, copy or delete apps.
- Launch goes through the existing launcher path (`/tmp/.next`), like games.
- Which stock apps exist and how each one launches must be taken from the stock inventory
  (`docs/stock-userspace.md`, appfs `bin/`), not guessed. Anything not confirmed there is left out
  or marked "not verified" until Dale runs it.
- Downloads (option B) stay in the backlog as a Dale-gated security item.
