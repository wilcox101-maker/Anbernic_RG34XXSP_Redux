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
├── YouTube                    (if feasible, no earlier than F6)
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
│   ├── Network                (same screen as top-level Network)
│   ├── System
│   └── Emulation
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
5. Top-level Network and Settings → Network open the same screen (same keys).

---

## 2. Screen skeleton

- Row 0: Title bar (left: screen name, right: status)
- Rows 1–28: Body (box borders on rows 1 and 28, 26 content rows inside)
- Row 29: Help bar

Status format (right-aligned to column 90): `BAT 87% CHG WIFI CPU 54C 12:34`. Fields drop from the left when a screen needs the room.

Every mock-up is exactly 30 rows × 90 columns: title (row 0), box top border (row 1), 26 body rows, box bottom border (row 28), help bar (row 29).

---

## 3. Mock-up 1 — Main menu

```
RG34XXSP Redux                                                      BAT 87% CHG WIFI 12:34
┌────────────────────────────────────────────────────────────────────────────────────────┐
│▓▓ Game Room                                                                            │
│   Music                                                                                │
│   Video                                                                                │
│   Documents                                                                            │
│   File Manager                                                                         │
│   YouTube (if feasible)                                                                │
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
│                                                                                        │
└────────────────────────────────────────────────────────────────────────────────────────┘
A:Select  B:Back  M:Menu  Start:Search
```

## 4. Mock-up 2 — System / ROM list

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
A:Launch  B:Back  X:Favorites  Y:Info  Start:Search  Select:Random
```

## 5. Mock-up 3 — Settings

```
Settings                                                            BAT 87% CHG WIFI 12:34
┌────────────────────────────────────────────────────────────────────────────────────────┐
│▓▓ Display                                                                              │
│   Audio                                                                                │
│   Input                                                                                │
│   Power & Lid                                                                          │
│   Network                                                                              │
│   System                                                                               │
│   Emulation                                                                            │
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

## 6. Mock-up 4 — Engineering Mode: CPU

```
[ENGINEERING] CPU                                                BAT 87% CHG CPU 54C 12:34
┌────────────────────────────────────────────────────────────────────────────────────────┐
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
│                                                                                        │
│                                                                                        │
└────────────────────────────────────────────────────────────────────────────────────────┘
A:Edit  B:Back  X:Save as profile  Y:Reset to stock
```

## 7. Mock-up 5 — Battery screen

```
Battery                                                                  BAT 87% CHG 12:34
┌────────────────────────────────────────────────────────────────────────────────────────┐
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
│                                                                                        │
│                                                                                        │
└────────────────────────────────────────────────────────────────────────────────────────┘
A:Refresh  B:Back  M:Menu
```

---

## 8. Settings schema (`settings.ini`)

Defaults match stock where known. Stock values come from `/mnt/mod/ctrl/configs/system.cfg` (see `docs/stock-userspace.md`); where no stock value is documented, the key is marked *unverified*. File lives on the data partition.

```ini
[display]
brightness = 128          ; stock default unverified
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
- Top-level Network and Settings → Network share these keys.

---

## 9. F0 exit criteria
- Dale approves the five mock-ups and the settings schema.
- No contradictions with `docs/ui-brief.md` or stock documentation.
- Battery thermistor status remains explicitly unverified until the on-device check is complete.
- Deliberate removals and Claude review flags are recorded.
