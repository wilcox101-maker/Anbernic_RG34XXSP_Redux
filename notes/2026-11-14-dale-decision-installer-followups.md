---
type: decision
from: dale
to: all
date: 2026-11-14
re: installer follow-ups (notes/2026-11-14-claude-decision-installer-followups.md)
status: closed
---

# Dale decision: installer follow-ups 1A 2A 3A 4B

Dale, 2026-11-14 17:21 ET, in the owning chat: **"1A 2A 3A 4B"**.

1. **A: start-failure guard in `run.sh`.** An exit code other than 0 within 10 s of start is a quick
   failure. After 3 in a row, `run.sh` starts the stock menu (`/mnt/vendor/bin/dmenu.bin`) for the rest
   of that boot and logs why. Counter and switch in `/tmp` (RAM); the next boot tries Redux again.
   `dmenu_ln` is not changed. Exit 0 resets the counter.
2. **A: no `REDUX_ROMS` export.** Redux scans TF1 and TF2 itself; the variable stays a documented override.
3. **A: uninstall keeps `/mnt/data/redux/`** (Redux settings), so a reinstall comes back as before.
4. **B: `dev` release now; tagged Beta releases (`v0.5-beta.N`) once Beta is near.** The user guide
   will point at the newest tag; `dev` stays for testing.

Done in the PR that adds this note: 1 (run.sh + 4 tests), 3 (uninstall message, docs), 4 (docs
wording). 2 needed no change (#187's run.sh has no export). Tagged releases are a later STATUS item.

## Not verified on device
The guard has run only in tests (sh and dash). Not checked: that stock `dmenu.bin` runs normally when
started from `run.sh` (its `.next` launches, the video-player return), and `date +%s` on the device.
