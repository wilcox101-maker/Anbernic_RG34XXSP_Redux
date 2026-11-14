---
type: message
from: dale
to: all
date: 2026-11-13
re: upstream links for the kernel and Ubuntu research; written up in docs/kernel-research/
status: open
---

# Kernel research: Dale's upstream links (2026-11-13)

Dale's words (Manager chat, 16:37 to 17:10): "Project upstream notes: Ubuntu Kernel roadmap"
(ubuntu.fan version comparison), then "Add" for the Knulli H700 extraction guide,
ogra1/linux-generic-allwinner, rg35xx-cfw.github.io and a Brave search on "Allwinner H700 Ubuntu
24.0 LTS kernel"; then "Review and process to plan a roadmap to 24.04 in parallel. Plan and strategy
only" (with MrJackSpade's RG35XXSP CFW gist); then "Record all work into new section of repo related
to Kernel research."

All of it is in [`docs/kernel-research/`](../docs/kernel-research/README.md): the links with summaries
(`upstream-references.md`), a mainline status matrix, a ROCKNIX H700 read-through, the Ubuntu 24.04
track plan (**plan only, Dale reviewing; not on the ROADMAP**) and open questions.

Warnings carried from the research:
- The Knulli guide's partition numbers don't match the RG34XXSP (boot.img is p4 here; p1 is Roms).
  Never use its `dd` offsets on this device.
- Nothing in docs/kernel-research has run on Dale's device.
