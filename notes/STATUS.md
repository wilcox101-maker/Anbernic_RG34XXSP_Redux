# Status board

Updated by whoever writes a note. Who visited and when is in [ACTIVITY.md](ACTIVITY.md). Keep it short: current state only, history lives in the notes.

| Track | Phase | State | Owner | Next |
|---|---|---|---|---|
| Kernel | Phase 0: dissect stock | done | claude | — |
| Kernel | Phase 1: stock-kernel tweaks | **boots on device** (≈20 s to menu, no issues seen) | dale | optional: governor check + DEPLOY.md §5 measurements |
| Kernel | Phase 2: RAM-resident system | design merged (`docs/ram-system-design.md`) | claude | measurements listed in the doc need the device |
| Frontend | F0: spec | **done**, signed off by Dale 2026-11-10 | grok | — |
| Frontend | F1: desktop prototype | **done**, signed off by Dale 2026-11-10 | grok + claude | — |
| Frontend | F2: launch parity on stock | **done**, signed off by Dale 2026-11-12 ([note](2026-11-12-dale-phase-gate-f2-signed-off.md)) | grok | — (Loading ROM splash confirmed on device) |

## Handoff (2026-11-12 15:00, Claude): start here in a new chat
- Previous chats archived as: **Redux 2026-11-10→11-12: F2 launch parity, Wi-Fi, BIOS UI and Expert Mode plans**; **Redux 2026-11-12→11-12: F2 sign-off, page jump, MAME + 27 systems, ROM cap, Beta roadmap**; **Redux 2026-11-12→11-12: device test, button remap, Wi-Fi verify, stock V1.0.6 diff, power menu**
- **Deploy only from main's CI runs** (the arm64 artifact). The title bar shows `v0.1-dev - <sha>` and must match main's **merge** commit, not the PR head.
- Device at work: 172.16.3.178 (`DLN_TeamWireless`). Saved Wi-Fi profiles: DLN_Guest, DLN_TeamWireless, SAND_BOX, home x2. Dale's PC: `Downloads\Anbernic RG34xxSP` holds the 2025 `RG34XXSP.img` and `.FW\` the stock V1.0.6 image (read-only use only; three scratch files of Claude's there can be deleted).
- **How Dale wants to work now:** Grok gets single tasks with CHECKPOINTS and asks Dale "proceed?" at each one. Dale brings Grok's questions here, Claude advises, Dale relays. Batch reviews; Dale merges docs-only PRs without asking. Usage is tight until the Monday reset.
- Device-verified 2026-11-12 ([hw-result](2026-11-12-dale-hw-result-build-0afecd9.md)): Settings without Network, colour preset survives reboot, System Tools (VE/DDR fixed in #83), Saturn BIOS game (BIOS path works), Restart and Power Off (#89).
- Merged this session: #78 (Settings: no Network), #80 (stock V1.0.6 vs 2025: revised boards need the V1.0.6 DTB; never put a 2025-based boot.img on them), #81/#82 colour presets on /mnt/data, #83 VE/DDR zone names, #85 **button remap** (device follows the DT: Select 310, Start 311, X 307, Y 306, L2 314, R2 315; L3/R3 313/316 unmapped pending Dale), #86 queue, #87 brightness = AXP2202 `brightness` attribute (no backlight class), #88 Wi-Fi verify (nmcli exits 0 on failure; device show is FIELD:value lines; saved-profile modify+up), #89 Restart/Power Off.
- **Not yet device-checked:** #85 buttons (Start submits Wi-Fi password, Y rescans, X Favorites, L2/R2 nothing) and #88 Wi-Fi (wrong password → Not connected; DLN_Guest right password → real IP; saved profile path).
- Lessons: `git add -A` committed build output once (#79, closed); `.gitignore` now excludes build dirs, so use explicit paths. Force-push is blocked here; open a new PR instead. A PR's fake nmcli must match the device's real output format (#88). The arm64 job sometimes fails in apt/dpkg; one re-run.

## Waiting on Dale
- Send Grok the #89 results + the System layout task (drafted in the last chat; summary under Queue item 3b), then device-check #85 and #88.

## Queue (inbox #41 is the source of truth; in order)
1. ~~Wi-Fi part 2: Connecting and result screens~~ merged in #73 (device test 2026-11-12: connect failed; fixed in #88). **Grok**
0a. ~~Wi-Fi connect fix~~ merged in #88 (verify by FIELD:value, saved-profile modify+up, one row per SSID). Not device-checked yet. **Grok**
1a. ~~Map unmapped stock folders~~ done in #70 (HBMAME/OPENBOR/VARCADE left). **Claude**
2. ~~System Tools (read-only, `sysinfo.c` data layer)~~ merged in #74, VE/DDR fixed in #83 (device-verified). **Grok**
3. ~~BIOS-style UI, colour presets~~ (#81/#82) and ~~Restart/Power Off~~ (#89, device-verified). **Grok**
3a. Confirm screens: a short A tap stays on the screen and resets the hold bar; only B/other buttons leave (Restart, Power Off, Stock Menu). **Grok**
3b. Settings → System layout: System Tools, Restart, Power Off, Boot Screen (not yet), 3 blank rows, Stock Menu, Reset All (not yet). Cursor skips blanks and "not yet" items. Reset All (later) = Redux's own `/mnt/data/redux/` settings to defaults only, 3 s hold. **Grok**
3c. Grok's task list from Dale (one at a time, CHECKPOINT + "proceed?" each): persistent volume (`lineout volume` 0–31 via amixer, keys 115/114, /mnt/data/redux/volume) → brightness (AXP2202 attribute; range check first, never 0) → TF2 ROM scan. **Grok**
4. App Center · 5. Ports/PortMaster · 6. Terminal/htop (SimpleTerminal `-e htop`, MENU quits) · 7. TF2 scan · 8. 640×480 boot logo (Dale-only installer)
- Open with Dale: what L3/R3 (313/316) should do; they're unmapped for now.
- Backlog: **DOS Mode** (Dale: keep the development menu style as a DOS Mode option alongside BIOS Mode; he'll define the layout as menus grow), ASCII art maker (Draw, Convert, Edit, Share/Save As; App Center app), PC link (browser file manager for both cards and Redux data; security item, Dale-gated; ROM upload after an ownership acknowledgement), Expert Mode shortcut code (undocumented; thermals untouched by any unlock), chiptune composer (port LittleGPTracker's RG35XXPLUS build, App Center app, after Ports; #41), power and lid (lid already sleeps; wake is power-only), Wi-Fi idle scanning, media (mpv/ffplay present), features.ini, Expert Mode, the Phase 2 mainline track.

## Open notes
- [2026-11-12-dale-hw-result-build-0afecd9.md](2026-11-12-dale-hw-result-build-0afecd9.md): dale → all (0afecd9 on device: Colors, System Tools, Saturn pass; Wi-Fi connect fails, nmcli exits 0 on error)
- [2026-11-12-claude-message-stock-v106-diff.md](2026-11-12-claude-message-stock-v106-diff.md): claude → all (stock V1.0.6 supports revised boards; 2025-based boot.img must not go on them)
- [2026-11-12-claude-review-system-tools.md](2026-11-12-claude-review-system-tools.md): closed (fixed in 5258c5d, merged as #74)
- [2026-11-12-claude-message-grok-next.md](2026-11-12-claude-message-grok-next.md): claude → grok (Wi-Fi part 2 done in #73; System Tools in #74)
- Done since the handoff: wait screens (#62, merged); F2 signed off; Left/Right page jump (#63, Claude, open).
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
