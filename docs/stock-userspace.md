# Stock userspace: what Anbernic's OS actually runs

Firmware `rg34xxsp-fw-sd_202505` (`RG34XXSP.img`, `oem/version.ini` = `20250408`, `board.ini` = `RG34xxSP`).
Inventoried read-only with [`tools/stock_inventory.py`](../tools/stock_inventory.py). The `Roms`
partition was never read. Nothing from the image is committed here; paths and short quotes only.

Status labels used below: **confirmed** means read from a script or config in the image.
**Strings only** means found in a compiled binary's strings, so the call is known to exist, but not
when it runs. **Unconfirmed** means inferred and still to be checked on the device.

## Partitions as found

| Part | Name | Size | Used | Contents |
|---|---|---|---|---|
| raw | bootloader | 36 MiB | — | boot0 (`eGON.BT0` at 8 KiB), u-boot/TOC1, GPT. Cut to `bootloader-36MiB.bin`, SHA-256 `2a9053b207369a486b2752fa5f00c8d1da2b70f5b2d926d9b28044b17b9e7042` |
| p1 | `Roms` | 44 GiB | not read | Mounted at `/mnt/mmc` |
| p5 | `rootfs` | 7 GiB | 4.1 GiB, 72 142 files | Ubuntu 22.04 LTS (systemd). Of that: `/usr` 3.5 GiB, `/var` 455 MiB, `/mnt/mod` 432 MiB, `/roms/ports` (PortMaster) 247 MiB, `/emuelec` 70 MiB |
| p6 | `appfs` | 4 GiB | 2.3 GiB, 32 413 files | Mounted at `/mnt/vendor`. `deep/retro` (RetroArch 1.19.1 + cores) 1.6 GiB, `bin/` (launcher, standalone emulators, apps), `ctrl/` (control scripts), `lib/`, `oem/`, `res1/`, `res2/` |
| p7 | `UDISK` | 2.5 GiB | 112 MiB (journal) | Mounted at `/mnt/data`. Nearly empty: `dmenu/dmenu_attr.ini`, empty `save/md/` and `misc/` |

The listings are in the tool's `inventory.zip` (`list-rootfs.tsv`, `list-appfs.tsv`, `list-UDISK.tsv`).

## Boot chain after `switch_root` (confirmed)

```
/init -> /sbin/init (systemd), default.target -> graphical.target
 ├─ enabled units: NetworkManager, wpa_supplicant, systemd-resolved, dnsmasq, smbd, nmbd, ufw,
 │  rsyslog, cron, dmesg, networkd-dispatcher, e2scrub_reap, getty@tty1, uuidd.socket;
 │  timers: fstrim, e2scrub_all, logrotate, man-db, motd-news, dpkg-db-backup
 ├─ /etc/modules (via modules-load.d/modules.conf symlink): rtl_btlpm
 ├─ rc-local.service -> /etc/rc.local: insmod 8821cs.ko (Wi-Fi), insmod /lib/modules/mali_kbase.ko (GPU)
 └─ SysV rc5 S50launcher -> /etc/init.d/launcher.sh start
     ├─ resize2fs /dev/mmcblk0p6            (EVERY boot: the flag-file `touch` is commented out)
     ├─ hwclock --hctosys
     ├─ mount p6 ext4 rw,noatime,nodiratime -> /mnt/vendor
     ├─ /mnt/vendor/bin/brightCtrl.bin &    (referenced, but not present in appfs)
     ├─ /mnt/vendor/bin/cexpert &           (8 KiB binary; mmc_new.sh says cexpert calls it)
     └─ /mnt/vendor/ctrl/loadapp.sh &
         ├─ resize2fs p5 once (flag /etc/allen_resize.cfg)
         ├─ p7 -> /mnt/data   (mkfs.ext4 if blkid finds no fs or mount fails)
         ├─ p1 -> /mnt/mmc    (mount -t vfat; mkfs.vfat if blkid finds no fs or mount fails)
         ├─ if /mnt/mmc/appfs.img exists: mount it over /mnt/vendor
         ├─ copy default retroarch.cfg to /.config/retroarch/ if missing
         ├─ boot_mode == 1 (charging): block BT/Wi-Fi, run bin/charg.dge, poweroff
         ├─ else restart NetworkManager + bluetooth, unblock rfkill
         ├─ set RTC once if empty
         ├─ if /mnt/mmc/gpio_keys_polled.ko exists: insmod gpio_keys_gpadc.ko + gpio_keys_polled.ko from the SD
         ├─ ctrl/mmc_new.sh add          (second SD card -> /mnt/sdcard, vfat/exfat/ntfs)
         ├─ /mnt/mod/ctrl/autostart      (see below)
         └─ loop: ctrl/dmenu_ln -> bin/dmenu.bin (launcher UI), then `sh /tmp/.next` (the chosen app)
```

