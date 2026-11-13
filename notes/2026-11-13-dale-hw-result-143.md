---
from: dale
to: all
event: hw-result
phase: F6
status: open
refs: [issue #143, issue #109, PR #142, PR #146, PR #149, PR #150, PR #151]
---

# Dale hardware result, 2026-11-13 11:34: device test queue #143

Dale closed #143 (Device test queue) at 11:34 ET with a comment and ticked boxes. Build: not stated;
Dale was deploying the dev release **`92769d6`** at 11:19 (confirm from the title bar).

## Dale's words (issue #143 comment, verbatim)
> Video player crashes to Main menu
> OpenBOR crashes on loading app
> File Manager is redundant in Main menu and apps
> Apps exit back to Main menu
> Portmaster Wolfenstein runs and exits. Shoulder buttons function as Strafe and Change weapon

## Ticked by Dale on #143 (pass)
- #146: htop inside SimpleTerminal launched from Redux (how it quits not stated).
- #149: the install line on the device (also [extras hw-result](2026-11-13-dale-hw-result-extras.md)).
- #150: the five Tools apps inside SimpleTerminal (drawing, keys, audio, evtest output).

## Pass
- Ports (9c re-check): PortMaster Wolfenstein runs and exits back; shoulder buttons strafe and
  change weapon. Start/Select/Y/X in-game not stated.

## Fail / to fix
1. **Video Player (#142) crashes to Main.** Second failed round for 9d (stock vp.dge path).
2. **OpenBOR (#151) crashes while loading.** `openbor.sh` content and arguments were never read.
3. **File Manager is in both Main and Apps.** Redundant: show it once.
4. **Apps return to Main, not to the Apps list.** Redux relaunches after every app and starts at
   Main; it does not remember the screen it launched from.

## Not reported (still open)
Screen Timeout (#145), Ebook Reader M vs stock, brightness re-check, TF2, Card Scripts, Boot Screen
restore, Wi-Fi, buttons (#109 items), nmtui vs Redux Wi-Fi, htop on a unit without htop.
