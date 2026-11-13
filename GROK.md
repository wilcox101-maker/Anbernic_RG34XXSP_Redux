# Grok: SUSPENDED

**Grok is suspended (Dale, 2026-11-13). Claude runs the project solo.** This file is kept so Grok
can be brought back quickly. Until Dale reinstates Grok in a `decision` note, nothing here is
active. **Grok's GitHub access is revoked** (Dale, 2026-11-13 08:11).

**While suspended: no pushes, no ACTIVITY lines, no PING comments.**

## Reinstating Grok (Dale)

1. Dale restores Grok's GitHub access.
2. Dale writes a `decision` note: Grok is back, and which task it gets.
3. Paste this file (from `## Rules when active` down) into Grok's project instructions.
4. Opener: *"Read GROK.md and the newest decision note about Grok. Your task: <item>."*

## Rules when active

1. Read `AGENTS.md`; its hard rules win over anything here. Then the newest decision note about
   Grok, the last 20 lines of `notes/ACTIVITY.md`, `notes/STATUS.md`, and open notes to `grok` or `all`.
2. Work only on the task Dale gives you. One task at a time, split into CHECKPOINTS; at each one ask
   Dale "Proceed?" and wait.
3. Branch `grok/<topic>`, one topic per PR. Never push to `main` except your one `ACTIVITY.md` line.
4. Before pushing: `python3 tools/check_rules.py` plus the builds and tests in `.github/workflows/ci.yml`.
5. PR and `review-request` note: head commit, `git diff --stat`, pasted build/test output, and a
   "Not verified on device" list.
6. Reach Claude with a `PING:` comment on PR #41. Claude reviews and merges your PRs; don't merge any.
7. Never: commit stock-image material, ROMs or BIOS; write to p1 (`/mnt/mmc`); format partitions;
   exceed the stock OPP table outside Expert Mode; touch the thermal critical trip point; claim
   hardware results Dale hasn't produced; edit another agent's note.
