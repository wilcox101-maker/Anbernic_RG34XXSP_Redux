---
type: decision
from: dale (recorded by claude)
to: claude
date: 2026-11-16
re: F4 power and lid, phase-start plan (docs/ui-brief.md F4 row; ROADMAP exit test)
status: closed (F4 signed off 2026-11-16)
---

# Decision: F4 plan, in-game lid = A (stock apps keep sleep, Redux flushes first)

Dale answered **A** at 13:06 to the phase-start plan sent in chat at 13:05.

## Plan (one PR per step)
1. **Lid in the menu.** event0 `axp2202-pek`: close = `KEY_INSERT` down (+ auto-repeat), open =
   `KEY_INSERT` up + a short `KEY_DELETE` tap ([hw-result #185](2026-11-16-dale-hw-result-185.md) row 28).
   Today `evdev_reader.c` runs the *power-key* path (`pwr_new.sh`) on **any** event0 key, so lid close
   is treated as a power press. Fix: only `KEY_POWER` runs `pwr_new.sh`; lid close runs
   `pwr_new.sh auto` (keeps stock `power.lock` / `power.hdmi`); `KEY_DELETE` and repeats ignored;
   lid debounced and ignored during boot and shutdown. Input codes live in the platform layer.
2. **Governor restore.** Before every launch, record `scaling_governor`, `scaling_min_freq`,
   `scaling_max_freq`; restore them after the app exits (all launch types, incl. music/video and
   DraStic). Never writes a value outside the stock OPP table: it only writes back what it read.
3. **Menu power settings** from [the 2026-11-10 decision](2026-11-10-dale-decision-power-management.md):
   dim after 60 s idle to 30 %, Wi-Fi off unless needed (Wi-Fi switching in its own PR).
4. **Lid during a game: option A (chosen).** Stock apps keep their own sleep handling. A small Redux
   watcher runs beside the game; on lid close it asks RetroArch to write saves and runs `sync` before
   stock suspends. No stock file is changed.
   - Rejected for now: **B**, Redux owns sleep by putting its script in place of `pwr_new.sh` at
     runtime (start-path change, Dale-gated). Revisit only if A fails the exit test.
   - Needs a read-only device check first (what suspends in-game today, RetroArch's command and
     save settings); batched with the #185 leftovers.

## Done when
F4 exit test (ROADMAP): 50 lid close/open cycles mid-game, no lost save, governor identical before
and after every app. Then Dale's sign-off.
