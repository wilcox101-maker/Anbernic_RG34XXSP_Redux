---
from: dale
to: all
event: hw-result
phase: F3
status: open
refs: [issue #185]
---

# Dale hardware result, 2026-11-16 10:53-12:25: device session #185

Build: dev **`5d7dea6`** (merge of #198; title bar `v0.1-dev - 5d7dea6`, `dmenu.bin` SHA256 `0ae0be26…`).
Device: Dale's stock 2 GB RG34XXSP on Wi-Fi at 172.16.3.178. Run step by step in the owner chat;
results are Dale's words there, recorded by Claude. Backup first: `dmenu_ln*` and `/mnt/data/redux`
copied to `D:\.SANDBOX\Redux CFW\backup-1116`.

## Results (#185 check numbers)
| # | Check | Result |
|---|---|---|
| 1 | `install.py --check` | PASS (launcher redux, `.bak` stock) |
| 2 | Uninstall → stock | PASS (Redux logo, then Anbernic screen, stock menu; `.bak` kept, stock content) |
| 3 | Install `5d7dea6` | PASS (Redux boots) |
| 4 | Install again (upgrade) | PASS (`install.log`: uninstall, install from=stock, install from=redux; sha256 matches) |
| 5-7 | OpenBOR: Contra Locked'n Loaded (TF2) plays, controls, M back to Redux | PASS |
| 8 | OpenBOR saves | TF1: `/mnt/mmc/Roms/OPENBOR/Saves` changed at 11:19, TF2 not |
| 9 | Video Player | **FAIL**: plays to the end without crashing (`playback exit=0`), but **no buttons during playback**; at the end the stock player's card screen shows, M → Close App returns to Redux |
| 10-12 | F3: five fonts redraw with joined box lines; Unifont kept after restart; Reset All (tap nothing, 3 s hold Done, defaults back, boot screen and Wi-Fi kept) | PASS |
| 13 | Apps resume | PASS (back on the app used; htop, Music Player, Terminal, Ebook) |
| 14 | File Manager on Main only | PASS |
| 15 | TF2 games launch | PASS |
| 16 | Ebook: M quits | PASS |
| 17 | Brightness changes and is kept | PASS |
| 18 | Card Scripts | Hold-A On PASS. With the switch Off, third-party `Roms/PORTS/*.sh` on TF2 were still listed and ran (cause not found in the code). **Replaced by Dale's decision A** (11:51: remove the switch; card scripts always listed and run; hard-rule rewording PR for Dale). |
| 19 | Buttons | Wi-Fi keyboard Start / list Y PASS; **Game Room X (Favorites): FAIL**, no option shown or working; **Wolfenstein 3D: Start/Select PASS, Y and X FAIL** |
| 20 | Wi-Fi: wrong password Not connected, saved network reconnects, SSH kept | PASS |
| 21 | Screen Timeout 1 min / Off | PASS |
| 22 | Terminal tearing | PASS (fixed) |
| 23 | vsync library | PASS (`redux-vsync.so` and SimpleTerminal ARM EABI5 32-bit, armhf libc, glibc 2.35) |
| 24 | `date +%s` | PASS |
| 25 | Start-failure guard | PASS with `kill -9` (exit 137 ×2 → `3 quick failures in a row` → stock menu). A plain `kill` exits 0, see findings. |
| 26 | Stock menu: game and video return to it; restart → Redux | PASS |
| 27 | Diagnostics | PASS: CPU table `480000 … 1512000`; `adc_cal` at `/sys/devices/platform/soc/soc@03000000:gpio_keys/adc_cal`; `openbor.sh` captured; after the stock-menu boot EGL was ready after 3.5 s of retries |
| 28 | Lid | PASS: **event0 `axp2202-pek`**; close = `KEY_INSERT` down (auto-repeat), open = `KEY_INSERT` up + a short `KEY_DELETE` tap |
| 29 | Boot Screen restore and install | PASS |
| 30 | Boot logo by hand over SSH | PASS |

## Findings (new queue items)
1. Video Player: buttons don't reach the player during playback (9).
2. Game Room: X / Favorites missing (19).
3. Wolfenstein 3D (ecwolf): Y and X do nothing (19).
4. A game on both cards is listed twice with no card shown (OpenBOR, Ebook Reader, every system).
5. A ROM-scan screen shows every time an app returns to Redux.
6. Redux doesn't clear `/tmp/.next` at start: a clean exit with no choice re-runs the last app (25).
7. Every Redux start writes the `amixer` output into `redux.log`.
8. No on-device way to pick a custom boot image (Dale's note).
