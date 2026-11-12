# Decision: Redux volume is the hardware master; RetroArch keeps its own trim

**To:** all
**Date:** 2026-11-12
**Decided by:** Dale

## What was asked

Device test of #94 (persistent lineout volume) found that with Redux showing
`VOL 20` (of 31), RetroArch reported its own volume as 70. Effective output is
therefore RA's 70% of the hardware's 20/31, and the two can be adjusted
independently without the user knowing which one moved: in the Redux menu
Vol+/- drives the hardware mixer, but inside a game RetroArch grabs those
buttons and drives its own software gain.

## Decision

**Leave both stages as they are (option A).** Redux's Vol+/- is the hardware
master volume. RetroArch's internal volume stays a per-game trim that Redux
does not touch.

Rejected: making Redux the only stage by setting RA's internal volume to unity.
That means editing `retroarch.cfg`, whose location on the device was not
established, and it would be forbidden outright if that file lives on a Roms
partition.

## What this means for future work

- Redux does not read, write or mirror RetroArch's audio volume. Do not add
  code that tries to keep the two numbers in agreement.
- The `VOL nn` overlay reports the hardware level only. It is not a prediction
  of loudness inside a game.
- This is not a defect to re-open. A master stage and an application stage
  multiplying is the intended model, the same as on most handheld firmware.
- If loudness at `VOL 31` is ever reported as too low, the thing to revisit is
  RetroArch's own default trim, not this decision.
