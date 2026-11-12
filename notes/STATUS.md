# Status board

Updated by whoever writes a note. Who visited and when is in [ACTIVITY.md](ACTIVITY.md). Keep it short: current state only, history lives in the notes.

| Track | Phase | State | Owner | Next |
|---|---|---|---|---|
| Kernel | Phase 0: dissect stock | done | claude | — |
| Kernel | Phase 1: stock-kernel tweaks | **boots on device** (≈20 s to menu, no issues seen) | dale | optional: governor check + DEPLOY.md §5 measurements |
| Kernel | Phase 2: RAM-resident system | design merged (`docs/ram-system-design.md`) | claude | measurements listed in the doc need the device |
| Frontend | F0: spec | **done**, signed off by Dale 2026-11-10 | grok | — |
| Frontend | F1: desktop prototype | **done**, signed off by Dale 2026-11-10 | grok + claude | — |
| Frontend | F2: launch parity on stock | **games launch on the device** (RetroArch, PSP, PS1, Neo Geo, CPS; #50, #52), TF2 ROMs via REDUX_ROMS, resume-to-list (#53), Wi-Fi (#48, #54), Stock Menu with a 3 s hold (#51, #58), build ID (#59) | grok | F2 sign-off by Dale after the build with #58/#59 is deployed |

## Handoff (2026-11-12, Claude): start here in a new chat
- **Deploy only from main's CI runs** (the arm64 artifact). The title bar shows `v0.1-dev - <sha>`; check it matches main.
- Device at work: 172.16.3.178 (home Wi-Fi profile `home` is also saved). Dale's PC folder `Documents\GitHub\redux-work` is linked for logs.
- Lessons: an old dmenu.bin got redeployed (hence the build ID). Redux kept reading buttons while SimpleTerminal ran over SSH and restored stock (hence the 3 s hold). RA_launch.sh must run with bash. The stock .dge launches fail, so those systems use RetroArch cores.
- Decisions this session: BIOS-style UI with 10 era-named colour presets (#41); App Center (top-level `APPS/*.sh` and `<dir>/launch.sh`); Ports/PortMaster with a FAT notice; System Tools (read-only) before the BIOS UI; Expert Mode policy (#56, five-gate unlock plus breadcrumbs, backlog); README is honest about status (#57).

## Waiting on Dale
- Merge #59, deploy main's build, sign off F2.

## Queue (inbox #41 is the source of truth; in order)
1. Wait screens everywhere (`ui_busy`), Loading ROM first. Grok is on it.
2. Wi-Fi part 2: Connecting and result screens.
3. System Tools (read-only, `sysinfo.c` data layer)
4. BIOS-style UI, colour presets, Settings → UI → Colors, Restart/Power Off with a 3 s hold
5. App Center · 6. Ports/PortMaster · 7. Terminal/htop (SimpleTerminal `-e htop`, MENU quits) · 8. TF2 scan · 9. 640×480 boot logo (Dale-only installer)
- Backlog: power and lid (lid already sleeps; wake is power-only), Wi-Fi idle scanning, media (mpv/ffplay present), features.ini, Expert Mode, the Phase 2 mainline track.

## Open notes
- [2026-11-11-grok-review-request-system-defaults.md](2026-11-11-grok-review-request-system-defaults.md): grok → claude (Stock Menu moved; #49 already merged)
- [2026-11-11-grok-review-request-back-to-stock.md](2026-11-11-grok-review-request-back-to-stock.md): closed (merged as #49; row has since moved)
- [2026-11-10-grok-review-request-f2-rom-list.md](2026-11-10-grok-review-request-f2-rom-list.md): grok → claude (ROM list, merged with #43)
- [2026-11-10-dale-hw-result-f2-menu-on-device.md](2026-11-10-dale-hw-result-f2-menu-on-device.md): dale → all (F2 menu on device)
- [2026-11-10-dale-decision-eng-tabs-back.md](2026-11-10-dale-decision-eng-tabs-back.md), [text-size](2026-11-10-dale-decision-text-size.md), [power-management](2026-11-10-dale-decision-power-management.md): decisions → grok
- [2026-11-10-dale-hw-result-phase1-boot.md](2026-11-10-dale-hw-result-phase1-boot.md): dale → all (Phase 1 boots)
- [2026-11-10-claude-phase-start-f2-launch-parity.md](2026-11-10-claude-phase-start-f2-launch-parity.md): claude → grok
- [2026-11-10-claude-message-f3-font-picker.md](2026-11-10-claude-message-f3-font-picker.md): claude → grok (F3, FYI)
- [2026-11-10-claude-milestone-ram-system-design.md](2026-11-10-claude-milestone-ram-system-design.md): claude → all (FYI)

F0 and F1 notes are all closed or acknowledged. See `notes/` for history.
