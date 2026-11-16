---
type: hw-result
from: dale (recorded by claude)
to: all
date: 2026-11-16
re: device session on 2bcdd3f then 6d87953 (F4 steps 1-4, #200 #202 #203 #205, crash fix #216); #185
status: open
---

# Device session 14:07-15:05 (Dale; deployed with wget on the device)

| # | Check | Result |
|---|---|---|
| 1 | `/tmp/redux-cpufreq` baseline, `/tmp/redux-lidwatch` installed (2bcdd3f) | PASS: `schedutil 480000 1512000`, `-rwxr-xr-x` |
| 2 | Card picker TF1/TF2, lists, launches (2bcdd3f) | PASS |
| 3 | Back from a game: same card and list, no freeze | **FAIL on 2bcdd3f**: RetroArch exit froze ~30 s, dropped to Main; every Card Select did the same. `redux.log`: `exit 139` ×3 (+1 more before redeploy). Cause: cache load overflow, fixed in #216. **PASS on 6d87953** |
| 4 | Ports on TF2 without a switch, one starts; no Card Scripts row | PASS (6d87953) |
| 5 | Clean exit does not relaunch the last app; volume; no amixer lines | PASS |
| 6, 7 | Dim after 1 min, first button restores; game from dimmed menu at full brightness | PASS |
| 8 | Governor set to `performance` in a game, back in Redux | PASS: `[cpufreq] app left performance 480000-1512000; restored schedutil 480000-1512000: ok`, reads `schedutil` |
| 9, 10, 11 | Lid in menu sleeps once and stays awake after wake; short Power sleeps; lid closed at power-on: no sleep | PASS |
| 12 | Save on lid close (SMW, then Pokémon Crystal) | `[lidwatch] lid closed: SAVE_FILES sent, sync done` every close; save-before-sleep timing not shown. **Waived by Dale** (14:59: "I don't care about Lid close saves"). In-game saves and save states both work. |
| 13 | No watcher left in the menu | PASS (`pidof redux-lidwatch` empty in the menu) |
| 14 | **F4 exit test**: 50 lid cycles mid-game | PASS (Dale, 15:05): all slept and woke; governor `schedutil` after; no `exit 139` on 6d87953 |

- `exit 138` at 14:42:43 = check 11's power-off (stock shutdown stops the menu); expected.
- Not run this session: lid during Restart/Power Off from the Redux menu; a ROM copied from the PC
  showing after restart; start-up time with the cache on 10,000+ ROMs.

## Dale's requests during the session (14:12)
1. Settings > System: an **Update** option for the CFW (on-device update, like the wget deploy).
2. Settings > UI: a switch for the **DOS menu layout**.
