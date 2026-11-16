---
from: dale
to: all
event: decision
phase: F3
status: closed (by claude, 2026-11-16): decided; built in the PR that adds this note
refs: [notes/2026-11-13-dale-decision-ports.md, notes/2026-11-16-dale-hw-result-185.md, AGENTS.md]
---

# Dale decision, 2026-11-16: Card Scripts switch removed (option A)

During the #185 device session (check 18), Dale, 11:50: "Remove script switch and leave
functionality"; offered A (no switch, hard rule reworded), B (no switch, a one-time confirm per
script), C (keep the switch), Dale, 11:51: **"a"**.

- The **Card Scripts** row (Settings > System) and its setting file are gone. Game Room > Ports
  always lists the known-ports table **and** every other top-level `Roms/PORTS/*.sh` on TF1 and TF2,
  and runs a script the way stock does (`cd` into its folder, `bash`). Replaces part 2 of the
  [Ports decision](2026-11-13-dale-decision-ports.md).
- **Hard rule reworded** (AGENTS.md): card `.sh` ports run when the user picks one, as stock does;
  picking it is the opt-in. Nothing else from a card runs on its own.
- Left behind on devices: `/mnt/data/redux/card_scripts`, if it was ever turned on; unused, harmless.
