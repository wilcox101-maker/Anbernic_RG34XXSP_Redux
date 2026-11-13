---
type: decision
from: claude
to: dale
date: 2026-11-13
re: Dale 2026-11-13 10:00 "What is installed and stock but not utilized? Consider adding as Core App."
status: closed
---

# Installed on the device but not used by Redux: Core App candidates

Source: `stock-apps.txt` from Dale's unit (2026-11-13: `dpkg -l`, `/usr/bin` etc., appfs and
`/mnt/mod/ctrl` listings). Read only, nothing copied. **Update 10:08:** Dale's
`/var/log/apt/history.log` has one entry, `apt install htop` (2026-11-10), so **htop is the only
package he added**; everything else below is stock on his unit. The same entry shows apt on the
device reached its package sources and installed cleanly (the path an installer would use).

## Candidates

| Tool | Where | What it gives | Redux today | Cost as a Core App |
|---|---|---|---|---|
| **alsamixer** | /usr/bin (alsa-utils) | full audio mixer, per control | volume only | S: Terminal `-e alsamixer` |
| **nmtui** | /usr/bin (network-manager) | Wi-Fi: hidden SSIDs, static IP, forget networks | scan + connect | S: Terminal `-e nmtui` |
| **bluetoothctl** | /usr/bin (bluez) | Bluetooth pair/connect (pads, headphones) | none (F3 item) | S: Terminal; a real UI later |
| **evtest** | appfs `bin/evtest` | Input Test: every button/stick code live | none | S: Terminal `-e` evtest on event1 |
| **speaker-test** | /usr/bin (alsa-utils) | left/right speaker and headphone test | none | S: Terminal, fixed args |
| **OpenBOR** | appfs `deep/openBOR/OpenBOR.dge` | beat-'em-up engine (OPENBOR folder) | folder unmapped (STATUS 1a) | S: Game Room system, not an app |
| **mpv** | /usr/bin | plays almost any video/audio, hardware-friendly | stock vp.dge / ap.dge | M: needs a Redux file picker |
| **PDF reader `green`** | appfs `bin/pdf` | PDF viewer | listed "later" | M: same file picker |
| python3.10, vim, git, wget | /usr/bin | dev tools in the Terminal | Terminal only | none: already reachable |
| sshd, smbd | /usr/bin | remote shell, file share | none | security item, Dale-gated (PC link) |
| WiliWili | appfs `bin/wiliwili` | Bilibili video client | left out on purpose (docs/ui-spec.md) | skip |

## Options
- **A. Recommended:** an **Apps > Tools** group of five terminal Core Apps (Audio Mixer, Network
  Setup, Bluetooth, Input Test, Speaker Test), each `SimpleTerminal -e <tool>` like Terminal and Task
  Monitor; plus **OpenBOR** in Game Room. One small PR each, all S. Same htop rule applies: a tool
  not stock on every unit needs Redux to supply it (queue 10).
- B. A, plus a Redux **file picker** so mpv (Media Player) and the PDF reader become apps (M).
- C. OpenBOR only; leave the tools in the Terminal for those who know them.

Not verified on device: any of these launched from Redux.

## Dale's answer, 2026-11-13 10:16: **A**
Five Tools apps in the Apps list (after Task Monitor), then OpenBOR in Game Room. The Tools rows sit
flat in the Apps list rather than in a sub-screen (13 rows fit); a Tools sub-screen can come later.
