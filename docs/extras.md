# Redux extras: installing Ubuntu tools on the device

StockOS is a full Ubuntu 22.04 (arm64). Redux uses a few tools that aren't on every stock unit,
htop first (Apps > Task Monitor). `tools/extras.py` installs them over SSH from your PC.

## What you need
- SSH on the device (StockOS Mod setting; `global.ssh=1` in `/mnt/mod/ctrl/configs/system.cfg`).
- The device on Wi-Fi with internet, and its address (Settings > Internet shows it).
- The root password you set on the device. Redux never stores it.
- Python 3 on the PC (Windows is fine) and the `ssh` command (built into Windows 10 and 11).

## Use
```
python tools\extras.py --ip 192.168.1.13
```
Tick what you want (htop is ticked), press Enter, and the tool prints two lines to copy into
PowerShell: **1. Install**, **2. Check they run**. Add `--run` to have it run them for you, after a
y/N. `--select htop,btop` skips the checklist; `--check` prints a check for every catalog and stock
tool.

## Safety
- **Never run `apt upgrade` or `full-upgrade` on the device.** An upgrade can break the library
  paths RetroArch needs (StockOS Mod author, cbepx-me issue #300).
- The install line simulates first. If installing would change the version of anything already
  installed, it prints **REFUSED** and the packages, and installs nothing.
- `--no-upgrade --no-install-recommends`: only the ticked packages and what they need.
- It writes only to the device's system partition (packages); never to the Roms card.

## Adding a package
Add a section to `tools/extras.ini` (package name, binary, a version argument, a description).

## Seen on Dale's unit (2026-11-13)
Stock and working (library check + version line): alsamixer, speaker-test, nmtui, nmcli,
bluetoothctl, evtest, mpv, python3 3.10, vim 8.2, git 2.34, wget. htop 3.0.5 after `apt install`.
**ffmpeg fails** (`libpangoft2` symbol error, `FcWeightFromOpenTypeDouble`): the StockOS Mod's
`RA_launch.sh` points `libfontconfig.so.1` at 1.10.1 for RetroArch. Not a Redux problem; don't
"fix" it, RetroArch relies on it.
