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

## Handoff (2026-11-14, Claude): start here in a new chat
- **Read [handoff-2026-11-14.md](handoff-2026-11-14.md) next** (current state, what's merged and not device-checked, facts from the device, next task), then [handoff-2026-11-13.md](handoff-2026-11-13.md) for the lessons. New chats: `HELLO` in the Redux Project, **started from the desktop app on DLN-DALEW2** (the computer link failed all of the last chat).
- Previous chats archived as: **Redux 2026-11-10→11-12: F2 launch parity, Wi-Fi, BIOS UI and Expert Mode plans**; **Redux 2026-11-12→11-12: F2 sign-off, page jump, MAME + 27 systems, ROM cap, Beta roadmap**; **Redux 2026-11-12→11-12: device test, button remap, Wi-Fi verify, stock V1.0.6 diff, power menu**; **Redux 2026-11-12: volume, accent bars, native arm64 CI**; **Redux 2026-11-12→11-13: Claude takeover, brightness, TF2 ROM scan, App Center plan, agent harness, command words**; **Redux 2026-11-13→11-13: Device fixes 9a-g, Screen Timeout, htop + extras installer, Tools apps, OpenBOR**
- **Deploy only from main's CI runs** (the arm64 artifact). The title bar shows `v0.1-dev - <sha>` and must match main's **merge** commit. A SHA not in `git log` is GitHub's PR merge preview. Testing an unmerged PR's own build is fine; say so.
- **Dale starts his own GitHub comments with `DALE:`**; Grok's start with `PING:`. Both post from `wilcox101-maker`. Every chat swap needs Dale's **SWAP**.
- Device: at work 172.16.3.178 (`DLN_TeamWireless`), at home the saved `home` profile, around 192.168.1.13. Dale's PC `Downloads\Anbernic RG34xxSP` holds the 2025 `RG34XXSP.img` and `.FW\` stock V1.0.6 (read-only use).
- Scheduled task **"Redux CI hourly check"** is **paused** (disabled 2026-11-12 ~21:23 ET; last run 21:05). Don't re-enable it unless Dale asks. If he does: its prompt uses `gh pr list`, which fails in cloud sessions (GraphQL blocked), so switch it to the REST calls in `CLAUDE.md`, and drop its Grok/`PING:` logic while Grok is suspended.

**2026-11-13: Grok is suspended; Claude runs the project solo ([decision](2026-11-13-dale-decision-grok-suspended.md), `GROK.md` to reinstate).** Every queue item below is Claude's, whatever its old owner marker.

## Repo ownership
- 2026-11-14 07:02, Dale: repo control is the owning chat's call. **Owner: the "Attach with push" chat in the Redux Project.** The "Device fixes 9a-g" chat is advisory and read-only (it opened #171, closed as a duplicate of #170). A revived old chat is advisory unless Dale names it owner.

## Waiting on Dale
- **DECIDE vsync** for SimpleTerminal/File Manager tearing: A, B or C ([note](2026-11-14-claude-decision-vsync-stock-apps.md)).
- Paste both Project instruction blocks from `docs/project-setup.md` into the Redux and Redux Manager Projects (Manager relay, #174 merged 2026-11-14).
- **Device test queue #172** for dev `fc17904`: OpenBOR, Video Player + redux-diag.txt, F3 font/Reset All, Apps resume, #109 leftovers, queue 13. (#109, #143 closed; results in [hw-result-143](2026-11-13-dale-hw-result-143.md).)
- Dale's local clone `Documents\GitHub\Anbernic_RG34XXSP_Redux`: state unknown (it was at `0afecd9` on 11-12 and has been pulled since; the H700 project saw `d0de8e0` on 11-13). CRLF-only diffs are expected. Before building from it: `git config core.autocrlf true && git fetch && git reset --hard origin/main`.

## Queue (inbox #41 is the source of truth; in order)
1. ~~Wi-Fi part 2: Connecting and result screens~~ merged in #73 (device test 2026-11-12: connect failed; fixed in #88). **Grok**
0a. ~~Wi-Fi connect fix~~ merged in #88 (verify by FIELD:value, saved-profile modify+up, one row per SSID). Not device-checked yet. **Grok**
1a. ~~Map unmapped stock folders~~ done in #70 (HBMAME/VARCADE left; OPENBOR in #151). **Claude**
2. ~~System Tools (read-only, `sysinfo.c` data layer)~~ merged in #74, VE/DDR fixed in #83 (device-verified). **Grok**
3. ~~BIOS-style UI, colour presets~~ (#81/#82) and ~~Restart/Power Off~~ (#89, device-verified). **Grok**
3a. ~~Confirm screens: a short A tap stays on the screen and resets the hold bar~~ merged in #91, device-verified. **Grok**
3b. ~~Settings → System layout~~ merged in #91, device-verified. Reset All still to come = Redux's own `/mnt/data/redux/` settings to defaults only, 3 s hold. **Grok**
3c. ~~persistent volume~~ (#94, device-verified) → ~~brightness~~ merged in #102 (Settings > Display, 1–20) → ~~TF2 ROM scan~~ merged in #104 (TF1 + TF2 merged per system). Brightness and TF2 **not device-checked yet**. **Claude**
4. ~~App Center~~ merged in #119 (Apps list; Music/Video/Documents/File Manager launch stock apps; .dge cd fix #115). **Not device-checked yet** (#109). **Claude** · 5. Ports, option B ([decision](2026-11-13-dale-decision-ports.md)): ~~known-ports table~~ merged in #124 (not device-checked); ~~Card Scripts switch~~ merged in #125 (Dale). Neither device-checked yet (#109). **Claude** · 6. ~~Terminal/htop~~ merged in #129: Apps > Terminal and Task Monitor (now **htop**, #146, Dale's choice C; htop is **not stock**, Dale apt-installed it, so Redux must supply it: queue 10), the mod's SimpleTerminal on rootfs. Not device-checked (#109). **Claude** · 7. ~~TF2 scan~~ (#104) · 8. Boot logo: ~~stock-format render~~ merged in #132 (640×480, 921,654 bytes); ~~Boot Screen manager~~ merged in #133 (Dale; writes boot-resource; in-place overwrite, backup + restore). **Install device-verified 2026-11-13**; restore not reported yet.
9. **Device result 2026-11-13** ([note](2026-11-13-dale-hw-result-app-center-ports-boot.md)), fixes in this order. **Claude**
   a. Brightness: bar moves, backlight unchanged; M button fails on Settings > Display
   b. ~~Boot Screen~~: build was `168c8bc`; the switch **works on the device** ([result](2026-11-13-dale-hw-result-boot-screen.md)); restore not reported yet
   c. Ports: Y, X, Start, Select don't work in-game. Dale tested **Dinothawr**, which uses only D-pad, B, A, X (libretro docs); `RA_launch.sh` sets nothing port-specific. Likely not a bug: re-checks on #109 (X reset, Wolf3D/Quake)
   d. ~~Video Player: playing a video drops back to Redux~~ fix in #142: `vp.dge` hands playback back through a new `/tmp/.next` (`./FFplay.sh`) and `/tmp/vpRun.ini`, which only stock's menu slot runs; Redux now runs the player in a loop. Not device-checked · e. Ebook Reader: M drops back to Redux. The reader is Pixel Reader; M may be its normal quit. Re-check against the stock menu
   f. ~~File Manager: screen tears while loading~~ **Stock OS bug**, same as SimpleTerminal tearing when changing screens (#41, 2026-11-11: stock SDL 2.0.12 has no vsync; `SDL_RENDER_VSYNC=1` didn't help). Not Redux: before a launch Redux already fills every buffer with the Loading screen. Fix only by rebuilding the app (backlog) · g. ~~Screen timeout setting~~ #145: Settings > Display > Screen Timeout (Off, 1-30 min, default 5); idle in the Redux menu runs stock `pwr_new.sh auto` (suspend; power wakes). Not device-checked
10. **htop for all** (Dale, 2026-11-13: option C): htop is not stock; Task Monitor runs `/usr/bin/htop` (#146). Dale chose a repo installer: ~~`tools/extras.py`~~ #149, checklist of Ubuntu packages (`tools/extras.ini`, htop ticked) → copy/paste SSH line; simulates first and **refuses any upgrade** (cbepx-me #300: upgrades can break RetroArch). **Run on the device 2026-11-13** ([hw-result](2026-11-13-dale-hw-result-extras.md)): htop already there, nothing upgraded; one-ssh-session `--run` and `--check --run` follow-up. Not yet run: REFUSED path, a fresh install. **Claude**
11. **Core Apps, option A** ([decision](2026-11-13-claude-decision-core-apps.md)): ~~Tools apps~~ #150 (Audio Mixer, Network Setup, Bluetooth, Input Test, Speaker Test; not device-checked) → ~~OpenBOR in Game Room~~ #151 (OPENBOR folder, `.pak` via stock `deep/openBOR/scripts/openbor.sh`; not device-checked). **Claude**
12. **Device result 2026-11-13 11:34** ([note](2026-11-13-dale-hw-result-143.md), #143), fixes in this order. **Claude**
   a. Video Player (#142) crashes to Main: second failed round. #159 adds diagnostics: `[video]` lines in `redux.log` (player/playback exit codes, what `vp.dge` leaves in `/tmp/.next` and `vpRun.ini`, last output). Waiting on Dale's log · b. OpenBOR (#151) crashes: cause `openbor.sh` only seeds config (#167). **Dale chose A** (2026-11-14): stock launch, `openbor.sh` then `OpenBOR.dge "<pak>"`, AGENTS.md exception (#170, merged by Dale). Not device-checked · c. ~~Apps return to Main~~ #157: an app started from the Apps list brings Redux back on the Apps list, same row (resume file, as for ROM lists); Music/Video/Documents/File Manager from Main still come back to Main. Not device-checked · d. ~~File Manager listed in both Main and Apps~~ #158 (Dale: A): stays on Main, removed from the Apps list. Not device-checked
   Passed: htop, the five Tools apps and the extras install line inside SimpleTerminal; PortMaster Wolfenstein (shoulders strafe/weapon). Not reported: Screen Timeout, Ebook M, brightness, TF2, Card Scripts, Boot Screen restore, Wi-Fi, buttons.
13. **Device checks from the kernel research** (one SSH session on stock; read-only). **Dale**
   a. `cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies` (the unit's CPU grade) · b. `find /sys -name '*adc_cal*'` (stick calibration file) · c. `evtest`, close and open the lid: which device sends KEY_DELETE / KEY_INSERT
14. **F3 settings parity** (Dale: A, 2026-11-14 03:21). **Claude**
   a. UI font picker (Settings > Display, five fonts, ui-spec 8a): built, not device-checked · b. Reset All (Settings > System, 3 s hold): built, not device-checked
- Tools: `tools/bench` (#130), Dale's read-only RAM/CPU probe over SSH, for the Phase 1/2 measurements; needs SSH on the device (stock mod's temporary SSH server).
- L3/R3 (313/316): stay unmapped, left available for mapping later ([decision](2026-11-12-dale-decision-brightness-l3r3.md)).
- Backlog: **vsync rebuilds of stock apps** that tear (SimpleTerminal from st-sdl, #41; File Manager, 9f), **DOS Mode** (Dale: keep the development menu style as a DOS Mode option alongside BIOS Mode; he'll define the layout as menus grow), ASCII art maker (Draw, Convert, Edit, Share/Save As; App Center app), PC link (browser file manager for both cards and Redux data; security item, Dale-gated; ROM copy only in **[Xroot Mode]**, after the hidden ownership acknowledgement, [decision](2026-11-13-dale-decision-xroot-mode.md)), Expert Mode shortcut code (undocumented; thermals untouched by any unlock), chiptune composer (port LittleGPTracker's RG35XXPLUS build, App Center app, after Ports; #41), power and lid (lid already sleeps; wake is power-only), Wi-Fi idle scanning, media (mpv/ffplay present), features.ini, Expert Mode, the Phase 2 mainline track, **ZRAM: v1.x (after 1.0)** ([decision](2026-11-13-dale-decision-zram-v1x.md)).

## Open notes
- [2026-11-14-claude-decision-vsync-stock-apps.md](2026-11-14-claude-decision-vsync-stock-apps.md): claude → dale (tearing in SimpleTerminal/File Manager: A rebuild, B vsync shim (recommended, after a vblank check), C leave as stock)
- [2026-11-14-dale-decision-manager-relay.md](2026-11-14-dale-decision-manager-relay.md): dale → all (Manager relays routine commands as `MGR for DALE:`; merges and decisions stay Dale's)
- [2026-11-14-dale-decision-manager-relay.md](2026-11-14-dale-decision-manager-relay.md): dale → all (Manager relays routine commands as `MGR for DALE:`; merges and decisions stay Dale's)
- [2026-11-14-dale-decision-openbor-launch.md](2026-11-14-dale-decision-openbor-launch.md): dale → all (12b OpenBOR: option A, stock launch; AGENTS.md p1 exception for the engine's own files)
- [2026-11-14-dale-decision-mainline-cpu-table.md](2026-11-14-dale-decision-mainline-cpu-table.md): dale → all (mainline CPU table: ROCKNIX H700 table, 1512 MHz behind Expert Mode; stock track unchanged)
- [2026-11-13-dale-message-kernel-research.md](2026-11-13-dale-message-kernel-research.md): dale → all (kernel and Ubuntu research in docs/kernel-research/; Ubuntu 24.04 track is plan only, not on the ROADMAP; never use the Knulli guide's `dd` offsets here)
- [2026-11-13-claude-decision-core-apps.md](2026-11-13-claude-decision-core-apps.md): claude → dale (Core Apps; Dale chose A: five Tools apps + OpenBOR)
- [2026-11-13-dale-hw-result-boot-screen.md](2026-11-13-dale-hw-result-boot-screen.md): dale → all (Boot Screen switch works on 168c8bc)
- [2026-11-13-dale-hw-result-app-center-ports-boot.md](2026-11-13-dale-hw-result-app-center-ports-boot.md): dale → all (device session 08:06: Terminal, Task Monitor, Music, Ports launch PASS; brightness, video, ebook M, port buttons FAIL; Boot Screen not seen)
- [2026-11-13-dale-decision-xroot-mode.md](2026-11-13-dale-decision-xroot-mode.md): dale → all (ROM copy in the PC link runs in [Xroot Mode], after the hidden ownership acknowledgement; first record of the #41 PC link decision; not Expert Mode)
- [2026-11-13-dale-decision-zram-v1x.md](2026-11-13-dale-decision-zram-v1x.md): dale → all (ZRAM required for v1.x on mainline, after 1.0; 2 GB default decided later from measurements)
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
