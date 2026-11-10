# Notes: how the AIs on this project talk to each other

One note = one file. Notes are **append-only**: you never edit someone else's note, except to change
its `status` line when you acknowledge or close it. Separate files mean two agents never collide in
the same file.

## When to write a note (events)

| Event | Write it when | Typical `to` |
|---|---|---|
| `phase-start` | you begin a phase (F0…F8, or a kernel phase) | all |
| `milestone` | a deliverable in a phase is done | all |
| `phase-gate` | a phase's exit test passes and it needs Dale's sign-off | dale |
| `handoff` | you stop partway and someone else should continue | the other agent |
| `review-request` | a pull request is ready for the other agent to check | the other agent |
| `review` | you've reviewed it (findings or approval) | the author |
| `question` | you need something from the other agent | the other agent |
| `decision` | a choice needs Dale (give 2–3 options + a recommendation) | dale |
| `blocker` | you can't continue | all |
| `hw-result` | a test ran on the real device (usually Dale) | all |
| `correction` | something an earlier note or doc said is wrong | all |
| `message` | anytime: a direct message to another agent or Dale with a need, concern or comment | anyone |

Routine progress goes in commit messages, not notes. Use `message` for anything you want someone to
read that doesn't fit another event. Replies to a message are new `message` notes listing the
original in `refs`.

## File name

`notes/YYYY-MM-DD-<from>-<event>-<short-topic>.md`, for example
`2026-11-10-claude-phase-start-f0-spec.md`. Use lowercase with hyphens. If a second note lands on the
same day with the same name, add `-2`.

## Format

```markdown
---
from: claude            # claude | grok | dale
to: grok                # grok | claude | dale | all
event: review-request   # from the table above
phase: F0
status: open            # open -> acknowledged (by <name>, <date>) -> closed (by <name>, <date>)
refs: [PR #3, docs/ui-spec.md, notes/2026-11-10-grok-milestone-mockups.md]
---

# One-line summary

What happened, in a few lines.

## Needs from you
- concrete asks, or "nothing; FYI"

## Not verified
- anything untested on real hardware
```

## Status

- `open`: not yet read by the addressee
- `acknowledged (by grok, 2026-11-11)`: read and being acted on
- `closed (by dale, 2026-11-12)`: answered or no longer relevant

[`STATUS.md`](STATUS.md) is the one shared board. Whoever writes a note updates it in the same pull
request.
