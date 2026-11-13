# Agent prompts: copy and paste

Short openers Dale sends. The rules live in `AGENTS.md` and `CLAUDE.md`. Current setup: Claude solo,
Grok suspended (see `GROK.md` to reinstate).

## Claude: new owning chat (after SWAP)
Inside the Redux Project (instructions do the rest, see `docs/project-setup.md`):
> HELLO

Outside the Project:
> Approve repo push access. Read AGENTS.md and notes/STATUS.md, then continue.
> Watch the Claude inbox, PR #41.

## Claude: side or advice chat (another Claude owns the repo)
> Read-only advisory session for the Redux repo. Clone it, but don't push, comment or log.
> Question: <...>. Give me text or files to pass to the owning chat.

## Claude: model switch (answering a "Model switch?" checkpoint)
> SWITCH        (then change the model in the picker and reply "switched")
> NO SWITCH

## Claude: status check
> PING

## Claude: hardware results
> Device test, build <sha>: <item> PASS / FAIL <what you saw>. Write the hw-result note.

## Local Qwen (offline, no repo access; for when Claude usage runs low)
> You are helping on a C99/SDL2 handheld frontend. No new dependencies, no file writes outside the
> function I give you, no hardware claims. Task: <function or file excerpt>. Return only the changed code.
Bring Qwen's output to the owning Claude chat, which reviews it and commits it on a `claude/` branch.
