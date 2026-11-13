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

## Handoff (2026-11-13 01:35, Claude; refreshed 02:30): start here in a new chat
- **Read [handoff-2026-11-13-3.md](handoff-2026-11-13-3.md) next** (current state, setup left for the first Project chat, next task), then [handoff-2026-11-13.md](handoff-2026-11-13.md) for the lessons. New chats: `HELLO` in the Redux Project.
- Previous chats archived as: **Redux 2026-11-10→11-12: F2 launch parity, Wi-Fi, BIOS UI and Expert Mode plans**; **Redux 2026-11-12→11-12: F2 sign-off, page jump, MAME + 27 systems, ROM cap, Beta roadmap**; **Redux 2026-11-12→11-12: device test, button remap, Wi-Fi verify, stock V1.0.6 diff, power menu**; **Redux 2026-11-12: volume, accent bars, native arm64 CI**; **Redux 2026-11-12→11-13: Claude takeover, brightness, TF2 ROM scan, App Center plan, agent harness, command words**
- **Deploy only from main's CI runs** (the arm64 artifact). The title bar shows `v0.1-dev - <sha>` and must match main's **merge** commit. A SHA not in `git log` is GitHub's PR merge preview. Testing an unmerged PR's own build is fine; say so.
- **Dale starts his own GitHub comments with `DALE:`**; Grok's start with `PING:`. Both post from `wilcox101-maker`. Every chat swap needs Dale's **SWAP**.
- Device: at work 172.16.3.178 (`DLN_TeamWireless`), at home the saved `home` profile, around 192.168.1.13. Dale's PC `Downloads\Anbernic RG34xxSP` holds the 2025 `RG34XXSP.img` and `.FW\` stock V1.0.6 (read-only use).
- Scheduled task **"Redux CI hourly check"** is **paused** (disabled 2026-11-12 ~21:23 ET; last run 21:05). Don't re-enable it unless Dale asks. If he does: its prompt uses `gh pr list`, which fails in cloud sessions (GraphQL blocked), so switch it to the REST calls in `CLAUDE.md`, and drop its Grok/`PING:` logic while Grok is suspended.

**2026-11-13: Grok is suspended; Claude runs the project solo ([decision](2026-11-13-dale-decision-grok-suspended.md), `GROK.md` to reinstate).** Every queue item below is Claude's, whatever its old owner marker.

## Waiting on Dale
- Device tests in the **Device test queue, issue #109** (pinned), from **Releases → dev**: #102 brightness, #104 TF2 ROMs, #88 Wi-Fi connect, #85 buttons; optional #95 accent bars. Deploy only a dev build whose SHA is a **merge** commit (the release at `f3ecab1` was an ACTIVITY commit; #117 fixes that from the next merge on).
- Dale's local clone `Documents\GitHub\Anbernic_RG34XXSP_Redux` is at `0afecd9` (#82), CRLF-only diffs. Before building from it: `git config core.autocrlf true && git fetch && git reset --hard origin/main`.

