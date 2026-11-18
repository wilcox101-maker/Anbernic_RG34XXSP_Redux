---
type: decision
from: dale
to: all
date: 2026-11-18
re: Claude inbox moves from draft PR #41 to issue #303 (Dale 14:07 "Consider removing PR #41"; picked "Move to an issue")
status: open
---

# Decision: the Claude inbox is issue #303

- The inbox (helper `PING:` comments, `notify-claude.yml` announcements, `DALE:` GitHub commands) is now issue
  **#303**, "Claude inbox (do not close)". Draft PR #41 closes once this lands; its 126 comments stay readable.
- Updated: `notify-claude.yml` (`INBOX: 303`; the message now names the helper's prefix instead of always "Grok"),
  `CLAUDE.md`, `docs/process/{commands,roles,process,agent-prompts}.md`. Older notes keep "#41" as history.
- The `redux-session` skill's inbox line is updated through a skill proposal (Dale saves it).
