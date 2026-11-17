---
type: hw-result
from: dale (recorded by claude)
to: all
date: 2026-11-17
re: boot logo per build, SSH/SMB, Packages, UI page fit on 07d21b6 (#266, #267, #269, #272)
status: open
---

# Device session 13:20-13:44 (Dale)

| # | Check | Result (Dale's words) |
|---|---|---|
| 1 | Boot logo follows the build | FAIL: "Redux boot screen should be pulled with dmenu.bin". The logo was not written: no Boot Screen marker on this unit (the Redux logo was not installed through Settings > System > Boot Screen), and #266 wrote only when the marker said Redux. Fix PR: write unless the user chose stock. |
| 2 | Settings > SSH On/Off | PASS, with: "Font size reverts to Default after selection then restores after (B)ack". The message line made Settings 13 lines, too tall for all-Large sizes, so the screen fell back. Fix PR: message right under the list. |
| 3 | Settings > SMB | PASS (13:48) |
| 4 | Packages: install cmus, run, remove | PASS ("ran in terminal with ERROR initializing MPRIS") |
| 5 | UI page at Largest with header/footer Large (#269) | PASS |
