---
type: question
from: claude
to: dale
date: 2026-11-18
re: MGR for DALE 13:36, rebuilt keyboard UI ("Uncrowded keys, buttons framed like a key, BIOS/DOS UI is good"); mock-ups for A/B before code
status: open
---

# Keyboard UI: three mock-ups

One design for both the global OSK and the Redux Terminal picker. Every option: framed key caps (black outline, grey
face, white top/left edge, dark shadow bottom/right), Redux's VGA 8x16 font (labels 2x where they fit), BIOS/DOS
palette (blue panel, double-line frame, cyan page tabs), yellow focus key with a white ring, latched Shift/Ctrl
green with a ▲. Chord Menu + Select and "no OSK while RetroArch runs" as decided. Desk renders only (Redux font,
Python/PIL on the desk; nothing on the device). Images: [`docs/img/osk-mockups/`](../docs/img/osk-mockups/).

| Option | Keys | Cap size | Pages | Panel (app visible above) | Images |
|---|---|---|---|---|---|
| **A (recommended)** | 10 a row, 4 rows, QWERTY | 62 x 40 | 3: abc / 123 (digits + symbols) / Fn (F1-F10, Esc, Ins, Del, Home, End, PgUp/PgDn, arrows, rest of the symbols) | 712 x 240 (240 px of app) | `A-abc`, `A-123`, `A-Fn` at 720 x 480; `A-abc-1280x720` (HDMI) |
| B | 11 a row, 5 rows, digits on the letter page | 56 x 36 | 2: abc / Sym+Fn | 704 x 256 (224 px) | `B-abc` |
| C | 9 a row, 4 rows (QWERTY wraps: p starts row 2) | 70 x 48 | 4: abc / 123 / sym / Fn | 712 x 272 (208 px) | `C-abc` |

- **A:** largest keys that keep the QWERTY rows intact; one L1/R1 press reaches digits or F-keys.
- **B:** fewest page flips (digits always there), smaller caps, 1x labels on most keys.
- **C:** the biggest caps, but the letter rows no longer match a keyboard and two more page flips.

Paging: L1/R1 (or the tab). Shift/Ctrl stay latched across pages. Done when Dale picks; then one shared layout table
replaces `frontend/term/osk.c`'s rows and #297's flat drawing (`osk_draw`) gets the cap style.
