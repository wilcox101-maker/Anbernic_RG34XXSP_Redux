# Activity log

Every agent appends **one line per visit** to the repo: when they checked in, what they read, and
what they did. Newest at the bottom. Read the last 20 lines at the start of every session, so you
know who was here and what they touched since you last looked.

**This file is the one exception to the branch-and-PR rule:** agents may append their own line and
push it straight to `main`. Append only. Never edit or remove anyone else's lines. A visit that also
changed something links its PR or commit. When the file passes about 200 lines, the next agent
moves everything except the last 50 lines into `notes/activity-archive/YYYY-MM.md` (that cleanup
goes through a PR).

Time is Dale's local time (America/New_York). Format:
`- YYYY-MM-DD HH:MM · <agent> · read: <what> · did: <what, or "nothing">`

- 2026-11-10 08:30 · claude · read: stock inventory · did: docs/stock-userspace.md, tools/stock_inventory.py (pushed to main before the PR rule existed)
- 2026-11-10 09:05 · claude · read: — · did: AGENTS.md, notes/README.md, notes/STATUS.md, F0 hand-off note to grok (main, 331bff9)
- 2026-11-10 09:25 · claude · read: STATUS, notes · did: docs/ram-system-design.md + milestone note (PR #1)
- 2026-11-10 09:39 · claude · read: STATUS, notes, branches, PRs (PING) · did: nothing; no grok activity yet
- 2026-11-10 09:43 · claude · read: STATUS, notes, PRs · did: this activity log, AGENTS.md rule for it (PR #1)
- 2026-11-10 09:46 · claude · read: ACTIVITY, STATUS, grok milestone note (ea42df4, f08c09e), PRs · did: acked grok's note, reviewed F0 tree → message note to grok, merged main into PR #1 (STATUS conflict resolved)
- 2026-11-10 09:48 · claude · read: PR #1, branches, notes · did: merged PR #1 at Dale's request (d55458a)
- 2026-11-10 09:48 · grok · read: STATUS, Claude tree-review note, Claude RAM design note, ACTIVITY · did: Dale locked UI tree; accepting Claude's five flags for ui-spec.md
- 2026-11-10 09:52 · claude · read: ACTIVITY, branches, PRs (PING) · did: nothing; found grok/f0-spec (ui-spec + review-request + phase-gate), no PR open yet
- 2026-11-10 09:55 · claude · read: ACTIVITY, main, branches, PRs (PING) · did: nothing; PR #2 (grok F0 ui-spec) merged, my review-request still open
- 2026-11-10 10:05 · claude · read: ACTIVITY, grok review-request + phase-gate, docs/ui-spec.md, stock system.cfg + scripts · did: reviewed F0 spec, fixes + review note in PR #3
- 2026-11-10 10:02 · claude · read: open PRs, branches · did: PR merge/notify policy added to AGENTS.md (PR #3); 2-hourly repo scan for grok/ PRs scheduled
- 2026-11-10 10:03 · grok · read: PR #3 diff, AGENTS.md (new PR/notify sections), ACTIVITY · did: approved PR #3 in review note (grok/f0-spec-review-reply)
