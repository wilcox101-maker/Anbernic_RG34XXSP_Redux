---
from: dale
to: all
event: decision
phase: F6
status: open
refs: [notes/2026-11-13-dale-decision-app-center.md, notes/2026-11-13-claude-message-stock-apps.md, docs/ui-spec.md]
---

# Dale decision, 2026-11-13: where the App Center sits (option A)

Recorded by Claude from Dale's chat reply "A".

## Options that were put to Dale

- **A. Chosen.** Music, Video, Documents and File Manager stay in the main menu and launch the
  matching stock app directly. A new top-level **Apps** entry lists every app, stock and Redux's
  own (Terminal, later the chiptune composer and ASCII art maker). Apps not yet run on the device
  are marked "not verified".
- B. One Apps entry for everything; the four entries removed. Rejected: music and video one click deeper.
- C. Keep the four entries; Apps holds only Redux tools. Rejected: the ebook readers have no home.

## What this means for the build

- Stock apps and launch commands come from [the stock-apps note](2026-11-13-claude-message-stock-apps.md).
  Documents opens the ebook reader for the card in use; the PDF reader is listed under Apps.
- `.dge` apps launch with a `cd` into their folder first (#115).
- Nothing from `/mnt/mmc/EXE` is listed: running code from removable media needs a separate opt-in.
