# Community mainline builds for H700 handhelds

Deep reads of other people's mainline builds, to reuse what they learned. Read-only clones,
2026-11-13. Nothing copied into this repo; nothing run on Dale's device.

## GregoryPletnev/rg35xxh-debian13 (High)
https://github.com/GregoryPletnev/rg35xxh-debian13 — `5861b51`, 2026-10-29, 2 commits, MIT.
**Debian 13 (trixie) on the RG35XX-H with no vendor code at all**: mainline U-Boot, Linux 7.1.2 with
the ROCKNIX H700 patch set, an `mmdebstrap` arm64 rootfs. It's the closest public template for
both Phase 2 and the Ubuntu track (U2): a plain distro userland on our own kernel.

| Area | What they do | Lesson for Redux |
|---|---|---|
| U-Boot | v2026.01 + ROCKNIX's two patches (LPDDR3 TPR6 fix, AXP717 charge-only boot) and ROCKNIX's `anbernic_rg35xx_h700_lpddr4_defconfig`; plain TF-A `sun50i_h616` BL31. Written at 8 KiB | Same base as ROCKNIX; no suspend stub, so no real suspend |
| **Hybrid U-Boot** (`build-hybrid-uboot.sh`) | Adds `SUNXI_DRAM_H616_AUTO`: SPL tries LPDDR4, then LPDDR3, then DDR3 and keeps the first that trains | One image for original and revised RG34XXSP boards is possible. **Caveat:** the LPDDR3/DDR3 entries zero TPR0/TPR2 and keep LPDDR4 clock and ODT settings, unlike ROCKNIX's tuned LPDDR3 config (672 MHz). The author says use it only for mixed hardware. Treat as a prototype |
| Kernel | Vanilla 7.1.2 + all ROCKNIX H700 patches **except 0140 (joypad driver) and 0150 (force feedback)**, then one local patch | Mainline `adc-joystick` + `gpio-keys` keep working without ROCKNIX's out-of-tree joypad driver (on the RG35XX-H; our SP needs the 4-channel mux, open question 6) |
| 7.1 → 7.2 gap | Backports the 7.2 SRAM-C layout in `sun50i-h616.dtsi` (`sram_c: sram-section@0`, `sun50i-h616-sram-c`). Without it `sunxi_sram_claim()` returns -EINVAL and the DE33 bus, so the whole display, never probes | If we pin a kernel older than 7.2, we need the same backport |
| Dangling refs | Removes `&joypad { amux-count = <4>; }` that patch 0144 adds, since 0140 is skipped | ROCKNIX patches aren't independent; dropping one needs DT fixups |
| Panel firmware | Built into the kernel with `EXTRA_FIRMWARE` (panel `.panel` files, rtw88 and BT firmware, `regulatory.db`) because there's **no initramfs** and the panel probes before the rootfs mounts | Matters for the RAM-resident design: either an initramfs or built-in firmware |
| Config | `DRM_PANEL_MIPI` on, `NEWVISION_NV3052C` and `PANEL_MIPI_DBI` off, `PWM_SUN8I`, fbcon without deferred takeover, `INPUT_MOUSEDEV` | A minimal display config to compare with ROCKNIX's |
| Panel revisions | Two DTBs (default and `rev6-panel`); `boot.scr` picks rev6 **only if a file `panel-rev6` exists on the FAT partition**. The README says "selects at runtime"; the script uses the marker file | No automatic panel detection here either. Our "identify the panel first" rule still needs a real probe |
| Wi-Fi fix | Local patch `0001-wifi-rtw88-sdio-drop-rx-aggregate-on-bad-descriptor`: a bad RX descriptor length on RTL8821CS caused `skb_over_panic()` → BUG in `ksdioirqd`, killing Wi-Fi until reboot (seen on 7.1.2). Drops the rest of the aggregate instead | **Same chip as ours.** Upstream `rtw_sdio_rxfifo_recv()` at 7.4-rc2 still has no such length check, and no rtw88 patch turned up in ROCKNIX's kernel patch dirs (searched 2026-11-13). Plan to carry this fix or an equivalent |
| Boot | `boot.scr`: `fatload` DTB to 0x4a000000, `Image` to 0x42000000, `booti`; `earlycon=uart8250,mmio32,0x05000000`, consoles tty0 + ttyS0 115200, `root=/dev/mmcblk0p2`, no initramfs | A working minimal boot recipe for the H700 |
| Image | MBR: p1 FAT32 256 MiB (`Image`, DTBs, `boot.scr`), p2 ext4 rootfs; U-Boot at 8 KiB | Different from stock (p1 = Roms on ours). Never reuse its `dd` on a stock card |
| Rootfs | `mmdebstrap --variant=minbase` trixie: systemd, networkd/resolved/timesyncd, **zram via `systemd-zram-generator` (ram/2)**, openssh, wpa_supplicant, alsa, triggerhappy (volume keys), first-boot units (grow root, SSH keys, mixer, Wi-Fi from `/boot/wifi.txt`) | A ready pattern for track U2. ZRAM works on mainline here (Redux decided ZRAM for 1.x) |
| GPU | Mesa from **trixie-backports (26.x)**: panvk about 3× faster than trixie's 25.0.7 on the Mali-G31 (author's Shadeburn test); `cage` Wayland kiosk | For the Ubuntu track: Noble's stock Mesa is older; plan on a newer Mesa (PPA or own build) |
| Security | Root password default `rg35xxh`, root SSH login, baked-in keys | Not acceptable for Redux (security items need Dale) |

