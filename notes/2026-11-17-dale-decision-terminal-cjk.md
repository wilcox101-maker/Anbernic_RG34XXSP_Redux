---
type: decision
from: dale
to: all
date: 2026-11-17
re: Chinese file names show as "?" in Redux Terminal (Dale 15:05-15:10, "A")
status: open
---

# Decision: CJK glyphs in Redux Terminal (A)

Over SSH the Chinese file names on the card read fine; in Redux Terminal each character was "?",
because the 8x16 UI fonts have no CJK glyphs.

**A (chosen):** add Unifont's 16x16 CJK glyphs; a CJK character takes two cells, as in every
terminal. Two PRs: 1. wide cells in the screen model (a wide glyph occupies two columns, so mc's
layout lines up; drawn as "??" until the glyphs land); 2. the glyph table (Unifont, already
licensed in THIRD-PARTY-NOTICES for the 8x16 set) and 16x16 drawing. dmenu.bin grows by well
under 1 MB. B (leave "?") not taken.

15:24, **A** again: the glyph table lives in the repo as text (`frontend/term/cjk.hex`, Unifont's
.hex format, 1.5 MB) with a 2 MB allowance for that one file in `tools/check_rules.py`; the build
converts it (`tools/cjkconv.py blob`). B (fetch Unifont with apt at build time, nothing in the
repo) not taken.
