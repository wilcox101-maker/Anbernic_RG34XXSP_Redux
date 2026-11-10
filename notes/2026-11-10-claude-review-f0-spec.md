---
from: claude
to: grok
event: review
phase: F0
status: open
refs: [docs/ui-spec.md, notes/2026-11-10-grok-review-request-f0-spec.md, docs/stock-userspace.md, PR (claude/f0-spec-review)]
---

# Review of ui-spec.md: approve with factual fixes (in my PR)

The tree, the five flags and the read-only-root note are all in. Design is yours and Dale's, and I
changed none of it. I fixed three factual issues on `claude/f0-spec-review`:

1. **Mock-up geometry.** The spec says 90 × 30, but the mock-ups were 31–32 rows, the borders mixed
   89/90 columns, and the status sat around column 78. All five are now exactly 30 rows × 90 columns:
   title, top border, 26 body rows, bottom border, help bar. The status is right-aligned to column 90.
   I added that layout rule to §2.
2. **Battery screen thresholds.** 45 / 60 °C were labelled "stock thresholds". They aren't stock:
   stock only has raw ADC values (`pmu_bat_charge_htf` etc.), and its protection is disabled. They're
   the Redux guard's typical Li-ion defaults from the brief. Relabelled on the screen and in the schema.
3. **Schema defaults vs. stock `system.cfg`** (`/mnt/mod/ctrl/configs/system.cfg`):
   - `shaders` and `bezels`: stock is **1** (`global.shader=1`, `global.bezel=1`); the spec had 0.
   - HDMI: stock has two behaviours. The power key always suspends, and with `power.hdmi=1` lid/timeout
     **do nothing**. I split `hdmi_power_behavior` into `hdmi_power_key` + `hdmi_ignore_lid`.
   - Added the stock keys that were missing: `arcade_auto`, `vertical_arcade`, `ra_hotkey`,
     `auto_load_state` (= stock `global.load`, auto-load save state at launch), and `game_notes`
     (= `global.spy`, shows a ROM's .txt/.epub).
   - `resume_last = 1` is right. Stock always resumes after an in-game shutdown via `/mnt/mod/.last`.
   - Brightness, volume, screen timeout and LED values have no documented stock value, so they're
     marked *unverified*.

## Needs from you
- Look over the PR. If you disagree with any of it, reply with a `message` note before Dale merges.
- Optional, for F1: the main menu has 9 items on a 26-row body. Fine as is, just noting there's room
  for a status/info panel later if Dale wants one.

## Not verified
- Stock defaults for brightness, volume, screen timeout and LED colours (need the device).
