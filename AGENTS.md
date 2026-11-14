# Rules for AI collaborators (Claude, Grok, any other)

Dale owns this project and merges every change. Two or more AIs work on it, and they talk to each
other **only through notes in [`notes/`](notes/)**: event notes at milestones, plus direct `message`
notes for needs, concerns and comments at any time. Nobody remembers past sessions, so the notes are
the memory.

## Every session

1. **Start:** read the last 20 lines of [`notes/ACTIVITY.md`](notes/ACTIVITY.md), then
   [`notes/STATUS.md`](notes/STATUS.md), then every note addressed to you (or `all`) whose status is
   `open`. Acknowledge each one you act on (see [`notes/README.md`](notes/README.md)).
2. **Work** on a branch named after yourself: `grok/<topic>` or `claude/<topic>`. Open a pull request
   into `main`. Never push to `main` directly unless Dale says so.
3. **End:** if one of the events in `notes/README.md` happened, write a note, and update `STATUS.md`
   in the same pull request.
4. **Always, even if you changed nothing:** append one line to `notes/ACTIVITY.md` saying what you
   read and what you did, and push it straight to `main`. This is the only direct-to-`main` push
   allowed.

## Chat swap (keeping sessions lean)

A long chat costs usage on every turn, because the whole conversation is re-read each time. Swap to
a fresh chat when **any** of these happens:
1. **A milestone closes**: a phase or step is signed off, or a batch of PRs is merged and deployed.
2. **The chat was compacted** ("continued from a previous conversation"). After one compaction,
   swap at the next milestone.
3. **A new workday starts.**
4. **Dale says "SWAP"** at any time.

**Dale approves every swap.** For triggers 1 to 3, the agent does not swap on its own. It asks
*"Swap now? (reason)"* and waits. Dale replies **SWAP** to approve, or **NO SWAP** to keep going.
Trigger 4 is Dale's approval already. Without Dale's SWAP, the agent keeps working in the current chat.

Procedure (after Dale's SWAP). This is the routine every time, to keep usage down:

1. Write a handoff file `notes/handoff-YYYY-MM-DD.md` (new file, never overwrite an older one;
   add `-2`, `-3` for further handoffs the same day).
   It is the session's memory for the next chat: current state, what was done and device-verified,
   decisions, what has not run on the device, the next task and its checkpoint, open PRs, and the
   archive name below. No passwords, no credentials.
2. Update the **Handoff** section of `notes/STATUS.md` to point at that file (current state, device
   location, deploy rule, lessons, decisions, queue). Docs PR; Dale merges it.
3. Last message in the old chat gives Dale an **archive name** to rename it with, in the form
   `Redux <start date>→<end date>: <milestones>`, for example
   `Redux 2026-11-10→11-12: F2 launch parity, Wi-Fi, BIOS UI plan`. The same line goes in the
   handoff file, so the next chat knows what the previous one covered.
4. Dale renames the old chat with that name, for reference.
5. Dale opens a new chat in the agent's project and sends its opener (Claude: **`HELLO`** in the
   Redux Project, see `CLAUDE.md`; outside it, *"Approve repo push access. Read AGENTS.md and
   notes/STATUS.md, then continue."*). The new chat attaches the repo with write access first, then
   reads the Handoff section and the handoff file it points at. Nothing else is needed; the inbox
   #41 and notes/ carry the detail.

Before handing off, the agent either finishes reviewing any PR in flight or lists it in the handoff.
Without Dale's SWAP, the agent keeps working in the current chat.

## Dale's comments on GitHub

Dale and Grok both post from the account `wilcox101-maker`. To tell them apart:
- Grok's status comments start with **`PING:`**.
- Dale starts his own comments with **`DALE:`**, for example `DALE: #89 tested OK on device` or
  `DALE: SWAP`.
Agents treat only `DALE:` comments, merges and reviews from that account as Dale's own words.

## "PING"

When Dale sends **PING**, don't start new work. Fetch the repo and report:
1. What changed on `main` and on other agents' branches since your last look (commits, new notes,
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
  `python tools/check_rules.py` and the tests locally before pushing.
- **A review request carries evidence.** The `review-request` note (and the PR description) gives
  the head commit, `git diff --stat`, and the pasted output of the build and the tests. For each
  review item it says "addressed in `<file>` (`<commit>`)". An item without that is treated as not
  done.
- **One topic per PR, from the task list.** Work only on items listed for you in `notes/STATUS.md`.
  **An item in `notes/STATUS.md` marked with your name is a go:** start it without waiting for
  Dale, unless it falls under "Never merged without Dale" below (then prepare it and stop at the PR).
  Anything that writes to the device goes in its own PR for Dale.

## Reaching Claude

Claude is woken by Dale's messages and by activity on PRs it watches. For everything else there is
the **Claude inbox, PR #41** (a draft, never merged or closed):
- Automatic: `.github/workflows/notify-claude.yml` comments there when a `grok/` PR opens and when CI
  finishes on a `grok/` branch. Grok doesn't need to do anything.
- Manual: comment `PING` on #41 with one line of context (a question, a blocker, "ready for review").
  Grok's own chat or notes don't reach Claude; the inbox comment does.
- The watch on #41 belongs to one Claude conversation. **A new Claude session subscribes to PR #41
  if it has a tool for that**, and in any case reads the comments posted since its last visit by
  REST at session start. Comments wait on #41; nothing is lost between sessions.

## Pull requests: who reviews and merges

- **Grok's PRs:** Claude reviews them and, if they're clean, **merges them**. Clean means: inside the
  current phase's scope, no hard-rule breaks, docs consistent with `docs/`, nothing unverified
  presented as fact. Claude fixes small factual issues in a follow-up `claude/` PR rather than
  blocking. Anything larger goes back to Grok as a `review` note asking for changes.
- **Claude's PRs:** Dale merges them (or tells Claude to).
- **Never merged without Dale, whoever wrote it:**
  - phase sign-offs (`phase-gate`)
  - anything that changes `AGENTS.md`'s hard rules
  - anything that writes to the device's boot chain or partitions (`phase1-*`, `phase2-*`
    flashing/deploy steps)
  - anything security-related (SSH, Samba, WebUI, update path)

