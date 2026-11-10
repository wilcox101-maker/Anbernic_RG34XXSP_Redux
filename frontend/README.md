# RG34XXSP Redux frontend (F1 desktop prototype)

A C + SDL2 prototype that reproduces the locked F0 UI (`docs/ui-spec.md`) in a 720×480 window: a
90×30 character grid, 8×16 VGA font, black / white / one accent colour.

## Requirements

- CMake 3.16+
- SDL2 development libraries (2.0.18 or newer)
- A C compiler (gcc, clang, or MSVC)
- Python 3, only to run the exit test

### Windows
Easiest is vcpkg: `vcpkg install sdl2`, then configure with
`-DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake`. Or download the SDL2 "VC"
development zip from the SDL releases page and point `SDL2_DIR` at its `cmake` folder. Copy
`SDL2.dll` next to the `.exe`.

### Linux
```bash
sudo apt install cmake libsdl2-dev
```

## Build and run

```bash
cmake -S frontend -B build
cmake --build build --config Release
./build/rg34xxsp-redux                    # Windows: build\Release\rg34xxsp-redux.exe
./build/rg34xxsp-redux --accent FFB000    # amber accent instead of the default cyan (00FFFF)
```

The default build uses fake hardware values (`platform.c`). For the device, configure with
`-DREDUX_PLATFORM=stock`, which swaps in `platform_stock.c` (real battery, CPU and Wi-Fi from sysfs).

## Exit test (F1)

```bash
python frontend/tests/check_mockups.py ./build/rg34xxsp-redux
```

This runs the binary with `--dump`, which prints the five F0 screens as text with the clock fixed
to 12:34, and compares them cell for cell with the mock-ups in `docs/ui-spec.md`. `▓▓` marks the
selected row in both. Every screen must print `ok`.

## Controls (desktop remap table, `src/remap.c`)

| Key | Logical button |
|-----|----------------|
| Arrow keys / WASD | D-pad |
| Enter / Z | A |
| Escape / X | B |
| C | X |
| V | Y |
| M | Menu |
| Space | Start |

UI code only sees logical buttons. On the device a second table maps the D-pad and both analog
sticks to the same buttons (see `docs/hardware-map.md`).

**Navigation:** Game Room → ROM list, Settings → settings list, Engineering Mode → confirm screen.
**Hold A for 2 seconds** to enter (release early to cancel). Inside Engineering Mode, Left/Right
switch between the CPU and Battery pages. B goes back.

## How it's built

| File | Job |
|---|---|
| `src/grid.c` | 90×30 cells in CP437. UTF-8 box characters (`┌─┐│└┘▓`) are mapped on input. Glyphs are cached in one texture. Accent colour is settable. Text dump for tests |
| `src/font.c` | 256-glyph 8×16 VGA font from the Linux kernel (`lib/fonts/font_8x16.c`), **GPL-2.0**, the same licence as this project |
| `src/ui.c` | screens, navigation, the hold-A confirm. Writes cells only, never pixels |
| `src/remap.c` | raw key → logical button. The only file that knows key codes |
| `src/platform.c` | battery, temperatures, clock. Desktop fakes; F2/F7 replace this file. UI never touches `/sys` |
| `src/main.c` | window, event loop. **Redraws only on change** (input, the hold timer, the clock), so an idle menu sleeps instead of drawing 60 frames a second |

## Not verified

- Nothing has run on the RG34XXSP. This is a desktop build only.
- The exit test and screenshots were produced on Linux (SDL 2.30, software renderer, about 0.25 ms
  per frame on x86). Windows build steps are written but untested here.