**Not reproducible from the repo alone:** the scripts expect a TF-A build, a base kernel `.config`
(`build/k712/.config`), firmware from a Buildroot tree and a ROCKNIX checkout in `$W`, none of which
are in the repo. `build-uboot-rk.sh` downloads from ROCKNIX's moving `next` branch. Treat it as a
recipe, not a build system.

## mitswan/armbian-anbernic (Low)
https://github.com/mitswan/armbian-anbernic — `0d11f67`, 2025-07-31, one branch.
Despite the name, it's a **plain fork of armbian/build** with no Anbernic-specific commits, board
configs or patches: its newest 50 commits are all upstream Armbian (the last by Armbian
maintainers). Nothing to reuse beyond what Armbian itself has.

What Armbian itself shows (useful facts found in the fork):
- Armbian's `sunxi64` family builds 6.6.75 (legacy), **6.12.35** (current) and **6.15.4** (edge).
- Its kernel patch archives carry the DE33 display-engine series for 6.12–6.15 and a Mali-G31
  node for H616, but no RG34XX-SP tree and no Anbernic board config.
- Its DT Makefile context lists `sun50i-h700-anbernic-rg35xx-sp.dtb` in the **6.12** archive and
  not in 6.6/6.7. So the upstream RG35XX-SP tree landed between 6.10 and 6.12 (partly answers open
  question 1).

## exdial/anbernic-apps (Medium: stock-firmware tuning ideas, not for our device as is)
https://github.com/exdial/anbernic-apps — `b31c3e2`, 2026-07-17, 45 commits, MIT. Apps for the
**RG35XX Plus** stock firmware, installed into `Roms/APPS`. Not kernel work; recorded here because it
touches the same stock system Beta runs on.

| App | What it does | For Redux |
|---|---|---|
| System Booster | Timezone UTC + `en_US.UTF-8`; replaces `sources.list` with `ports.ubuntu.com` jammy (drops the stock mirrors); removes `unattended-upgrades`; sysctl (`swappiness 10`, `vfs_cache_pressure 50`, dirty ratios 15/5, NMI watchdog off); journald in RAM (4 MiB cap); `/tmp` and `/var/tmp` as tmpfs; disables ModemManager, rsyslog, cron, timesyncd, NM-wait-online, apt-daily, motd-news; on every boot sets the governor (schedutil first), `mq-deadline` and `read_ahead_kb 128` on mmcblk0/1, renices `dmenu.bin`, drops caches; backs up configs once | Good list to measure in Phase 1 (boot time, idle drain). Overlaps Redux: Redux's initramfs already sets schedutil. **Conflicts:** it turns Wi-Fi power save **on**, while ROCKNIX turns it off for rtw88/8821CS; check on our chip before copying |
| — partition step | `parted resizepart 7 100%` + `resize2fs` on `mmcblk0p7`, assuming p1 = FAT32 boot … p7 = user data | **Wrong layout for the RG34XXSP**: here p1 is the exFAT Roms partition and p7 is `UDISK` (`docs/stock-boot-chain.md`). Its guards (devices exist, ≥ 1 MiB free) probably make it skip on our cards, but Redux must never run it. Hard rule: never touch p1, never auto-format |
| SSH Enabler | Installs `openssh-server`, `PermitRootLogin yes`, `StrictModes no`, password auth, **sets root's password to `root`** | Not acceptable for Redux (security items need Dale; the PC-link decision requires a password and local network only) |
| The Real Retro theme | Copies its `res1`/`res2` icons, wallpapers, loading screens over `/mnt/vendor`, and mounts `mmcblk0p2` to replace `bootlogo.bmp`, **without a backup**. Its logo is 640×480 **32-bit** BMP (1,228,938 bytes) | Confirms the community uses p2 for the boot logo, as Redux's Boot Screen manager does (with a backup). Stock's logo is 24-bit (921,654 bytes); whether u-boot shows a 32-bit BMP on our unit is unverified, so keep Redux's 24-bit output |

