# Building the ARM binary

The device needs an aarch64 ELF built with `-DREDUX_PLATFORM=stock`.
Install it as **`/mnt/vendor/redux/dmenu.bin`** so stock `pwr_new.sh` finds it with `pidof dmenu.bin`.

## Option 0: download it from CI (recommended)

Every push and PR builds the device binary in GitHub Actions (job `arm64`), inside an arm64
Ubuntu 22.04 container: the same release as the stock rootfs, so glibc and SDL2 match.

1. Open the PR or commit on GitHub → **Checks** → the `ci` run → **Summary** → **Artifacts**.
2. Download `redux-dmenu-bin-aarch64` (a zip holding `dmenu.bin`) and unzip it.
3. `python tools/deploy_frontend.py <device-ip> dmenu.bin`

SDL2 on the device (Dale, 2026-11-10, Ubuntu GLIBC 2.35, aarch64): **two** copies.
- `/usr/lib/libSDL2-2.0.so.0` → **2.0.12**. `dmenu_ln` puts `/usr/lib` first in `LD_LIBRARY_PATH`, so
  this is the one the menu (stock and Redux) loads. It's probably Anbernic's build with the display backend.
- `/usr/lib/aarch64-linux-gnu/libSDL2-2.0.so.0` → 2.28.5 (and an unused 2.0.18 file).

The binary is built against 2.0.20 headers but uses only 23 SDL functions, all present since ≤ 2.0.5
(`nm -D --undefined-only`). SDL2 has no symbol versioning, so it runs on 2.0.12. Keep it that way:
**don't use SDL calls newer than 2.0.12** (e.g. `SDL_GetTicks64`) in the frontend.

## Option 1: build on the device

```bash
# on the RG34XXSP, after apt install cmake gcc libsdl2-dev
cd /mnt/vendor/redux-src   # or wherever the source is copied
cmake -B build -DREDUX_PLATFORM=stock
cmake --build build
file build/rg34xxsp-redux   # should say aarch64
cp build/rg34xxsp-redux /mnt/vendor/redux/dmenu.bin
```

## Option 2: cross-build from Windows / Linux

1. Install an aarch64 cross toolchain (e.g. `aarch64-linux-gnu-gcc` or the Linaro toolchain).
2. Point CMake at it:
   ```bash
   cmake -B build -DREDUX_PLATFORM=stock \
         -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
         -DCMAKE_SYSTEM_NAME=Linux \
         -DCMAKE_SYSTEM_PROCESSOR=aarch64
   cmake --build build
   ```
3. Confirm the header:
   ```bash
   python -c "import sys; d=open(sys.argv[1],'rb').read(20); print(hex(d[18]) if d[:4]==b'\\x7fELF' else 'not elf')" build/rg34xxsp-redux
   ```
   Should print `0xb7`.
4. Copy to the device as `/mnt/vendor/redux/dmenu.bin`.

## Notes

- Link against the stock SDL2 if you cross-build. The device has it in `/usr/lib`.
- The deploy script (`tools/deploy_frontend.py`) refuses anything that is not an aarch64 ELF.
- The install name **must** be `dmenu.bin` so `pwr_new.sh` sees the process on power-key shutdown.

## Not verified yet

- Neither option has been run. The device's Ubuntu 22.04 rootfs may not have working `apt` or
  network, so Option 1 is untested.
- Cross-building needs the device's libraries, not just a compiler. Use a sysroot from the stock
  rootfs (`EXTRACTED_202505`, `--sysroot` plus `-DCMAKE_FIND_ROOT_PATH`), or Ubuntu 22.04 arm64
  packages (`libsdl2-dev:arm64`), which match the stock release.
