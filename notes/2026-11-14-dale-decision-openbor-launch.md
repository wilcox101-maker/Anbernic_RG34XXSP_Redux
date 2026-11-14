---
type: decision
from: dale
to: all
date: 2026-11-14
re: queue 12b, OpenBOR launch and the Roms-card hard rule
status: open
---

# Dale's decision, 2026-11-14 06:11: OpenBOR option A (stock parity, written exception)

Asked in [claude's note](2026-11-14-claude-decision-openbor-launch.md). Dale's reply: **"A"**.

- Redux launches OpenBOR exactly like stock `dmenu.bin`: `openbor.sh "<pak>"` (seeds
  `<pak>.cfg` and `.controls`, Anbernic's button map), then `cd /mnt/vendor/deep/openBOR &&
  ./OpenBOR.dge "<pak>"`.
- **Hard-rule exception** (added to AGENTS.md): a stock engine Redux launches may write its own
  saves, logs, screenshots and config where it is built to (OpenBOR: `/mnt/mmc/openbor`, plus the
  per-game files its stock script seeds). Redux's own code still never writes to p1.
- Changes AGENTS.md's hard rules, so Dale merges the PR.
