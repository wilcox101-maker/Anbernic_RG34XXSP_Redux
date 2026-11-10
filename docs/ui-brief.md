# Brief: RG34XXSP Redux frontend (for an AI collaborator)

You are helping Dale build **RG34XXSP Redux**, a custom firmware for the Anbernic RG34XXSP clamshell
handheld. Repo: https://github.com/wilcox101-maker/Anbernic_RG34XXSP_Redux (GPL-2.0).
Read `README.md`, `ROADMAP.md` and everything in `docs/` before proposing anything (this file is `docs/ui-brief.md`). Those files are the
source of truth. This brief summarises them and adds the UI direction.

**How decisions work:** Dale guides the UI at a high level: the look, priorities and what matters.
You make the detailed design calls yourself, consistent with this brief. Bring Dale decisions only at
phase gates, or when a choice changes what the user sees or gives up. Then offer 2–3 concrete
options with a recommendation, not open questions.

Your job is **the user-facing layer**: the launcher/frontend, its settings, and "Engineering Mode". The
kernel, boot chain and system image are handled separately. Don't redesign them.

---

## 1. Project rules (non-negotiable)

- **Stability first.** The goal is to raise the hardware to what it was built for, *without* instability.
- **No overclocking or undervolting past the stock OPP table** (a stated non-goal). "No hard caps"
  means no artificial software limits *below* what the hardware and its validated tables allow.
- **Never commit Anbernic binaries, ROMs or BIOS files.** Reference stock files by path only.
- **Never format a partition automatically.** Stock reformats the ROM card when a mount fails. We
  mount read-only and show a warning instead.
- **No silent root code from SD cards.** Stock runs `update.sh` from any card as root. We have one
  explicit, opt-in update path.
