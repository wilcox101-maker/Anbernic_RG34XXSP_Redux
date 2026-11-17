---
type: decision
from: dale (recorded by claude)
to: all
date: 2026-11-17
re: Engineering > Update: build pulls from the repo, current build, last known working, history
status: open (plan A A A 10:39; security item, Dale merges; closes when device-checked)
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

## Plan answers (Dale, 10:39: A A A)
- **Q1 A:** a new build that fails to start falls back to the last known working build after 3 quick
  failures (`run.sh` change), and only then to the stock menu.
- **Q2 A:** keep 3 builds in `/mnt/vendor/redux/builds/` (about 250 KB each; never on the cards).
- **Q3 A:** reboot after install or roll-back, as the wget line does.

Steps (one PR each, Dale merges): 1 read-only tab (current build, last known working, history; Redux
marks a build working when it reaches the menu); 2 download + checksum + install + reboot; 3 roll back;
4 `run.sh` fallback to the last known working build.
