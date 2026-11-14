---
type: decision
from: claude
to: dale
date: 2026-11-14
re: installer follow-ups (#184, #187); Read Only's installer plan (FROM-REDUX-READONLY-2026-11-14-b) questions 2-4 and its start-failure guard
status: open
---

# Decision: installer follow-ups (four answers)

The installer (#184, #187) is merged and not device-checked. Read Only's plan raised these; question 1
(A/B/C) is already answered (A). Each has a recommendation; answer like `1A 2A 3A 4B`.

## 1. Start-failure guard (changes boot behaviour)
Today, if Redux crashes at start, stock `dmenu_ln` waits 30 s and starts it again, forever. SSH is the
only way out.
- **A (recommended):** `run.sh` counts quick failures (exit ≠ 0 within 10 s of start, counter in
  `/tmp`, so RAM only). After **3 in a row** it starts the **stock menu** (`/mnt/vendor/bin/dmenu.bin`)
  for the rest of that boot and logs why. The next boot tries Redux again. `dmenu_ln` is not changed.
- B: same trigger, but it restores the stock `dmenu_ln` from `.bak`, so stock stays until a reinstall.
- C: no guard; keep the SSH recovery in `docs/install.md`.

## 2. `REDUX_ROMS`
Your hand-made run.sh exported `REDUX_ROMS=/mnt/sdcard/Roms`; the repo's run.sh (#187) doesn't.
- **A (recommended):** no export. Redux scans TF1 and TF2 together (#104), and the variable stays a
  documented override.
- B: export a default in run.sh (say which path).

## 3. Uninstall and Redux settings (`/mnt/data/redux/`)
- **A (recommended):** keep them, so a reinstall comes back as you left it. The guide says where they are.
- B: `--uninstall --purge` deletes them (asks first); plain `--uninstall` keeps them.

## 4. Which build the installer is for
- A: the rolling `dev` release only (what exists today).
- **B (recommended):** `dev` now, plus tagged Beta releases (`v0.5-beta.N`) once Beta is near. The
  user guide points at the newest tag, and `dev` stays for testing.

## Not verified on device
Nothing here has run. Option 1A has never been tried on the device: whether stock `dmenu.bin` runs
normally from `run.sh`, with `.next` and the video-player return working.
