---
from: dale
to: all
event: decision
phase: backlog
status: open
refs: [AGENTS.md, notes/STATUS.md, ROADMAP.md]
---

# Dale decision, 2026-11-13: ROM copy runs in [Xroot Mode]

Recorded by the Redux Manager chat. The PC link decision of 2026-11-12 lived only on inbox #41,
AGENTS.md and STATUS; this note is its first record in `notes/`, with today's name for the mode.

## Dale's words

- 2026-11-12 (Redux chat): "Full file manager style." / "ROM copy is allowed after a previously hidden
  checkbox is confirmed that only personally owned ROM's will be uploaded and used with clear
  disclaimer of warnings and potential violations."
- 2026-11-13 03:20 (Redux Manager): "yesterday added ROM copy in File Manager after hidden features
  enabled. This enabled mode will be labeled [Xroot Mode]."

## Decision

- **[Xroot Mode]** is the name of the state in which ROM copy (ROM uploads into the TF1/TF2 ROM
  folders) is allowed. It is entered only by ticking the previously hidden one-time ownership
  acknowledgement, and left by **Settings → PC link → Reset acknowledgement**.
- The UI labels it exactly `[Xroot Mode]`, brackets included, wherever the mode's state is shown
  (at least the PC link screen and its Settings row). Where else it appears (title-bar badge or not)
  is settled in the security PR.
- It is **not** Expert Mode. Expert Mode (above-factory CPU/GPU settings, the classic code) is
  unchanged and still governed by [its decision](2026-11-12-dale-decision-expert-mode.md).

## The PC link, as decided on #41 (2026-11-12), unchanged

- Browser page served by Redux: browse, upload, download, rename, move and delete on the TF1 and TF2
  ROM folders and the Redux data folder. It never touches p2–p7.
- Safeguards: off by default, password set on first enable, local network only, delete moves files
  to `<card>/.trash`, overwrite always asks.
- Download, rename and move work as soon as the link is on. The first upload into a ROM folder shows
  the hidden checkbox, which must be ticked before any ROM upload:
  "☐ I will only upload and play games and BIOS files I personally own. I understand that copying
  ROMs or BIOS files I don't own may violate copyright law where I live, and that I am responsible
  for what I upload. Redux doesn't provide or check ROMs."
  Ticked = [Xroot Mode]. Stored with a date in the Redux data folder. Without it, ROM uploads are
  refused and everything else still works.
- This is UI wording, not legal advice. Dale signs off the final text in the security PR.
- Security item: every PC link PR is Dale-only. The AGENTS.md hard-rule wording changes in this
  note's PR, so **Dale merges it**.

## Not verified on device

- Nothing: the PC link is not built yet (backlog).
