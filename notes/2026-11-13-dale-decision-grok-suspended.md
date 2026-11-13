---
from: dale
to: all
event: decision
phase: F3
status: open
refs: [GROK.md, CLAUDE.md, notes/2026-11-13-dale-decision-claude-runs-phase.md]
---

# Dale decision, 2026-11-13: Grok is suspended; Claude runs the project solo

Recorded by Claude from Dale's chat message (00:35 America/New_York) and the harness he supplied
(#107: `GROK.md`, `CLAUDE.md` "Solo mode").

## Decision

- **Grok is suspended** until Dale reinstates him in a new `decision` note. This replaces
  "sidelined, called in as needed" in
  [the earlier note](2026-11-13-dale-decision-claude-runs-phase.md): Grok gets no tasks, runs no
  hourly check, and opens no `grok/` branches or PRs while suspended.
- **Claude runs the project solo.** It takes every queue item in `STATUS.md`, whatever its old
  owner marker, and merges with Dale's standing permission after a self-review (CI green, hard
  rules re-read against the diff, "Not verified on device" filled in, one topic per PR).
- **Unchanged:** the AGENTS.md hard rules and the "never merged without Dale" list. Device tests
  are Dale's; Claude batches them.

## Leftovers

- #101 (Grok's brightness PR) is replaced by #102 and should be closed (Dale; Claude's close was
  blocked).
- `notify-claude.yml` and inbox #41 stay; they go quiet with no `grok/` branches.
- AGENTS.md "Scheduled checks" still says Grok checks hourly. It is not edited here; `CLAUDE.md`
  solo mode governs while Grok is suspended.

## Reinstating Grok

Follow `GROK.md`: Dale writes a decision note naming Grok's task, pastes the active rules into
Grok's project instructions, and sends the opener there.
