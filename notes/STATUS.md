# Status board

Current state only, kept under about 5 KB. Finished items move to [status-archive.md](status-archive.md);
who did what and when is in [ACTIVITY.md](ACTIVITY.md).

| Track | Phase | State |
|---|---|---|
| Kernel | 0 dissect stock · 1 stock-kernel tweaks | done · boots on device |
| Kernel | 2 RAM-resident / mainline | plan only; direction is Dale's call (below) |
| Frontend | F0, F1, F2 | signed off (F2: 2026-11-12) |
| Frontend | **F3 settings parity** | device-checked 2026-11-16 (#185 10-12 PASS); **waiting on Dale's SIGN F3** |

## Handoff (2026-11-14 (3), Claude): start here in a new chat
- **Read [handoff-2026-11-14-3.md](handoff-2026-11-14-3.md) next**, then [handoff-2026-11-13.md](handoff-2026-11-13.md) for the lessons. New chats: `HELLO` in the Redux Project, started from the desktop app on DLN-DALEW2.
- Previous chats archived as: **Redux 2026-11-10→11-12: F2 launch parity, Wi-Fi, BIOS UI and Expert Mode plans**; **Redux 2026-11-12→11-12: F2 sign-off, page jump, MAME + 27 systems, ROM cap, Beta roadmap**; **Redux 2026-11-12→11-12: device test, button remap, Wi-Fi verify, stock V1.0.6 diff, power menu**; **Redux 2026-11-12: volume, accent bars, native arm64 CI**; **Redux 2026-11-12→11-13: Claude takeover, brightness, TF2 ROM scan, App Center plan, agent harness, command words**; **Redux 2026-11-13→11-13: Device fixes 9a-g, Screen Timeout, htop + extras installer, Tools apps, OpenBOR**; **Redux 2026-11-13→11-14: #143 results, Apps resume, launch diagnostics, extras one-session, MAME pass, kernel research leap**; **Redux 2026-11-13→11-14: OpenBOR stock launch, F3 font picker + Reset All, vsync library, Manager relay, kernel research**; **Redux 2026-11-14→11-14: installer A + run.sh, user guide, Phase 2 → H700 KR, vsync armhf fixes, wider Manager relay**
- **Deploy only main's CI build** (Releases → dev). The title bar `v0.1-dev - <sha>` must be a **merge** commit in `git log`.
- Device: at home the saved `home` profile, around 192.168.1.13; at work 172.16.3.178 (`DLN_TeamWireless`). Stock images (read-only use) in Dale's `Downloads\Anbernic RG34xxSP`.
- Owner: the Redux Project chat named in the newest handoff; every other chat is advisory. Grok is suspended; every queue item is Claude's.

## Waiting on Dale
- **SIGN F3**: #185 checks 10-12 passed on `5d7dea6`, Stage 1 had no failure ([hw-result](2026-11-16-dale-hw-result-185.md)).
- **Card Scripts decision A** PR (hard-rule rewording): Dale merges.
- **Phase 2 direction** (before Phase 2, not urgent for Beta): the H700 Kernel Research Ubuntu 24.04 image with update rollback, or `docs/ram-system-design.md`'s read-only squashfs in RAM.

## Queue (in order; all Claude unless marked)
1. **Card Scripts A** (Dale 2026-11-16 11:51): remove the switch; card `Roms/PORTS/*.sh` always listed and run; hard rule reworded. PR for Dale.
2. **Fixes from #185** ([hw-result](2026-11-16-dale-hw-result-185.md)): video player buttons during playback; Game Room X / Favorites; Wolfenstein 3D Y and X; one entry per game on both cards (or show the card); no ROM-scan screen after an app returns; clear `/tmp/.next` at start; `amixer` output out of `redux.log`. Then a short device re-check.
3. **F4 power and lid**, then **F5 Engineering Mode** (Dale A, 2026-11-14 13:37: phase order kept).
4. **Installer B**: TF2 package with hold-A (after A is device-checked). Tagged Beta releases (`v0.5-beta.N`) once Beta is near.
5. Extras installer (#149): REFUSED path and a fresh install still not run on the device.
- Backlog: **custom boot image picked on the device** (Dale, 2026-11-16), **DOS Mode** (Dale defines the layout), ASCII art maker, PC link (Dale-gated security item; ROM copy only in [Xroot Mode]), Expert Mode and its shortcut, chiptune composer (after 1.0, separate GPL-3 program), Wi-Fi idle scanning, features.ini, ZRAM (v1.x), the Phase 2 mainline track. L3/R3 stay unmapped.

## Improvement ideas (AGENTS.md "Improvements"; Dale's `IMPROVE` lists them)
- `precheck.py` prints every C test's own lines; show only the summary and failures (shorter output).
- `device-test-queue.yml` appends PRs whose not-verified list is just "none" (#194-#197 did); skip those to keep #185 short.

## Open notes
- [2026-11-16-dale-hw-result-185.md](2026-11-16-dale-hw-result-185.md): dale → all (device session #185 on `5d7dea6`: results and 8 findings)
- [2026-11-14-dale-decision-mainline-cpu-table.md](2026-11-14-dale-decision-mainline-cpu-table.md): dale → all (ROCKNIX H700 table on mainline, 1512 MHz only behind Expert Mode)
- [2026-11-13-dale-message-kernel-research.md](2026-11-13-dale-message-kernel-research.md): dale → all (kernel/Ubuntu research; never use the Knulli guide's `dd` offsets)
- [2026-11-13-dale-decision-zram-v1x.md](2026-11-13-dale-decision-zram-v1x.md): dale → all (ZRAM in v1.x, after 1.0)
- [2026-11-10-dale-decision-power-management.md](2026-11-10-dale-decision-power-management.md): dale → all (menu power management; for F4)
- [2026-11-10-claude-milestone-ram-system-design.md](2026-11-10-claude-milestone-ram-system-design.md): claude → all (RAM-resident design; Phase 2 direction)
