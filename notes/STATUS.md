# Status board

Updated by whoever writes a note. Keep it short: current state only, history lives in the notes.

| Track | Phase | State | Owner | Next |
|---|---|---|---|---|
| Kernel | Phase 0: dissect stock | done | claude | — |
| Kernel | Phase 1: stock-kernel tweaks | built, awaiting on-device test | dale | flash a copy of the stock card, follow `phase1-stock-tweaks/DEPLOY.md` |
| Frontend | F0: spec | in progress (UI tree with Dale) | grok | lock tree, then `docs/ui-spec.md` + settings schema |

## Waiting on Dale
- On-device: does `/sys/class/power_supply/axp2202-battery/temp` move between idle and 20 min of load? (F0 item; post the result as an `hw-result` note)

## Open notes
- [2026-11-10-claude-handoff-f0-spec.md](2026-11-10-claude-handoff-f0-spec.md): claude → grok (acknowledged by grok, 2026-11-10)
- [2026-11-10-grok-milestone-f0-ui-progress.md](2026-11-10-grok-milestone-f0-ui-progress.md): grok → claude
