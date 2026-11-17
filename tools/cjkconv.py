#!/usr/bin/env python3
"""Build frontend/term/cjk.bin, the 16x16 CJK glyphs Redux Terminal draws, from GNU Unifont.

Python 3 standard library only; runs on Windows. The source is not kept in the repo: it is
unifont.hex from the Debian/Ubuntu package `unifont` (/usr/share/unifont/unifont.hex) or from
unifoundry.com (GPL-2.0-or-later with the font embedding exception, also OFL-1.1; see
THIRD-PARTY-NOTICES.md). Then:

    python tools/cjkconv.py /usr/share/unifont/unifont.hex frontend/term/cjk.bin

Blob layout (little-endian): magic "RDXCJK1\\0", u32 range count, then per range u32 first code
point and u32 count, then the bitmaps in range order: 32 bytes per code point, two bytes per row,
MSB = leftmost pixel. A code point Unifont lacks (or has as 8 pixels wide) is all zero: the
terminal shows "?" for it. The ranges cover what Chinese, Japanese and the fullwidth forms need;
Hangul and the CJK Extension A block can be added here later (about 0.35 MB and 0.2 MB).
"""
import struct
import sys

RANGES = [
    (0x3000, 0x30FF),   # CJK punctuation, hiragana, katakana
    (0x4E00, 0x9FFF),   # CJK Unified Ideographs
    (0xFF00, 0xFF60),   # fullwidth forms
]
MAGIC = b"RDXCJK1\0"


def read_hex(path):
    glyphs = {}
    with open(path, encoding="ascii", errors="replace") as f:
        for line in f:
            line = line.strip()
            if ":" not in line:
                continue
            code, bits = line.split(":", 1)
            try:
                cp = int(code, 16)
            except ValueError:
                continue
            if len(bits) == 64:
                glyphs[cp] = bytes.fromhex(bits)
    return glyphs


def build(glyphs):
    out = bytearray(MAGIC)
    out += struct.pack("<I", len(RANGES))
    for first, last in RANGES:
        out += struct.pack("<II", first, last - first + 1)
    missing = 0
    for first, last in RANGES:
        for cp in range(first, last + 1):
            g = glyphs.get(cp)
            if g is None:
                missing += 1
                g = bytes(32)
            out += g
    return bytes(out), missing


def main(argv):
    if len(argv) != 3:
        print(__doc__)
        return 2
    glyphs = read_hex(argv[1])
    blob, missing = build(glyphs)
    with open(argv[2], "wb") as f:
        f.write(blob)
    total = sum(last - first + 1 for first, last in RANGES)
    print("cjkconv: %d code points, %d without a 16-wide glyph, %d bytes -> %s" % (total, missing, len(blob), argv[2]))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
