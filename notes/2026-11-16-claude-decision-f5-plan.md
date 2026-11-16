---
type: decision
from: claude
to: dale
date: 2026-11-16
re: F5 Engineering Mode, phase-start plan (docs/ui-brief.md §6 and F5 row; ROADMAP Beta)
status: open (plan GO by Dale 15:20; Q1-Q3 open)
---

# Decision: F5 plan, and three questions

Plan sent in chat 15:19; Dale's `GO` (15:20) starts step 1. Q1-Q3 are still open.
Everything stays inside the stock tables: CPU 480-1512 MHz, GPU only its own listed speeds.
Expert Mode is not part of F5.

## Plan (one PR per step)
1. **Real read-outs, no writes.** CPU (governor, min/max, per-core, OPP list from sysfs), Battery
   (honest: no sensor, no fake time-left or drain), Thermal (zones and trip points, read-only),
   Display, System (the System Tools rows). New tabs (GPU, Power, Input) arrive with their content.
2. **Safety net.** Write log (what, old, new, time); values only from the device's own lists; live
   until restart unless "Save as profile" (in `/mnt/data/redux`, never p1); two failed starts in a
   row ignore the profile. F4's governor restore takes a saved profile as the normal state.
3. **CPU and GPU controls.** Governor, min/max, cores on/off (cpu0 always on), GPU fixed or auto.
4. **Heat guard** (Q1).
5. **Input viewer, LED and rumble test, log viewer, storage check** (Q3).

Device read first (read-only, batched into #185, before step 3): GPU devfreq path and speeds,
charge-enable attribute, LED and rumble paths.

## Correction to the chat plan
The battery temperature file is a **fixed dummy** (stays 300 under load and charging;
[hardware-map](../docs/hardware-map.md), 2026-11-10). There is no battery sensor, so a battery
guard can only use the SoC temperature, which reaches about 59 C under normal full load. The 45/60 C
battery limits in the brief can't be applied to it. Q1 is reworded for that.

## Questions
- **Q1 Heat guard.** **A (recommended):** build the guard on SoC temperature; pick its limits after a
  measurement on the device (SoC temperature at full load while charging), then you choose the
  numbers and what happens at the top limit (stop charging, save, shut down). **B:** no extra guard
  in Beta; the stock thermal trip points protect the SoC, the Battery tab says "no sensor", and the
  exit test's guard line is dropped at sign-off.
- **Q2 Thermal trip points.** **A (recommended):** read-only in F5 (raising them is Expert, after
  1.0; the critical trip is never changed). **B:** allow lowering only, behind a second confirm.
- **Q3 Storage check.** **A (recommended):** check only (`fsck -n`, never repairs). **B:** offer repair.

## Done when
F5 exit test (ui-brief F5 row): every control reads back the real sysfs value; a deliberately bad
setting recovers through the fallback without reflashing; 30 min at max CPU/GPU while charging with
the guard working as decided in Q1. Then Dale's sign-off.
