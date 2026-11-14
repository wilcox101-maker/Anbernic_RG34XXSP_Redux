# UI Specification — RG34XXSP Redux Frontend (F0)

**Status**: Locked tree + draft for review  
**Grid**: 90 columns × 30 rows (8×16 bitmap font at 720×480)  
**Colours**: Background black, text white, one accent (default BIOS cyan).  
Selected row = accent background bar with black text.  
**Input**: Single remap table only. Stock event codes are never hard-coded.  
**Unverified**: Battery thermistor. Display "unknown" until on-device confirmation that `/sys/class/power_supply/axp2202-battery/temp` changes under load.  
**Phase 2 note**: Root filesystem is read-only. Frontend writes only to the data partition via the platform module.

---

## 1. Locked interface tree

```
RG34XXSP Redux
│
├── Game Room
│   ├── Recently Played
│   ├── Favorites
│   └── Systems
│       ├── Arcade
│       ├── Game Boy
│       ├── Game Boy Color
│       ├── Game Boy Advance
│       ├── NES
│       ├── SNES
│       ├── Genesis / Mega Drive
│       ├── PlayStation
│       ├── Nintendo 64
│       ├── Dreamcast
│       ├── Nintendo DS
│       ├── PSP
│       ├── ...
│       └── PortMaster
│
├── Music
├── Video
├── Documents
├── File Manager
├── Apps                       (App Center: stock apps + Redux tools; decision 2026-11-13)
├── Internet
│   ├── YouTube                (if feasible, no earlier than F6)
│   └── Web browser (text)
│
├── Network
│   ├── WebUI                  (On/Off, default Off)
│   ├── SSH                    (On/Off, default Off)
│   ├── Samba                  (On/Off, default Off)
│   ├── Terminal
│   └── ...
│
├── Settings
│   ├── Display
│   ├── Audio
│   ├── Input
│   ├── Power & Lid
│   ├── System               (System Tools, Restart, Power Off, Boot Screen*, blanks, Stock Menu, Reset All*)
│   ├── Emulation
│   └── UI                     (colour presets)
│
└── Engineering Mode           (deliberate confirm required)
    ├── CPU
    ├── GPU
    ├── Thermal
    ├── Battery
    ├── Power / PMIC
    ├── Input / Remap
    ├── Display
    ├── Storage
    └── Logs
```

**Deliberate removals from stock parity** (so F3 does not treat them as missing):
- WiliWili (Bilibili client)
- Network Radio
- TV_Live

**Claude review flags incorporated**:
1. Deliberate removals recorded above.
2. YouTube marked "if feasible", no earlier than F6.
3. Engineering Mode requires deliberate confirm (hold A for 2 s or brief combo).
4. SSH / Samba / WebUI default Off. First enable requires password. No default credentials. WebUI local-network only while On.
5. ~~Top-level Network and Settings → Network open the same screen.~~ Settings → Network removed as redundant (Dale, 2026-11-12); Network is on the main menu only.

---

## 2. Screen skeleton

- Row 0: Title bar (left: screen name, right: status)
- Rows 1–28: Body (box borders on rows 1 and 28, 26 content rows inside)
- Row 29: Help bar

Status format (right-aligned to column 90): `BAT 87% CHG WIFI CPU 54C 12:34`. Fields drop from the left when a screen needs the room.

