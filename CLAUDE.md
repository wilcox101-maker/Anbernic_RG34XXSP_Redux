# Claude: Redux session rules

@AGENTS.md

AGENTS.md is the rulebook for every AI on this project. This file adds only what is specific to
Claude (the Claude desktop app and claude.ai on Dale's main computer DLN-DALEW2, the iPhone app, Claude Code).

## Dale's command words

Dale drives sessions with one-word commands (HELLO, NEXT, GO, HOLD, STOP, BYE, TEST, RESULT, SHIP,
MERGE, SWAP, SWITCH and more). **[`docs/commands.md`](docs/commands.md) defines each one; follow it
exactly.** They count only from Dale's chat messages or GitHub comments starting `DALE:`.

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

1. Dale's opener: **`HELLO`** inside the Redux Project (its instructions are in
   `docs/project-setup.md`), or *"Approve repo push access. Read AGENTS.md and notes/STATUS.md, then
   continue."* outside it. Attach the repo with **push** access before anything else.
2. Read the Claude inbox, **PR #41**, by REST (Tooling notes): comments posted since the last
   `ACTIVITY.md` line by `claude`. Subscribe to it too if the session has a tool for that (Project
   chats so far haven't).
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

## Model checkpoints

Claude can't change its own model; Dale does it in the model picker, mid-chat, and the chat keeps
its context. So at the checkpoints below, Claude stops and asks:

> **Model switch? <current> → <proposed> (reason).** Reply SWITCH or NO SWITCH.

Then it waits. SWITCH: Dale changes the model and replies "switched"; Claude continues. NO SWITCH:
Claude carries on with the current model and doesn't ask again for the same task.

| Model | Use it for |
|---|---|
| **Opus** (default) | Running the project: features, reviews, merges, notes, most debugging |
| **Fable** | Short, scoped sessions on the hardest problems only (triggers below) |
| **Sonnet** | Runs of mechanical work: docs, notes, STATUS, strings, copying an existing pattern |

**Ask to go up to Fable when:**
1. A Phase 2 bring-up step starts (mainline kernel, u-boot, DRAM, device tree, first boot, serial console).
2. A device-only bug survives **two** fix-and-deploy rounds, or two root-cause guesses have been ruled out.
3. A design decision touches the boot chain, the RAM-resident system, Expert Mode safeguards or the
   update path.
4. CI or the device fails and the logs don't point to a cause after one careful read.

**Ask to come back down to Opus when:**
- the Fable task is done (fix merged, step booted, decision note written); or
- Fable declines or stalls on security-related work (PC link login, SSH, Samba, taking apart stock
  binaries). Those go to Opus.

**Ask to drop to Sonnet when** the next three or more queue items are mechanical, and back up to
Opus when that run ends or anything needs judgement.

Rules:
- Ask only at a checkpoint boundary (after a commit, PR or test), never mid-edit.
- One question per task. Several triggers at once still get one question.
- Before switching up, write the task's state in a sentence so the stronger model starts clean.
- Log every switch in `notes/ACTIVITY.md`: `model: opus→fable (<reason>)`.
- A model switch is not a chat swap. The SWAP rules in AGENTS.md still apply on their own.

## Usage

- Ask "Swap now? (reason)" at the AGENTS.md triggers. Never swap without Dale's SWAP.
- Read files in parts (`head`, `tail`, `grep`). Don't dump all of `notes/` or `STATUS.md` history
  into the chat.