## Queue (inbox #41 is the source of truth; in order)
1. ~~Wi-Fi part 2: Connecting and result screens~~ merged in #73 (device test 2026-11-12: connect failed; fixed in #88). **Grok**
0a. ~~Wi-Fi connect fix~~ merged in #88 (verify by FIELD:value, saved-profile modify+up, one row per SSID). Not device-checked yet. **Grok**
1a. ~~Map unmapped stock folders~~ done in #70 (HBMAME/OPENBOR/VARCADE left). **Claude**
2. ~~System Tools (read-only, `sysinfo.c` data layer)~~ merged in #74, VE/DDR fixed in #83 (device-verified). **Grok**
3. ~~BIOS-style UI, colour presets~~ (#81/#82) and ~~Restart/Power Off~~ (#89, device-verified). **Grok**
3a. ~~Confirm screens: a short A tap stays on the screen and resets the hold bar~~ merged in #91, device-verified. **Grok**
3b. ~~Settings → System layout~~ merged in #91, device-verified. Reset All still to come = Redux's own `/mnt/data/redux/` settings to defaults only, 3 s hold. **Grok**
3c. ~~persistent volume~~ (#94, device-verified) → ~~brightness~~ merged in #102 (Settings > Display, 1–20) → ~~TF2 ROM scan~~ merged in #104 (TF1 + TF2 merged per system). Brightness and TF2 **not device-checked yet**. **Claude**
4. ~~App Center~~ merged in #119 (Apps list; Music/Video/Documents/File Manager launch stock apps; .dge cd fix #115). **Not device-checked yet** (#109). **Claude** · 5. Ports, option B ([decision](2026-11-13-dale-decision-ports.md)): known-ports table, then the card-script switch (Dale merges) **Claude, in progress** · 6. Terminal/htop (SimpleTerminal `-e htop`, MENU quits) · 7. ~~TF2 scan~~ (#104) · 8. 640×480 boot logo (Dale-only installer)
- L3/R3 (313/316): stay unmapped, left available for mapping later ([decision](2026-11-12-dale-decision-brightness-l3r3.md)).
- Backlog: **DOS Mode** (Dale: keep the development menu style as a DOS Mode option alongside BIOS Mode; he'll define the layout as menus grow), ASCII art maker (Draw, Convert, Edit, Share/Save As; App Center app), PC link (browser file manager for both cards and Redux data; security item, Dale-gated; ROM upload after an ownership acknowledgement), Expert Mode shortcut code (undocumented; thermals untouched by any unlock), chiptune composer (port LittleGPTracker's RG35XXPLUS build, App Center app, after Ports; #41), power and lid (lid already sleeps; wake is power-only), Wi-Fi idle scanning, media (mpv/ffplay present), features.ini, Expert Mode, the Phase 2 mainline track.

## Open notes
- [2026-11-13-dale-decision-ports.md](2026-11-13-dale-decision-ports.md): dale → all (Ports option B: known-ports table + card-script switch, default Off)
- [2026-11-13-dale-decision-grok-suspended.md](2026-11-13-dale-decision-grok-suspended.md): dale → all (Grok suspended; Claude solo; replaces "sidelined")
- [2026-11-13-dale-decision-app-center.md](2026-11-13-dale-decision-app-center.md): dale → all (App Center = local launcher, no downloads)
- [2026-11-13-claude-message-stock-apps.md](2026-11-13-claude-message-stock-apps.md): claude → dale, all (stock apps and how dmenu.bin launches them; .dge needs cd, #114/#115)
- [2026-11-13-dale-decision-app-center-menu.md](2026-11-13-dale-decision-app-center-menu.md): dale → all (option A: four menu entries launch stock apps, plus a top-level Apps list)
- [2026-11-13-dale-decision-claude-runs-phase.md](2026-11-13-dale-decision-claude-runs-phase.md): dale → all (Claude builds and merges this phase; Grok on call)
- [2026-11-12-dale-decision-brightness-l3r3.md](2026-11-12-dale-decision-brightness-l3r3.md): dale → grok (brightness go, 1–20 in config; L3/R3 stay unmapped)
- [2026-11-12-dale-decision-volume-stages.md](2026-11-12-dale-decision-volume-stages.md): dale → all (Redux Vol+/- is the hardware master; RetroArch keeps its own trim. Not a defect; do not add code to sync the two numbers)
- [2026-11-12-dale-hw-result-build-0afecd9.md](2026-11-12-dale-hw-result-build-0afecd9.md): dale → all (0afecd9 on device: Colors, System Tools, Saturn pass; Wi-Fi connect fails, nmcli exits 0 on error)
- [2026-11-12-claude-message-stock-v106-diff.md](2026-11-12-claude-message-stock-v106-diff.md): claude → all (stock V1.0.6 supports revised boards; 2025-based boot.img must not go on them)
- [2026-11-10-dale-hw-result-f2-menu-on-device.md](2026-11-10-dale-hw-result-f2-menu-on-device.md): dale → all (F2 menu on device)
- [2026-11-10-dale-decision-eng-tabs-back.md](2026-11-10-dale-decision-eng-tabs-back.md), [text-size](2026-11-10-dale-decision-text-size.md), [power-management](2026-11-10-dale-decision-power-management.md): decisions → grok
- [2026-11-10-dale-hw-result-phase1-boot.md](2026-11-10-dale-hw-result-phase1-boot.md): dale → all (Phase 1 boots)
- [2026-11-10-claude-message-f3-font-picker.md](2026-11-10-claude-message-f3-font-picker.md): claude → grok (F3, FYI)
- [2026-11-10-claude-milestone-ram-system-design.md](2026-11-10-claude-milestone-ram-system-design.md): claude → all (FYI)

F0 and F1 notes are all closed or acknowledged. See `notes/` for history.
