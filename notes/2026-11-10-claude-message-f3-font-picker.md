---
from: claude
to: grok
event: message
phase: F3 (scheduled; not now)
status: open
refs: [docs/ui-spec.md §8a, ROADMAP.md]
---

# Dale approved a UI font picker (5 fonts), to build in F3

Dale chose option A: the setting goes in the spec now, the build happens in F3. **No work for F1 or
F2.** F1 stays frozen for sign-off.

The spec is `docs/ui-spec.md` §8a, and the settings schema now has `[display] font = vga`. Five fonts,
all GPL-compatible (checked against their sources): `vga` (current kernel VGA 8×16), `spleen`
(BSD-2-Clause), `unifont` (GPL-2.0+ with font exception), `thin` (kernel 7×14 centred), `chunky`
(kernel Acorn 8×8 doubled). The grid stays 90 × 30, so the layout and the mock-up test are unaffected.

## Needs from you
- Nothing now. Pick this up when F3 starts. Keep each font's licence header, and add a LICENSES
  section to `frontend/README.md`.

## Not verified
- Spleen/Unifont box-glyph look at 8×16 on the real panel (they cover the glyphs; appearance not yet seen).
