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

## Label text
```
[Redux mark]  RG34XXSP REDUX
              Allwinner H700
   [chip icon] 2GB DDR4
```
- An original logo mark (simple, pixel-style) in place of the Anbernic "R".
- Optional fourth line: the Redux version (`v0.x`), drawn at build time.

## Constraints
- 720×480, matching the panel; the format the stock boot logo uses (verify: likely a BMP on the
  boot partition, p4. Read the stock file's format and size before replacing it).
- **Original artwork only**: drawn by us, generated from our own description, or from openly licensed
  parts. Record the source and licence in the commit. No vendor photos or logos.
- It's replaced as part of the boot image, like Phase 1. The swap is a device-writing step, so it gets
  its own Dale-only PR with a restore path to the stock logo.

## Order for Grok
After the F2 follow-ups (ROM-list items on #41) and the queue in STATUS.md. First step: find and
document the stock boot-logo file (path, format, size) in `docs/hardware-map.md` or
`docs/stock-userspace.md`. Then draft the artwork for Dale's approval before any device PR.