- Host tooling is **Python 3 standard library only** and must run on Windows (Dale's PC).
- Staged kernel plan: Phase 1 = stock 4.9 BSP kernel with small patches. Phase 2 = mainline
  (ROCKNIX-derived) kernel, Panfrost GPU, read-only squashfs system plus an ext4 data partition. The
  frontend must work on **both**: write against SDL2, which can run on the stock fbdev path and on
  KMSDRM under mainline. No dependency on Anbernic's custom `SDL_VIDEODRIVER=anbernic`.

## 2. Hardware the UI must respect

| Item | Fact |
|---|---|
| SoC | Allwinner H700, 4× Cortex-A53, one cluster, up to 1512 MHz (bin-dependent OPPs: 480/600/792/1008/1200/1296/1416/1512) |
| GPU | Mali-G31. Stock DVFS tops out at 648 MHz @ 960 mV |
| RAM | 2 GB on Dale's unit (512 MB / 1 GB variants exist: treat RAM as unknown and adapt) |
| Display | **720 × 480**, ≈ 59.16 Hz, PWM backlight 0–255 |
| Buttons | D-pad, A B X Y, L1 L2 R1 R2, Start, Select, Menu (M), power, volume. Stock emits scrambled codes (see `docs/hardware-map.md`). Phase 2 emits standard gamepad codes. **Use one remap table, never hard-coded codes** |
| Lid | Hall sensor. Stock: `/sys/class/power_supply/axp2202-battery/hallkey`. Phase 2: standard `SW_LID` input |
| Power | AXP2202 PMIC: battery %, charging state, temperatures, LEDs, rumble via sysfs |
| Thermal | SoC: passive at 60/70 °C, critical 115 °C (kept as stock) |
| **Battery heat** | Dale's unit has a **silicone thermal pad between the battery and the mainboard chips**, so SoC heat flows into the battery under load. Stock sets `pmu_bat_temp_enable = <0x0>`: the PMIC's battery-temperature protection is **off**, although charge/shutdown thresholds are defined. Whether the battery even has a working thermistor is unverified |
| Wireless | RTL8821CS Wi-Fi (known to fail probe or resume sometimes: the UI must cope) and Bluetooth |

## 3. UI direction: 90s BIOS / DOS terminal

**Three colours, no exceptions:**
- Background: **black**
- Text: **white** (one shade only)
- **One** accent colour for focus/highlight (selected row = accent bar with black text, or accent
  text). Pick it once and make it a setting. Suggested default: BIOS cyan or amber.

**Layout:**
- Fixed bitmap font, **8×16** (VGA-style), which gives a **90 × 30 character grid** at 720×480. Every
  screen is laid out in character cells, with no free positioning. Optional 2× scale (45 × 15) for
  accessibility.
- Text-mode box drawing (single/double line characters) for frames and section headers.
- Screen skeleton: title bar (row 0), body, help bar on the last row (`A:Select  B:Back  M:Menu …`),
  status in the top-right (battery %, charging, Wi-Fi, clock, CPU temp).
- No images, animations, wallpapers or transparency in the core UI. (Box art can be an optional
  per-list toggle later. It is not in v1.)
- Instant response: every input redraws within one frame. No transitions.

**Why it fits the hardware:** a text grid redraws in microseconds, needs no GPU, and uses almost no RAM.
That leaves the full CPU/GPU budget to the emulators, and it renders the same on fbdev and on DRM.

## 4. Feature parity: everything stock does must still exist

From `docs/stock-userspace.md`, the stock OS provides:

- **Launch:** RetroArch 1.19.1 + cores, PPSSPP, DraStic (NDS), standalone `.dge` emulators (FC, SFC,
  MD, SMS/GG, GB/GBC, GBA, PCE, WS, NGP, MAME/FBA variants), PCSX, PortMaster ports, PICO-8 stub
- **Apps:** file manager, music player, video player, ebook/PDF reader, charging screen
- **Resume last game** at boot (`/mnt/mod/.last`)
- **Power key:** tap = sleep, M + power = shut down (configurable 0/1/2). HDMI connected = sleep only
- **Lid close / screen timeout:** sleep (or shut down if "lock" is set), nothing while on HDMI
- **SD-card hot-plug:** second card mounted at `/mnt/sdcard`
- **Settings:** Wi-Fi, Bluetooth (pads/audio), HDMI out, brightness, volume, rumble, status LED colours
  (boot/launch/exit), language, Samba and SSH toggles, RetroArch shaders/bezels, per-game overrides,
  random-game, cheats (NDS), USB pad autoconfig

## 5. Features stock left out (candidates, cheap on hardware)

- **One power daemon** instead of each app handling the power key itself (stock: if the foreground app
  hangs, nothing sleeps). Suspend sequence: pause → flush saves → `sync` → suspend → resume to the same
  frame.
- **Battery-heat guard:** the stock PMIC protection is disabled, so the frontend watches battery
  temperature (if readable) and the SoC temperature as a proxy (the thermal pad couples them).
- **Performance profiles** per system/game (Powersave / Balanced = schedutil / Performance), applied
  at launch and *restored on exit*. Stock lets the music/video players and DraStic leave the governor
  in a random state.
- **Battery screen:** %, voltage, current, charge state, estimated time left, drain per hour log.
- **Filesystem health:** run fsck on boot when a filesystem is dirty, show errors, never format.
- **Wi-Fi recovery:** reload the driver after a failed probe or resume, and show the state clearly.
- **Save-state safety:** auto-save on lid close and low battery.
- **Text-mode system log viewer** (dmesg / last boot).
- Integer-scaling and refresh-rate settings surfaced plainly (720×480 = 3× GBA).

## 6. Engineering Mode

Entered by a deliberate combo (e.g. hold M + Select at boot or from the settings screen). Visibly
marked (`[ENGINEERING]` in the title bar). Everything here is real and live, read straight from
sysfs, with no artificial limits inside the validated hardware tables:

- **CPU:** governor (all available), min/max frequency across the *full* OPP list the chip's bin
  allows, per-core online/offline, live frequency and load per core
- **GPU:** DVFS on/off, fixed frequency within its table, live frequency
- **Thermal:** all zones live, trip points (read-only by default; editing behind a second confirm)
- **Battery temperature:** live reading, plus the max seen this session. This is the one limit that
  stays enforced even in Engineering Mode. It is a cell-safety limit, not a performance cap: above a
  configurable threshold, drop to the Balanced profile and warn; above a hard threshold, stop charging
  and save + shut down. Defaults come from the battery's datasheet once identified (typical Li-ion
  ceilings are about 45 °C charging, 60 °C discharging). Until there's a verified sensor, show
  "battery temp: unknown" rather than a guess
- **Power:** PMIC registers/attributes read-out, charge current, LED and rumble test
- **Input:** live event viewer (codes as received and after remap), lid state, poll interval
- **Display:** backlight raw value, refresh/timing read-out
- **Storage:** partition table, mount options, free space, fsck on demand
- **Logs:** dmesg, frontend log, last-crash dump

