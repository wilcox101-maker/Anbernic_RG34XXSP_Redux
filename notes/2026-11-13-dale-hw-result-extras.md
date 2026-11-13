---
from: dale
to: all
event: hw-result
phase: F6
status: closed
refs: [tools/extras.py, docs/extras.md, notes/STATUS.md queue 10]
---

# Dale hardware result, 2026-11-13 11:04: tools/extras.py run against the device

From DLN-DALEW2 (PowerShell, repo at `3a2f418`), device at home 192.168.1.13.

## What ran
- `--select htop,btop`: printed the install and check lines (not run).
- `--select htop --run`: install step ran. apt: "Skipping htop, it is already installed and
  upgrade is not set", `0 upgraded, 0 newly installed, 0 to remove and 533 not upgraded`.
  Check: `htop | /usr/bin/htop | missing-libs=0 | htop 3.0.5`.
- `--check`: printed the check line only (by design then; not run).

## Findings
- Works end to end on Windows with the built-in `ssh`; nothing was upgraded.
- The root password was asked **twice** (install and check were two ssh sessions).
- `--check` only printed; there was no way to have it run. Both fixed in the follow-up PR
  (one ssh session for `--run`; `--check --run` runs the check).
- 533 packages are upgradable on the device: the no-upgrade guard matters.

## Not verified on device
- The REFUSED path (a package that would upgrade something installed): tests only.
- Installing a package that is not there yet (btop etc.) through the tool.
- The full `--check` output across all catalog and stock tools.