`/mnt/mod/ctrl/` (in **rootfs**, not appfs) is a second layer of scripts signed "G.R.H". It covers
RetroArch launching (`RA_launch.sh`), bezels, shaders, power handling (`pwr_new.sh`), the `joy` power-key
watcher, and settings in `configs/system.cfg`. Its `autostart` creates `fb.cfg`, **runs
`/mnt/mmc/update/update.sh` or `/mnt/sdcard/update/update.sh` as root if present**, resumes the last
game, sets the LED, toggles Samba/SSH from `system.cfg`, repairs `retroarch.cfg`, and installs the
PICO-8 stub.

## CPU governor

| Who | What | Status |
|---|---|---|
| Kernel | `CONFIG_CPU_FREQ_DEFAULT_GOV_PERFORMANCE=y`. Built in: performance, powersave, userspace, ondemand, conservative, interactive, schedutil | confirmed |
| Boot scripts (rootfs, appfs, `/mnt/mod`) | **None of them set a governor** | confirmed |
| `appfs/ctrl/cpu_setting.sh` | writes cpu0 `scaling_governor`: `full` → performance, `powersave` → powersave, `user` → userspace @ 792 MHz, anything else → interactive. Also has a commented-out DDR DVFS block. Frequencies listed in a comment: 480/600/792/1008/1200/1512 MHz | script confirmed |
| …called by `bin/charg.dge` (charging screen), `bin/video/vp.dge` (video player), `bin/music/ap.dge` (music player) | only the arguments `full` and `user` appear next to the call, so it ends up `performance` or `userspace` @ 792 MHz, never the `interactive` default | strings only (which argument, when, is not visible) |
| `bin/dmenu.bin` (launcher), RetroArch, PPSSPP, the `.dge` emulators | **do not reference `cpu_setting.sh` or any cpufreq file** | strings: confirmed absent |
| DraStic (`deep/drastic-modify/res/libs/libSDL2-2.0.so.0`, preloaded by its `launch.sh`) | contains `echo performance > …cpu0/cpufreq/scaling_governor` and `echo ondemand > …` | strings only. Most likely performance on start and ondemand on exit |
| RetroArch 1.19.1 | has the CPU-scaling menu built in, but the shipped `/.config/retroarch/retroarch.cfg` contains no `cpu_scaling` keys | confirmed not configured |
| `/mnt/vendor/deep/retro` cores, players | only unrelated `ondemand` / `cpufreq` strings | — |

So with stock userspace the CPU **stays on the kernel default `performance`**. It changes only when the
music player, video player or charging screen runs, or when DraStic runs. Menus, RetroArch, PPSSPP and the
built-in emulators leave it alone.

**Effect on Phase 1:** setting `schedutil` in the initramfs survives the boot scripts. It can still be
overwritten later by the music/video players (`performance` or `userspace` @ 792 MHz) and by DraStic,
which probably leaves the CPU on `ondemand` after exit. Phase 1 can neutralise both by having
`cpu_setting.sh` write `schedutil` for `full`, and by dropping the governor lines from DraStic's launch. Only cpu0's file is written. The H700 has one
cluster, so policy0 should cover all four cores (check `related_cpus` on the device). Checks to add to
[DEPLOY.md](../phase1-stock-tweaks/DEPLOY.md): read `scaling_governor` after boot, after a RetroArch
game, after a standalone emulator, and after DraStic.

## Kernel modules

Almost everything is built in (`modules.builtin`). The loadable `.ko` files in rootfs:

