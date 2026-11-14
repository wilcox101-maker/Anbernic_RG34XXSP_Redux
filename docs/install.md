# Installing Redux on a stock RG34XXSP card

> **Not checked on the device yet.** The installer has only run against a test copy of the card's
> folders. Until a device test is logged in `notes/`, treat this as a beta tool and keep a backup of your card.

Redux replaces the stock menu. Games, saves, RetroArch and the stock apps stay as they are. The
installer runs on your PC and talks to the handheld over Wi-Fi with SSH. It changes **one line** of
the stock launcher script and adds the Redux program next to it. `--uninstall` puts the stock line
back.

## What you need
- The handheld on the stock Anbernic firmware, on your Wi-Fi.
- **SSH switched on** on the handheld (StockOS Mod setting; `global.ssh=1` in
  `/mnt/mod/ctrl/configs/system.cfg`), and its root password.
- A PC with Python 3 and the `ssh` command (built into Windows 10 and 11).
- From the [dev release](https://github.com/wilcox101-maker/Anbernic_RG34XXSP_Redux/releases/tag/dev):
  `dmenu.bin`, `install.py`, `redux_install.sh`, `run.sh` (and `bootlogo.bmp` if you want the Redux boot screen).
  Put them in one folder.

Find the handheld's IP address in the stock Wi-Fi settings.

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
Restore** in Redux, before you uninstall and restart.
The Redux program stays in `/mnt/vendor/redux/` but is no longer used. You can delete that folder by hand.

If the handheld doesn't start at all, flash the stock image you downloaded from Anbernic.

## What exactly changes on the handheld
| Where | What |
|---|---|
| `/mnt/vendor/redux/dmenu.bin` | the Redux program (checked by size and SHA256 after copying) |
| `/mnt/vendor/redux/run.sh` | the start wrapper the stock launcher runs: starts Redux and logs to `redux.log` (kept under 256 KiB) |
| `/mnt/vendor/redux/bootlogo.bmp` | only with `--bootlogo` |
| `/mnt/vendor/ctrl/dmenu_ln` | one line: `CMD="/mnt/vendor/bin/dmenu.bin"` becomes `CMD="/mnt/vendor/redux/run.sh"` (an older hand install that ran `dmenu.bin` directly is moved to `run.sh`) |
| `/mnt/vendor/ctrl/dmenu_ln.bak` | the stock `dmenu_ln`, saved once before the first install and never overwritten |
| `/mnt/data/redux/install.log` | one line per install or uninstall |

Nothing is written to the ROM card. The boot partition is untouched and nothing is formatted. Every
file goes to a temporary name first and is renamed into place, so an interrupted run leaves the old
file working.

## If Redux won't start
The stock launcher has no fallback: if Redux exits with an error, it waits 30 s and starts Redux
again. Until a guard is added, the way out is SSH:
`python install.py --uninstall <ip>`, or reflash the stock image.

## When it stops
The installer stops before changing anything it can't confirm. It prints `STOPPED:` and the reason:
- **unknown CMD= line**: `dmenu_ln` isn't the stock one (edited by hand, or another firmware). Nothing was touched.
- **no dmenu_ln.bak**: Redux is switched on but the stock backup is missing, so it can't promise a way back.
- **sha256 / size**: the copy arrived damaged. Run it again.
- **appfs has … KiB free**: free some space on the system partition.
- SSH errors (`Connection refused`, `timed out`): check the IP address, Wi-Fi and the SSH setting.
