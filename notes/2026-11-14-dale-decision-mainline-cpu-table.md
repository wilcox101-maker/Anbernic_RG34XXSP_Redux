---
from: dale
to: all
event: decision
phase: 2
status: open
refs: [AGENTS.md, notes/2026-11-12-dale-decision-expert-mode.md, docs/kernel-research/stock-kernel-ghidra.md, docs/kernel-research/mainline-status.md]
---

# Dale decision: CPU speed/voltage table on mainline (2026-11-14)

Dale (Manager chat, 01:32): "Consider adopting the ROCKNIX solution...it is low impact." Then, choosing
between three options the Manager laid out: **"2"**.

**Decision: option 2.** On the mainline kernel (Phase 2 and later), Redux uses **ROCKNIX's H700 CPU table**
(upstream `sun50i-h616-cpu-opp.dtsi` plus ROCKNIX patch `0200-h700-update-opps`, and `0210` for the 480 MHz
suspend step), with these conditions:

1. **1512 MHz is off by default and only reachable through Expert Mode**, with every Expert Mode safeguard
   (notes/2026-11-12-dale-decision-expert-mode.md). ROCKNIX already marks it `turbo-mode`; Redux keeps
   boost disabled unless Expert Mode turns it on. This is stricter than stock for best-bin chips (stock gives
   them 1512 MHz by default) and the same as stock for the rest.
2. **ROCKNIX's top-speed voltages are accepted** although they are lower than stock H700's
   (1416 MHz at 1100 mV against stock's 1150–1160 mV), on the strength of ROCKNIX's field use on H700
   devices **and** a Redux stress test on Dale's unit before Beta: sustained load at each top step for the
   unit's grade, no crash, no throttling beyond the stock thermal trips. If the test fails, raise that step
   to stock's voltage for the grade.
3. **The thermal emergency shutdown (critical trip) is unchanged**, as the hard rule says.
4. This is the **mainline reference table**. It does not change the stock-kernel track (Beta), where the
   stock table stays untouched.

Why: low effort, proven on the same SoC family, and the one real overshoot (1512 MHz on grades stock caps
lower) goes behind the existing Expert Mode gate.

Not settled by this decision: the exact mapping between stock's `opp-supported-hw` bits and mainline's
`speed0`–`speed5` grades. Needed for the stress test to know which steps Dale's unit gets
(docs/kernel-research/open-questions.md, question 5).

Hard rules: AGENTS.md says nothing runs past the stock OPP table by default and no undervolting. This
decision records that, **on mainline**, the reference table is ROCKNIX's H700 table as conditioned above.
It is a hard-rule interpretation, so it is in the "never merged without Dale" list.
