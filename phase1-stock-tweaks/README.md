# Phase 1: Stock-kernel baseline

The goal is a patched `boot.img` that keeps Anbernic's kernel, bootloader and userspace, and changes
only what is reachable without kernel source. It gives a measurable before/after on the same card.

## Changes

| # | Where | Change | Why |
|---|---|---|---|
| 1 | DTB `ANBERNIC-keys` | `poll-interval` 20 → 5 ms | 20 ms polling adds up to about a frame of input lag |
| 2 | initramfs `/init` | write `schedutil` to each `cpufreq/policy*/scaling_governor` | stock defaults to `performance` (pinned at max clock) |
| 3 | initramfs `/init` | replace unconditional `e2fsck -y` with a check only when the fs state is not clean | removes a full 7 GiB fsck from every boot |
| 4 | initramfs `/init` | `barrier=0` → `barrier=1` | journal safety on power loss |

All changes stay inside the stock OPP table and thermal limits.

## Workflow

```bash
python3 tools/bootimg.py   unpack 3.boot.img work/boot
python3 tools/initramfs.py unpack work/boot/ramdisk work/initramfs
# apply changes (scripted patch to come in this folder)
python3 tools/initramfs.py pack   work/initramfs work/boot/ramdisk
python3 tools/bootimg.py   pack   work/boot work/boot-redux.img --pad-to 67108864
```

DTB edits need `dtc` (WSL: `sudo apt install device-tree-compiler`):

```bash
dtc -I dtb -O dts -o work/board.dts work/boot/dtb
# edit poll-interval
dtc -I dts -O dtb -o work/boot/dtb work/board.dts
```

## Flashing (spare card only)

1. Write the stock `RG34XXSP.img` to a **spare** SD card.
2. Overwrite only the boot partition (p4) with the patched image:
   - Linux/WSL: `sudo dd if=work/boot-redux.img of=/dev/sdX4 bs=1M conv=fsync`
3. Boot. To revert, write the original `3.boot.img` back to p4.

## Measurements

| Metric | Method | Stock | Redux |
|---|---|---|---|
| Boot time | power-on → menu, stopwatch ×5 | | |
| Idle drain | % per hour in menu, screen on | | |
| Sustained clock / temp | `cpuinfo_cur_freq`, `thermal_zone*/temp` during a game | | |
| Button latency | 240 fps phone video, press → screen | | |
