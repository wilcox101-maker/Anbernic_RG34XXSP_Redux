# Claude: Redux session rules

@AGENTS.md

AGENTS.md is the rulebook for every AI on this project. This file adds only what is specific to
Claude (claude.ai chats, the iPhone app, Claude Code on Dale's Windows PC).

## One Claude owns the repo at a time

- Only the **owning** Claude session pushes, comments, merges or edits `notes/STATUS.md`. The owner
  is the chat named in the newest `notes/handoff-*.md` (or the one Dale says is running the project).
- Any other Claude session (a side chat, an advice chat, Claude Code on the PC) is **advisory**:
  read-only. It may clone, read, build and test locally, and give Dale files or text to pass to the
  owner. It does not push branches, open PRs, comment on #41, or append to `ACTIVITY.md`.
- Ownership moves only through the SWAP routine in AGENTS.md, or when Dale says so.

## Solo mode (current: Grok is suspended)

Dale's decision of 2026-11-13: Claude runs the project alone until Dale reinstates Grok.
- Claude takes every queue item in `notes/STATUS.md`, whatever its old owner marker.
- No second reviewer, so each PR gets a **self-review** before merging: CI green, hard rules
  re-read against the diff, `Not verified on device` section filled in, one topic per PR. Claude
  merges with Dale's standing permission. The "never merged without Dale" list is unchanged.
- Event notes still go in `notes/`: they are the memory between chats, not just messages to Grok.
  Address them to `dale` or `all`.
- `notify-claude.yml` and #41 stay as they are; they go quiet with no `grok/` branches. Don't
  delete them.
- Hardware tests are Dale's. Batch them so he runs several per device session.

## Session start (owner)

1. Dale's opener: *"Approve repo push access. Read AGENTS.md and notes/STATUS.md, then continue."*
   Attach the repo with **push** access before anything else.
2. Subscribe to the Claude inbox, **PR #41**, then read comments posted since the last
   `ACTIVITY.md` line by `claude`.
3. Read: last 20 lines of `notes/ACTIVITY.md` → `notes/STATUS.md` Handoff → the handoff file it
   points at → open notes addressed to `claude` or `all`.

## Tooling notes

- GitHub GraphQL is blocked in Claude cloud sessions, so `gh pr list` / `gh pr view` fail. Use REST:
  `gh api repos/wilcox101-maker/anbernic_rg34xxsp_redux/pulls?state=open`, and
  `.../issues/41/comments?since=<ISO time>` for the inbox.
- Clone shallow (`git clone --depth 1`). Push with `git -c push.negotiate=true push -u origin <branch>`.
  Force-push is blocked; open a new PR instead.
- Stage explicit paths, never `git add -A` (build output was committed that way once, in #79).
- Before every push: `python3 tools/check_rules.py`, then both frontend builds and the tests the
  same way `.github/workflows/ci.yml` runs them.
- Deploy only main's CI arm64 artifact. The title-bar SHA must be a **merge** commit in `git log`.

## Usage

- Opus for kernel/boot-chain work, debugging, reviews and decisions. Sonnet is fine for docs, notes,
  STATUS updates and mechanical ports.
- Ask "Swap now? (reason)" at the AGENTS.md triggers. Never swap without Dale's SWAP.
- Read files in parts (`head`, `tail`, `grep`). Don't dump all of `notes/` or `STATUS.md` history
  into the chat.
