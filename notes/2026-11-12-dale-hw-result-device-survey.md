---
from: dale
to: all
event: hw-result
status: open
refs: [PR #67, inbox #41]
---

# Device survey (2026-11-12, read-only script, recorded by Claude)

- **ROM counts**: well over 10,000 ROMs across 50+ folders (FBNEO 1797, MAME 1713, MD 1241, FC 1211,
  SFC 1138, GB 1137, MSX 969, GBC 875, A2600 696...; counts include `Imgs/` box art). The old
  4096-ROM cap in `romscan.c` dropped whole systems, so it is removed in this PR.
- **Folders with ROMs but no Redux entry yet**: A2600, ATOMISWAVE, DREAMCAST, GW, HBMAME, MDCD, MSX,
  N64, NAOMI, OPENBOR, PICO, SATURN, SEGA32X, VARCADE, VB (plus APPS and PORTS, queue items).
- **Subfolders**: only `Imgs/` (box art) in ROM folders, and app folders under APPS. There are no
  game subfolders and no cue/bin pairs (PS1 is .chd/.PBP).
- **Stock core mapping**: no text table found. Stock remaps exist for Flycast (NAOMI/ATOMISWAVE/DC),
  Mupen64Plus-Next and ParaLLEl N64, YabaSanshiro, DOSBox-pure, Beetle NeoPop, FB Alpha, FinalBurn Neo, PCSX-ReARMed.
- **RetroArch** (`/mnt/vendor/deep/retro/retroarch.cfg`): saves in `~/.config/retroarch/{saves,states}`
  (rootfs p5, rw). `system_directory = "~/mnt/vendor/deep/retro/system"` looks like a typo for
  `/mnt/vendor/...`. **Not verified** whether `/root/mnt` exists, i.e. whether BIOS lookups work.
  `/mnt/mmc/bios` exists. Hotkey btn 8, exit btn 7, menu toggle combo 7.
- **Mounts**: p5 / rw, p6 /mnt/vendor rw, p7 /mnt/data rw (2.3 GB free, empty), TF1 p1 /mnt/mmc vfat,
  TF2 /mnt/sdcard vfat (117 GB).
- **CPU**: schedutil; OPPs 480–1512 MHz. Thermal zones ≈ 41 °C idle.
- **Battery**: capacity 78, temp `300`, i.e. tenths of °C (30.0 °C). This answers the F0 battery-temperature unit question.
- **RAM**: 1979 MB total (the 2 GB unit). Kernel cmdline confirms the partition map.
- **dmesg**: rtl8821cs probe fails once with -123 then works; repeated `RF-K not allowed... site-survey`
  lines are Wi-Fi background scans (backlog item: Wi-Fi idle scanning). The HDMI DDC pinctrl error is expected (no HDMI port).
