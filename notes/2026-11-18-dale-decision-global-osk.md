---
type: decision
from: dale
to: all
date: 2026-11-18
re: global on-screen keyboard plan D1-D6 (Dale 11:23, "A, A, B(if no conflicts) B, A, A")
status: acknowledged (D3 and D4 replaced by 2026-11-18-dale-decision-global-osk-2.md)
---

# Decision: global on-screen keyboard (OSK)

Plan: [2026-11-18-claude-question-global-osk-plan.md](2026-11-18-claude-question-global-osk-plan.md). Not yet on device.

- **D1 A:** `redux-osk`, C, aarch64, embedded in `dmenu.bin`; Redux starts it and runs `redux-osk --off` (ch3/l0 off) at every start, on switch Off and before Restart / Power Off.
- **D2 A:** Settings > On-screen Keyboard Off / On, default Off until the K26 probe passes.
- **D3 B:** **Menu + X** opens and closes it, if the probe (K27) shows no conflict; otherwise back to Dale with what clashed. While open: D-pad moves, A types, B closes, Select = Shift.
- **D4 B:** Esc and F1 are left off the keyboard while RetroArch runs.
- **D5 A:** first device step = one read-only, RAM-only probe session K24-K28 with KR's kit (KR-1; Owner reply FROM-REDUX-OWNER-2026-11-18-s in Claude Files), before the daemon is built.
- **D6 A:** Redux Terminal's own Select picker is replaced by the global OSK once K27 shows uinput keys reach the terminal.

Task split: KR-1, then KR-2 (display) + KR-3 (input) by Kernel Research; DR-1..3 by Deep Research; KR-4 (lifecycle) and integration by Claude.
