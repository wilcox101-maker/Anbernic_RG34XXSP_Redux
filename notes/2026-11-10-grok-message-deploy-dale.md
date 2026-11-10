---
from: grok
to: dale
event: message
phase: F2
status: open
refs: [tools/deploy_frontend.sh]
---

# Deploy script (Dale-only)

Split from PR #22 per review.

**Changes from the first version**
- Writes to `/mnt/vendor/redux/` (appfs), never to the Roms partition.
- Refuses non-aarch64 binaries.
- `--restore` is a real command that checks for the backup first.
- Does not replace `dmenu_ln` automatically. Prints the steps after you have read the stock wrapper.

## Needs from you
- Review before any device write.
- Confirm the stock `dmenu_ln` contents (env, cd, LD paths) before swapping.
