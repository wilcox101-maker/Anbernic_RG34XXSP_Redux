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

## Handoff (2026-11-12 16:20, Claude): start here in a new chat
- Previous chats archived as: **Redux 2026-11-10→11-12: F2 launch parity, Wi-Fi, BIOS UI and Expert Mode plans**; **Redux 2026-11-12→11-12: F2 sign-off, page jump, MAME + 27 systems, ROM cap, Beta roadmap**; **Redux 2026-11-12→11-12: device test, button remap, Wi-Fi verify, stock V1.0.6 diff, power menu**; **Redux 2026-11-12: volume, accent bars, native arm64 CI**
- **Deploy only from main's CI runs** (the arm64 artifact). The title bar shows `v0.1-dev - <sha>` and must match main's **merge** commit, not the PR head. 2026-11-12: Dale deployed `fadf977`, which is GitHub's PR merge-preview commit, not a repo commit — if a title-bar SHA is not in `git log`, that is what happened. Testing an unmerged PR from its own build is fine; just say so.
- **Dale starts his own GitHub comments with `DALE:`** (AGENTS.md). Grok's status comments start with `PING:`. Both post from `wilcox101-maker`.
- **Every chat swap needs Dale's approval** (AGENTS.md, #93). For the automatic triggers the agent asks "Swap now?" and waits for **SWAP** or **NO SWAP**. Dale saying SWAP himself still works.
- Device at work: 172.16.3.178 (`DLN_TeamWireless`). **Dale left work 2026-11-12 16:20; at home the device returns to the saved `home` profile, around 192.168.1.13.** Dale's PC: `Downloads\Anbernic RG34xxSP` holds the 2025 `RG34XXSP.img` and `.FW\` the stock V1.0.6 image (read-only use only).
- **How Dale wants to work now:** Grok gets single tasks with CHECKPOINTS and asks Dale "proceed?" at each one. Dale brings Grok's questions here, Claude advises, Dale relays. Batch reviews; Dale merges docs-only PRs without asking. Dale watches CI himself and only forwards failures that persist past about three attempts.
- A scheduled task **"Redux CI hourly check"** runs hourly 09:00–21:00 America/New_York and pushes to Dale's phone. Read-only: it never comments, pushes or merges. It skips the hour if Dale merged, reviewed, or left a non-`PING:` comment; otherwise it reports branches failing 3+ runs in a row and green PRs with no Claude review, else "All quiet".
- Merged this session: #93 (AGENTS.md: swap approval + `DALE:` prefix), #94 persistent volume, #95 hold bars use the accent colour, #96 volume-stages decision note, #97 **arm64 CI builds natively** on `ubuntu-22.04-arm` instead of under QEMU.
- Device-verified 2026-11-12 (Dale, from the #94 build a78d5e4): Vol+/− steps and the `VOL nn` overlay appears and clears; the level survives a reboot; the level survives returning from a game. #91's short-A fix and System layout verified from `03f530f`.
- **Not yet device-checked:** #95 accent hold bars (a solid accent block replaced the white stipple — check it reads well on the darker presets), and #97's artifact (the first binary built on the native arm64 runner; confirm it still runs before trusting the new pipeline).
- Lessons: `git add -A` committed build output once (#79, closed); `.gitignore` now excludes build dirs, so use explicit paths. Force-push is blocked here; open a new PR instead. A PR's fake nmcli must match the device's real output format (#88). The emulated arm64 job's apt/dpkg failures are gone with #97 — if arm64 fails now, it is real. The arm64 job went from 2m33s to 49s.

**2026-11-13: Claude runs this phase; Grok is sidelined until Dale calls him in ([decision](2026-11-13-dale-decision-claude-runs-phase.md)).** Queue items below are Claude's unless marked otherwise.

## Waiting on Dale
- Device check of main after #104: Settings > Display brightness (Left/Right, survives reboot, never dark); TF2 games appear in Game Room, merged with TF1, and launch.
- Close #101 (Grok's brightness PR, replaced by #102; Claude's close was blocked by a permission check).
- Optional device checks: #95 accent hold bars, and that the native-runner artifact runs.

## Queue (inbox #41 is the source of truth; in order)
1. ~~Wi-Fi part 2: Connecting and result screens~~ merged in #73 (device test 2026-11-12: connect failed; fixed in #88). **Grok**
0a. ~~Wi-Fi connect fix~~ merged in #88 (verify by FIELD:value, saved-profile modify+up, one row per SSID). Not device-checked yet. **Grok**
1a. ~~Map unmapped stock folders~~ done in #70 (HBMAME/OPENBOR/VARCADE left). **Claude**
2. ~~System Tools (read-only, `sysinfo.c` data layer)~~ merged in #74, VE/DDR fixed in #83 (device-verified). **Grok**
3. ~~BIOS-style UI, colour presets~~ (#81/#82) and ~~Restart/Power Off~~ (#89, device-verified). **Grok**
3a. ~~Confirm screens: a short A tap stays on the screen and resets the hold bar~~ merged in #91, device-verified. **Grok**
3b. ~~Settings → System layout~~ merged in #91, device-verified. Reset All still to come = Redux's own `/mnt/data/redux/` settings to defaults only, 3 s hold. **Grok**
3c. ~~persistent volume~~ (#94, device-verified) → ~~brightness~~ merged in #102 (Settings > Display, 1–20) → ~~TF2 ROM scan~~ merged in #104 (TF1 + TF2 merged per system). Brightness and TF2 **not device-checked yet**. **Claude**
4. App Center · 5. Ports/PortMaster · 6. Terminal/htop (SimpleTerminal `-e htop`, MENU quits) · 7. ~~TF2 scan~~ (#104) · 8. 640×480 boot logo (Dale-only installer)
- L3/R3 (313/316): stay unmapped, left available for mapping later ([decision](2026-11-12-dale-decision-brightness-l3r3.md)).
- Backlog: **DOS Mode** (Dale: keep the development menu style as a DOS Mode option alongside BIOS Mode; he'll define the layout as menus grow), ASCII art maker (Draw, Convert, Edit, Share/Save As; App Center app), PC link (browser file manager for both cards and Redux data; security item, Dale-gated; ROM upload after an ownership acknowledgement), Expert Mode shortcut code (undocumented; thermals untouched by any unlock), chiptune composer (port LittleGPTracker's RG35XXPLUS build, App Center app, after Ports; #41), power and lid (lid already sleeps; wake is power-only), Wi-Fi idle scanning, media (mpv/ffplay present), features.ini, Expert Mode, the Phase 2 mainline track.

## Open notes
- [2026-11-13-dale-decision-claude-runs-phase.md](2026-11-13-dale-decision-claude-runs-phase.md): dale → all (Claude builds and merges this phase; Grok on call)
- [2026-11-12-dale-decision-brightness-l3r3.md](2026-11-12-dale-decision-brightness-l3r3.md): dale → grok (brightness go, 1–20 in config; L3/R3 stay unmapped)
- [2026-11-12-dale-decision-volume-stages.md](2026-11-12-dale-decision-volume-stages.md): dale → all (Redux Vol+/- is the hardware master; RetroArch keeps its own trim. Not a defect; do not add code to sync the two numbers)
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
