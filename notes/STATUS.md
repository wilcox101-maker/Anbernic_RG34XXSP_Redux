# Status board

Current state only, kept under about 5 KB. Finished items move to [status-archive.md](status-archive.md);
who did what and when is in [ACTIVITY.md](ACTIVITY.md).

| Track | Phase | State |
|---|---|---|
| Kernel | 0 dissect stock · 1 stock-kernel tweaks | done · boots on device |
| Kernel | 2 RAM-resident / mainline | plan only; direction is Dale's call (below) |
| Frontend | F0, F1, F2 | signed off (F2: 2026-11-12) |
| Frontend | F3 settings parity | signed off 2026-11-16 ([note](2026-11-16-dale-phase-gate-f3-signed-off.md)) |
| Frontend | F4 power and lid | signed off 2026-11-16 ([note](2026-11-16-dale-phase-gate-f4-signed-off.md)) |
| Frontend | F5 Engineering Mode | signed off 2026-11-17 ([note](2026-11-17-dale-phase-gate-f5-signed-off.md)); heat-guard cap and 30 min run in the Beta exit test |

## Handoff (2026-11-18, Claude): start here in a new chat
- **Read [handoff-2026-11-18.md](handoff-2026-11-18.md) next**, then [handoff-2026-11-13.md](handoff-2026-11-13.md) for the lessons. New chats: `HELLO` in the Redux Project, started from the desktop app on DLN-DALEW2.
- **Inbox:** issue #303 (was draft PR #41 until 2026-11-18).
- Earlier chat names and finished items: [status-archive.md](status-archive.md).
- **Deploy only main's CI build** (Releases → dev). The title bar `v0.1-dev - <sha>` must be a **merge** commit in `git log`.
- Device: at home the saved `home` profile, around 192.168.1.13; at work 172.16.3.178 (`DLN_TeamWireless`). Stock images (read-only use) in Dale's `Downloads\Anbernic RG34xxSP`.
- Owner: the Redux Project chat named in the newest handoff; every other chat is advisory. Grok is suspended; every queue item is Claude's.

