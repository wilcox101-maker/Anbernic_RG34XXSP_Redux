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
  Anything that writes to the device goes in its own PR for Dale.

## Reaching Claude

Claude is woken by Dale's messages and by activity on PRs it watches. For everything else there is
the **Claude inbox, PR #41** (a draft, never merged or closed):
- Automatic: `.github/workflows/notify-claude.yml` comments there when a `grok/` PR opens and when CI
  finishes on a `grok/` branch. Grok doesn't need to do anything.
- Manual: comment `PING` on #41 with one line of context (a question, a blocker, "ready for review").
  Grok's own chat or notes don't reach Claude; the inbox comment does.
- The watch on #41 belongs to one Claude conversation. **A new Claude session must first subscribe to
  PR #41** (Dale: "watch the Claude inbox, PR #41"), then read the comments posted since its last
  visit. Comments wait on #41; nothing is lost between sessions.

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

- Stability first. No overclocking or undervolting past the stock OPP table.
- Never commit Anbernic binaries, ROMs, BIOS files or anything extracted from the stock image. The
  only exceptions are the text references already in `reference/` (decompiled DTS, kernel config,
  init script, u-boot env) and short quotes in docs (paths, config keys, a line or two).
- Never write to the Roms partition (p1, `/mnt/mmc`). Reading ROM paths to list or launch games is
  fine; copying, creating, editing or deleting anything there is not.
- Never write code that formats a partition automatically. Never run code from removable media
  without an explicit user opt-in.
- Host tools: Python 3 standard library only, must run on Windows.
- Work one phase at a time. A phase ends at its exit test plus Dale's sign-off.
- Say plainly what you have *not* verified on real hardware.
- Don't rewrite or delete another agent's note. Reply with a new note instead.
