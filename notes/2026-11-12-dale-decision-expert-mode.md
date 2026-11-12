# Dale decision: Expert Mode (2026-11-12)

To: all

**Principle (Dale):** Don't break hardware, but in the spirit of the open-source community it isn't
our place to stop a user. Allow above-factory settings, but make sure they can never be turned on
by accident.

**Decision: option A, Expert Mode, with the thermal emergency shutdown kept locked.** This changes the
AGENTS.md hard rule; the factory envelope is now the *default*, not an absolute.

## Required safeguards (all of them, every Expert feature)

1. **Off by default.** No above-factory value ships enabled, and no migration or config file turns
   one on.
2. **Hidden unlock.** *Settings → System → Expert* appears only after holding **L1 + R1 + Select
   for 5 s**. Then comes a full-screen warning (heat, instability, SD-card corruption, reduced
   battery life, no support), and the user must **type `I UNDERSTAND`** on the on-screen keyboard.
3. **Per-change confirmation.** Each risky setting (CPU max above 1512 MHz, GPU clock,
   undervolt, raised throttle points) has its own confirm screen.
4. **Keep-or-revert.** After a change is applied, a 30 s countdown asks "Keep these settings?".
   No answer means it reverts automatically.
5. **Boot fallback.** New expert settings are trial-booted once. If that boot doesn't reach the
   menu cleanly (a boot-OK flag written by Redux), the next boot uses factory settings.
   **Safe Mode:** holding a chosen button at power-on ignores all expert settings. The button is
   picked when this is implemented.
6. **Visible.** An `EXPERT` badge stays in the title bar while any above-factory value is active.
   *System Defaults → Reset All* clears them.
7. **Build switch.** `REDUX_EXPERT=OFF` (CMake) compiles Expert Mode out entirely, for
   locked-down distributions.
8. **Never changeable:** the thermal emergency shutdown (critical trip point). Throttle points and
   performance limits may be raised; the last-resort shutdown may not.

## Scope and timing

- CPU frequencies above the stock OPP table need a patched DTB in `boot.img`, so this belongs to
  the Phase 1 boot-image track. It's a **backlog** item: nothing starts without Dale.
- Settings are stored in `/mnt/vendor/redux/expert.ini` (appfs), never on the Roms partitions.
- Every Expert PR is **Dale-only** (it writes to the device or changes limits).