## Waiting on Dale
- **Global OSK probe session** (K24-K28): kit ready in Claude Files (`redux-oskprobe-kr1-96ef78f.zip`), run sheet inside.
- **Phase 2 direction** (before Phase 2, not urgent for Beta): the H700 Kernel Research Ubuntu 24.04 image with update rollback, or `docs/ram-system-design.md`'s read-only squashfs in RAM.
- **Device checks once the keyboard is on:** UI fonts/colours (#259), terminal CJK (#283, #284), Bluetooth at start, extras installer (#149).
- **Save the `redux-session` skill proposal** so the skill reads inbox #303.

## Queue (in order; all Claude unless marked)
0. **Global on-screen keyboard (top task):** on main as 7ce026e: drawing #297 (Co-Programmer), layout #301, daemon + osklib #306, Redux starts it when Settings > Keyboard is On #307 (default Off). Next: Dale's K24-K28 probe, then Keyboard On and a device check; then every paused item is re-checked against it. DR-1..3 pending. [Plan](2026-11-18-claude-question-global-osk-plan.md), [decision](2026-11-18-dale-decision-global-osk.md), [chord/RetroArch](2026-11-18-dale-decision-global-osk-2.md), [UI A](2026-11-18-dale-decision-osk-ui.md), [Co-Programmer](2026-11-18-dale-decision-add-coprog.md).
1. **#185 fixes** ([hw-result](2026-11-16-dale-hw-result-185.md)): #202 and #203 done; Game Room X = Favorites is a placeholder, not built; then a short device re-check. Wolfenstein 3D X/Y is not a bug (GBA ROM).
2. **UI font and colour settings** (#259, [decision](2026-11-17-dale-decision-ui-text-sizes.md)): built, not yet on device.
3. **F6:** Wi-Fi off unless needed; Power & Lid page; Redux front-ends for the stock apps in the BIOS/DOS style (Video Player first, then Music, Ebook, File Manager; drop the stock loading and card-select screens); a Redux network page to replace nmtui, which freezes on Edit. Needs FFplay.sh read from the device.
4. **Redux Terminal follow-ups** ([plan](2026-11-17-claude-question-terminal-apps.md), [hw-result](2026-11-17-dale-hw-result-8a9c2b5.md)): the picker opens in mc but not in Terminal/htop (#285 log to read back); CJK #283/#284 not yet on device; then Music, Video, Bluetooth, Documents; drop the vsync helper once device-checked.
5. **Bluetooth at start** ([decision](2026-11-17-dale-decision-bluetooth-init.md)): `setBluetooth.sh init` once per boot + Settings switch; built, not yet on device.
6. **Installer B** (TF2 package, hold-A) after A is device-checked; tagged Beta releases (`v0.5-beta.N`) once Beta is near.
7. **Extras installer** (#149): the REFUSED path and a fresh install not yet run on the device.
8. Follow-up: analog stick values on the Engineering Input tab.
- Backlog: **Update option in Settings > System** (release builds; dev builds via Engineering > Update, item 5), **DOS layout switch in Settings > UI** (Dale 2026-11-16), **custom boot image picked on the device** (Dale, 2026-11-16), **DOS Mode** (Dale defines the layout; request recorded in `docs/ui-brief.md` §3; graphical inspiration: [DESQview/X and Lotus-style launchers](2026-11-17-dale-message-dos-gui-inspiration.md), after the current revision), ASCII art maker, PC link (Dale-gated security item; ROM copy only in [Xroot Mode]), Expert Mode and its shortcut, chiptune composer (after 1.0, separate GPL-3 program), Wi-Fi idle scanning, features.ini, ZRAM (v1.x), the Phase 2 mainline track. L2/R2/L3/R3 have no menu action (shown on the Engineering Input tab only).

## Improvement ideas (AGENTS.md "Improvements"; Dale's `IMPROVE` lists them)
- Audit every boot job the stock menu ran that Redux dropped with it (Bluetooth was silently missing for a week, [decision](2026-11-17-dale-decision-bluetooth-init.md)); Read Only's table ([message](2026-11-17-dale-message-stock-scripts-audit.md)) closes the boot-job question (only Bluetooth); still to check as menu actions: SD hot-plug after boot (`mmc_new.sh add`), `setNDS.sh run` vs `drastic-modify/launch.sh`, Saturn BIOS/HLE and Flycast setup before those launches; a `ps` diff stock vs Redux would settle it.
- Run the C tests once under AddressSanitizer in CI (it caught the 2026-11-16 cache overflow at once; plain builds passed).
- AGENTS.md + CLAUDE.md are 6.3 KB (target about 6 KB): move the p1 exceptions' detail into a doc and link it.
- `precheck.py` prints every C test's own lines; show only the summary and failures (shorter output).
- `device-test-queue.yml` appends PRs whose not-verified list is just "none" (#194-#197 did); skip those to keep #185 short.

## Open notes
- [2026-11-18-dale-decision-osk-ui.md](2026-11-18-dale-decision-osk-ui.md): dale → all (keyboard UI = mock-up A)
- [2026-11-18-dale-decision-add-coprog.md](2026-11-18-dale-decision-add-coprog.md): dale → all (Co-Programmer helper, coprog/)
- [2026-11-18-dale-decision-global-osk-2.md](2026-11-18-dale-decision-global-osk-2.md): dale → all (OSK chord Menu + Select; no OSK in RetroArch)
- [2026-11-16-dale-hw-result-f4.md](2026-11-16-dale-hw-result-f4.md): dale → all (F4 session on 6d87953: exit test PASS; crash fixed by #216; Update + DOS switch requests)
- [2026-11-16-dale-hw-result-reads-f4-2.md](2026-11-16-dale-hw-result-reads-f4-2.md): dale → claude (RetroArch 1.20.0, SAVE_FILES works, saves on cards, Wolf3D is GBA)
- [2026-11-16-dale-hw-result-reads-f4.md](2026-11-16-dale-hw-result-reads-f4.md): dale → claude (FFplay.sh, no ecwolf, RetroArch network/save settings)
- [2026-11-16-dale-decision-app-screens-card-picker.md](2026-11-16-dale-decision-app-screens-card-picker.md): dale → all (card picker; Redux app screens in BIOS/DOS style; DOS layout on record)
- [2026-11-16-dale-hw-result-185.md](2026-11-16-dale-hw-result-185.md): dale → all (device session #185 on `5d7dea6`: results and 8 findings)
- [2026-11-14-dale-decision-mainline-cpu-table.md](2026-11-14-dale-decision-mainline-cpu-table.md): dale → all (ROCKNIX H700 table on mainline, 1512 MHz only behind Expert Mode)
- [2026-11-13-dale-message-kernel-research.md](2026-11-13-dale-message-kernel-research.md): dale → all (kernel/Ubuntu research; never use the Knulli guide's `dd` offsets)
- [2026-11-13-dale-decision-zram-v1x.md](2026-11-13-dale-decision-zram-v1x.md): dale → all (ZRAM in v1.x, after 1.0)
- [2026-11-10-dale-decision-power-management.md](2026-11-10-dale-decision-power-management.md): dale → all (menu power management; dim done in F4, Wi-Fi off in F6)
- [2026-11-10-claude-milestone-ram-system-design.md](2026-11-10-claude-milestone-ram-system-design.md): claude → all (RAM-resident design; Phase 2 direction)
