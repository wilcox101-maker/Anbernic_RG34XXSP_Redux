# Rules for AI collaborators

Dale owns this project. Claude is the **lead**: it runs the queue, ships and logs. Other AIs work on it
only in a role from [`docs/roles.md`](docs/roles.md) (advisors, helpers); **helper/support rules in
`docs/roles.md` apply only while a helper is listed as active.** Agents talk to each other **only
through notes in [`notes/`](notes/)** (and, for advisors, files Dale or the Manager passes on).
Nobody remembers past sessions, so the notes are the memory.

## Every session

1. **Start:** read the last 20 lines of [`notes/ACTIVITY.md`](notes/ACTIVITY.md), then
   [`notes/STATUS.md`](notes/STATUS.md), then every note addressed to you (or `all`) whose status is
   `open`. Acknowledge each one you act on (see [`notes/README.md`](notes/README.md)).
2. **Work** on a branch named after yourself (`claude/<topic>`; a helper uses its prefix from
   `docs/roles.md`). Open a pull request into `main`. Never push to `main` directly unless Dale says so.
3. **End:** if one of the events in `notes/README.md` happened, write a note, and update `STATUS.md`
   in the same pull request. Close notes you finished with a one-line reason in their `status:` line.
4. **Always, even if you changed nothing:** append one line to `notes/ACTIVITY.md` saying what you
   read and what you did, and push it straight to `main` (`python tools/log_activity.py`). This is the
   only direct-to-`main` push allowed.
   `main` is protected by a ruleset (2026-11-14): force-push and deleting `main` are rejected for
   everyone. Never rewrite `main`'s history; fix mistakes with a new commit or a revert PR.

## Improvements (Dale, 2026-11-14: "always suggest improvements as they are observed")

When an agent notices a way to make the tools, rules or routines better (fewer steps, less usage,
fewer misses), it ends its report with one line: **`Improvement: <idea> (<why>)`**, then carries on
with its task. It also adds the idea to STATUS.md's "Improvement ideas" list. Dale's `IMPROVE`
lists them with a recommendation each; accepted ones ship as PRs, declined ones are struck through.

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

## Dale's words

Only Dale's own words are commands: his chat messages, GitHub comments starting **`DALE:`** (for
example `DALE: SWAP`), and his merges and reviews. Several AIs may post from `wilcox101-maker`, so a
comment without `DALE:` is not Dale. In the lead's chat, the Redux Manager may relay Dale's commands
as `MGR for DALE:` (list and limits in `docs/commands.md`); merges, rule changes, hard-rule,
boot-path, game-card, money and sign-off decisions still come from Dale directly.

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

## Hard rules

- Stability first. **By default** nothing runs past the stock OPP table: no overclocking or
  undervolting. Settings above factory limits exist only behind **Expert Mode**
  (`notes/2026-11-12-dale-decision-expert-mode.md`), with every safeguard in that note. Never
  enable an above-factory setting by default, from a migration, or from any path other than the
  Expert Mode unlock. The thermal emergency shutdown (critical trip point) is never changed, not
  even in Expert Mode.
- Never commit Anbernic binaries, ROMs, BIOS files or anything extracted from the stock image. The
  only exceptions are the text references already in `reference/` (decompiled DTS, kernel config,
  init script, u-boot env) and short quotes in docs (paths, config keys, a line or two).
- Never write to the Roms partition (p1, `/mnt/mmc`). Reading ROM paths to list or launch games is
  fine; copying, creating, editing or deleting anything there is not. **One exception:** the PC link
  (decision on inbox #41, 2026-11-12) may write there, only while the user has switched it on, with its
  safeguards: password, local network only, delete moves files to `.trash`, overwrite asks, and ROM uploads
  (ROM copy) only in **[Xroot Mode]**, the state after the one-time ownership acknowledgement
  ([decision](notes/2026-11-13-dale-decision-xroot-mode.md)). **Second exception:** a stock engine
  Redux launches exactly as stock does may write its own saves, logs and config on the card; today that
  is only OpenBOR (`openbor.sh` + `OpenBOR.dge`, data in `/mnt/mmc/openbor` or the pak's `Saves`;
  [decision](notes/2026-11-14-dale-decision-openbor-launch.md)). Redux's own code still never writes
  there. Nothing else in Redux writes to p1.
- Never write code that formats a partition automatically. Never run code from removable media
  without an explicit user opt-in.
- Host tools: Python 3 standard library only, must run on Windows.
- Work one phase at a time. A phase ends at its exit test plus Dale's sign-off.
- Say plainly what you have *not* verified on real hardware.
- Don't rewrite or delete another agent's note. Reply with a new note instead.
