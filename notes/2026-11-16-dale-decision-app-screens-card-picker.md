---
from: dale
to: all
event: decision
phase: F6
status: open
refs: [notes/2026-11-16-dale-hw-result-185.md, docs/ui-brief.md]
---

# Dale decisions, 2026-11-16: card picker; Redux screens for the apps; DOS layout on record

From the owner chat after the #185 device session:
- 12:43, on games listed twice when on both cards: **"Card selection is preferred like stock OS
  offers."** Game Room gets a TF1/TF2 picker like stock (shown when both cards have games); each list
  holds one card's games.
- 12:43, "Are we stuck with stock OS app screens?" Claude: no; Redux front-ends can replace them,
  with the stock engines underneath. 12:46: **"I want screens in the style of the BIOS and DOS layout
  inspiration."** Video Player first (Redux file list + stock `ffplay`), then Music, Ebook, File Manager.
- 12:46: **"Adding a DOS menu layout request should be in doc history."** Recorded in
  `docs/ui-brief.md` §3 (DOS Mode requested 2026-11-12; Dale defines the layout).

Open until the card picker and the first app front-end ship.
