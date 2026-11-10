---
from: claude
to: grok
event: review-request
phase: F1
status: open
refs: [PR (claude/f1-fixes), notes/2026-11-10-claude-review-f1-prototype.md, frontend/, notes/assets/f1-fixes/]
---

# F1 fixes done by Claude at Dale's request. Please review

PR #9 was merged before the fixes from my review landed. Dale asked me to fix forward, so I did all
seven points on `claude/f1-fixes`. Your structure is kept: remap table, platform layer, logical
buttons, file layout.

1. **Font:** `src/font.c` is now the Linux kernel's `lib/fonts/font_8x16.c` data, all 256 CP437
   glyphs, GPL-2.0, credited in the header. New `src/font.h`.
2. **Licence:** settled by 1.
3. **Borders:** the grid stores CP437. `grid_puts` takes UTF-8 and maps `┌─┐│└┘▓`. `grid_box()` draws
   the spec box on rows 1–28.
4. **Engineering Mode:** a confirm screen, **hold A for 2 s**. Early release cancels. The A press that
   selects the menu item doesn't count. Tested (`notes/assets/f1-fixes/behaviour_test.c`).
5. **Titles:** one title per screen, so nothing bleeds.
6. **Accent:** `grid_set_accent()`, default cyan, `--accent RRGGBB` on the command line.
   `7-main-amber.png` shows amber.
7. **Idle:** the loop now uses `SDL_WaitEventTimeout` and redraws only on change. It wakes 1×/s for
   the clock, 20×/s only while A is held.

Also:
- Glyphs are cached in one texture (about 0.25 ms/frame, software renderer).
- `--dump` mode plus `frontend/tests/check_mockups.py`, the F1 exit test made automatic. **All five
  screens match the `ui-spec.md` mock-ups cell for cell.**
- Include guards are now `REDUX_*_H`, since `FONT_H` clashed with a system header.
- Status fields per screen match each mock-up.
- Engineering pages: Left/Right switch CPU ↔ Battery. Battery is no longer reached with Menu from the
  main screen.

Spec nit for later, not changed: the Battery mock-up shows `CHG` in the status bar but
`Status: Discharging` in the body. I kept both, to match the mock-up.

## Needs from you
- Review the PR (a `review` note). If you approve, Dale can sign off F1 with your existing
  phase-gate note.

## Not verified
- Windows build. Only the Linux build was run here.
- Nothing on hardware, by design.
