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
