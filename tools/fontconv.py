#!/usr/bin/env python3
"""Convert the UI fonts of docs/ui-spec.md section 8a into frontend/src/font_extra.c.

Python 3 standard library only; runs on Windows. The font sources are not kept in the repo
(they are large and change rarely). Fetch them into one folder, then run:

    python tools/fontconv.py <source-folder> frontend/src/font_extra.c

Sources (2026-11-14), each converted to 256 CP437 glyphs of 8 x 16 pixels:
  spleen   spleen-8x16.bdf             github.com/fcambus/spleen (and its LICENSE)  BSD-2-Clause
  unifont  Unifont-APL8x16.psf.gz      Debian/Ubuntu package psf-unifont           GPL-2.0+ w/ font exception
  thin     font_7x14.c                 Linux lib/fonts/font_7x14.c                  GPL-2.0
  chunky   font_acorn_8x8.c            Linux lib/fonts/font_acorn_8x8.c             GPL-2.0

A glyph the font doesn't have is written as all zero; the runtime falls back to the VGA glyph
for it (font.c). Codes 0x00-0x1F and 0x7F always fall back: Redux never draws them.
"""
import gzip
import os
import re
import struct
import sys

W, H = 8, 16


def cp437_unicode(code):
    """Unicode code point for a printable CP437 code, or None for codes Redux never draws."""
    if 0x20 <= code <= 0x7E:
        return code
    if code >= 0x80:
        return ord(bytes([code]).decode("cp437"))
    return None


def strip_c_comments(text):
    return re.sub(r"/\*.*?\*/", "", text, flags=re.S)


def kernel_font(path, width, height):
    """Glyph rows from a Linux lib/fonts/*.c file (CP437 order, one byte per row)."""
    text = strip_c_comments(open(path, encoding="latin-1").read())
    m = re.search(r"struct\s+font_data\s+\w+\s*=\s*\{", text)   # the glyph array, not font_desc
    body = text[m.end():text.index("font_desc", m.end())]
    data = [int(x, 16) for x in re.findall(r"0x[0-9a-fA-F]{2}\b", body)]
    if len(data) < 256 * height:
        raise ValueError("%s: %d bytes, expected %d" % (path, len(data), 256 * height))
    assert width <= 8
    return [data[g * height:(g + 1) * height] for g in range(256)]


def thin(path):
    """7x14, centred vertically in the 8x16 cell (one blank row above and below)."""
    src = kernel_font(path, 7, 14)
    out = []
    for code, rows in enumerate(src):
        rows = [0] + rows + [0]
        if 0xB0 <= code <= 0xDF:
            # Line and shade glyphs must reach the cell edges, or box lines break into dashes:
            # repeat the edge rows, and copy the 7th column into the empty 8th.
            rows[0], rows[15] = rows[1], rows[14]
            rows = [r | ((r >> 1) & 1) for r in rows]
        out.append(rows)
    return out


def chunky(path):
    """Acorn 8x8, each row doubled to 8x16."""
    src = kernel_font(path, 8, 8)
    return [[r for r in rows for _ in (0, 1)] for rows in src]


