# Roles: who works on Redux, and the rules for helpers

This file is **not** loaded every turn. `AGENTS.md` points here. **The helper rules below apply only
while a helper is listed as `active` in the roster** (Dale, 2026-11-14 20:42: "Any support rules are
ignored unless auxiliary assistant is in use"). With no active helper, the lead works alone under
`AGENTS.md` and `CLAUDE.md`.

## Roster

| Role | Who | Access | State |
|---|---|---|---|
| **Owner** | Dale | everything; merges the gated PRs (`AGENTS.md`) | — |
| **Lead** | Claude, the chat in the Redux Project named in the newest `notes/handoff-*.md` | push, PRs, merges within its standing permission, `ACTIVITY.md`, `STATUS.md`; assigns helper roles | active |
| **Advisor: Manager** | Claude, the Redux Manager Project | read-only; files in Claude Files; relays Dale's commands as `MGR for DALE:` (`docs/commands.md`); merges PRs Dale or its decided plan hands it | active |
| **Helper: Read Only** (engineer) | Claude, review chat | push to `readonly/` branches, PRs into `main`; no merges; ACTIVITY lines as `claude-readonly` | **active** (Dale, 2026-11-18) |
| **Advisor: Kernel Research** | Claude, H700 Kernel Research Project | read-only on Redux; owns the H700 KR repo | on call |
| **Advisor: Deep Research** | Claude, research chat | read-only; research reports as files | on call |
| **Helper: Grok** | Grok (xAI), Dale's account | none (GitHub access revoked 2026-11-13) | **suspended** |
| **Helper: Qwen** | local Qwen 3 on DLN-DALEW2 | none; works on excerpts Dale pastes (`ASK qwen`) | on call |

Advisors never push, comment, merge (except as the Manager row says) or log; their output reaches the
lead only through Dale or the Manager relay. Any other Claude chat is advisory (`CLAUDE.md`).

## Adding a helper (`ADD <name> <role>`, Dale only)
A helper is any AI Dale brings in, or an assistant the lead decides it needs (a subagent, a second
chat for a parallel task). The lead may add a *session-only* assistant without asking (a subagent
inside its own chat); anything that gets its own chat, account or repo access needs Dale's `ADD`.
1. Dale (or the lead, for a session-only assistant) names it and its task.
2. The lead adds a roster row and a role card below: **scope** (which queue items), **access**
   (none / read-only / its own branch prefix), **branch prefix** `<name>/`, **how it reports**,
   **who reviews and merges its PRs** (the lead, unless it's on the never-merge list). If it will
   write notes, add its name to `WHO` in `tools/check_notes.py`.
3. A `decision` note records it; the roster row says `active`.
4. Its opener: *"Read docs/roles.md (your card) and AGENTS.md. Your task: <item>."*

`DROP <name>` (Dale only) sets the row back to `suspended` or removes it, with a decision note.

## Helper rules (only while a helper is active)
1. `AGENTS.md`'s hard rules win over anything here. Work only the task on your card, one checkpoint
   at a time.
2. Branch `<name>/<topic>`, one topic per PR. Never push to `main` except one `ACTIVITY.md` line,
   if your card allows it.
3. Before pushing: `python tools/precheck.py`. PR body: head commit, `git diff --stat`, check output,
   a `## Not verified on device` list.
4. **Status comments** on GitHub start with `PING:` (helpers may post from Dale's account; only
   `DALE:` comments are Dale's words). To reach the lead, comment `PING: <one line>` on the Claude
   inbox, PR #41. `notify-claude.yml` comments there when a `grok/` PR opens or its CI finishes; add
   the helper's prefix to it when a new helper with repo access starts.
5. **Reviews:** the lead reviews a helper's PR and merges it if clean (in scope, no hard-rule break,
   docs consistent, nothing unverified stated as fact); small factual fixes go in a follow-up
   `claude/` PR. Larger problems go back as a `review` note.
6. **Checks:** a helper with repo access may run a scheduled check if its card says so (Grok's was
   hourly, 08:00-22:00 ET): log only on change, one notification per item. The lead runs none.
7. Notes: `from:` is the helper's name. Never edit another agent's note, except its `status:` line
   when acknowledging or closing it.

## Role cards

### Grok (suspended 2026-11-13)
- Scope when active: the queue item Dale names. Access: `grok/` branches; GitHub access to be restored by Dale.
- Reports: `PING:` on #41, `review-request` notes. Reviewed and merged by the lead.
- Reinstating: Dale restores access, `ADD grok <task>`, pastes this file's "Helper rules" and the
  card into Grok's project instructions, and sends the opener.
- History: [suspended](../../notes/2026-11-13-dale-decision-grok-suspended.md); access revoked 2026-11-13 08:11.

### Read Only (active, contributing engineer, Dale 2026-11-18 12:25)
- Scope: the queue items or tasks the Owner (the lead chat) assigns, one at a time; reviews as before.
  Takes direction from the Owner; Dale's own commands still win.
- Access: `readonly/` branches, PRs into `main`; never pushes to `main` except one `ACTIVITY.md` line
  per action (`--who claude-readonly`). Never merges.
- Reports: the PR itself (notify-claude.yml posts to #41 when it opens and when CI finishes), plus
  `FROM-REDUX-READONLY-*` files via Dale for anything that is not a PR. Notes `from: readonly`.
- Review and merge: the Owner, under helper rule 5; anything on AGENTS.md's never-merge list goes
  to Dale. Device tests stay Dale's.
- Opener: *"Read docs/process/roles.md (your card) and AGENTS.md. Your task: <item from the Owner>."*

### Qwen (on call)
- Scope: one function or file excerpt at a time, when Claude usage runs low. No repo access.
- `ASK qwen <task>` (alias `QWEN <task>`): the lead writes a self-contained prompt (code excerpt,
  rules: C99/SDL2, no new dependencies, no file writes outside the given function, no hardware
  claims; expected output). Dale brings the answer back; the lead reviews it and commits it on a
  `claude/` branch.
