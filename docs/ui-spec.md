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
├── Internet
│   ├── YouTube              (if feasible, no earlier than F6)
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
│   ├── Network                (same screen as top-level Network)
│   ├── System
│   └── Emulation
│
└── Engineering Mode           (deliberate confirm required)
    ├── CPU
    ├── Battery
    ├── Thermal
    ├── Display
    └── System
```

**Deliberate removals from stock parity** (so F3 does not treat them as missing):
- WiliWili (Bilibili client)
- Network Radio
- TV_Live

---

## 3. Mock-up 1 — Main menu

```
RG34XXSP Redux                                                      BAT 87% CHG WIFI 12:34
┌─────────────────────────────────────────────────────────────────────────────────────────────────────┐
│▓▓ Game Room                                                                            │
│   Music                                                                                │
│   Video                                                                                │
│   Documents                                                                            │
│   File Manager                                                                         │
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
│                                                                                        │
│                                                                                        │
└─────────────────────────────────────────────────────────────────────────────────────────────┘
A:Select  B:Back  M:Menu  Start:Search
```

## 4. Mock-up 2 — System / ROM list

```
Game Room > Systems > Game Boy Advance                              BAT 87% CHG WIFI 12:34
┌─────────────────────────────────────────────────────────────────────────────────────────────────────┐
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
└─────────────────────────────────────────────────────────────────────────────────────────────┘
A:Launch  B:Back  X:Favorites  Y:Info  Start:Search  Select:Random
```

## 5. Mock-up 3 — Settings

```
Settings                                                            BAT 87% CHG WIFI 12:34
┌─────────────────────────────────────────────────────────────────────────────────────────────────────┐
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
│                                                                                        │
└─────────────────────────────────────────────────────────────────────────────────────────────┘
A:Enter  B:Back  M:Menu
```

---

## Notes

- Engineering Mode is now one screen with a tab bar (CPU, Battery, Thermal, Display, System).
- L1/R1 and Left/Right switch tabs. B exits to the main menu via the back stack.
- Internet is a sub-menu with YouTube (if feasible) and Web browser (text).
- Full eng/internet mock-ups are a follow-up. The three screens above are still checked.
