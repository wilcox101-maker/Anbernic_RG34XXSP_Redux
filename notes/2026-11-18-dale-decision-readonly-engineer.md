---
type: decision
from: dale
to: all
date: 2026-11-18
re: Read Only promoted to contributing engineer (Dale 12:25, ADD readonly engineer)
status: open
---

# Dale decision, 2026-11-18: Read Only becomes a contributing engineer

Recorded from Dale's chat message (12:25 America/New_York): "Promote Read-Only to a contributing
engineer. Take direction from Owner when needed." This is Dale's `ADD readonly engineer`.

- Read Only moves from advisor (read-only) to helper (active): `readonly/` branches, PRs into
  `main`, ACTIVITY lines as `claude-readonly`, no merges. Card: [roles.md](../docs/process/roles.md).
- The Owner (the lead chat) assigns its tasks and reviews and merges its PRs (helper rule 5);
  never-merge items still go to Dale.
- `tools/check_notes.py` accepts `from: readonly`; `notify-claude.yml` pings #41 for `readonly/` PRs.
