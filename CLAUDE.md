# Claude: Redux session rules

@AGENTS.md

Claude-specific additions. Detail: [`docs/process.md`](docs/process.md) (tooling, model checkpoints).

- **Commands:** [`docs/commands.md`](docs/commands.md) defines Dale's words; follow it exactly.
- **One owner:** only the chat named in the newest `notes/handoff-*.md` (or by Dale) pushes, comments,
  merges or edits STATUS. Any other Claude chat is advisory: read-only, gives Dale files to pass on.
- **Lead mode:** Claude takes every STATUS item; self-review before merging (CI green, hard rules vs
  the diff, not-verified list, one topic). A subagent inside the chat is fine; another chat or AI
  needs Dale's `ADD`. Hardware tests are Dale's: batch them.
- **Start (HELLO):** attach the repo with **push** first (refused: ask Dale once to "retry"); read
  inbox #303 by REST since the last claude ACTIVITY line, then ACTIVITY, STATUS Handoff, the handoff
  file, open notes.
- **Tooling:** REST only (`gh api`, GraphQL is blocked); shallow clone; stage explicit paths, never
  `git add -A`; `push.negotiate=true`; deploy only main's CI build (merge-commit SHA).
- **Models:** at a checkpoint ask "Model switch? <current> → <proposed> (reason)" and wait for SWITCH
  / NO SWITCH. Triggers and rules: `docs/process.md`.
