# Status board

Updated by whoever writes a note. Who visited and when is in [ACTIVITY.md](ACTIVITY.md). Keep it short: current state only, history lives in the notes.

| Track | Phase | State | Owner | Next |
|---|---|---|---|---|
| Kernel | Phase 0: dissect stock | done | claude | — |
| Kernel | Phase 1: stock-kernel tweaks | built, awaiting on-device test | dale | flash a copy of the stock card, follow `phase1-stock-tweaks/DEPLOY.md` |
| Kernel | Phase 2: RAM-resident system | design merged (`docs/ram-system-design.md`) | claude | measurements listed in the doc need the device |
| Frontend | F0: spec | **done**, signed off by Dale 2026-11-10 | grok | — |
| Frontend | F1: desktop prototype | PR #9 + fixes PR #11 (approved by grok); exit test passes 5/5 | dale | merge #11, then F1 sign-off |

## Waiting on Dale
- On-device: does `/sys/class/power_supply/axp2202-battery/temp` move between idle and 20 min of load? (F0 item; post the result as an `hw-result` note)

## Open notes
- [2026-11-10-claude-message-f3-font-picker.md](2026-11-10-claude-message-f3-font-picker.md): claude → grok (F3, FYI)
- [2026-11-10-claude-review-request-f1-fixes.md](2026-11-10-claude-review-request-f1-fixes.md): claude → grok (answered: grok approved)
- [2026-11-10-claude-review-f1-prototype.md](2026-11-10-claude-review-f1-prototype.md): claude → grok (resolved by the fixes PR)
- [2026-11-10-grok-phase-gate-f1-prototype.md](2026-11-10-grok-phase-gate-f1-prototype.md): grok → dale (F1 sign-off: ready, review passed)
- [2026-11-10-claude-milestone-ram-system-design.md](2026-11-10-claude-milestone-ram-system-design.md): claude → all (FYI)

F0 notes are all closed or acknowledged. See `notes/` for history.
