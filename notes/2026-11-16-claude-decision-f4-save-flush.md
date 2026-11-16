---
type: decision
from: claude
to: dale
date: 2026-11-16
re: F4 step 4, flushing RetroArch saves before a lid sleep (AGENTS.md hard rule "never write to p1")
status: open
---

# Decision: may Redux ask RetroArch to write its saves before a lid sleep?

**Why it needs you:** RetroArch keeps its saves on the cards, including TF1 (p1)
([reads](2026-11-16-dale-hw-result-reads-f4-2.md)). With `autosave_interval = 0`, they reach the card
only when the game closes. `SAVE_FILES` (tested, works on 1.20.0) makes RetroArch write them now. Redux
writes nothing itself, but it would be triggering a write to p1 that stock only does at exit. The
hard-rule exception today names only OpenBOR, so this touches a hard rule: your call, and you merge
the rule change.

## Options
- **A (recommended):** extend the second p1 exception: RetroArch, launched as stock does, may write
  its own saves on the cards, and Redux may send it `SAVE_FILES` before a lid sleep. Redux still
  writes nothing to p1 itself. Covers the exit test ("no lost save") even if the battery dies asleep.
- **B:** no `SAVE_FILES`; before sleep Redux only runs `sync`. Nothing new written to p1, but
  in-game saves made since the game started are still only in RAM while asleep: a dead battery or a
  hard reset loses them.
- **C:** A, but off by default behind a Settings → Power & Lid switch ("Save before sleep").

Done when: your answer here, then the AGENTS.md change (A or C) in its own PR for you to merge, then
step 4 code.