## mporrato/alpine-h700 and its issue #2 (Medium)
https://github.com/mporrato/alpine-h700 — `1e24c81`, 2024-07-13, 3 commits, scripts under UNLICENSE.
Alpine Linux on the **stock** H700 boot chain, tested on an RG35XX Plus: SPL, U-Boot, the 4.9 kernel,
modules and firmware are **extracted from a stock card image** (`factory.img`); only the rootfs and
initramfs are new. The same idea as Ubuntu-track step U3 (own rootfs, stock kernel), done with Alpine.

Facts it records about the stock H700 boot chain (RG35XX Plus image; compare before trusting on ours):
- SPL found by the `eGON.BT0` signature at **8 KiB** (it also checks 128 and 256 KiB); its length is
  the 32-bit value at SPL+16.
- U-Boot found by the `sunxi-package` signature at **16400 KiB**; length at +36. The first partition
  starts at 36 MiB.
- Its rebuilt card uses GPT with the main table moved to sector **73696**, partitions `env`,
  `boot-resource`, `boot`, `rootfs` with attributes `c000000000000001` (sgdisk). **Our RG34XXSP keeps
  p1 = Roms first** (`docs/stock-boot-chain.md`), so its layout and `dd` offsets are not ours.
- Anbernic's U-Boot environment format is **not** what `mkenvimage` writes; it ships its own
  stdlib-only `mkenv.py` (128 KiB image, CRC32). Its `env.txt` matches the stock variables
  (`boot_normal=sunxi_flash read 45000000 boot;bootm 45000000`, `cma=64M`, `gpt=1`).
- Kernel and `bootimg.cfg` come out of the stock `boot` partition with `abootimg`.
- The repo **commits Anbernic files** (`resources/`: boot logos, battery frames, u-boot fonts,
  `magic.bin`). Redux never does (hard rule); we only describe them.

Issue #2, "Switching out the kernel zImage for custom compiled Image" (comments undated in the page
capture; the linked comment couldn't be fetched separately):
- tokyovigilante (handle used by Ryan Walklin, author of the upstream RG35XX device trees) runs Alpine
  3.20 on an RG35XX-H with **Linux 6.13-rc1**: the audio codec driver is upstream, headphone-jack
  detection was still being written. Invites work in the ROCKNIX Discord and `#sunxi-linux`.
- mporrato (then): mainline isn't "just a device tree" because whole drivers were missing; the stock
  4.9 kernel builds most things in; the **DTB sits inside `u-boot.img`** (a small one for U-Boot, a
  larger one for the kernel; `binwalk … | grep "device tree"`); a replacement must be an
  "ARM64 boot executable Image"; stock config has `# CONFIG_SYSVIPC is not set` (JACK audio needs it,
  can't be a module); swapping the kernel needs matching `modules.tar` and `firmware.tar`.
- manyids2: `snd-usb-audio` missing on stock, loaded with `insmod -f` after building it.

For Redux:
- The 2024 "drivers missing" view is out of date: ROCKNIX now runs the RG34XXSP on 7.2 (rocknix-h700.md).
- **Where does the stock kernel get its DTB?** `docs/stock-boot-chain.md` says from `boot.img`
  (Android boot image v2 has a DTB section); mporrato says from the U-Boot package. Both may carry one.
  Worth settling before Phase 2, because it decides what a kernel swap must replace (open question 12).
- No SysV IPC in the stock kernel: anything in Beta that needs it (JACK, some apps) won't run on 4.9.

