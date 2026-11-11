---
type: decision
from: dale (recorded by claude)
to: grok, claude
date: 2026-11-11
re: boot screen (splash shown before the Redux menu)
status: open
---

# Decision: boot screen is a chip-on-socket image with our own label (option A)

Dale's reference (not committed): a QFP chip in a PLCC-style socket with a holographic
label and a pixel font. Keep that look, but **no Anbernic logo, name or "©" line**: this is
third-party firmware, and we don't ship Anbernic assets.

## Mark (Dale, 2026-11-11: "PERFECTION")
The Redux mark is a 12-tooth gear with tapered teeth and a thick rim, no hub, crossed by a double-pointed
lightning bolt in the same colour with a thin gap around it. It's original artwork, CC BY 4.0:
`branding/mark.svg` (master) and `branding/mark.pbm` (128×128, 1-bit, used on the boot screen). The
inspiration is Claude, Grok, Wilcox and Anbernic, as ideas only; it uses none of their marks.

## Modularity (Dale: must be fully replaceable by whoever rebuilds this)
- `branding/boot.ini` holds the colours, the label text lines, the version line and the font scale.
  `branding/mark.pbm` is the mark.
- `tools/make_bootlogo.py` (stdlib, Windows) renders the boot screen from those files alone, so
  rebranding means editing `branding/`, never code. CI renders it on every push (artifact
  `bootlogo-preview`).
- `branding/README.md` explains how to rebrand, plus the artwork rules (no third-party marks).

## Label text
```
[gear+bolt]   RG34XXSP REDUX
              Allwinner H700
   [chip icon] 2GB DDR4
```
- The mark replaces the Anbernic "R"; the fourth line is the version (`show_version` in boot.ini).

## Constraints
- 720×480, matching the panel; the format the stock boot logo uses (verify: likely a BMP on the
  boot partition, p4. Read the stock file's format and size before replacing it).
- **Original artwork only**: drawn by us, generated from our own description, or from openly licensed
  parts. Record the source and licence in the commit. No vendor photos or logos.
- It's replaced as part of the boot image, like Phase 1. The swap is a device-writing step, so it gets
  its own Dale-only PR with a restore path to the stock logo.

## Order for Grok
The art and the generator are done (`branding/`, `tools/make_bootlogo.py`). Left for Grok, after the
F2 follow-ups (#41) and the STATUS queue:
1. Find and document the stock boot-logo file (path, format, size) in `docs/stock-userspace.md`.
2. Add an output writer for that format to `make_bootlogo.py`, chosen by `[output] format` in boot.ini.
3. A Dale-only PR that installs it on the device, with a restore path to the stock logo.
