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

Some screens change the device itself (Restart, Power Off, Boot Screen, Stock Menu,
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
| File Manager | Midnight Commander (`mc`) in the terminal; needs `mc` installed (see Extras). M quits. On Main only |
| Apps | The app list (below) |
| Internet | YouTube / Web browser **[placeholder]** |
| Network | Wi-Fi: scan, pick a network, type the password, connect |
| Settings | Settings (below) |
| Engineering Mode | Live system view (below) |

After an app closes you come back to Redux. After an app started from **Apps** you come back to the
Apps list on the same row **[not yet on device]**.

## Game Room
With games on **both cards**, Game Room first asks which card, **TF1** or **TF2**, like stock; each
card then has its own `Systems` list, so a game on both cards never shows twice **[not yet on
device]**. With one card, Game Room opens its `Systems` straight away. Left/Right pages through long
lists.

Pick a system, then a game, then A. A "Loading ROM" screen shows while it starts. Each system uses
the stock emulator stock uses for it: RetroArch with the stock core for most, and PPSSPP (PSP),
DraStic (NDS), PCSX, OpenBOR, the stock `.dge` emulators (`GBA_DGE`, `GBC_DGE`, `MD_DGE`) and
PortMaster where stock uses them.

- **Checked on the device:** RetroArch games, MAME through the stock core, PortMaster Wolfenstein
  (shoulder buttons strafe and change weapon).
- **OpenBOR:** fixed on main (stock's two steps, #170); **[not yet on device]**. OpenBOR keeps its own
  saves and logs in `/mnt/mmc/openbor` on the card, as on stock.
- **Ports:** the stock ports, plus every third-party `.sh` port in `Roms/PORTS` on either card
  (PortMaster and others). Card scripts run with full access to the handheld: only put scripts
  you trust on the card.
- The game list's X (Favorites), Y (Info), Start (Search) and Select (Random) labels are
  **[placeholder]**.

## Apps
Sorted by name. Music Player, Video Player and File Manager are on Main, not in this list.
Audio Mixer (`alsamixer`) and Speaker Test are in **Settings > Audio**; Bluetooth (`bluetoothctl`),
Input Test (`evtest`), Network Setup (`nmtui`) and Task Monitor (htop) are rows of **Settings**
(moved 2026-11-17). All were device-checked from Apps. Network Setup: choosing Edit freezes it, and
it stays frozen when opened again (Dale 2026-11-17; a Redux network page is planned).

| App | What it is | Status |
|---|---|---|
| Ebook Reader (TF1) / (TF2) | stock reader (Pixel Reader) for each card; M quits | TF1 device-checked; TF2 not yet |
| PDF Reader | later (needs a file picker) | placeholder |
| NeoFetch | system summary (`neofetch`, no logo so it fits); needs `neofetch` installed; M quits | device-checked (before the no-logo change) |
| Terminal | Redux's own terminal (Redux Terminal) with a `bash` shell: sharp 8x16 text in the font chosen in Settings > UI, 90 x 30. Buttons: D-pad / left stick = arrows, A = Enter, B = Esc, X = Tab, Y = Backspace, L1 / R1 = Page Up / Down, L2 / R2 = Home / End, Start = F10, **M quits**. **Right stick** moves a mouse pointer (yellow frame; hides after 4 s), **L3** left click, **R3** right click, for programs that take the mouse (mc, htop, alsamixer). **Select** opens the on-screen keys over the bottom rows: D-pad moves, A types, Shift stays on, Ctrl applies to the next key, B or Select closes. File Manager (mc), Task Monitor (htop), Audio Mixer, Network Setup, Bluetooth, Input Test, Speaker Test and NeoFetch all run in it; the stock SimpleTerminal is no longer used. Chinese, Japanese and fullwidth characters are drawn 16 pixels wide (two cells) from GNU Unifont; other scripts the fonts lack show as `?` | **[not yet on device]** |

On Main: **Music Player** (stock `ap.dge`, device-checked), **Video Player** (stock `vp.dge`: plays,
but takes no buttons during playback; a Redux front-end is planned in F6).

Terminal apps quit with **M**. NeoFetch and Speaker Test stay on screen when they finish until a
button is pressed; a program that fails keeps its screen and shows the error line at the bottom.

## Settings
| Item | Status |
|---|---|
| **Audio** | Audio Mixer, Speaker Test (stock terminal tools) |
| Power & Lid, Emulation | **[placeholder]** |
| **SSH**, **SMB (Samba)** | A switches On/Off, the stock way: sets `global.ssh` / `global.samba` in `/mnt/mod/ctrl/configs/system.cfg` and starts or stops `ssh` / `smbd` + `nmbd` at once, so it also holds after a reboot. On means anyone on your Wi-Fi can try to log in as root: use a strong root password. **[not yet on device]** |
| **System** | see below |
| **UI** | **Brightness** (1-20; never fully dark) **[device-checked]**, **Screen Timeout** (Off, 1, 2, 3, 5, 10, 15, 30 min; default 5), **Font** (five fonts, same layout in each) (these three were the Display menu). **Accent Color** (selection bar: BIOS cyan, Amber, Green, White, then the 16 standard VGA text colours except black; A opens the list). **Menu Text**, **Header/Footer Text**, **Window Color** (any of the 16 VGA colours; a choice that would hide text is skipped). **Header Size**, **Footer Size** (Default, Large = twice as tall), **Window Size** (Default, Large = twice as tall, Largest = also twice as wide). A screen whose content doesn't fit drops a size there. **Reset Fonts** puts the three sizes back to Default. Left/Right changes a row, applies at once, saved. Sizes and the three colours **[not yet on device]** |

Screen Timeout: after that long with no button press **in the Redux menu**, the handheld sleeps the
way stock does; Power wakes it. Games and stock apps keep their own behaviour.

After 1 minute with no button press in the Redux menu, the screen dims to about a third of your
brightness; any button brings it back (the press still counts). Closing the lid in the menu sleeps the
way stock does; open it and press Power to wake. In a RetroArch game, closing the lid first makes
RetroArch write the game's in-game save to the card, then the handheld sleeps as stock does. **[not yet on device]**

### Settings > System
| Row | Does |
|---|---|
| System Tools | Read-only: battery, CPU governor, speed and range, temperatures (CPU, GPU, VE, DDR), RAM **[device-checked]** |
| Restart / Power Off | `sync`, then reboot / power off (hold A 3 s) **[device-checked]** |
| Boot Screen (Stock/Redux) | Swaps the picture shown at power-on. The stock picture is saved first and can be put back here. Writes to the boot partition: keep the handheld on until it says Done. Install **[device-checked]**, restore **[not yet on device]** |
| Stock Menu | Puts the stock launcher back (from `dmenu_ln.bak`) and reboots. Refuses if the backup is missing |
| Packages | Install (A) or remove (X) the Ubuntu packages in Redux's list (`tools/extras.ini`: htop, btop, s-tui, ncdu, tmux, mc, neofetch, cmus) over Wi-Fi, after a 3-second hold. Same safety as the PC tool: never upgrades, refuses anything that would change other packages; a remove that would take other packages with it is refused. Log: `/mnt/data/redux/packages.log` **[not yet on device]** |
| Reset All | Redux's own settings back to defaults: colours, text sizes, brightness, screen timeout, font, saved volume. Never touches games, saves, BIOS, Wi-Fi networks, the boot screen or stock settings **[not yet on device]** |

## Network (Wi-Fi)
Main > Network scans; **Y** rescans. Pick a network, type the password on the on-screen keyboard
(`shift`, `sym`, `del`, `space`, `OK`), **Start** connects. A result screen says Connected or Not
connected. **[not yet on device]** since the #88 fix.

## Rumble
Redux unlocks rumble, a hidden firmware feature: the stock launcher never offers it. The RG34XXSP
has a vibration motor, and Redux's Engineering Mode LED/rumble test runs it
**[device-checked]** ([result](../notes/2026-11-17-dale-hw-result-070427e.md) #2; by hand:
[result](../notes/2026-11-17-dale-hw-result-2917a5c.md) #7). **[not yet on device]**: rumble inside
games (RetroArch force feedback), and strength levels other than the ones tested.

## Engineering Mode
Hold A for 3 seconds to enter. Tabs (L1/R1): CPU, GPU, Battery, Thermal, Display, Input, Storage, Logs,
System, Update (build history), all live read-outs. On CPU, Up/Down picks a row and A edits the governor, min/max speed or
cores 1-3; X saves the settings as a profile, Y removes it (F5, **[partly not yet on device]**). It
never goes past the stock CPU frequency table.

**Update tab:** A on the top row downloads the newest dev build of this repo (checksum-checked) and,
after a 3-second hold, installs it and reboots; pick a history line to roll back. Each build comes
with its own boot logo (`v0.1-dev <build>`); unless you put the stock boot screen back, the logo is
updated with the build. If a new build fails to start 3 times, Redux goes back to the last build that
reached the menu, then to the stock menu. **[not yet on device]**

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
  `python install.py --uninstall <device address>` ([install.md](install.md)). Your Redux settings
  are kept for a later reinstall.
- If Redux fails to start three times in a row, the stock menu starts instead for that boot
  ([install.md](install.md#if-redux-wont-start)).

## Troubleshooting and FAQ

**`install.py --check` says…**
| Message | Means | Do |
|---|---|---|
| `ready to install` | stock launcher found, nothing installed | install |
| `not ready: Stock OS Mod not found (currently required; …)` | the Stock OS Mod's scripts aren't on the handheld | install the Stock OS Mod first ([install.md](install.md#what-you-need)) |
| `Redux is installed; install again to upgrade, or uninstall` | Redux is already the launcher | install again to upgrade |
| `STOPPED: no …/dmenu_ln` | not a stock RG34XXSP card, or the system partition isn't mounted | check you are on the stock firmware |
| `STOPPED: dmenu_ln has an unknown CMD= line` | the launcher was changed by hand or by another firmware | nothing was touched; restore stock first |
| `missing tool: …` | a command the installer needs isn't on the device | report it ([bug report](https://github.com/wilcox101-maker/Anbernic_RG34XXSP_Redux/issues)) |
| SSH `Connection refused` / `timed out` | wrong address, Wi-Fi off, or SSH switched off | see the next question |

The install and uninstall messages are explained in [install.md](install.md#when-it-stops).

**The handheld's IP address changed.** Your router hands out addresses, so it can change between
sessions. Look it up again in the stock Wi-Fi settings (or your router's device list) and use the new
one. To keep it fixed, reserve the address for the handheld in your router (DHCP reservation).

**Redux doesn't start, or I want stock back.**
- On the device: *Settings → System → Stock Menu* (hold A for 3 s).
- From the PC: `python install.py --uninstall <ip>`, then restart.
- If Redux fails to start three times in a row, the stock menu starts by itself for that boot; SSH
  still works, so the PC uninstall above still works ([install.md](install.md#if-redux-wont-start)).

**What does Redux never touch?** Redux's own code never writes to, formats or resizes your ROM cards
(TF1's Roms partition and TF2). The stock engines it launches (RetroArch, OpenBOR) save there exactly
as on stock. Redux never writes the bootloader region, and the installer changes one line of the stock
launcher plus its own folder ([what changes](install.md#what-exactly-changes-on-the-handheld)).

## Known problems (2026-11-14)
- Video Player crashes when you play a video (12a).
- The stock SimpleTerminal tore and looked soft (Dale 2026-11-17); Redux Terminal replaces it for
  every terminal app **[not yet on device]**.
- Placeholders listed above.
