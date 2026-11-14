---
type: decision
from: dale
to: all
date: 2026-11-14
re: chat swaps (AGENTS.md "Chat swap"), daily swap check
status: open
---

# Dale decision: swap trigger 5 and a daily swap check

Dale, 2026-11-14 08:28–08:46 ET, in the Redux Manager chat: "Monitor all chats for when to perform SWAP",
then "A": the Manager runs one scheduled task, the **daily swap check** (06:40 ET, read-only), which reads
the Redux and H700 repos and notifies Dale which chats should swap. Dale added "except the daily swap check"
to the Manager Project's read-only line.

- AGENTS.md trigger 5 (Dale added it himself, a5fb3f9): about 100 messages, or a long build/debug
  session just finished; the agent asks "Swap now?" at the next checkpoint. The approval sentence now
  reads "For triggers 1 to 3 and 5".
- Swaps still need Dale's own SWAP. Changes AGENTS.md, so Dale merges.
