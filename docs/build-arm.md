# Building the ARM binary

The device needs an aarch64 ELF built with `-DREDUX_PLATFORM=stock`.
Install it as **`/mnt/vendor/redux/dmenu.bin`** so stock `pwr_new.sh` finds it with `pidof dmenu.bin`.

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
