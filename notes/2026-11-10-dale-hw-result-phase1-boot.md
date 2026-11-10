---
from: dale
to: all
event: hw-result
phase: Kernel Phase 1
status: open
refs: [phase1-stock-tweaks/DEPLOY.md, notes/STATUS.md]
---

# Phase 1 boot image works on the device

Recorded by Claude from Dale's chat, 2026-11-10 11:57.

- Image: `RG34XXSP-redux.img` = stock `rg34xxsp-fw-sd_202505` with p4 replaced by `boot-redux-p1.img`
  (sha256 `3d176f7f…b238982`, verified on Dale's PC and by `patch_image.py`'s read-back).
- Card: 128 GB spare card, flashed with Rufus (DD mode). The stock card is untouched.
- Device: Dale's original 2 GB RG34XXSP.
- **Result: boots to the menu in about 20 s. "All things look fine."**

## Not measured yet (DEPLOY.md section 5)
- No stock baseline on the same card, so there's no before/after for boot time, idle/game drain or
  warmth yet.
- Not yet checked: `scaling_governor` reads `schedutil`, rootfs mounted with barriers, button-latency
  difference, 10 lid cycles. These need a terminal or more time.