| Module | Loaded by | Status |
|---|---|---|
| `8821cs.ko` (RTL8821CS Wi-Fi) | `/etc/rc.local` insmod | confirmed |
| `/usr/lib/modules/mali_kbase.ko` (Mali-G31, outside the versioned tree) | `/etc/rc.local` insmod | confirmed |
| `rtl_btlpm.ko` (BT low-power) | `/etc/modules` → systemd-modules-load | confirmed |
| `gpio_keys_gpadc.ko`, `gpio_keys_polled.ko` | `loadapp.sh`, **only if they exist on the Roms SD** (`/mnt/mmc/`). An input-driver override hook, not shipped | confirmed |
| `backlight`, `generic_bl`, `lcd`, `uvcvideo`, `gspca_main`, `videobuf2-vmalloc`, `bcm_btlpm`, camera sensors (`gc*`, `imx317`, `ov5640`) | not loaded by any script. udev may autoload some if matching hardware appears (e.g. a USB camera) | unconfirmed |
| unknown | `bin/game/gba/gba_emu.dge` contains `/sbin/insmod %s verbose=1` / `/sbin/rmmod %s` | strings only |

`/etc/modprobe.d/` holds only the Ubuntu default blacklists.

## Power key, lid and suspend

**Power key (confirmed):**
- The kernel exposes it as `axp2202-pek` (`input0`, from the stock boot log).
- systemd-logind is told to ignore it (`/etc/systemd/logind.conf`: `HandlePowerKey=ignore`), so
  userspace handles it entirely.
- `appfs/ctrl/pwr_new.sh [auto]` hands off to `/mnt/mod/ctrl/pwr_new.sh` when present (it is). If not,
  it does `echo -n mem > /sys/power/state` with a 0.3 s lock file as debounce.
- `/mnt/mod/ctrl/pwr_new.sh` behaviour, from `configs/system.cfg`:
  - Key press with stock `power.key=2`: for 1 s it polls `BTN_TL2` (the M/function key) with `evtest --query`. If M is held, it shuts down; otherwise it suspends. `power.key=1` always shuts down; `0` always suspends.
  - HDMI connected: always suspends.
  - Shut-down path: quit RetroArch over its command interface, kill `dmenu_ln`, save state to `/mnt/mod/.last` (for resume-last-game), a rumble pulse, `sync`, `poweroff`.
- Apps also read the key themselves:
  - Anbernic's RetroArch has a `read_pwrkey_thread` that polls `/sys/class/power_supply/axp2202-battery/nds_pwrkey`. `RA_launch.sh` resets that file and starts `/mnt/mod/ctrl/joy`, an `evtest` watcher that toggles `nds_pwrkey` with rumble on `KEY_POWER` release.
  - Standalone players and readers (`ffplay`, `ap.dge`, `pdf/green`, file manager, ebook) open `/dev/input/event0`/`event1` directly (strings only).
- `pwr_new.sh` is called from compiled code, by whichever program is in the foreground: `dmenu.bin`, RetroArch
  (both binaries), PPSSPP, every built-in `.dge` emulator, `pcsx`, the ebook readers, the music/video
  players and `charg.dge` (strings only). There is no central power daemon. If the foreground app hangs
  or doesn't handle the key, nothing suspends.
- `cexpert` only handles SD-card hot-plug (`mmc_new.sh add` / `remove`); it doesn't touch power.

**Lid (Hall sensor):**
- It is **not** an input switch. The stock kernel exposes it as
  `/sys/class/power_supply/axp2202-battery/hallkey`, an attribute of Anbernic's AXP2202 driver. `SW_LID`
  appears only in `evtest`'s name table. logind's `HandleLidSwitch` never fires.
- Read by `bin/charg.dge` (charging screen) and both ebook readers (strings only).
- `pwr_new.sh auto` is the lid-close / screen-timeout path, per its own comment ("屏幕超时&合盖" =
  screen timeout & lid closed):
  - HDMI connected with `power.hdmi=1` (stock): nothing happens.
  - `power.lock=1`: shuts down.
  - Stock `power.lock=0`: suspends.
- The kernel side is interrupt-driven: the stock boot log shows `hall_switch_isr`,
  `hall_press_work_func` / `hall_release_work_func` (Anbernic debug prints tagged `[allen]`).
  `dmenu.bin` reads `hallkey`, `os_sleep` and `workled_sleep`, has an `enter_sleep_state` routine, and
  calls `pwr_new.sh auto`. The music/video players also call `pwr_new.sh auto` (strings only).

