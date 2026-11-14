# RG34XXSP Redux: user guide

> Drafted by the read-only Redux chat against main `3bfc6e9` (2026-11-14); screens re-checked by
> the owner chat against main `97210d8`. **[device-checked]** passed on Dale's unit ·
> **[not yet on device]** built, not tested · **[placeholder]** on screen but does nothing yet.
> To install Redux, see [install.md](install.md).

Redux is a replacement menu for the stock Anbernic RG34XXSP firmware. It runs on the stock kernel
and launches the stock emulators and apps. Your games, saves and BIOS files stay where stock keeps
them.

## Buttons everywhere
| Button | Does |
|---|---|
| D-pad | Move |
| A | Select / launch |
| B | Back |
| M (menu) | Back to the Main menu from any screen |
| Volume + / - | Volume; the level shows in the title bar and is saved **[device-checked]** |
| Power | Stock behaviour (stock `pwr_new.sh`): sleep, or power off if the stock power-lock setting is on |

The bottom line of every screen lists that screen's buttons.

Some screens change the device itself (Restart, Power Off, Boot Screen, Card Scripts, Stock Menu,
Reset All). These ask you to **hold A for 3 seconds**. Releasing early, or pressing any other
button, cancels. **[device-checked]**

## Main menu
The title bar shows the build (`v0.1-dev <sha>`).

| Item | Opens |
|---|---|
| Game Room | Your game systems (below) |
| Music | Stock Music Player |
| Video | Stock Video Player. **Known problem:** playing a video crashes back to Main (queue 12a) |
| Documents | Stock Ebook Reader for TF1 |
| File Manager | Stock File Manager (on Main only, not in Apps) |
| Apps | The app list (below) |
| Internet | YouTube / Web browser **[placeholder]** |
| Network | Wi-Fi: scan, pick a network, type the password, connect |
| Settings | Settings (below) |
| Engineering Mode | Live system view (below) |

After an app closes you come back to Redux. After an app started from **Apps** you come back to the
Apps list on the same row **[not yet on device]**.

## Game Room
`Game Room > Systems` lists every system that has games, from **TF1 and TF2 together** (TF2 merging
**[not yet on device]**). Left/Right pages through long lists.

Pick a system, then a game, then A. A "Loading ROM" screen shows while it starts. Each system uses
the stock emulator stock uses for it: RetroArch with the stock core for most, and PPSSPP (PSP),
DraStic (NDS), PCSX, OpenBOR, the stock `.dge` emulators (`GBA_DGE`, `GBC_DGE`, `MD_DGE`) and
PortMaster where stock uses them.

- **Checked on the device:** RetroArch games, MAME through the stock core, PortMaster Wolfenstein
  (shoulder buttons strafe and change weapon).