**Safety net (required, so no-caps can't brick a session):**
- Changes apply live but are **not persisted** until "Save as profile" is chosen.
- A boot-time **watchdog/fallback**: if the system fails to reach the menu twice in a row, boot with
  engineering settings ignored.
- Every write is logged (`what, old value, new value, time`) so a bad change is easy to find.

## 7. Frontend phases

These are the **frontend's own phases** (F0, F1, …). They are separate from the project's kernel phases
in `ROADMAP.md`:
- Project Phase 0: dissect stock (done)
- Project Phase 1: stock-kernel tweaks (built, on-device test pending)
- Project Phase 2: mainline kernel

Work one phase at a time. A phase is done only when its **exit test** passes and Dale signs it off.
Don't start the next phase early.

| Phase | Goal | Deliverables | Exit test |
|---|---|---|---|
| **F0: Spec** | Agree on what gets built, on paper | Confirm on the device whether `/sys/class/power_supply/axp2202-battery/temp` gives a real battery temperature (compare idle vs. after 20 min of load). `docs/ui-spec.md`: grid, colours, font (licence must be GPL-compatible), screen list, navigation map, button map. 90 × 30 ASCII mock-ups of: main menu, system/ROM list, settings, Engineering CPU page, battery screen. `docs/settings-schema` (one INI or JSON file) covering every stock setting in §4, defaults = stock behaviour | Dale approves the mock-ups and the schema. No code yet |
| **F1: Desktop prototype** | Prove the look and the navigation | C + SDL2, one binary: bitmap-font grid renderer, the three-colour theme, the remap table, menu navigation with fake data. Runs on a PC window at 720×480 | Every F0 mock-up is reproduced cell-for-cell. Each input redraws within one frame. Controls work through the remap table only |
| **F2: Launch parity on stock** | Replace Anbernic's menu *without* losing any launcher | Runs on a **copy** of the stock card in place of `dmenu_ln` / `dmenu.bin`. Reads the ROM folders and starts RetroArch, PPSSPP, DraStic, the `.dge` emulators, PCSX and PortMaster the way stock does. Returns to the menu on exit. Resume-last-game | Every launch item in §4 starts a game and returns cleanly. The stock card is untouched |
| **F3: Settings and system parity** | Every stock setting exists and works | Wi-Fi, Bluetooth, HDMI, brightness, volume, rumble, LED colours, language, Samba/SSH, shaders/bezels, per-game overrides, random game, NDS cheats, USB pad autoconfig, SD hot-plug, charging screen | A checklist of every §4 item, each ticked on the device |
| **F4: Power and lid** | One reliable owner of sleep and shutdown | A small power daemon that owns the power key and lid. Suspend sequence: pause → flush saves → `sync` → suspend → resume. Performance profiles applied at launch and **restored on exit** | 50 lid-close/open cycles mid-game with no lost save. The governor is identical before and after every app, including the music/video players and DraStic |
| **F5: Engineering Mode** | No-caps control inside the validated tables | Everything in §6, including the safety net: live-not-persisted changes, the twice-failed-boot fallback, the write log | Every control reads back the real sysfs value. Deliberately bad settings recover through the fallback without reflashing. 30 min at max CPU/GPU while charging: the battery-heat guard triggers before the threshold is crossed |
| **F6: Features stock left out** | Add §5 items one at a time | Battery screen, fsck on dirty filesystems, Wi-Fi recovery, auto-save on lid/low battery, log viewer, scaling/refresh settings | Each feature has its own test, and battery drain or boot time is no worse than F4 |
| **F7: Mainline port** | Same frontend on the project's Phase 2 kernel | SDL2 on KMSDRM, standard gamepad codes (remap table updated only), `SW_LID` instead of `hallkey`, Panfrost | F2–F6 checklists pass again on mainline with no UI code changes outside the hardware layer |
| **F8: Release** | Something others can install | Install/update path (one explicit, opt-in method), user docs, measurements against stock (boot time, idle drain, input latency) | A clean card boots to the menu, and the docs match what's on screen |

Phases after F8 aren't decided yet. They'll be set from what F6–F8 turn up.

**Hardware-layer rule (it makes F7 cheap):** every sysfs path, input code and device name lives in one
small "platform" module with a stock implementation and a mainline implementation. UI code never
touches `/sys` directly.

When you propose code, keep it small, readable and commented, and say what you have *not* verified on
real hardware.
