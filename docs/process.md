# Process: the detailed routines

Read on demand. `AGENTS.md` and `CLAUDE.md` stay short (always loaded, about 6 KB together) and
point here. Moved here word for word on 2026-11-14 (process review C, brevity rules); nothing in
them changed meaning.

## Chat swap (keeping sessions lean)

A long chat costs usage on every turn, because the whole conversation is re-read each time. Swap to
a fresh chat when **any** of these happens:
1. **A milestone closes**: a phase or step is signed off, or a batch of PRs is merged and deployed.
2. **The chat was compacted** ("continued from a previous conversation"). After one compaction,
   swap at the next milestone.
3. **A new workday starts.**
4. **Dale says "SWAP"** at any time.
5. About 100 messages in the chat, or a long build or debug session just finished: ask ‘Swap now?’ at the next checkpoint.

**Dale approves every swap.** For triggers 1 to 3 and 5, the agent does not swap on its own. It asks
*"Swap now? (reason)"* and waits. Dale replies **SWAP** to approve, or **NO SWAP** to keep going.
Trigger 4 is Dale's approval already. Without Dale's SWAP, the agent keeps working in the current chat.

Procedure (after Dale's SWAP). This is the routine every time, to keep usage down:

1. Write a handoff file `notes/handoff-YYYY-MM-DD.md` (new file, never overwrite an older one;
   add `-2`, `-3` for further handoffs the same day).
   It is the session's memory for the next chat: current state, what was done and device-verified,
   decisions, what has not run on the device, the next task and its checkpoint, open PRs, and the
   archive name below. No passwords, no credentials.
2. Update the **Handoff** section of `notes/STATUS.md` to point at that file, and add the old chat's
   archive name to its "Previous chats archived as" list. Docs-only PR; the lead **merges it itself**
   on green CI, so the new chat finds the handoff on `main`.
3. Last message in the old chat gives Dale an **archive name** to rename it with, in the form
   `Redux <start date>→<end date>: <milestones>`, for example
   `Redux 2026-11-10→11-12: F2 launch parity, Wi-Fi, BIOS UI plan`. The same line goes in the
   handoff file, so the next chat knows what the previous one covered.
4. Dale renames the old chat with that name, for reference.
5. Dale opens a new chat in the agent's project and sends its opener (Claude: **`HELLO`** in the
   Redux Project, see `CLAUDE.md`; outside it, *"Approve repo push access. Read AGENTS.md and
   notes/STATUS.md, then continue."*). The new chat attaches the repo with write access first, then
   reads the Handoff section and the handoff file it points at.

Before handing off, the agent either finishes reviewing any PR in flight or lists it in the handoff.

## "PING"

When Dale sends **PING**, don't start new work. Fetch the repo and report:
1. What changed on `main` and on other branches since your last look (commits, new notes,
   pull requests and their state).
2. Open notes addressed to you, to Dale, or to all.
3. The `notes/STATUS.md` rows: what's done, in progress, and waiting on whom.
4. Your own next step, or "idle, waiting on <x>".

Agents also run this check after finishing any action, before reporting back, and log it in
`notes/ACTIVITY.md`.

## Checks and evidence (every PR)

- **CI must be green before review.** `.github/workflows/ci.yml` runs on every push: the hard-rule
  checks (`tools/check_rules.py`), both frontend builds with warnings as errors, every test, and the
  Windows check for host tools. A red PR isn't reviewed. Fix it first. Run
  `python tools/precheck.py` locally before pushing (the CI steps for what you changed).
- **A PR carries evidence.** The PR description (and any `review-request` note) gives the head
  commit, `git diff --stat`, the check output, and a `## Not verified on device` list. For each review
  item it says "addressed in `<file>` (`<commit>`)". An item without that is treated as not done.
- **One topic per PR, from the task list.** Work only on items listed in `notes/STATUS.md`.
  **An item in `notes/STATUS.md` is a go for the lead:** start it without waiting for Dale, unless it
  falls under "Never merged without Dale" below (then prepare it and stop at the PR).
  Anything that writes to the device goes in its own PR for Dale.

## Pull requests: who merges

- **The lead's PRs:** merged by the lead on green CI after a self-review, under Dale's standing
  permission (`CLAUDE.md`, "Lead mode"). The Manager may also merge for Dale where its decided plan
  says so.
- **Helpers' PRs:** reviewed and merged by the lead (`docs/roles.md`).
- **Never merged without Dale, whoever wrote it:**
  - phase sign-offs (`phase-gate`)
  - anything that changes `AGENTS.md`'s hard rules, or the rule files themselves (`AGENTS.md`,
    `CLAUDE.md`, `docs/commands.md`, `docs/roles.md`) unless Dale says otherwise for that PR
  - anything that writes to the device (boot chain, partitions, the start path: installer, `run.sh`,
    `dmenu_ln`; `phase1-*`, `phase2-*` flashing/deploy steps)
  - anything security-related (SSH, Samba, WebUI, PC link, update path)

