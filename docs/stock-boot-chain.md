# Stock firmware: SD card layout and boot chain

Firmware: `rg34xxsp-fw-sd_202505` (`RG34XXSP.img`), GPT layout.

## Card layout

| Linux dev | Name | Size | Contents |
|---|---|---|---|
| *(raw, first 36 MiB)* | — | 36 MiB | boot0 at 8 KiB, u-boot/TOC1, GPT. **Copy raw and never modify.** |
| mmcblk0p1 | `Roms` | ~44 GiB | exFAT: ROMs, BIOS, media. Physically first so Windows sees it |
| mmcblk0p2 | `boot-resource` | 32 MiB | FAT16 (label `Volumn`): `bootlogo.bmp` (**640×480, 24-bit BMP, uncompressed, bottom-up, 921,654 bytes**), `fastbootlogo.bmp` (246×257, 24-bit), `bat/` charging frames, `font24.sft`/`font32.sft` (u-boot fonts), `magic.bin` (read from Dale's `RG34XXSP.img`, 2026-11-13) |
| mmcblk0p3 | `env` | 16 MiB | u-boot environment (see [`env.txt`](../reference/stock-202505/env.txt)) |
| mmcblk0p4 | `boot` | 64 MiB | Android boot image v2: kernel + initramfs + DTB, tail padded with 0xFF |
| mmcblk0p5 | `rootfs` | 7 GiB | ext4: Ubuntu 22.04 base + an `/emuelec` tree |
| mmcblk0p6 | `appfs` | 4 GiB | ext4: Anbernic launcher (`bin`, `ctrl`, `deep`, `res1`, `res2`, `oem`) |
| mmcblk0p7 | `UDISK` | ~2.5 GiB | ext4: `dmenu`, `save`, `misc` |

## Boot sequence

1. **BROM** loads **boot0** from 8 KiB. boot0 trains the 2 GB DRAM.
2. **u-boot** (Allwinner BSP) reads `env`, shows `bootlogo.bmp`, and runs
   `boot_normal = sunxi_flash read 45000000 boot; bootm 45000000`.
   On SD boot it uses `setargs_mmc`:
   ```
   console=ttyS0,115200 loglevel=4 root=/dev/mmcblk0p5 rootwait quiet splash
   init=/init cma=64M selinux=0 gpt=1 ...
   ```
3. **Kernel 4.9.170** starts with the DTB from the boot image (u-boot patches the memory node).
4. **initramfs** (Buildroot 2015.02, BusyBox) runs [`/init`](../reference/stock-202505/initramfs-init.sh):
   - `e2fsck -y` on the rootfs. Without `-f` this only does a full check if the fs is marked
     unclean (or a mount-count/interval limit trips). With `barrier=0`, an unclean power-off makes
     that more likely
   - mount with `barrier=0` (fast, but risks corruption if power is lost mid-write)
   - `switch_root /mnt /init`, which hands off to Ubuntu
5. Ubuntu userspace (systemd) starts Anbernic's launcher from `appfs`. The details are in
   [stock-userspace.md](stock-userspace.md).

## Serial console

`ttyS0` at 115200 (UART0 at `0x05000000`). If the board exposes UART pads, that's the debugging
lifeline for Phase 2. Pad locations still to be found on this model.

## What Phase 1 changes in this chain

Only step 4 and the DTB inside `boot.img`. The bootloader, env and kernel binary stay untouched.
See [`phase1-stock-tweaks/`](../phase1-stock-tweaks/).
