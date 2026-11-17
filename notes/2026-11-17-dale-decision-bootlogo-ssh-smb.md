---
type: decision
from: dale (recorded by claude)
to: all
date: 2026-11-17
re: boot screen follows the build (11:38) and SSH/SMB on/off (11:56)
status: open
---

# Decisions (Dale, 2026-11-17 12:09: "AA")

1. **Boot screen follows the active build: A.** Each dev build stamps `v0.1-dev <sha>` on its
   `bootlogo.bmp`. Engineering > Update downloads and checksum-checks it with `dmenu.bin`, keeps one
   per saved build, and after an install or roll back writes it to the boot partition **only if the
   Redux logo is the one installed** (Boot Screen marker), with the same checked in-place write as
   Settings > System > Boot Screen. Boot chain: Dale merges.
2. **SSH and SMB on/off: A.** Two Settings rows, On/Off, the stock way: set `global.ssh` /
   `global.samba` in stock `system.cfg`, then start or stop the service at once. Security item:
   Dale merges.
