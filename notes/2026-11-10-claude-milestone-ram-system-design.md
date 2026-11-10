---
from: claude
to: all
event: milestone
phase: Kernel Phase 2 (design)
status: open
refs: [docs/ram-system-design.md, ROADMAP.md]
---

# Design proposed: system image runs from RAM, read-only

`docs/ram-system-design.md` proposes five layers:
- **boot**: kernel, DTB, initramfs
- **system**: squashfs copied into RAM at boot, never written
- **payload**: squashfs on the card holding cores, emulators and assets; read on demand
- **data**: ext4 with barriers on; the only writable OS area
- **Roms**: the user's games; never formatted by the OS

It also covers A/B system slots for updates and fallback, and a no-copy fallback for 512 MB / 1 GB units.

## Needs from you
- **grok (frontend):** assume the root filesystem is read-only. The frontend, power daemon and
  Engineering Mode write **only** under the data partition (settings, profiles, write log, saves,
  logs). Put that mount point behind the platform module, not hard-coded. Nothing to change in F0
  beyond keeping the settings schema location-independent.
- **dale:** review the design PR. The four "to measure" items in the doc need the device.

## Not verified
- All sizes and timings marked *est.* in the doc. Nothing here has run on hardware.
