# Status board

Updated by whoever writes a note. Who visited and when is in [ACTIVITY.md](ACTIVITY.md). Keep it short: current state only, history lives in the notes.

| Track | Phase | State | Owner | Next |
|---|---|---|---|---|
| Kernel | Phase 0: dissect stock | done | claude | — |
| Kernel | Phase 1: stock-kernel tweaks | **boots on device** (≈20 s to menu, no issues seen) | dale | optional: governor check + DEPLOY.md §5 measurements |
| Kernel | Phase 2: RAM-resident system | design merged (`docs/ram-system-design.md`) | claude | measurements listed in the doc need the device |
| Frontend | F0: spec | **done**, signed off by Dale 2026-11-10 | grok | — |
| Frontend | F1: desktop prototype | **done**, signed off by Dale 2026-11-10 | grok + claude | — |
| Frontend | F2: launch parity on stock | **menu runs on the device as the launcher** (2026-11-10 hw-result); ROM list merged (#43) | grok | Wi-Fi screen PR, plus this **Dale-only** back-to-stock draft (do not merge). Then Dale launches a real game → F2 sign-off |

## Waiting on Dale
- Launch a real game from Redux on the device (last F2 test).
- Battery thermistor: confirmed dummy (stays 300); nothing more needed.

## Queue (inbox #41, 2026-11-11, in order)
1. Wi-Fi setup screen (grok) — separate PR on `grok/wifi-setup`. Password is not logged or stored by that UI.
2. Back to stock menu (grok) — this draft. Dale-only: it writes `/mnt/vendor/ctrl/dmenu_ln` using dale-scripts S7. Do not merge.
3. TF2 ROM scan of `/mnt/sdcard/Roms` — not started.
4. Boot logo at 640×480 — not started. The installer is Dale-only.

## Queue (decided 2026-11-10, after the inbox items)
1. Real ROM list (grok) — merged (#43)
2. Battery "Full", CPU governor/min/max from sysfs, "--" for unmeasured fields
3. Text size Large 12×24 (`2026-11-10-dale-decision-text-size.md`)
4. Power & Lid: dim / screen off / sleep timers. Wi-Fi switching was pulled forward by inbox #41 (`2026-11-10-dale-decision-power-management.md`)
5. Graphical browser: NetSurf (framebuffer) proposed for Internet → Web browser; WPE/Cog for Phase 2. Not yet decided.

## Open notes
- [2026-11-11-grok-review-request-back-to-stock.md](2026-11-11-grok-review-request-back-to-stock.md): grok → dale (DALE-ONLY, do not merge)
- [2026-11-10-grok-review-request-f2-rom-list.md](2026-11-10-grok-review-request-f2-rom-list.md): grok → claude (ROM list, merged with #43)
- [2026-11-10-dale-hw-result-f2-menu-on-device.md](2026-11-10-dale-hw-result-f2-menu-on-device.md): dale → all (F2 menu on device)
- [2026-11-10-dale-decision-eng-tabs-back.md](2026-11-10-dale-decision-eng-tabs-back.md), [text-size](2026-11-10-dale-decision-text-size.md), [power-management](2026-11-10-dale-decision-power-management.md): decisions → grok
- [2026-11-10-dale-hw-result-phase1-boot.md](2026-11-10-dale-hw-result-phase1-boot.md): dale → all (Phase 1 boots)
- [2026-11-10-claude-phase-start-f2-launch-parity.md](2026-11-10-claude-phase-start-f2-launch-parity.md): claude → grok
- [2026-11-10-claude-message-f3-font-picker.md](2026-11-10-claude-message-f3-font-picker.md): claude → grok (F3, FYI)
- [2026-11-10-claude-milestone-ram-system-design.md](2026-11-10-claude-milestone-ram-system-design.md): claude → all (FYI)

F0 and F1 notes are all closed or acknowledged. See `notes/` for history.
