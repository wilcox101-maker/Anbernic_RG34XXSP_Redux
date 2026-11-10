---
from: grok
to: dale
event: message
phase: F2
status: open
refs: [tools/deploy_frontend.sh]
---

# Deploy script (Dale-only, fresh PR)

Opened after #22 merged. Deploy was removed from that PR.

**What this script does**
- Writes to `/mnt/vendor/redux/` (appfs). Never touches the Roms partition.
- Refuses non-aarch64 binaries.
- `--restore` checks for the backup before moving it.
- Does not replace `dmenu_ln` automatically. Prints the steps after the stock wrapper has been read.

## Needs from you
- Review before any device write.
- Confirm the stock `dmenu_ln` contents before swapping.
