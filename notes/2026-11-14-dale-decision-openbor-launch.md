---
type: decision
from: dale
to: all
date: 2026-11-14
re: queue 12b, OpenBOR launch (notes/2026-11-14-claude-decision-openbor-launch.md)
status: closed
---

# Dale decision: OpenBOR runs as stock does (option A)

Dale, 2026-11-14 06:10 ET, in the owning chat: **"A"**, the recommended option of the
[decision request](2026-11-14-claude-decision-openbor-launch.md).

## What it means
- Redux launches an OpenBOR `.pak` exactly like stock `dmenu.bin`: `openbor.sh "<pak>"` (seeds the
  per-game cfg and controls), then `cd /mnt/vendor/deep/openBOR && ./OpenBOR.dge "<pak>"`.
- **Hard-rule exception, added to AGENTS.md:** a stock engine Redux launches exactly as stock does may
  write its own saves, logs and config on the card. Today that is only OpenBOR (`/mnt/mmc/openbor`,
  or the pak's `Saves`). Redux's own code still never writes to p1.
- Exit codes and output of both steps go to `redux.log` (`[openbor] script exit=`, `[openbor] engine exit=`).

## Not verified on device
- That `OpenBOR.dge` accepts the pak path as its argument, and which card it uses when both are in.
- That the controls are right after `openbor.sh` seeds them.