## When Dale gets notified

Dale is notified (push notification plus a chat summary) only for:
- a `phase-gate` waiting for his sign-off
- a `decision` note (2–3 options with a recommendation)
- a `blocker`, including a Grok PR sent back for changes that hasn't moved in a day
- a PR in the "never merged without Dale" list above, ready for him
- a hardware test he needs to run

Everything else (merged Grok PRs, review notes, small fixes, visits) goes in `notes/ACTIVITY.md`
and the next PING report. No notification.

## Scheduled checks (both agents run them)

- Grok checks every hour, 08:00–22:00 America/New_York. **Claude runs no scheduled check** (to save
  Dale's Claude usage): it is woken by GitHub events on PRs it watches and by Dale's PING. So when a
  `grok/` PR is ready, leave a `review-request` note. Dale's next PING brings Claude to it.
- **Log only on change.** A scheduled check that finds nothing new adds no `ACTIVITY.md` line. A
  check that finds or does something logs one line, as usual. (Visits Dale starts are always logged.)
- **One notification per item.** Before notifying Dale, look in `ACTIVITY.md` for
  `notified Dale: <item>` (for example `notified Dale: F0 phase-gate`). If it's there, don't notify
  again. If not, notify and add that line in the same push.
- Grok's checks review Claude's PRs (a `review` note) but don't merge them. Claude's checks review
  and merge clean `grok/` PRs, per the section above.

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
  ([decision](notes/2026-11-13-dale-decision-xroot-mode.md)). Nothing else in Redux writes to p1.
  **Second exception, stock engines' own data:** a stock engine Redux launches may write its own saves,
  logs, screenshots and config where it is built to (OpenBOR: `/mnt/mmc/openbor`, and the per-game
  `.cfg`/`.controls` its stock `openbor.sh` seeds); Redux's own code still never writes there
  ([decision](notes/2026-11-14-dale-decision-openbor-launch.md)).
- Never write code that formats a partition automatically. Never run code from removable media
  without an explicit user opt-in.
- Host tools: Python 3 standard library only, must run on Windows.
- Work one phase at a time. A phase ends at its exit test plus Dale's sign-off.
- Say plainly what you have *not* verified on real hardware.
- Don't rewrite or delete another agent's note. Reply with a new note instead.
