# Installing Redux on a stock RG34XXSP card

> **Pre-Beta.** Check, install, upgrade and uninstall were device-checked on Dale's unit on 2026-11-16
> ([result](../notes/2026-11-16-dale-hw-result-185.md), checks 1-4). Only that one unit has run it:
> keep a backup of your card.

Redux replaces the stock menu. Games, saves, RetroArch and the stock apps stay as they are. The
installer runs on your PC and talks to the handheld over Wi-Fi with SSH. It changes **one line** of
the stock launcher script and adds the Redux program next to it. `--uninstall` puts the stock line
back.

## What you need
- The handheld on the stock Anbernic firmware, on your Wi-Fi.
- **SSH switched on** on the handheld (StockOS Mod setting; `global.ssh=1` in
  `/mnt/mod/ctrl/configs/system.cfg`), and its root password.
- A PC with Python 3 and the `ssh` command (built into Windows 10 and 11).
- From the [dev release](https://github.com/wilcox101-maker/Anbernic_RG34XXSP_Redux/releases/tag/dev)
  (the rolling test build; once Beta is near, tagged releases `v0.5-beta.N` follow and this guide will
  point at the newest one):
  `dmenu.bin`, `install.py`, `redux_install.sh`, `run.sh` (and `bootlogo.bmp` if you want the Redux boot screen).
  Put them in one folder.

Find the handheld's IP address in the stock Wi-Fi settings.

## Compatibility
| | Tested | Notes |
|---|---|---|
| Hardware | **Original revision, 2 GB LPDDR4** (Dale's unit; 1979 MB seen by Linux, [survey](../notes/2026-11-12-dale-hw-result-device-survey.md)) | Revised "Metallic Blue" units (512 MB / 1 GB LPDDR3, different panel) are **untested**; see [field-reports.md](field-reports.md) |
| Stock firmware | `rg34xxsp-fw-sd_202505` for the Phase 1 boot test (2026-11-10). The firmware version on the card the Redux frontend was tested on is not recorded yet | V1.0.6 (2026-05-26) has been read on the PC only ([diff](../notes/2026-11-12-claude-message-stock-v106-diff.md)); not run with Redux |
| SD cards | The stock card in TF1 (system plus the Roms partition); a FAT (vfat) ROM card in TF2 (117 GB on Dale's unit) | Redux's own code never writes to either ROM card. Other card sizes and formats are untested |

## Install
Open PowerShell in that folder:

```powershell
python install.py --check 192.168.1.13      # looks only; says "ready to install"
python install.py 192.168.1.13              # installs
python install.py 192.168.1.13 --bootlogo bootlogo.bmp   # installs and copies the boot screen too
```

Each run asks for the root password once. When it says **done**, restart the handheld (Power Off,
then on). Redux starts instead of the stock menu.

The boot screen is not switched automatically. To use it, in Redux go to **Settings > System > Boot Screen**
and hold A for 3 s.

To **upgrade**, run the same install command with a newer `dmenu.bin`. Your Redux settings
(`/mnt/data/redux/`) are kept.

## Back to stock
```powershell
python install.py --uninstall 192.168.1.13
```
Then restart. If you installed the Redux boot screen, first go to **Settings > System > Boot Screen >
Restore** in Redux, before you uninstall and restart. If Redux doesn't start, do it over SSH instead
([Boot logo by hand](#boot-logo-by-hand)).
The Redux program stays in `/mnt/vendor/redux/` but is no longer used. You can delete that folder by hand.
Your Redux settings in `/mnt/data/redux/` are kept, so installing again brings them back as you left them.

If Redux won't start, the handheld itself still boots and **SSH still works**, so go back over SSH
first (see [If Redux won't start](#if-redux-wont-start)). Flashing the stock image is the **last
resort**: see [Reflashing](#reflashing-last-resort).

### Boot logo by hand
Only if you installed the Redux boot screen and can't reach **Settings > System > Boot Screen >
Restore**. Log in over SSH as root and paste this one line. It overwrites the logo **in place**
(same size, no new file on that partition, as Redux does), checks the copy and unmounts:
```
mkdir -p /tmp/redux-bootres && mount -t vfat -o rw,noatime /dev/mmcblk0p2 /tmp/redux-bootres && [ "$(wc -c < /mnt/vendor/redux/bootlogo.stock.bmp)" -eq 921654 ] && [ "$(wc -c < /tmp/redux-bootres/bootlogo.bmp)" -eq 921654 ] && dd if=/mnt/vendor/redux/bootlogo.stock.bmp of=/tmp/redux-bootres/bootlogo.bmp bs=4096 conv=notrunc,fsync && cmp /mnt/vendor/redux/bootlogo.stock.bmp /tmp/redux-bootres/bootlogo.bmp && echo RESTORED; sync; umount /tmp/redux-bootres
```
It prints `RESTORED` when it worked. No `RESTORED` means a size check or the copy failed; stop there
and ask. `bootlogo.stock.bmp` is the stock logo Redux saved the first time it installed its own. If
that file isn't there, Redux never changed the logo.

### Reflashing (last resort)
Flashing the stock image you downloaded from Anbernic rewrites the **whole card**: the system, and
also the **Roms partition (games, BIOS files)** and **saves** on the same card. **Back up the Roms
partition and your saves first** (put the card in your PC and copy them off), then flash.

## What exactly changes on the handheld
| Where | What |
|---|---|
| `/mnt/vendor/redux/dmenu.bin` | the Redux program (checked by size and SHA256 after copying) |
| `/mnt/vendor/redux/run.sh` | the start wrapper the stock launcher runs: starts Redux and logs to `redux.log` (kept under 256 KiB) |
| `/mnt/vendor/redux/bootlogo.bmp` | only with `--bootlogo` |
| `/mnt/vendor/ctrl/dmenu_ln` | one line: `CMD="/mnt/vendor/bin/dmenu.bin"` becomes `CMD="/mnt/vendor/redux/run.sh"` (an older hand install that ran `dmenu.bin` directly is moved to `run.sh`) |
| `/mnt/vendor/ctrl/dmenu_ln.bak` | the stock `dmenu_ln`, saved whenever an install finds the stock launcher (never from a Redux one); uninstall copies it back and keeps it |
| `/mnt/data/redux/install.log` | one line per install or uninstall |
| `/tmp/redux-start.fails`, `/tmp/redux-start.stock` | the start-failure guard's counter and switch; in RAM, gone at the next boot |

Nothing is written to the ROM card. The boot partition is untouched and nothing is formatted. Every
file goes to a temporary name first and is renamed into place, so an interrupted run leaves the old
file working.

## If Redux won't start
`run.sh` watches for Redux failing at start: an error exit within 10 seconds. After **3 in a row** it
starts the **stock menu** instead, for the rest of that boot, and writes why in `redux.log`. The next
boot tries Redux again; nothing on the card is changed. (Between failures the stock launcher waits about
30 s, so the stock menu appears after roughly a minute.)
If it keeps happening, look at `/mnt/vendor/redux/redux.log`, and go back to stock **over SSH** (the
handheld still boots and SSH still works):
1. From the PC: `python install.py --uninstall <ip>`, then restart.
2. Or, logged in over SSH, the one-liner **S7** from [dale-scripts.md](dale-scripts.md) (copies
   `dmenu_ln.bak` back and reboots).
3. If the Redux boot screen is installed: [Boot logo by hand](#boot-logo-by-hand).

Reflash the stock image only as a last resort, and **back up the Roms partition and saves first**
([Reflashing](#reflashing-last-resort)): they are on the same card.

## When it stops
The installer stops before changing anything it can't confirm. It prints `STOPPED:` and the reason:
- **unknown CMD= line**: `dmenu_ln` isn't the stock one (edited by hand, or another firmware). Nothing was touched.
- **no dmenu_ln.bak**: Redux is switched on but the stock backup is missing, so it can't promise a way back.
- **sha256 / size**: the copy arrived damaged. Run it again.
- **appfs has … KiB free**: free some space on the system partition.
- SSH errors (`Connection refused`, `timed out`): check the IP address, Wi-Fi and the SSH setting.
