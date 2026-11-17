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
| Frontend | **F5 Engineering Mode** | steps 1-4 and 5a device-checked ([11-16](2026-11-16-dale-hw-result-f5-steps-1-2.md), [11-17](2026-11-17-dale-hw-result-7ee31d1.md)); heat-guard cap not yet triggered on the device; next: input viewer, LED/rumble (by-hand write test first), `fsck -n`, then the 30 min exit test |

## Handoff (2026-11-16-2, Claude): start here in a new chat
- **Read [handoff-2026-11-16-2.md](handoff-2026-11-16-2.md) next**, then [handoff-2026-11-13.md](handoff-2026-11-13.md) for the lessons. New chats: `HELLO` in the Redux Project, started from the desktop app on DLN-DALEW2.
- Previous chats archived as: **Redux 2026-11-10→11-12: F2 launch parity, Wi-Fi, BIOS UI and Expert Mode plans**; **Redux 2026-11-12→11-12: F2 sign-off, page jump, MAME + 27 systems, ROM cap, Beta roadmap**; **Redux 2026-11-12→11-12: device test, button remap, Wi-Fi verify, stock V1.0.6 diff, power menu**; **Redux 2026-11-12: volume, accent bars, native arm64 CI**; **Redux 2026-11-12→11-13: Claude takeover, brightness, TF2 ROM scan, App Center plan, agent harness, command words**; **Redux 2026-11-13→11-13: Device fixes 9a-g, Screen Timeout, htop + extras installer, Tools apps, OpenBOR**; **Redux 2026-11-13→11-14: #143 results, Apps resume, launch diagnostics, extras one-session, MAME pass, kernel research leap**; **Redux 2026-11-13→11-14: OpenBOR stock launch, F3 font picker + Reset All, vsync library, Manager relay, kernel research**; **Redux 2026-11-14→11-14: installer A + run.sh, user guide, Phase 2 → H700 KR, vsync armhf fixes, wider Manager relay**; **Redux 2026-11-14→11-16: process review, brevity + roles rules, tools, device session #185, F3 signed off, Card Scripts A, card picker**; **Redux 2026-11-16→11-16: F4 power and lid, cache crash fix, F4 signed off**
- **Deploy only main's CI build** (Releases → dev). The title bar `v0.1-dev - <sha>` must be a **merge** commit in `git log`.
- Device: at home the saved `home` profile, around 192.168.1.13; at work 172.16.3.178 (`DLN_TeamWireless`). Stock images (read-only use) in Dale's `Downloads\Anbernic RG34xxSP`.
- Owner: the Redux Project chat named in the newest handoff; every other chat is advisory. Grok is suspended; every queue item is Claude's.

## Waiting on Dale
- **Phase 2 direction** (before Phase 2, not urgent for Beta): the H700 Kernel Research Ubuntu 24.04 image with update rollback, or `docs/ram-system-design.md`'s read-only squashfs in RAM.

## Queue (in order; all Claude unless marked)
1. **Fixes from #185** ([hw-result](2026-11-16-dale-hw-result-185.md)): video player buttons (folded into item 4, the Redux video front-end); Wolfenstein 3D Y and X: not a bug, it is a GBA ROM (no X/Y). Done: `/tmp/.next` + amixer (#202), scan cache (#203). Game Room X = Favorites, not built yet (placeholder). Then a short device re-check.
2. ~~Game Room card picker~~ #205, device-checked 2026-11-16 (with the #216 crash fix).
3. **F5 Engineering Mode** (in progress: [plan](2026-11-16-claude-decision-f5-plan.md), steps 1-2 device-checked, steps 3-4 and 5a built; next input viewer, LED/rumble, `fsck -n`). F4 signed off 2026-11-16 ([note](2026-11-16-dale-phase-gate-f4-signed-off.md)).
4. **Dale's requests 2026-11-17 08:40:** all six done (#231-#234), device-checked on 7ee31d1.
5. **UI after F5 sign-off** (Dale 2026-11-17 09:19): (a) font size settings for the common UI text elements; (b) three colour settings: menu text colour, header/footer window text colour, window colour. Plan to Dale first (what scales on a 90×30 grid, which elements, defaults, Reset All).
6. **F6**: Wi-Fi off unless needed + Power & Lid settings page (moved from F4, Dale A 15:09); **Redux front-ends for the stock apps** in the BIOS/DOS style (Dale 2026-11-16): Video Player first (Redux file list + stock `ffplay`), then Music, Ebook, File Manager. Needs the FFplay.sh read from the device. Dale 2026-11-17 09:23: for Music and Video, drop the stock app's loading screen and card-select screen and use Redux's BIOS-style screens instead; the stock card-select screens only exit with M, B does nothing (seen on the device).
7. **Installer B**: TF2 package with hold-A (after A is device-checked). Tagged Beta releases (`v0.5-beta.N`) once Beta is near.
8. Extras installer (#149): REFUSED path and a fresh install still not run on the device.
- Backlog: **Update option in Settings > System** (Dale 2026-11-16; security item, Dale merges), **DOS layout switch in Settings > UI** (Dale 2026-11-16), **custom boot image picked on the device** (Dale, 2026-11-16), **DOS Mode** (Dale defines the layout; request recorded in `docs/ui-brief.md` §3), ASCII art maker, PC link (Dale-gated security item; ROM copy only in [Xroot Mode]), Expert Mode and its shortcut, chiptune composer (after 1.0, separate GPL-3 program), Wi-Fi idle scanning, features.ini, ZRAM (v1.x), the Phase 2 mainline track. L3/R3 stay unmapped.

## Improvement ideas (AGENTS.md "Improvements"; Dale's `IMPROVE` lists them)
- Run the C tests once under AddressSanitizer in CI (it caught the 2026-11-16 cache overflow at once; plain builds passed).
- AGENTS.md + CLAUDE.md are 6.3 KB (target about 6 KB): move the p1 exceptions' detail into a doc and link it.
- `precheck.py` prints every C test's own lines; show only the summary and failures (shorter output).
- `device-test-queue.yml` appends PRs whose not-verified list is just "none" (#194-#197 did); skip those to keep #185 short.

## Open notes
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
