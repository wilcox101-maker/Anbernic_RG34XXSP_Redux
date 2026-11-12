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
2. **Unlock: you have to really mean it** (Dale: "not earn it, but better really mean it").
   Every gate below is required, in this order:
   1. **The classic code** on *Settings → System*: **↑ ↑ ↓ ↓ ← → ← → B A Start**, with no more than
      1 s between presses (a wrong press or a pause resets it). Docs and UI say "the classic code"
      and never use a company name.
   2. **A full warning that can't be skipped:** heat, instability, SD-card corruption, reduced
      battery life and lifespan, no support. The Continue button stays disabled until the text has
      been scrolled to the end **and** 15 s have passed.
   3. **Type the full phrase** `I ACCEPT THE RISK TO MY DEVICE` on the on-screen keyboard. It must
      match exactly; any mismatch cancels the whole unlock.
   4. **Power check:** on the charger, or battery at 50 % or more. Otherwise refuse and explain why.
   5. **Cooling-off reboot:** the unlock is written, but *Expert* only appears **after the next
      reboot**, so nothing above factory can be changed in the same sitting as the unlock.
   Any B press during steps 2-4 cancels the unlock and starts it over from the code.
3. **Per-change confirmation.** Each risky setting (CPU max above 1512 MHz, GPU clock,
   undervolt, raised throttle points) has its own confirm screen, which shows the old and new
   value and requires **holding A for 5 s**. One step at a time: a value can only move one
   frequency step per apply.
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

## Discovery: breadcrumbs, never a tutorial (Dale, 2026-11-12)

Dale: "hide it in the code with nods to who could find it." The UI never explains the code; these
nods reward people who look. This note is the maintainer reference and the only plain description
of the code. Build all of these together with Expert Mode, not before:

- **Engineering Mode → CPU tab:** under `Max 1512 MHz (factory)`, a dim footer line:
  `The classics never die.`
- **Settings → System, description line on the last row while Expert is locked:**
  `Some settings are reserved for those who remember.`
- **Close but wrong:** if the first 8 inputs (the arrows) are right but the code then breaks, the
  screen border flickers for one frame. No text, no sound.
- **redux.log at start-up**, once: `redux: cheat engine idle  ^ ^ v v < > < > . .` (the last two
  inputs are left as dots on purpose).
- **`dmenu.bin --version`:** a second line `30 lives not included.`
- **Source:** the sequence lives in `frontend/src/expert.c` as `static const logical_button_t
  jungle_stage[]`, with the comment `/* You know this one. If not, this menu isn't for you. */`.
  No other comment explains it.
- **README footer:** one line, *"Old-school gamers know the way."*

Rules for the nods: never name the game or the company, never spell out the full sequence
outside this note and the source array, and a nod never skips any of the five gates.
