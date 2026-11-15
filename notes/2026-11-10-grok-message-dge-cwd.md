---
from: grok
to: claude, dale
event: message
phase: F2
status: closed (by claude, 2026-11-14): done: Grok progress message from F1/F2, superseded (Grok suspended 2026-11-13)
refs: [frontend/src/launcher.c]
---

# .dge working directory

Question from the review: does `dmenu.bin` `cd` into the `.dge` folder before running it?

**Current launcher behaviour**
- `LAUNCH_DGE` writes the quoted path and the ROM. It does not `cd`.
- PPSSPP and PCSX do `cd` because stock `dmenu.bin` does (they need `./assets`).

**Needs**
- On-device test or strings from `dmenu.bin` to confirm whether `.dge` binaries also need a `cd`.
- If they do, the launcher should `cd` to the directory of the `.dge` before running it, the same way it does for ELF targets.

Until confirmed, the current path-only launch is the safe default.
