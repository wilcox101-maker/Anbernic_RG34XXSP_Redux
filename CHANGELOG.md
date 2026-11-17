# Changelog

All notable changes to Redux. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/);
versions follow [Semantic Versioning](https://semver.org/) once tagged releases start. Until then the
rolling [dev build](https://github.com/wilcox101-maker/Anbernic_RG34XXSP_Redux/releases/tag/dev) is the
only download.

Entries are phase sign-offs. **Device-checked** items link the result note in [`notes/`](notes/);
anything else is marked. A release's notes are taken from its section here by the release workflow
(`.github/workflows/release.yml`): add a `## [x.y.z…]` section before tagging `vx.y.z…`.

## [Unreleased]

### Added
- **F5 Engineering Mode** (signed off 2026-11-17, [sign-off](notes/2026-11-17-dale-phase-gate-f5-signed-off.md)):
  live read-outs on every tab (CPU, GPU, Battery, Thermal, Display, Input, Storage, Logs, System), validated
  and logged writes with a saved profile, CPU controls inside the chip's own tables, the heat guard, an LED
  and rumble test, a check-only `fsck -n`. Device-checked on `d334375`, `7ee31d1`, `2917a5c`, `070427e`.
  Not checked on the device: the heat guard's 75 °C cap and the 30 min full-load run (moved to the Beta exit test).
- **F4 power and lid** (signed off 2026-11-16, [sign-off](notes/2026-11-16-dale-phase-gate-f4-signed-off.md)):
  lid sleep in the menu and in games, governor restored after a game, menu dimming after 60 s. Exit test:
  50 lid cycles mid-game on `6d87953`, all slept and woke.
- **F3 settings parity** (signed off 2026-11-16, [sign-off](notes/2026-11-16-dale-phase-gate-f3-signed-off.md)):
  five UI fonts, Reset All, brightness, Screen Timeout, Wi-Fi, Boot Screen install and restore, TF2 scan.
  Device-checked on `5d7dea6`.
- **Installer A** (device-checked 2026-11-16, [result](notes/2026-11-16-dale-hw-result-185.md)): check,
  install, upgrade and uninstall from a PC over SSH; the start-failure guard falls back to the stock menu.
- **F2 launch parity** (signed off 2026-11-12, [sign-off](notes/2026-11-12-dale-phase-gate-f2-signed-off.md)):
  Redux replaces the stock menu and launches games through the stock launchers and RetroArch cores, reads
  TF2 ROMs, returns to the same ROM list, sets up Wi-Fi and restores the stock menu with a 3 s hold.
- **F1 desktop prototype** (signed off 2026-11-10, [sign-off](notes/2026-11-10-dale-phase-gate-f1-signed-off.md)):
  C + SDL2 frontend matching the spec cell for cell (desktop only).
- **F0 UI spec** (signed off 2026-11-10, [sign-off](notes/2026-11-10-dale-phase-gate-f0-signed-off.md)):
  screen tree, mock-ups and settings schema in [`docs/ui-spec.md`](docs/ui-spec.md).