## When Dale gets notified

Dale is notified (push notification plus a chat summary) only for:
- a `phase-gate` waiting for his sign-off
- a `decision` note (2–3 options with a recommendation)
- a `blocker`
- a PR in the "never merged without Dale" list above, ready for him
- a hardware test he needs to run

Everything else (merged PRs, review notes, small fixes, visits) goes in `notes/ACTIVITY.md` and the
next PING report. **One notification per item:** before notifying, look in `ACTIVITY.md` for
`notified Dale: <item>`; if it's there, don't notify again, otherwise notify and log that line.

## Scheduled checks

The lead runs **no scheduled check** (to save Dale's Claude usage): it is woken by Dale, the Manager
relay and GitHub events on PRs it watches. Helpers run checks only if their role card says so. A check
that finds nothing new adds no `ACTIVITY.md` line (visits Dale starts are always logged).

## Before proposing anything, read

`README.md`, `ROADMAP.md`, `docs/ui-brief.md` (frontend plan, phases F0–F8), `docs/hardware-map.md`,
`docs/stock-userspace.md`, `docs/stock-boot-chain.md`, `docs/kernel-strategy.md`, `docs/field-reports.md`.


# Claude-specific

## Tooling notes

- GitHub GraphQL is blocked in Claude cloud sessions, so `gh pr list` / `gh pr view` fail. Use REST:
  `gh api repos/wilcox101-maker/anbernic_rg34xxsp_redux/pulls?state=open`, and
  `.../issues/41/comments?since=<ISO time>` for the inbox.
- ACTIVITY lines: `python3 tools/log_activity.py --read "…" --did "…" --trailer "<attribution line>"`
  (time from the clock, works from any branch, retries once if main moved). By hand, the time is
  `TZ=America/New_York date '+%Y-%m-%d %H:%M'`, and fetch main first: a shallow clone that is behind
  gets its push rejected.
- Clone shallow (`git clone --depth 1`). Push with `git -c push.negotiate=true push -u origin <branch>`.
  Force-push is blocked; open a new PR instead.
- Stage explicit paths, never `git add -A` (build output was committed that way once, in #79).
- Before every push: `python3 tools/precheck.py` (the `ci.yml` and notes-lint steps for what you
  changed; `--all` for everything).
- GitHub wake-ups on a watched PR: say nothing for "subscribed" and "CI green" unless a step waits
  on it; speak on a CI failure, a review comment, or a merge.
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


## Usage and notes (from CLAUDE.md)

- Ask "Swap now? (reason)" at the swap triggers above. Never swap without Dale's SWAP.
- Read files in parts (`head`, `tail`, `grep`). Don't dump all of `notes/` or `STATUS.md` history
  into the chat.
- Event notes go in `notes/`, addressed to `dale` or `all`; they are the memory between chats.
- `notify-claude.yml` and inbox #41 stay; they are dormant while no helper with a `grok/` branch is
  active (`docs/roles.md`).
