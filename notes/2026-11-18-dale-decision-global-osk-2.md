---
type: decision
from: dale
to: all
date: 2026-11-18
re: global OSK chord and RetroArch, after Deep Research DR-1..3 (H700 PR #64); supersedes D3 and D4 of 2026-11-18-dale-decision-global-osk.md
status: open
---

# Decision: OSK chord Menu + Select; no OSK while RetroArch runs

DR-3 (H700 PR #64, `docs/dr-osk-desk-checks-2026-11-18.md`, desk only): in stock `retroarch.cfg` Menu is RetroArch's
hotkey enable and X is pause, so Menu + X is not free; with that config many plain keyboard keys are live hotkeys
in a game (h reset, F2/F4 save/load state, r rewind, e slow motion, ...).

- **Chord (replaces D3): Menu + Select** (Dale 12:47, the recommended option). osklib default `km.chord` changes from X
  to Select; while open, Shift moves off Select (it was Select = Shift).
- **RetroArch (replaces D4):** "No keyboard available when RA is running. Device controls only." The daemon does not
  open while RetroArch runs (the chord does nothing there); no Esc/F1 filter is needed. No `retroarch.cfg` change.
- Unchanged: D1, D2, D5, D6. DR-2: the video player uses ch1/l0 (vp.dge) and fb0 (ffplay), not ch0; ch3 stays the OSK
  channel. DR-1: one layer per `SET_CONFIG2` call; create the uinput device at daemon start.
- Probe session: skip `K27-type retroarch` (not needed now, and r/e/u in "redux" are live RetroArch hotkeys).

- Co-Programmer: the "keys hidden while RetroArch runs" part of its card no longer applies (drawing needs no RetroArch mode).

Not yet on device.
