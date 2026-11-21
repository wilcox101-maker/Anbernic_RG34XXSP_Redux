# Status board

Current state only, kept under about 5 KB. Finished items move to [status-archive.md](status-archive.md);
who did what and when is in [ACTIVITY.md](ACTIVITY.md).

| Track | Phase | State |
|---|---|---|
| Kernel | 0 dissect stock · 1 stock-kernel tweaks | done · boots on device |
| ReduxOS | mainline image + Redux frontend (1.0) | Owner leads (Q12); S1 image port next; nothing booted yet |
| Frontend | F0, F1, F2 | signed off (F2: 2026-11-12) |
| Frontend | F3 settings parity | signed off 2026-11-16 ([note](2026-11-16-dale-phase-gate-f3-signed-off.md)) |
| Frontend | F4 power and lid | signed off 2026-11-16 ([note](2026-11-16-dale-phase-gate-f4-signed-off.md)) |
| Frontend | F5 Engineering Mode | signed off 2026-11-17 ([note](2026-11-17-dale-phase-gate-f5-signed-off.md)); heat-guard cap and 30 min run in the Beta exit test |

## Handoff (2026-11-21, Claude): start here in a new chat
- **Read [handoff-2026-11-21.md](handoff-2026-11-21.md) next**, then [handoff-2026-11-13.md](handoff-2026-11-13.md) for the lessons. New chats: `HELLO` in the Redux Project, started from the desktop app on DLN-DALEW2.
- **The Owner leads ReduxOS** (Q12, [decision](2026-11-21-dale-decision-owner-leads-reduxos.md)): roadmap in [ReduxOS `docs/roadmap.md`](https://github.com/wilcox101-maker/ReduxOS-Allwinner-H700/blob/main/docs/roadmap.md) (PR #12); attach ReduxOS-Allwinner-H700 with push too.
- **Inbox:** issue #303. Earlier chat names and finished items: [status-archive.md](status-archive.md).
- **Deploy only main's CI build** (Releases → dev); the title bar `v0.1-dev - <sha>` must be a **merge** commit.
- Device: home `home` profile around 192.168.1.13; work 172.16.3.178. Stock images (read-only) in Dale's `Downloads\Anbernic RG34xxSP`.

## Waiting on Dale
- **ReduxOS first light:** merge H700 #77 (device gate); pick the image (Owner: R33 LPDDR4 now); P1 Redux menu vs RetroArch first (Owner: Redux menu); P2 Ubuntu 24.04 on record.
- **Stock-kernel line:** OSK probe K24-K28 (kit in Claude Files), then Keyboard On; device checks #259, #283/#284, Bluetooth, #149.
- **Save the `redux-session` skill proposal** (inbox #303).

## Queue (in order; all Claude unless marked)
0. **ReduxOS 1.0 (top, Q12):** S1 image in the ReduxOS repo (DR ports kernel/u-boot/TF-A), S2 first light (bench session 1), S3 bring-up, S4 frontend port (F7, H700 R21 map), S5 release candidate (F8). [Roadmap](https://github.com/wilcox101-maker/ReduxOS-Allwinner-H700/blob/main/docs/roadmap.md).
1. **Global on-screen keyboard (stock line):** on main since 7ce026e (Settings > Keyboard, default Off). Next: K24-K28 probe, then Keyboard On; then the paused items below are re-checked. Decisions: [UI A](2026-11-18-dale-decision-osk-ui.md), [chord/RetroArch](2026-11-18-dale-decision-global-osk-2.md).
2. **#185 fixes** ([hw-result](2026-11-16-dale-hw-result-185.md)): #202 and #203 done; Game Room X = Favorites is a placeholder, not built; then a short device re-check. Wolfenstein 3D X/Y is not a bug (GBA ROM).
3. **UI font and colour settings** (#259, [decision](2026-11-17-dale-decision-ui-text-sizes.md)): built, not yet on device.
4. **F6:** Wi-Fi off unless needed; Power & Lid page; Redux front-ends for the stock apps in the BIOS/DOS style (Video Player first, then Music, Ebook, File Manager; drop the stock loading and card-select screens); a Redux network page to replace nmtui, which freezes on Edit. Needs FFplay.sh read from the device.
5. **Redux Terminal follow-ups** ([plan](2026-11-17-claude-question-terminal-apps.md), [hw-result](2026-11-17-dale-hw-result-8a9c2b5.md)): the picker opens in mc but not in Terminal/htop (#285 log to read back); CJK #283/#284 not yet on device; then Music, Video, Bluetooth, Documents; drop the vsync helper once device-checked.
6. **Bluetooth at start** ([decision](2026-11-17-dale-decision-bluetooth-init.md)): `setBluetooth.sh init` once per boot + Settings switch; built, not yet on device.
7. **Installer B** (TF2 package, hold-A) after A is device-checked; tagged Beta releases (`v0.5-beta.N`) once Beta is near.
8. **Extras installer** (#149): the REFUSED path and a fresh install not yet run on the device.
9. Follow-up: analog stick values on the Engineering Input tab.
- Backlog: Settings > System Update (release builds), DOS layout switch and DOS Mode ([inspiration](2026-11-17-dale-message-dos-gui-inspiration.md)), custom boot image picker, ASCII art maker, PC link (Dale-gated), Expert Mode, chiptune composer (after 1.0), Wi-Fi idle scanning, features.ini, ZRAM (1.x).

## Improvement ideas (AGENTS.md "Improvements"; Dale's `IMPROVE` lists them)
- Audit every boot job the stock menu ran that Redux dropped with it (Bluetooth was silently missing for a week, [decision](2026-11-17-dale-decision-bluetooth-init.md)); Read Only's table ([message](2026-11-17-dale-message-stock-scripts-audit.md)) closes the boot-job question (only Bluetooth); still to check as menu actions: SD hot-plug after boot (`mmc_new.sh add`), `setNDS.sh run` vs `drastic-modify/launch.sh`, Saturn BIOS/HLE and Flycast setup before those launches; a `ps` diff stock vs Redux would settle it.
- Run the C tests once under AddressSanitizer in CI (it caught the 2026-11-16 cache overflow at once; plain builds passed).
- AGENTS.md + CLAUDE.md are 6.3 KB (target about 6 KB): move the p1 exceptions' detail into a doc and link it.
- `precheck.py` prints every C test's own lines; show only the summary and failures (shorter output).
- `device-test-queue.yml` appends PRs whose not-verified list is just "none" (#194-#197 did); skip those to keep #185 short.

## Open notes
- [2026-11-21-dale-decision-owner-leads-reduxos.md](2026-11-21-dale-decision-owner-leads-reduxos.md): dale → all (Q12: Owner leads ReduxOS; v0.5 gate lifted for ReduxOS)
- [2026-11-18-dale-decision-osk-ui.md](2026-11-18-dale-decision-osk-ui.md): dale → all (keyboard UI = mock-up A)
- [2026-11-18-dale-decision-add-coprog.md](2026-11-18-dale-decision-add-coprog.md): dale → all (Co-Programmer helper, coprog/)
- [2026-11-18-dale-decision-global-osk-2.md](2026-11-18-dale-decision-global-osk-2.md): dale → all (OSK chord Menu + Select; no OSK in RetroArch)
- [2026-11-16-dale-hw-result-f4.md](2026-11-16-dale-hw-result-f4.md): dale → all (F4 session on 6d87953: exit test PASS; crash fixed by #216; Update + DOS switch requests)
- [2026-11-16-dale-hw-result-reads-f4-2.md](2026-11-16-dale-hw-result-reads-f4-2.md): dale → claude (RetroArch 1.20.0, SAVE_FILES works, saves on cards, Wolf3D is GBA)
- [2026-11-16-dale-hw-result-reads-f4.md](2026-11-16-dale-hw-result-reads-f4.md): dale → claude (FFplay.sh, no ecwolf, RetroArch network/save settings)
- [2026-11-16-dale-decision-app-screens-card-picker.md](2026-11-16-dale-decision-app-screens-card-picker.md): dale → all (card picker; Redux app screens in BIOS/DOS style; DOS layout on record)
- [2026-11-16-dale-hw-result-185.md](2026-11-16-dale-hw-result-185.md): dale → all (device session #185 on `5d7dea6`: results and 8 findings)
- [2026-11-14-dale-decision-mainline-cpu-table.md](2026-11-14-dale-decision-mainline-cpu-table.md): dale → all (ROCKNIX H700 table on mainline, 1512 MHz only behind Expert Mode)
- [2026-11-13-dale-message-kernel-research.md](2026-11-13-dale-message-kernel-research.md): dale → all (never use the Knulli guide's `dd` offsets)
- [2026-11-13-dale-decision-zram-v1x.md](2026-11-13-dale-decision-zram-v1x.md): dale → all (ZRAM in v1.x, after 1.0)
- [2026-11-10-dale-decision-power-management.md](2026-11-10-dale-decision-power-management.md): dale → all (menu power management; dim done in F4, Wi-Fi off in F6)
- [2026-11-10-claude-milestone-ram-system-design.md](2026-11-10-claude-milestone-ram-system-design.md): claude → all (RAM-resident design; Phase 2 direction)
