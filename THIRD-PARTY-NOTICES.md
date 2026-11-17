# Third-party notices

What a Redux release ships, and the licences of the parts that are not Redux's own code.
Redux itself is GPL-2.0 ([LICENSE](LICENSE)).

**No BIOS files or ROMs are included**, in this repo or in any release. No Anbernic binaries are
included either.

## What a release contains
The [dev release](https://github.com/wilcox101-maker/Anbernic_RG34XXSP_Redux/releases/tag/dev) holds
`dmenu.bin`, `bootlogo.bmp`, `install.py`, `redux_install.sh`, `run.sh`, `BUILD.txt` and `SHA256SUMS.txt`.

| Part | Where | Licence |
|---|---|---|
| Redux frontend, its helpers (heat guard, lid watcher, vsync library), installer and tools | `dmenu.bin`, `install.py`, `redux_install.sh`, `run.sh` | GPL-2.0 |
| VGA 8×16 font | built into `dmenu.bin` (`frontend/src/font.c`), from Linux `lib/fonts/font_8x16.c` | GPL-2.0 |
| Thin 7×14 and Chunky (Acorn 8×8) fonts | built into `dmenu.bin` (`frontend/src/font_extra.c`), from Linux `lib/fonts/font_7x14.c` and `font_acorn_8x8.c` | GPL-2.0 |
| Spleen 8×16, © 2018-2026 Frederic Cambus | built into `dmenu.bin` (`frontend/src/font_extra.c`) | BSD-2-Clause, full text in [`frontend/README.md`](frontend/README.md#licenses-ui-fonts) |
| GNU Unifont 8×16 and the 16×16 CJK glyphs, © 1998-2023 Roman Czyborra, Paul Hardy and contributors | built into `dmenu.bin` (`frontend/src/font_extra.c`; `frontend/term/cjk.bin` from `tools/cjkconv.py`, drawn by Redux Terminal) | GPL-2.0-or-later with the GNU font embedding exception (also SIL OFL-1.1) |
| Redux boot screen and mark | `bootlogo.bmp`, [`branding/`](branding/README.md) | CC BY 4.0 (original artwork) |

## Used on the device, not shipped
`dmenu.bin` links at run time to libraries already on the stock firmware, and launches programs that
are already there. Redux does not ship or change them:

- **SDL2** (Anbernic's SDL 2.0.12 build), zlib licence; **glibc**, LGPL-2.1-or-later.
- The stock emulators and apps (RetroArch and its cores, PPSSPP, OpenBOR, the stock players and
  tools), each under its own licence.

## The Stock OS Mod
Redux currently relies on the **Stock OS Mod by cbepx-me**
([Anbernic H700 StockOS Modification](https://github.com/cbepx-me/Anbernic-H700-RG-xx-StockOS-Modification)):
on the device, Redux calls the Mod's `RA_launch.sh`, `pwr_new.sh` and `SimpleTerminal`, and the
installer uses its SSH switch. Redux ships none of the Mod's files and copies none of its code. (The
Mod's repository had no licence file when checked on 2026-11-17, at `9511ace`.) If Redux ever copies
a Mod file, its notice goes here.

## Reference material in this repo
`reference/stock-202505/` holds text extracted from Anbernic's firmware: the decompiled device tree
and kernel config (GPL-2.0 kernel sources), the u-boot environment and the initramfs init script.
No binaries.