def bdf(path):
    """Unicode-keyed 8x16 glyphs from a BDF file, placed by their bounding boxes."""
    glyphs, ascent = {}, None
    enc = bbx = bitmap = None
    for line in open(path, encoding="latin-1"):
        parts = line.split()
        if not parts:
            continue
        key = parts[0]
        if key == "FONT_ASCENT":
            ascent = int(parts[1])
        elif key == "ENCODING":
            enc = int(parts[1])
        elif key == "BBX":
            bbx = [int(p) for p in parts[1:5]]
        elif key == "BITMAP":
            bitmap = []
        elif key == "ENDCHAR":
            w, h, xo, yo = bbx
            rows = [0] * H
            top = ascent - (yo + h)              # cell row of the glyph's first bitmap row
            for i, hexrow in enumerate(bitmap):
                y = top + i
                if 0 <= y < H:
                    nbytes = max(1, (w + 7) // 8)
                    v = int(hexrow, 16) >> (8 * (nbytes - 1))   # leftmost 8 pixels
                    rows[y] = (v >> xo) & 0xFF if xo >= 0 else (v << -xo) & 0xFF
            glyphs[enc] = rows
            enc = bbx = bitmap = None
        elif bitmap is not None:
            bitmap.append(key)
    if ascent is None:
        raise ValueError("%s: no FONT_ASCENT" % path)
    return glyphs


def psf(path):
    """Unicode-keyed glyphs from a PSF1 or PSF2 console font with a Unicode table (8x16 only)."""
    data = gzip.open(path).read() if path.endswith(".gz") else open(path, "rb").read()
    glyphs = {}
    if data[:2] == b"\x36\x04":
        mode, size = data[2], data[3]
        count = 512 if mode & 1 else 256
        off = 4
        if size != H or not mode & 2:
            raise ValueError("%s: need 8x16 with a Unicode table" % path)
        table = data[off + count * size:]
        pos = 0
        for g in range(count):
            in_seq = False
            while True:
                (u,) = struct.unpack_from("<H", table, pos)
                pos += 2
                if u == 0xFFFF:
                    break
                if u == 0xFFFE:
                    in_seq = True
                if not in_seq and u != 0xFFFE:
                    glyphs.setdefault(u, list(data[off + g * size: off + (g + 1) * size]))
        return glyphs
    raise ValueError("%s: not a PSF1 font" % path)


def by_cp437(uni_glyphs):
    out = []
    for code in range(256):
        u = cp437_unicode(code)
        out.append(uni_glyphs.get(u, [0] * H) if u is not None else [0] * H)
    return out


def blank_controls(table):
    return [([0] * H if (c < 0x20 or c == 0x7F) else rows) for c, rows in enumerate(table)]


HEADERS = {
    "spleen": """/* Spleen 8x16, (c) 2018-2026 Frederic Cambus. BSD-2-Clause:
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met: 1. Redistributions of source code must retain
 * the above copyright notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided
 * with the distribution. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES ... ARE DISCLAIMED (full text: frontend/README.md,
 * LICENSES). Source: github.com/fcambus/spleen, spleen-8x16.bdf. */""",
    "unifont": """/* GNU Unifont 8x16 (psf-unifont, Unifont-APL8x16), (c) 1998-2023 Roman Czyborra, Paul Hardy
 * and contributors. GPL-2.0-or-later with the GNU font embedding exception (also SIL OFL-1.1).
 * Source: unifoundry.com/unifont, via the Debian/Ubuntu psf-unifont package. */""",
    "thin": """/* Linux kernel lib/fonts/font_7x14.c, GPL-2.0, centred in the 8x16 cell; line and shade
 * glyphs (0xB0-0xDF) stretched to the cell edges so boxes join. */""",
    "chunky": """/* Linux kernel lib/fonts/font_acorn_8x8.c (Acorn 8x8), GPL-2.0, rows doubled to 8x16. */""",
}


def emit(out_path, fonts):
    lines = [
        "// SPDX-License-Identifier: GPL-2.0",
        "/* GENERATED by tools/fontconv.py from the font sources named below; do not edit by hand.",
        " * Each table: 256 CP437 glyphs, 16 bytes each, one byte per row, MSB = leftmost pixel.",
        " * An all-zero glyph for a code above 0x20 means \"not in this font\": font.c uses VGA. */",
        "#include <stdint.h>",
        "#include \"font.h\"",
        "",
    ]
    for name, table in fonts:
        lines.append(HEADERS[name])
        lines.append("const uint8_t font_%s_8x16[256][16] = {" % name)
        for code, rows in enumerate(table):
            lines.append("    { %s }, /* 0x%02X */" % (", ".join("0x%02x" % r for r in rows), code))
        lines.append("};")
        lines.append("")
    with open(out_path, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines))


def main(argv):
    if len(argv) != 3:
        print(__doc__)
        return 2
    src, out = argv[1], argv[2]
    p = lambda n: os.path.join(src, n)
    fonts = [
        ("spleen", blank_controls(by_cp437(bdf(p("spleen-8x16.bdf"))))),
        ("unifont", blank_controls(by_cp437(psf(p("Unifont-APL8x16.psf.gz"))))),
        ("thin", blank_controls(thin(p("font_7x14.c")))),
        ("chunky", blank_controls(chunky(p("font_acorn_8x8.c")))),
    ]
    for name, table in fonts:
        have = sum(1 for c in range(0x21, 0x100) if c != 0x7F and any(table[c]))
        print("%-8s %3d of 222 drawable glyphs (0x21-0xFF, not 0x7F)" % (name, have))
    emit(out, fonts)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