**Suspend itself** is a bare `echo -n mem > /sys/power/state`. Nothing flushes emulator saves or
pauses audio first, beyond each app's own handling.

**Other AXP2202 sysfs attributes** the launcher and apps use: `moto` (rumble), `work_led`,
`brightness`, `openbor_volume`, `spk_state`, `nds_esckey`, `boot_mode`, `display_id`, `otg_id`,
`led_test`. Phase 2 needs mainline equivalents for these, or a small shim.

## Other findings that matter for this project

- **The stock launcher can format the ROM card.** If `mount -t vfat /dev/mmcblk0p1` fails (a dirty or
  unsupported FS), `loadapp.sh` runs `mkfs.vfat` on it. The same applies to UDISK with `mkfs.ext4`.
  Since it only ever mounts p1 as `vfat`, the stock `Roms` partition is probably FAT32, not the exFAT
  listed in [stock-boot-chain.md](stock-boot-chain.md). Not verified, because p1 was not read. Phase 2 must never do this. Mount failures should fall back to read-only plus a warning.
- **`resize2fs` on appfs every boot** (flag-file creation is commented out in `launcher.sh`). It slows
  boot and puts needless writes on a filesystem mounted `rw`.
- **Root-run hooks from removable media:** `update.sh` on either SD card, `/mnt/mmc/dmenu.bin` (replaces
  the launcher), `/mnt/sdcard/debug.ini` + `dmenu.bin`, `/mnt/mmc/appfs.img` (replaces all of appfs),
  and `.ko` files from `/mnt/mmc`. These are convenient for modders and also mean any card runs code as
  root. Phase 2 should keep one explicit, opt-in update path.
- appfs is mounted read-write. Phase 2's read-only system image removes a whole class of corruption.
- Swap on p7 exists in `loadapp.sh` but is commented out.
- The image ships leftovers from Anbernic's own boot on 15 Apr: `/var/log/syslog` and `kern.log`,
  plus `/mnt/sdcard/.config/ppsspp`. The boot log is the best available record of a stock boot (see
  open items).
- appfs contains a copyrighted Neo Geo BIOS archive under `bin/game/*/bios/`. As with everything else
  from the image, it must never be committed.

## What the stock boot log shows

`/var/log/kern.log` and `syslog` (about 7 000 lines each) cover many boots from 7–16 April by **several
factory units** (five different Wi-Fi chips). Early boot before about 1 s isn't captured.

- **Input devices:** `input0` = `axp2202-pek` (power key, AXP on `twi5` i2c-5 @ 0x34), `input1` =
  `ANBERNIC-keys` (gpio_keys), `input2` = `dierct-keys-polled` [sic]. USB pads show up as extra
  inputs.
- **rootfs mount options:** `barrier=0,noauto_da_alloc` (write barriers off). That is fast, but
  `RG34XXSP.img` is more exposed to corruption on power loss than it needs to be. Phase 2 should keep
  barriers on.
- **GPU:** `mali_kbase` warns it is out of tree and taints the kernel. Its DVFS table tops out at 648 MHz @ 960 mV, with
  600 MHz @ 950 mV as the backup step.
- **Wi-Fi is the flakiest part:** `rtl8821cs: probe of mmc2:0001:1 failed with error -123` on 7 boots,
  and `PM: Device mmc2:0001:1 failed to resume async` after suspend. A Phase 2 stability target.
- Other noise: `Error reading temperature for gpu thermal zone: -11`, `refresh hdcp key failed`,
  `avcc is not calibrated` (thermal sensor), a `FAT-fs (mmcblk1p1) not properly unmounted` on the
  second SD card, and `cpu3: _add_opp_dev: Failed to register opp debugfs (-12)`.
- The first-boot `resize2fs` of rootfs is visible (`resized filesystem to 1835008`, so 7 GiB with 4 KiB
  blocks).

## Open items

1. ~~Callers of `cpu_setting.sh` and `pwr_new.sh`~~: resolved by the second run of the tool (see above).
   Still unknown: which of `full` / `user` the players pass, and when.
2. **On-device checks** (stock card): `scaling_governor` after boot, game, DraStic. `cat
   …/axp2202-battery/hallkey` lid open vs. closed. `lsmod`. Which `/dev/input/event*` is `axp2202-pek`
   vs. `gpio_keys`.
