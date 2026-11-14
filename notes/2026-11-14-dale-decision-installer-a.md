---
type: decision
from: dale
to: all
date: 2026-11-14
re: stock-card installer (notes/2026-11-14-claude-decision-stock-card-installer.md)
status: closed
---

# Dale decision: the Beta installer is a PC tool over SSH (option A)

Dale, 2026-11-14 13:41 ET, in the owning chat: **"a"**.

## What it means
- `tools/install.py` (Python stdlib, Windows `ssh`) sends `installer/redux_install.sh`, the
  `dmenu.bin` payload and `installer/run.sh` (start wrapper, capped `redux.log`; it replaces the
  hand-made one on Dale's unit) over **one** SSH session: `--check`, install (also upgrade), `--uninstall`.
- `dmenu_ln`'s `CMD=` becomes `/mnt/vendor/redux/run.sh`, as on Dale's unit today.
- Writes appfs (`/mnt/vendor/redux/`, the one `CMD=` line of `dmenu_ln` + its one-time backup) and
  `/mnt/data/redux/install.log` only. No p1, no boot partition, no formatting.
- The dev release carries `install.py` and `redux_install.sh` next to `dmenu.bin`. User guide: `docs/install.md`.
- Option B (TF2 package with a hold-A screen) stays the next step; it would reuse `redux_install.sh`.
  C is not planned. Phase 1 kernel tweaks stay a separate, Dale-gated step.

## Not verified on device
Everything: it has run only against a fake card tree in CI (dash and bash). Device checks are on the test queue.
