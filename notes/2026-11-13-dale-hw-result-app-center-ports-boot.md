---
from: dale
to: all
event: hw-result
phase: F6
status: open
refs: [issue #109, notes/STATUS.md]
---

# Dale hardware result, 2026-11-13 08:06

Build: dev release expected `168c8bc` (deployed per chat 07:32). **Title-bar SHA not reported yet**:
confirm it, because the Boot Screen result below may mean an older build was running.

## Dale's words (verbatim, lightly re-flowed)

- No boot errors, Stock OS boot logo.
- **Brightness and TF2 ROMs:** "Brightness L/R moves bar, No effect, M button FAIL". "TF card select is
  reachable through Music Play/Back button/Music Player App settings/((Close App, Shut down, Play Mode,
  EQ, Backlight brightness, Background Settings (stock OS background screens - no custom files))".
  "Screen display timeout is ON, Power to return, no way to change setting".
- **File Manager:** "screen tears in loading, opens Stock OS file manager, M button loads app Menu, Quit
  select to Main menus".
- **App Center:** "No TF1 or TF2 select, straight to App menu".
  - Music Player: "Loads Stock app, A button selects OK, B button [Return} loop (storage select, track
    select, player), M button Stock OS Close/Shutdown screen."
  - Video Player: "Loads Stock app, A button selects OK, Playing video crashes back to Main menus".
  - E-Book Reader: "loads file selection, M button crashes back to Main menus".
  - PDF Reader: n/a.
  - Terminal: "working as before tested".
  - Task Monitor: "working but it's not htop...top only, M button quits to Main menus".
- **Ports, then Card Scripts:** "A button selects OK, B button OK, L/R buttons OK, Y X Start Select FAIL".
- **Wi-Fi connect:** "Appears to work is previously tested."
- **Boot Screen:** "Persistent stock boot screen, no settings". "Stock OS Background Settings (stock boot
  screens - no custom files) reachable through Menu bug."

## Reading (Claude)

| Item | Result |
|---|---|
| Boots, no errors | PASS |
| Brightness (#102) | **FAIL**: bar moves, backlight unchanged; M button fails on that screen |
| TF2 ROM scan (#104) | not reported |
| File Manager (stock, via Main) | PASS with **tearing** while loading |
| Music Player | PASS (stock app's own storage/track/player flow; M = stock close/shutdown screen) |
| Video Player | **FAIL**: playing a video drops back to Redux |
| Ebook Reader | **FAIL**: M drops back to Redux (crash or exit, unknown) |
| Terminal | PASS |
| Task Monitor | PASS (`top`; stock has no htop, as stated in #129) |
| Ports | launch PASS; **Y, X, Start, Select don't work in-game** |
| Card Scripts switch | not reported |
| Wi-Fi connect (#88) | appears PASS |
| Boot Screen manager (#133) | **not reached**: no Boot Screen setting seen; logo still stock. Build to be confirmed |
| Screen timeout | no Redux setting exists (feature gap, not a regression) |

Also seen: the **stock Music Player's menu** has "Backlight brightness" and "Background Settings" (stock
boot/background screens). A stock app that can change boot screens is a second writer to boot-resource;
the Boot Screen manager's "refuse if the logo on the partition isn't stock format" check matters here.