- **OpenBOR:** fixed on main (stock's two steps, #170); **[not yet on device]**. OpenBOR keeps its own
  saves and logs in `/mnt/mmc/openbor` on the card, as on stock.
- **Ports:** the stock ports always work. Third-party `.sh` ports from your card need
  **Card Scripts** turned on (Settings > System).
- The game list's X (Favorites), Y (Info), Start (Search) and Select (Random) labels are
  **[placeholder]**.

## Apps
| App | What it is | Status |
|---|---|---|
| Music Player | stock `ap.dge` | not yet on device |
| Video Player | stock `vp.dge` | crashes on play (12a) |
| Ebook Reader (TF1) / (TF2) | stock reader (Pixel Reader) for each card | not yet on device; M may be its normal quit |
| PDF Reader | later (needs a file picker) | placeholder |
| Terminal | stock SimpleTerminal | device-checked |
| Task Monitor (htop) | htop in the terminal; needs htop (see Extras) | device-checked |
| Audio Mixer | `alsamixer` | device-checked |
| Network Setup | `nmtui`: hidden networks, static IP, forget a network | device-checked |
| Bluetooth | `bluetoothctl` | device-checked |
| Input Test | `evtest` on the buttons: every press prints | device-checked |
| Speaker Test | says "Front Left", "Front Right" | device-checked |

Terminal apps quit with **M**. The terminal apps start with a vsync helper meant to reduce the
stock screen tearing **[not yet on device]**. File Manager can't use it and still tears (a stock bug).

## Settings
| Item | Status |
|---|---|
| **Display** | Brightness (1-20; never fully dark), **Screen Timeout** (Off, 1, 2, 3, 5, 10, 15, 30 min; default 5), **Font** (five fonts, same layout in each). Up/Down picks the row, Left/Right changes it, applies at once, saved. Brightness **[device-checked]**; Screen Timeout and Font **[not yet on device]** |
| Audio, Input, Power & Lid, Emulation | **[placeholder]** (arrive with F3-F5) |
| **System** | see below |
| **UI** | Colours: BIOS cyan, Amber, Green, White. Saved. |

Screen Timeout: after that long with no button press **in the Redux menu**, the handheld sleeps the
way stock does; Power wakes it. Games and stock apps keep their own behaviour.

### Settings > System
| Row | Does |
|---|---|
| System Tools | Read-only: battery, CPU governor, speed and range, temperatures (CPU, GPU, VE, DDR), RAM **[device-checked]** |
| Restart / Power Off | `sync`, then reboot / power off (hold A 3 s) **[device-checked]** |
| Boot Screen (Stock/Redux) | Swaps the picture shown at power-on. The stock picture is saved first and can be put back here. Writes to the boot partition: keep the handheld on until it says Done. Install **[device-checked]**, restore **[not yet on device]** |
| Card Scripts (Off/On) | Lets Game Room > Ports run `.sh` scripts from your card (PortMaster, third-party ports). They run with full access: only for scripts you trust |
| Stock Menu | Puts the stock launcher back (from `dmenu_ln.bak`) and reboots. Refuses if the backup is missing |
| Reset All | Redux's own settings back to defaults: colours, brightness, screen timeout, font, Card Scripts (Off), saved volume. Never touches games, saves, BIOS, Wi-Fi networks, the boot screen or stock settings **[not yet on device]** |

## Network (Wi-Fi)
Main > Network scans; **Y** rescans. Pick a network, type the password on the on-screen keyboard
(`shift`, `sym`, `del`, `space`, `OK`), **Start** connects. A result screen says Connected or Not
connected. **[not yet on device]** since the #88 fix.

## Engineering Mode
Hold A for 2 seconds to enter. Tabs (L1/R1): CPU, Battery, Thermal, Display, System. Today it is a
**preview**: battery status and current are live, most other values are sample text, and nothing can
be changed yet (F5). It never goes past the stock CPU frequency table.

## Extras (installed from your PC)
StockOS is a full Ubuntu 22.04. `tools/extras.py` (in the repo; Python 3, works on Windows) installs
extra tools over SSH:

```
python tools\extras.py --ip <device address>          # tick packages, prints the commands
python tools\extras.py --select htop --run           # install over ssh (asks y/N, one password prompt)
python tools\extras.py --check --run                 # check every catalog and stock tool runs
```
Catalog (`tools/extras.ini`): **htop** (ticked; Task Monitor needs it), btop, s-tui, ncdu, tmux, mc.
Needs: SSH on (StockOS Mod setting), Wi-Fi with internet, the root password. The install simulates
first and **refuses if anything already installed would be upgraded**: never run `apt upgrade` on
the handheld, it can break RetroArch. **[device-checked]** (htop).

## Going back to stock
- Boot picture first: Settings > System > **Boot Screen** while it shows Redux.
- Menu: Settings > System > **Stock Menu** (reboots into the stock launcher), or from your PC
  `python install.py --uninstall <device address>` ([install.md](install.md)).

## Known problems (2026-11-14)
- Video Player crashes when you play a video (12a).
- Stock File Manager and Terminal tear when the screen changes (stock graphics library has no
  vsync). The vsync helper covers Terminal only and is not yet tested; File Manager stays as stock.
- Placeholders listed above.
