# RG34XXSP Redux — Frontend (F1 desktop prototype)

C + SDL2 prototype that reproduces the locked F0 UI on a PC window (720×480, 90×30 grid, 8×16 bitmap font).

## Requirements

- CMake 3.16+
- SDL2 development libraries
- C compiler (gcc, clang, or MSVC)

### Windows
Install SDL2 via vcpkg or the official SDL2 development package and set `SDL2_DIR` if needed.

### Linux
```bash
sudo apt install cmake libsdl2-dev
```

## Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

The binary is `rg34xxsp-redux` (or `rg34xxsp-redux.exe` on Windows).

## Font

Uses a built-in 8×16 bitmap font derived from the public-domain VGA ROM font (GPL-compatible). The font data is embedded in `src/font.c`. No external font files are required.

## Controls (desktop remap)

| Key | Logical button |
|-----|----------------|
| Arrow keys / WASD | D-pad |
| Enter / Z | A (Select) |
| Escape / X | B (Back) |
| C | X |
| V | Y |
| M | Menu |
| Space | Start |

All input goes through the remap table in `src/remap.c`. No hard-coded key codes in UI code.

## Platform layer

`src/platform.c` supplies fake battery, temperature, and clock values. F2/F7 will replace this module; UI code never touches `/sys` or real hardware directly.

## What is not yet verified

- This is a desktop prototype only. Nothing has been run on the RG34XXSP.
- Screenshots for the F0 exit test will be added once the full navigation is complete.
