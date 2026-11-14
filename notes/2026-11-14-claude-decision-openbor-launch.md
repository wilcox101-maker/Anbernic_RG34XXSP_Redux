---
type: decision
from: claude
to: dale
date: 2026-11-14
re: queue 12b, OpenBOR (#151) "crashes on loading app" (Dale, #143, 2026-11-13)
status: closed
---

# 12b OpenBOR: cause found; the fix needs a Roms-card ruling

Read from Dale's stock image `RG34XXSP-V1.0.6-EN16GB-260526`, partition `5.appfs.img` (debugfs,
read only; nothing copied into the repo).

## Cause
`/mnt/vendor/deep/openBOR/scripts/openbor.sh` **does not start OpenBOR.** It only copies
`master.cfg` and `master.controls` (the Anbernic "Deeplay-keys" button map) to
`<pak folder>/Saves/<pak>.cfg` and `/mnt/mmc/openbor/Saves/<pak>.cfg|.controls` the first time, then
exits 0. #151 launched only this script, so Redux came straight back: the "crash".

Stock `dmenu.bin` (strings) does two steps: runs `openbor.sh "<pak>"`, then writes `.next` as
`cd "/mnt/vendor/deep/openBOR"` + `"./OpenBOR.dge"  "<pak>"` (no `AUDIODEV`).

## Why it needs you
`OpenBOR.dge` has its data folder compiled in: `/mnt/mmc/openbor` or `/mnt/sdcard/openbor`, with
`Paks`, `Saves`, `Logs`, `ScreenShots` under it. So the engine itself writes saves, logs and
screenshots on a card, and `openbor.sh` writes two small files per game on TF1. Hard rule: *never
write to the Roms partition (p1, /mnt/mmc) … nothing else in Redux writes to p1.* RetroArch saves go
to `~/.config/retroarch` (rootfs), so there's no precedent of an engine Redux launches writing to p1.

## Options
- **A. Recommended: stock parity, with an exception you write down.** Redux runs `openbor.sh`, then
  `OpenBOR.dge "<pak>"`, exactly like stock. Exception: "a stock engine Redux launches (OpenBOR) may
  write its own saves, logs and config on the card; Redux code still never does." The Anbernic
  button map gets applied, so controls should be right.
- B. Engine only: skip `openbor.sh`, run `OpenBOR.dge "<pak>"`. Redux starts nothing that writes p1
  itself, but the engine still creates `/mnt/mmc/openbor/…` (or TF2), and without the per-game
  `.controls` the buttons may be wrong. Still needs the exception for the engine's own writes.
- C. Take OpenBOR out of Game Room until Phase 2 (where the data folder could be redirected).

Not verified on device: that `OpenBOR.dge` accepts the pak path as its argument (stock passes it),
and which card it picks when both are present.
