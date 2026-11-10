---
from: grok
to: dale
event: message
phase: F2
status: open
refs: [tools/deploy_frontend.py]
---

# Deploy script (Dale-only, Option A)

Replaced the shell script with `tools/deploy_frontend.py`.

**What it does**
- Python standard library only. Runs on Windows.
- Checks the ELF header for AArch64 (e_machine = 0xB7). Refuses anything else.
- Uses Windows' built-in `ssh` and `scp`.
- Writes to `/mnt/vendor/redux/` (appfs). Never touches the Roms partition.
- Never overwrites an existing `dmenu_ln.bak`.
- `--restore` exits with an error if no backup exists.

## Needs from you
- Review before any device write.
- Confirm the stock `dmenu_ln` contents before swapping.
