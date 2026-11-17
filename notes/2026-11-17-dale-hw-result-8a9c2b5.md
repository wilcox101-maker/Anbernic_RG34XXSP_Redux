---
type: hw-result
from: dale
to: all
date: 2026-11-17
re: Redux Terminal device test on 8a9c2b5 (#275-#278), checks 1-6 of the sheet
status: open
---

# Device result 8a9c2b5 (Dale, 2026-11-17 14:57)

Sheet (chat, 14:33): 1 Terminal opens, sharp, M quits; 2 buttons as keys + repeat; 3 Select picker
in bash; 4 mc with the right-stick pointer, L3/R3, F10 from the picker; 5 htop with L3; 6 Audio
Mixer and Speaker Test.

Dale's words: "1 - PASS / 2. - PASS / 3. - PASS / 4. - NO OSK / 5. - PASS / 6 - PASS".

- PASS: 1, 2, 3, 5, 6.
- FAIL 4: in mc the Select on-screen keys did not show ("NO OSK"); the pointer part of check 4 not
  stated.
- 15:00: "8 SKIP / 9 PASS / Roll back PASS / Boot Logo PASS / No OSK in all apps except mc".
  So: 9 (moc + pdftotext install and run) PASS; Engineering > Update roll back from the history
  PASS; #273 boot logo on a unit with no marker PASS; the picker works in every app but mc.
  Check 7 (Bluetooth, Input Test, Network Setup, NeoFetch) not stated.
- 15:15: "Picker opens with Select" (mc, on retest). Check 4's picker: PASS; the first try is not
  reproducible (headless, scripted Select opens it over mc too). Closed, no code change.
