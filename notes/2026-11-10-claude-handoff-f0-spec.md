---
from: claude
to: grok
event: handoff
phase: F0
status: open
refs: [AGENTS.md, docs/ui-brief.md, docs/stock-userspace.md, docs/hardware-map.md]
---

# Frontend F0 (spec) is yours to start

Hello Grok. Claude here. I did the stock-firmware dissection (kernel Phase 0) and built the Phase 1
boot patches. Dale has asked you to take the frontend, starting at F0.

What exists:
- `docs/ui-brief.md` is the frontend plan: three-colour 90s-BIOS text UI on a 90 × 30 grid (8×16
  font at 720×480), stock parity list, features stock left out, Engineering Mode with its safety net,
  and phases F0–F8 with exit tests.
- `docs/stock-userspace.md` describes what the stock launcher really does: boot chain, every setting,
  how power, lid and the governor are handled. Use it for the parity list and the settings schema.
- `docs/hardware-map.md` has the buttons (stock codes are scrambled; use a remap table), display,
  PMIC, and the disabled battery-temperature protection.

## Needs from you
- F0 deliverables per `docs/ui-brief.md` §7: `docs/ui-spec.md` (with the five ASCII mock-ups) and the
  settings schema. Open them as a PR from a `grok/f0-spec` branch.
- When the PR is up, leave a `review-request` note to me and a `phase-gate` note to Dale.
- If anything in the brief or the stock docs looks wrong or contradictory, write a `question` or
  `correction` note rather than working around it.

## Not verified
- Whether the battery has a working thermistor. That's Dale's on-device check, still open.
- Everything in `docs/stock-userspace.md` marked "strings only" or "unconfirmed".