List movement: Up/Down move one row and wrap. On the Game Room lists (Systems and a system's ROMs), Left/Right move one page (the 20-row window) and stop at the first or last row instead of wrapping; the cursor keeps its row on screen. Engineering uses Left/Right for tabs; other screens leave them unused for now (for example, for changing values later).

Every mock-up is exactly 30 rows × 90 columns: title (row 0), box top border (row 1), 26 body rows, box bottom border (row 28), help bar (row 29).

---

## 3. Mock-up 1 — Main menu

```
RG34XXSP Redux  v0.1-dev - dev                                      BAT 87% CHG WIFI 12:34
┌────────────────────────────────────────────────────────────────────────────────────────┐
│▓▓ Game Room                                                                            │
│   Music                                                                                │
│   Video                                                                                │
│   Documents                                                                            │
│   File Manager                                                                         │
│   Apps                                                                                 │
│   Internet                                                                             │
│   Network                                                                              │
│   Settings                                                                             │
│   Engineering Mode                                                                     │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
└────────────────────────────────────────────────────────────────────────────────────────┘
A:Select  B:Back  M:Menu  Start:Search
```

## 4. Mock-up 2 — System / ROM list

ROMs come from both cards, read only: TF1 `/mnt/mmc/Roms` and TF2 `/mnt/sdcard/Roms` (skipped if no card is in). The same system on both cards is one list, sorted by name; the Loading screen shows which card a game is on. `REDUX_ROMS` overrides the roots (one path, or several joined with `:`).

```
Game Room > Systems > Game Boy Advance                              BAT 87% CHG WIFI 12:34
┌────────────────────────────────────────────────────────────────────────────────────────┐
│  Search: _                                                                             │
│                                                                                        │
│▓▓ Advance Wars                                                                         │
│   Fire Emblem                                                                          │
│   Golden Sun                                                                           │
│   Metroid Fusion                                                                       │
│   Pokemon Emerald                                                                      │
│   ...                                                                                  │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
└────────────────────────────────────────────────────────────────────────────────────────┘
A:Launch  B:Back  X:Favorites  Y:Info  Start:Search  Select:Random  Left/Right:Page
```

## 5. Mock-up 3 — Settings

```
Settings                                                            BAT 87% CHG WIFI 12:34
┌────────────────────────────────────────────────────────────────────────────────────────┐
│▓▓ Display                                                                              │
│   Audio                                                                                │
│   Input                                                                                │
│   Power & Lid                                                                          │
│   System                                                                               │
│   Emulation                                                                            │
│   UI                                                                                   │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
└────────────────────────────────────────────────────────────────────────────────────────┘
A:Enter  B:Back  M:Menu
```

## 6. Mock-up 4 — Engineering Mode, CPU tab

Engineering Mode is one screen with an AMI/Phoenix-style tab bar (decision 2026-11-10): CPU,
Battery, Thermal, Display, System. L1/R1 or Left/Right switch tabs and wrap; the active tab is
inverted in the accent colour (shown here as `▓CPU▓`). B exits to the main menu. Tabs without content
yet say "Not yet".

```
[ENGINEERING]                                                    BAT 87% CHG CPU 54C 12:34
┌────────────────────────────────────────────────────────────────────────────────────────┐
│ ▓CPU▓ Battery  Thermal  Display  System                                                │
│────────────────────────────────────────────────────────────────────────────────────────│
│  Governor:     schedutil                                                               │
│  Min freq:     480 MHz                                                                 │
│  Max freq:     1512 MHz                                                                │
│  Current:      1008 MHz                                                                │
│                                                                                        │
│  Per-core:                                                                             │
│    cpu0  1008 MHz   online                                                             │
│    cpu1  1008 MHz   online                                                             │
│    cpu2  1008 MHz   online                                                             │
│    cpu3  1008 MHz   online                                                             │
│                                                                                        │
│  Stock OPP table:                                                                      │
│    480 / 600 / 792 / 1008 / 1200 / 1296 / 1416 / 1512                                  │
│                                                                                        │
│  Live changes only. Persist with "Save as profile".                                    │
│  Boot watchdog resets settings after two failed boots.                                 │
│  Battery-heat guard remains active.                                                    │
│  Entry required deliberate confirm.                                                    │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
└────────────────────────────────────────────────────────────────────────────────────────┘
L1/R1:Tab  A:Edit  B:Exit  X:Save profile  Y:Reset
```

## 7. Mock-up 5 — Engineering Mode, Battery tab

```
[ENGINEERING]                                                    BAT 87% CHG CPU 54C 12:34
┌────────────────────────────────────────────────────────────────────────────────────────┐
│  CPU ▓Battery▓ Thermal  Display  System                                                │
│────────────────────────────────────────────────────────────────────────────────────────│
│  Capacity:     87 %                                                                    │
│  Voltage:      3.92 V                                                                  │
│  Current:     -420 mA                                                                  │
│  Status:       Discharging                                                             │
│  Time left:    ~4 h 12 m                                                               │
│                                                                                        │
│  Temperature:  unknown   (thermistor unverified)                                       │
│  Max seen:     --                                                                      │
│                                                                                        │
│  SoC temp:     54 C                                                                    │
│                                                                                        │
│  Stock PMIC protection: disabled. Redux guard limits:                                  │
│    Charge stop:     45 C   (typical Li-ion, not stock)                                 │
│    Discharge stop:  60 C   (typical Li-ion, not stock)                                 │
│                                                                                        │
│  Drain (last hour): -8 %                                                               │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
└────────────────────────────────────────────────────────────────────────────────────────┘
L1/R1:Tab  A:Edit  B:Exit  X:Save profile  Y:Reset
```


## 7a. Mock-up 6 — Internet

Replaces the old "YouTube (if feasible)" item (decision 2026-11-10). Items appear only once they
work; B returns to the main menu.

```
Internet                                                            BAT 87% CHG WIFI 12:34
┌────────────────────────────────────────────────────────────────────────────────────────┐
│▓▓ YouTube (if feasible)                                                                │
│   Web browser (text)                                                                   │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
│                                                                                        │
└────────────────────────────────────────────────────────────────────────────────────────┘
A:Enter  B:Back
```

---

## 7b. Mock-up 7 — Loading ROM

Wait screen drawn by `ui_busy` before a blocking action. The title bar uses the pointer colour.
No progress bar. The same window is used for the Wi-Fi scan, connect, ROM scan, and stock restore.

```
                                                                                          
                                                                                          
                                                                                          
                                                                                          
                                                                                          
                                                                                          
                                                                                          
                                                                                          
                                                                                          
                                                                                          
                  ┌▓▓───────────────────Loading ROM────────────────────┐                  
                  │ Game    Super Mario World                          │                  
                  │ System  SNES                                       │                  
                  │ Engine  RetroArch / snes9x                         │                  
                  │ Card    TF1                                        │                  
                  │────────────────────────────────────────────────────│                  
                  │ Starting emulator, please wait...                  │                  
                  │ Tip: press MENU in-game to open the RetroArch menu │                  
                  └────────────────────────────────────────────────────┘                  
                                                                                          
                                                                                          
                                                                                          
                                                                                          
                                                                                          
                                                                                          
                                                                                          
                                                                                          
                                                                                          
                                                                                          
                                                                                          
```

---

## 8. Settings schema (`settings.ini`)

Defaults match stock where known. Stock values come from `/mnt/mod/ctrl/configs/system.cfg` (see `docs/stock-userspace.md`); where no stock value is documented, the key is marked *unverified*. File lives on the data partition.

```ini
[display]
brightness = 128
accent = cyan            ; cyan | amber | green | white  (default BIOS cyan)          ; stock default unverified
font = vga                ; vga | spleen | unifont | thin | chunky (see "UI fonts" below)
hdmi_enabled = 0
integer_scale = 1
refresh_lock = 0

[audio]
volume = 70               ; stock default unverified
mute = 0

[input]
rumble = 1
usb_pad_autoconfig = 1
remap_file = default

[power]
power_key = 2             ; stock power.key=2
lid_action = suspend
screen_timeout = 300      ; seconds; stock default unverified
lock_on_lid = 0           ; stock power.lock=0 (1 = lid/timeout shuts down)
hdmi_power_key = suspend  ; stock: power key always suspends on HDMI
hdmi_ignore_lid = 1       ; stock power.hdmi=1: lid/timeout do nothing on HDMI

[network]
wifi_enabled = 1
bluetooth_enabled = 0
samba = 0                 ; stock global.samba=0
ssh = 0                   ; stock global.ssh=0
webui = 0

[system]
language = en
led_boot = default        ; stock: colour files led_boot/launch/exit.cfg, values unverified
led_launch = default
led_exit = default
resume_last = 1           ; stock: always on (shutdown in-game writes /mnt/mod/.last, boot resumes it)

[emulation]
default_core = auto
shaders = 1               ; stock global.shader=1
bezels = 1                ; stock global.bezel=1
arcade_auto = 0           ; stock arcade.auto=0
vertical_arcade = 0       ; stock varcade.vertical=0
ra_hotkey = 0             ; stock ra.hotkey=0
auto_load_state = 0       ; stock global.load=0 (auto-load save state at game start)
game_notes = 0            ; stock global.spy=0 (show a ROM's .txt/.epub guide)
cheats_nds = 0
random_game = 0
per_game_overrides = 1

[performance]
profile = balanced
restore_on_exit = 1

[engineering]
battery_temp_guard = 1
charge_temp_limit = 45    ; Redux default (typical Li-ion), not stock
discharge_temp_limit = 60 ; Redux default (typical Li-ion), not stock
```

**Schema notes**
- `power_key=2` (stock default): Menu held + power = shutdown; otherwise suspend. On HDMI the power key always suspends, and with `hdmi_ignore_lid=1` the lid and screen timeout do nothing (stock `power.hdmi=1`).
- `lid_action`: `suspend` or `shutdown`.
- SSH / Samba / WebUI default Off. First enable requires password. No default credentials.
- Performance profiles are applied at launch and restored on exit.
- Battery-heat guard is always active (even in Engineering Mode). Unverified thermistor shows "unknown" and falls back to SoC temperature.
- Remap table is external to this file.

---

## 8a. UI fonts (`display.font`)

Every font is drawn in the same 8×16 cell, so the 90 × 30 grid, every mock-up and the exit test stay
the same whichever font is chosen. Switching rebuilds one glyph texture; it costs nothing in games.
Each font must cover the box characters `┌ ─ ┐ │ └ ┘ ▓`. Missing glyphs fall back to `vga`.
Licences were checked against each font's own source (2026-11-10).

| Value | Font | Look | Licence | Source |
|---|---|---|---|---|
| `vga` (default) | VGA 8×16 | classic PC BIOS | GPL-2.0 | Linux `lib/fonts/font_8x16.c` (in use since F1) |
| `spleen` | Spleen 8×16 | clean modern terminal | BSD-2-Clause (© Frederic Cambus) | `github.com/fcambus/spleen`, `spleen-8x16.bdf` |
| `unifont` | GNU Unifont 8×16 | plain, very readable | GPL-2.0+ with font embedding exception (dual OFL-1.1) | `unifoundry.com/unifont` |
| `thin` | 7×14, centred in the 8×16 cell | lighter, airier | GPL-2.0 | Linux `lib/fonts/font_7x14.c` |
| `chunky` | Acorn 8×8, doubled in height | bold retro (Amiga/C64 feel) | GPL-2.0 | Linux `lib/fonts/font_acorn_8x8.c` |

Implementation is in **F3** (settings parity): fonts are converted by a stdlib Python tool into C
arrays at build time, keep their copyright/licence headers, and are listed in a `LICENSES` section of
`frontend/README.md`. The picker lives in Settings → Display and previews the font live.

**Built (F3, 2026-11-14):** Settings → Display row 3, `Font  < name >`, Left/Right cycles the five
fonts and the whole screen redraws in the new one; a sample line and box preview show while the row
is selected. Saved to `/mnt/data/redux/font` (one key, e.g. `spleen`). Generated tables are committed
(`frontend/src/font_extra.c`); the sources are fetched when regenerating, not kept in the repo.
Unifont lacks 16 CP437 glyphs, which come from VGA. **Not verified on the device.**

## 9. F0 exit criteria
- Dale approves the five mock-ups and the settings schema.
- No contradictions with `docs/ui-brief.md` or stock documentation.
- Battery thermistor status remains explicitly unverified until the on-device check is complete.
- Deliberate removals and Claude review flags are recorded.


## 9. System layout and Restart / Power Off

Settings → System, in order:

    System Tools
    Restart
    Power Off
    Boot Screen   Stock/Redux   (3 s hold: installs the Redux boot logo, or restores the saved stock one)
    Card Scripts  Off/On        (3 s hold to turn on; Ports option B)
    <blank>
    <blank>
    Stock Menu
    Reset All               (3 s hold: Redux's own settings back to defaults)

The cursor never lands on a blank row. **Reset All** (built 2026-11-14, F3) opens a confirm; a 3 s hold of A puts Redux's own settings in /mnt/data/redux/ back to defaults: colours (BIOS cyan), brightness (level 5, applied at once), screen timeout (5 min), font (VGA), Card Scripts (Off), and the saved volume (forgotten; the current level stays until the next start, then stock's level applies). It removes those files by name, nothing else in the folder. It never touches ROMs, saves, BIOS, Wi-Fi profiles, stock settings or the **Boot Screen** marker (that records what is on the boot partition; use Boot Screen to change it). **Not verified on the device.**

Restart and Power Off each open a confirm screen. Hold A for 3 seconds (HOLD_DESTRUCT_MS) to run. A tap that releases before 3 s stays on the confirm and resets the hold bar; nothing runs. Only B or another button leaves. The hold bar is the same as Stock Menu.

Restart runs `sync && reboot`. Power Off runs `sync && poweroff`. Desktop builds do not run either command; a test hook logs the command instead. Neither writes to /mnt/mmc.

    Restart                                                             BAT 87% CHG WIFI 12:34
      Runs sync, then reboot.
      Hold A for 3 seconds. Release, or any other button, cancels.
      [                                        ]
    Hold A 3s:Restart  B:Cancel

    Power Off                                                           BAT 87% CHG WIFI 12:34
      Runs sync, then poweroff.
      Hold A for 3 seconds. Release, or any other button, cancels.
      [                                        ]
    Hold A 3s:Power Off  B:Cancel

Not run on the device yet. Dale merges this one and checks that Restart returns to Redux and Power Off powers the unit off.
