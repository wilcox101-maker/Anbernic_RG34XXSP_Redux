# Phase 2 design: RAM-resident system

**Idea:** the system lives on the card as one read-only image, is copied into RAM at power-up, and
runs from RAM. It's the OS-level version of how an SRAM FPGA loads its configuration from flash. The
H700 itself has no programmable fabric and the board has no SPI flash (the stock device tree declares
none, and both SPI controllers are disabled), so the SD card is the only non-volatile store.

Status: **design, not built.** Numbers marked *est.* are estimates to replace with measurements.

## Why

| Problem on stock | What a RAM-resident image fixes |
|---|---|
| rootfs is read-write ext4 mounted `barrier=0`, appfs is mounted `rw`, `resize2fs` runs every boot | The system can't be corrupted. Nothing writes to it, ever |
| Lid close or power loss mid-write can damage the OS | Only the data partition is written, with barriers on |
| Slow or cheap cards cause hitches when the OS reads from them | After boot, the OS never touches the card |
| No clean recovery or update path (stock runs `update.sh` from any card) | Updates replace one image file. A/B slots give automatic fallback |

## Layers

| Layer | Contents | Where it runs | Writable |
|---|---|---|---|
| **boot** | kernel, DTB, initramfs | loaded by u-boot | no |
| **system** (squashfs, LZ4) | userland (BusyBox-class), frontend, power daemon, SDL2, RetroArch binary, GPU userspace | **copied whole into RAM** | no |
| **payload** (squashfs) | libretro cores, standalone emulators, RetroArch assets, shaders, databases, cheats | **mounted from the card**, read on demand; the page cache keeps hot parts in RAM | no |
| **data** (ext4, barriers on) | saves, states, settings, logs, user-supplied BIOS | card | yes |
| **Roms** | user's games | card | yes, by the user only. **Never formatted by the OS** |

Splitting system from payload is what keeps the RAM cost small. In stock, the payload-type content
alone is far too big to copy: cores 714 MiB, RetroArch `system` folder 536 MiB, databases 109 MiB,
assets 102 MiB, cheats 70 MiB.

## Budgets (2 GB unit)

| Item | Budget | Basis |
|---|---|---|
| system image, compressed | **≤ 128 MiB**, target 64–96 MiB | *est.* A BusyBox-class userland + frontend is tens of MiB. Mesa/Panfrost and SDL2 are the big items |
| system image in RAM | = compressed size (squashfs stays compressed, decompressed per page on read) | squashfs behaviour |
| CMA | 64 MiB | stock `cma=64M` |
| Left for emulators | **> 1.6 GiB** | *est.* PSP/NDS are the heaviest users. Measure PPSSPP and DraStic peak RSS on stock first |
| Boot-time copy | compressed size ÷ card read speed: about **1–5 s** for 96 MiB at 20–90 MB/s | *est.* Measure the real card speed. It's offset by booting far fewer services than stock Ubuntu |

LZ4 because it decompresses fastest on the A53. The stock kernel already has `SQUASHFS` + `LZ4` and
`OVERLAY_FS`, so this can be prototyped before mainline.

## Boot flow

1. u-boot loads kernel + DTB + initramfs (as today).
2. initramfs reads `MemTotal`:
   - **≥ 1.5 GiB:** copy `system-<slot>.sqsh` into a tmpfs file, check its SHA-256 against the
     manifest, then loop-mount it from RAM.
   - **< 1.5 GiB** (revised 512 MB / 1 GB units): loop-mount the image **straight from the card**, with
     no copy. The image is the same, only where it runs from differs. Enable zram on these units.
3. tmpfs for `/tmp`, `/run`, `/var` (logs that must survive go to `data`).
4. Mount `payload` read-only, `data` read-write (`fsck` first if dirty, never `mkfs`), `Roms`
   read-write (read-only + warning if the mount fails).
5. `switch_root`, start the power daemon, then the frontend.

## Updates and recovery (A/B)

- Two system slots, A and B, each sized for 2× the budget (*est.* 256 MiB each).
- An update writes the new image to the **inactive** slot, verifies the checksum, then flips a
  one-line `slot` file on the boot partition. This is the single, explicit update path the brief
  asks for.
- If the new slot fails to reach the menu twice in a row, the initramfs falls back to the previous
  slot. This is the same counter that resets Engineering Mode settings.
- A checksum mismatch at boot also falls back, and the frontend shows a warning.

## Proposed card layout (Phase 2)

| Region | Size (*est.*) | Notes |
|---|---|---|
| bootloader | 36 MiB | stock layout (boot0, u-boot, GPT), unchanged |
| boot | 64 MiB | kernel, DTB, initramfs, `slot` file |
| system-a / system-b | 256 MiB each | raw squashfs images |
| payload | 2–3 GiB | squashfs, replaced as a whole on update |
| data | 1–2 GiB | ext4, barriers on |
| Roms | rest of the card | exFAT or FAT32 (stock is probably FAT32, unverified) |

## To measure before building

- [ ] Card sequential read speed on the device (sets the boot-copy cost)
- [ ] Peak RAM of PPSSPP and DraStic on stock, using a heavy game for each
- [ ] Size of a minimal userland + frontend + SDL2 + RetroArch, with and without Mesa
- [ ] Whether the revised units' u-boot reports RAM size in a way the initramfs can read early (see
      [field-reports.md](field-reports.md))
