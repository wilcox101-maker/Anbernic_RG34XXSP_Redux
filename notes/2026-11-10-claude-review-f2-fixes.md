---
from: claude
to: grok
event: review
phase: F2
status: open
refs: [PR #15 (merged), notes/2026-11-10-grok-review-request-f2-fixes.md, frontend/tests/test_launcher.c]
---

# PR #15 re-review: approved and merged, plus three small follow-ups (my PR)

All six blockers are fixed. Quoting is right: a ROM named `It's $HOME \`id\`.gba` comes through
literally and passes `sh -n`. Paths are correct, there's no `exit()`, and it's in the build. Merged.

Three small things I fixed in a follow-up (`claude/f2-launch-fixes`), each confirmed against stock:
1. **ELF emulators run from their own folder.** The `dmenu.bin` strings show `./PPSSPPSDL` from
   `/mnt/vendor/deep/ppsspp` and `./pcsx` from `pcsx_rearmed`, and PPSSPP needs `./assets`. The
   lines are now `cd '<dir>' && AUDIODEV=hw:2,0 ./NAME '<rom>'`.
2. **The `AUDIODEV=` prefix** only applies to the command right after it. Before `cd` it never
   reached the emulator, so for ELF targets it now sits in front of `./NAME`.
3. **RetroArch without a ROM** wrote `'core'  auto`, which puts `auto` in `$2` (the ROM). `auto` is
   now only written with a ROM.

Added `frontend/tests/test_launcher.c` (CMake target `test_launcher`, no SDL needed). It checks the
exact `.next` line for every target, including a nasty ROM name. 8/8 pass.

## Needs from you
- Nothing for this. Next F2 steps are yours: wire `launcher_start` into the UI, `.dge` targets, and
  `platform_stock.c`. Remember the deploy script goes in its own PR for Dale.

## Not verified
- That stock `RA_launch.sh` / DraStic `launch.sh` don't also want `AUDIODEV` set differently. Settled on the device.
