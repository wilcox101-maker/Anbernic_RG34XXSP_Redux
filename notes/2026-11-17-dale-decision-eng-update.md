---
type: decision
from: dale (recorded by claude)
to: all
date: 2026-11-17
re: Engineering > Update: build pulls from the repo, current build, last known working, history
status: open (starts right after F5 sign-off; security item, Dale merges)
---

# Decision: on-device build updates in Engineering (Dale, 2026-11-17 10:08, A)

Dale asked for, in Engineering: access to his repo for build pulls; the current build shown; the last
known working build; and a history. Timing **A**: its own item right after F5 sign-off (not F5 step 6).

Plan (to confirm with Dale before code):
- **Update tab** in Engineering: current build (SHA), last known working (last build that reached the menu),
  history (each install: SHA, time, reached the menu yes/no).
- **A: pull** the newest `dev` build from `wilcox101-maker/Anbernic_RG34XXSP_Redux` releases, checksum-checked
  against `SHA256SUMS.txt`, installed as `.new` + rename, as the wget line does today.
- **Roll back** to any history entry that reached the menu; the replaced build is kept for that.
- Same feature as the backlog "Update option in Settings > System" (that one: release builds later).
- AGENTS.md: update path = security item; Dale merges the PRs.
