---
type: decision
from: claude
to: dale
date: 2026-11-14
re: Beta "Stock-card install" (ROADMAP), relayed as MGR for DALE PLAN stock-card installer (2026-11-14 13:36); Dale A (13:37)
status: open
---

# Decision: how Redux gets onto a stock card (Beta installer)

Plan only, no code. Beta needs "one explicit, opt-in installer, a documented way back to stock, the
640×480 boot logo", installable by someone other than Dale (ROADMAP). Read Only's user-doc draft for
the installer was not in `Claude Files` yet; it goes into `docs/` when it arrives and doesn't change
the choice below.

## What an install has to do (the same in every option)
Taken from today's manual deploy (`tools/deploy_frontend.py`, [dmenu_ln finding](2026-11-10-claude-finding-dmenu-ln.md)):
1. **Preflight:** confirm it's a stock RG34XXSP card. `/mnt/vendor/ctrl/dmenu_ln` must hold the known
   stock line `CMD="/mnt/vendor/bin/dmenu.bin"` (or the Redux one, for an upgrade); there must be enough
   free space on appfs; and the binary must be an aarch64 ELF. Any mismatch stops the install with nothing written.
2. **Copy** `dmenu.bin` (Redux, with the vsync library embedded) to `/mnt/vendor/redux/` under a temporary name, then
   rename it, so a cut-off copy never replaces a working launcher.
3. **Switch** the launcher: back up `dmenu_ln` once (`dmenu_ln.bak`, never overwritten), change the
   `CMD=` line only, read it back.
4. **Optional:** stage the Redux boot logo (`/mnt/vendor/redux/bootlogo.bmp`). The user installs it
   on the handheld in Settings > System > Boot Screen (existing manager, with its own backup).
5. **Record** what was done in `/mnt/data/redux/install.log` (version, SHA, the backup's hash).

Writes: appfs (`/mnt/vendor`) and UDISK (`/mnt/data`) only. **Never p1** (hard rule), never the boot
partition, nothing formatted. Settings already live in `/mnt/data/redux/` and survive an upgrade.

**Way back to stock (all options):** uninstall puts `dmenu_ln.bak` back, checks the `CMD=` line is the
stock one and leaves `/mnt/vendor/redux/` (harmless, removable by hand). Boot Screen > Restore puts
the stock logo back. Last resort: reflash the stock image (the user's own download).

## Options

| | A. PC installer over SSH | B. SD-card package with an on-device confirm | C. Patched card image |
|---|---|---|---|
| How | `tools/install.py <ip>` (+ `--uninstall`, `--check`), Python stdlib, Windows `ssh`/`scp`. Grows out of `deploy_frontend.py` | `redux-install/` folder copied to **TF2**, run by stock's `update/update.sh` hook at boot. It shows a Redux screen: **hold A 3 s** to install, B skips. It never writes to the card | `tools/` patches the user's own stock `.img` (appfs ext4 edited offline), flashed with Rufus/Etcher |
| User needs | PC with Python, Wi-Fi, SSH switched on (StockOS Mod `global.ssh=1`), the root password | A second card (TF2), a PC to copy one folder | A PC, Rufus, a spare card; reflash wipes the card |
| Opt-in | Running the command | The hold-A screen (stock runs the hook; Redux does nothing without the hold) | Flashing |
| Risk | Lowest: the same steps Dale has run since 11-10, proven restore | Medium: runs before the menu as root; must be idempotent (re-runs every boot while the card is in, see below) | Highest: an ext4 writer in Python stdlib is new, untested code; image size issues on big cards |
| Effort | Small (1 PR + docs) | Medium (installer screen, script, tests) | Large |
| Also gives | Uninstall and `--check` for support | The easiest path for people without SSH | A one-file "flash and go" |

**Re-runs (B):** stock's `autostart` runs `update/update.sh` from either card on every boot. The
package can't delete itself (Redux never deletes from cards). It compares the installed SHA with its
own and exits silently when they match. So it asks once per new version, and the user takes the
folder off TF2 afterwards. TF2 only, because a package on the Roms card (p1) can't be removed or
marked done by Redux. The user's own file copy there is fine, but the docs keep it simple: TF2.

**Not in any option:** the Phase 1 kernel tweaks (boot partition). They write the boot chain, so
they're a separate Dale-gated step with its own plan and a Fable checkpoint. The Beta exit test
can run without them, and they stay a Beta box of their own.

## Recommendation: A now, B next
A first: it's small, appfs-only and already proven by every dev deploy. It's enough for the Beta
installer box with a written user guide. B after that, as the no-PC-tools route, since "someone other
than Dale" mostly won't have SSH set up. B reuses A's install steps (one shell script, `install.sh`,
called both by `install.py` over SSH and by the TF2 hook), so B is the A code plus a confirm screen.
C: no.

## Needs the device (goes on the next test queue when built)
1. A: fresh stock card → `install.py --check` → install → reboot to Redux → `--uninstall` → stock
   menu back, `dmenu_ln` byte-identical to the backup.
2. A: upgrade over an older Redux (settings kept), and a run with SSH off (clear error, nothing written).
3. B (when built): stock runs `/mnt/sdcard/update/update.sh` from TF2 (only read from the stock
   scripts so far, **not seen on the device**); B skips cleanly; a second boot with the same version
   says nothing.
4. Both: a V1.0.6 (2026-05-26) card, not just the 2025 image. `dmenu_ln` was not compared in the
   V1.0.6 diff.

## Not verified
Nothing here has run. The `update.sh` hook and the V1.0.6 `dmenu_ln` are known from stock scripts only.
Whether SSH is on by default on a fresh stock card is unknown (Dale's unit had it switched on).

## Needs from Dale
Reply **A**, **B** or **C** (or "A then B").
