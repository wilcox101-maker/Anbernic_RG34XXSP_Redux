# Status board

Updated by whoever writes a note. Who visited and when is in [ACTIVITY.md](ACTIVITY.md). Keep it short: current state only, history lives in the notes.

| Track | Phase | State | Owner | Next |
|---|---|---|---|---|
| Kernel | Phase 0: dissect stock | done | claude | — |
| Kernel | Phase 1: stock-kernel tweaks | built, awaiting on-device test | dale | flash a copy of the stock card, follow `phase1-stock-tweaks/DEPLOY.md` |
| Kernel | Phase 2: RAM-resident system | design proposed (`docs/ram-system-design.md`) | claude | Dale review; measurements listed in the doc |
| Frontend | F0: spec | `docs/ui-spec.md` merged (PR #2); claude review fixes in PR #3 (approved by grok); awaiting Dale merge + F0 sign-off | grok | Dale: sign off F0 after the review PR |

## Waiting on Dale
- On-device: does `/sys/class/power_supply/axp2202-battery/temp` move between idle and 20 min of load? (F0 item; post the result as an `hw-result` note)

## Open notes
- [2026-11-10-claude-review-f0-spec.md](2026-11-10-claude-review-f0-spec.md): claude → grok (answered: grok approved)
- [2026-11-10-grok-phase-gate-f0-spec.md](2026-11-10-grok-phase-gate-f0-spec.md): grok → dale (F0 sign-off)
- [2026-11-10-claude-milestone-ram-system-design.md](2026-11-10-claude-milestone-ram-system-design.md): claude → all
- [2026-11-10-claude-handoff-f0-spec.md](2026-11-10-claude-handoff-f0-spec.md): claude → grok (acknowledged by grok, 2026-11-10)
- [2026-11-10-grok-milestone-f0-ui-progress.md](2026-11-10-grok-milestone-f0-ui-progress.md): grok → claude
- [2026-11-10-claude-message-f0-tree-review.md](2026-11-10-claude-message-f0-tree-review.md): claude → grok
