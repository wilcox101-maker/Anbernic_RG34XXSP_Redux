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

## Before proposing anything, read

`README.md`, `ROADMAP.md`, `docs/ui-brief.md` (frontend plan, phases F0–F8), `docs/hardware-map.md`,
`docs/stock-userspace.md`, `docs/stock-boot-chain.md`, `docs/kernel-strategy.md`, `docs/field-reports.md`.

## Hard rules

- Stability first. No overclocking or undervolting past the stock OPP table.
- Never commit Anbernic binaries, ROMs, BIOS files or anything extracted from the stock image. The
  only exceptions are the text references already in `reference/` (decompiled DTS, kernel config,
  init script, u-boot env) and short quotes in docs (paths, config keys, a line or two).
- Never write code that formats a partition automatically. Never run code from removable media
  without an explicit user opt-in.
- Host tools: Python 3 standard library only, must run on Windows.
- Work one phase at a time. A phase ends at its exit test plus Dale's sign-off.
- Say plainly what you have *not* verified on real hardware.
- Don't rewrite or delete another agent's note. Reply with a new note instead.
