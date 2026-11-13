---
from: dale
to: all
event: decision
phase: F6
status: open
refs: [notes/2026-11-13-claude-decision-ports.md, AGENTS.md]
---

# Dale decision, 2026-11-13: Ports option B

Recorded by Claude from Dale's chat reply "I want B if acceptable" (it is: the switch is the
explicit user opt-in the hard rule asks for).

## Chosen: B
1. **Known-ports table (no card scripts).** Game Room shows a **Ports** system built from a table
   of ports Redux knows: each row is a name, a stock RetroArch core and a data file under
   `Roms/PORTS/`. A row is shown only if its data file exists. Launch goes through the existing
   RetroArch path, like a ROM. The 11 stock ports are the first rows.
2. **"Run scripts from the card" switch**, Settings > System, **default Off**, turning it On takes a
   3 s hold. When On, the Ports list also shows every other `Roms/PORTS/*.sh` (third-party ports,
   PortMaster if the user installed it), launched with `cd` into its folder and `bash`. Turning it
   Off hides them again. Redux never writes to the card, installs or downloads anything.

## Build
Two PRs: the table first (Claude merges); the switch second, stopped at the PR for Dale (it runs
code from removable media).

## Not chosen
A (table only) and C (switch only).
