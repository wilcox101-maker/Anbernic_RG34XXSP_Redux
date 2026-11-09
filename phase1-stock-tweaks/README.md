# Phase 1: Stock-kernel baseline

A patched `boot.img` that keeps Anbernic's kernel, bootloader and userspace, and changes only what
can be reached without kernel source. It gives a measurable before/after on the same card.

**Original 2 GB board only** (firmware `rg34xxsp-fw-sd_202505`). Both scripts refuse any other
input. See [`../docs/field-reports.md`](../docs/field-reports.md) for the revised units.

## Changes

| # | Where | Change | Why |
|---|---|---|---|
| 1 | DTB `/soc@03000000/gpio_keys` | `poll-interval` 20 → 5 ms | 20 ms polling adds up to about a frame of input lag |
| 2 | initramfs `/init` | set each CPU's governor to `schedutil` before handing off to Ubuntu | stock defaults to `performance` (pinned at max clock) |
| 3 | initramfs `/init` | rootfs mount `barrier=0` → `barrier=1` | journal safety on power loss |

The kernel binary, the bootloader and every other partition are byte-for-byte stock. All changes stay
inside the stock OPP table and thermal limits.

*Dropped from the original plan:* skipping the boot-time `e2fsck`. Stock runs it without `-f`, so a
clean filesystem is skipped already, and keeping the check helps stability.

## Files

| File | Purpose |
|---|---|
| `build.py` | stock `3.boot.img` → `boot-redux-p1.img` (verifies the stock SHA-256 first) |
| `patch_image.py` | writes a boot image into the `boot` partition of a **copy** of `RG34XXSP.img`, with GPT, size, stock-hash and read-back checks. `--restore` puts stock back |
| [`DEPLOY.md`](DEPLOY.md) | step-by-step test plan |

## Expected hashes (SHA-256)

| Image | SHA-256 |
|---|---|
| stock `3.boot.img` | `87913826e15dab962b060703a0552270793b9e14cc303e5f8ec0f38e179ef50b` |
| `boot-redux-p1.img` | `3d176f7f796722aeedfa1d9f18ff89ddf0789ad95a75c3086b00f6670b238982` |

## Build

```bash
python3 phase1-stock-tweaks/build.py 3.boot.img boot-redux-p1.img
```
