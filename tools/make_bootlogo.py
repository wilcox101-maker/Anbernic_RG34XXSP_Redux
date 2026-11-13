#!/usr/bin/env python3
"""Render the RG34XXSP Redux boot screen from branding/boot.ini and branding/mark.pbm.

    python tools/make_bootlogo.py [--ini branding/boot.ini] [--version 0.2] [--out file.bmp]
    python tools/make_bootlogo.py --check file.bmp     # is it a valid stock-format boot logo?

Python 3 standard library only; runs on Windows. Writes a 24-bit BMP. Only the INI, the mark
and the font decide what it looks like, so a rebuilder rebrands by editing branding/, not code.
With [output] format = stock-bootlogo the file is exactly the stock boot logo's format, read from
the stock image (boot-resource partition, bootlogo.bmp): 640x480, 24-bit, uncompressed,
bottom-up rows, 54-byte header, 921654 bytes.
"""
import argparse, configparser, os, re, struct, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

def rgb(s):
    s = s.strip().lstrip("#")
    if not re.fullmatch(r"[0-9a-fA-F]{6}", s):
        sys.exit("colour must be RRGGBB, got %r" % s)
    return (int(s[0:2], 16), int(s[2:4], 16), int(s[4:6], 16))

def load_font():
    """The kernel VGA 8x16 font already in the frontend: 256 glyphs x 16 rows."""
    src = open(os.path.join(ROOT, "frontend", "src", "font.c"), encoding="utf-8").read()
    src = re.sub(r"/\*.*?\*/", "", src, flags=re.S)          # comments hold hex too
    vals = [int(h, 16) for h in re.findall(r"0x([0-9a-fA-F]{2})\b", src)]
    if len(vals) < 4096:
        sys.exit("font.c: expected 4096 bytes, found %d" % len(vals))
    return vals[:4096]

def load_pbm(path):
    data = open(path, "rb").read()
    toks, i = [], 0
    def token():
        nonlocal i
        while i < len(data):
            c = data[i:i + 1]
            if c == b"#":
                while i < len(data) and data[i:i + 1] not in b"\r\n": i += 1
            elif c.isspace(): i += 1
            else: break
        j = i
        while i < len(data) and not data[i:i + 1].isspace(): i += 1
        return data[j:i]
    magic = token()
    w, h = int(token()), int(token())
    if magic == b"P1":
        bits = []
        while len(bits) < w * h:
            t = token()
            if not t: break
            bits.extend(int(ch) for ch in t.decode() if ch in "01")
    elif magic == b"P4":
        i += 1
        row = (w + 7) // 8
        bits = [(data[i + y * row + x // 8] >> (7 - x % 8)) & 1 for y in range(h) for x in range(w)]
    else:
        sys.exit("%s: only PBM (P1/P4) marks are supported" % path)
    if len(bits) < w * h:
        sys.exit("%s: truncated" % path)
    return w, h, bits

class Canvas:
    def __init__(self, w, h, bg):
        self.w, self.h = w, h
        self.px = [list(bg) for _ in range(w * h)]
    def set(self, x, y, c):
        if 0 <= x < self.w and 0 <= y < self.h: self.px[y * self.w + x] = list(c)
    def rect(self, x0, y0, w, h, c, r=0):
        for y in range(y0, y0 + h):
            for x in range(x0, x0 + w):
                if r:  # rounded corners
                    cx = min(max(x, x0 + r), x0 + w - 1 - r); cy = min(max(y, y0 + r), y0 + h - 1 - r)
                    if (x - cx) ** 2 + (y - cy) ** 2 > r * r: continue
                self.set(x, y, c)
    def bitmap(self, x0, y0, w, h, bits, c, scale=1):
        for y in range(h):
            for x in range(w):
                if bits[y * w + x]:
                    for dy in range(scale):
                        for dx in range(scale): self.set(x0 + x * scale + dx, y0 + y * scale + dy, c)
    def text(self, x0, y0, s, font, c, scale):
        for k, ch in enumerate(s.encode("cp437", "replace")):
            g = font[ch * 16:(ch + 1) * 16]
            for y in range(16):
                for x in range(8):
                    if g[y] & (0x80 >> x):
                        for dy in range(scale):
                            for dx in range(scale):
                                self.set(x0 + (k * 8 + x) * scale + dx, y0 + y * scale + dy, c)
    def save_bmp(self, path):
        os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
        row = (self.w * 3 + 3) & ~3
        with open(path, "wb") as f:
            f.write(b"BM" + struct.pack("<IHHI", 54 + row * self.h, 0, 0, 54))
            f.write(struct.pack("<IiiHHIIiiII", 40, self.w, self.h, 1, 24, 0, row * self.h, 2835, 2835, 0, 0))
            pad = b"\0" * (row - self.w * 3)
            for y in range(self.h - 1, -1, -1):
                f.write(b"".join(bytes((b, g, r)) for r, g, b in self.px[y * self.w:(y + 1) * self.w]) + pad)

STOCK_W, STOCK_H = 640, 480
STOCK_SIZE = 54 + STOCK_W * 3 * STOCK_H   # 921654; 640*3 is already a multiple of 4

def check_stock(data):
    """Return None if data is a stock-format boot logo, else the reason it isn't."""
    if len(data) != STOCK_SIZE: return "size %d, want %d" % (len(data), STOCK_SIZE)
    if data[:2] != b"BM": return "not a BMP"
    fsize, _, _, off = struct.unpack_from("<IHHI", data, 2)
    hdr, w, h, planes, bpp, comp = struct.unpack_from("<IiiHHI", data, 14)
    if fsize != STOCK_SIZE or off != 54 or hdr != 40: return "header layout differs from stock"
    if (w, h) != (STOCK_W, STOCK_H): return "%dx%d, want %dx%d (positive height = bottom-up)" % (w, h, STOCK_W, STOCK_H)
    if planes != 1 or bpp != 24 or comp != 0: return "want 24-bit uncompressed"
    return None

def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--ini", default=os.path.join(ROOT, "branding", "boot.ini"))
    ap.add_argument("--version")
    ap.add_argument("--out")
    ap.add_argument("--check", metavar="BMP", help="only check that BMP is a valid stock-format boot logo")
    a = ap.parse_args()
    if a.check:
        why = check_stock(open(a.check, "rb").read())
        print("%s: %s" % (a.check, "ok, stock boot-logo format" if why is None else "NOT stock format: " + why))
        sys.exit(0 if why is None else 1)
    ini = configparser.ConfigParser(inline_comment_prefixes=(";", "#"))
    if not ini.read(a.ini, encoding="utf-8"):
        sys.exit("cannot read %s" % a.ini)
    base = os.path.dirname(os.path.abspath(a.ini))
    W, H = ini.getint("screen", "width"), ini.getint("screen", "height")
    cv = Canvas(W, H, rgb(ini.get("screen", "background")))
    font = load_font()
    fs = ini.getint("text", "font_scale", fallback=3)
    lines = [ini.get("text", "line%d" % n, fallback="").strip() for n in range(1, 5)]
    lines = [l for l in lines if l]
    if ini.getboolean("text", "show_version", fallback=False):
        lines.append("v" + (a.version or ini.get("text", "version", fallback="dev")))
    mw, mh, mbits = load_pbm(os.path.join(base, ini.get("mark", "file")))
    ms = ini.getint("mark", "scale", fallback=1)

    # Layout: socket > chip > label; mark on the label's left, text lines to its right.
    tw = max(len(l) for l in lines) * 8 * fs if lines else 0
    lw = 24 + mw * ms + 24 + tw + 24
    lh = max(mh * ms, len(lines) * 16 * fs) + 48
    cw, ch = lw + 60, lh + 60
    sw, sh = cw + 120, ch + 120
    if sw > W or sh > H:
        sys.exit("label too big for %dx%d: shorten the text or lower font_scale/mark scale" % (W, H))
    sx, sy = (W - sw) // 2, (H - sh) // 2
    sock, pin = rgb(ini.get("socket", "color")), rgb(ini.get("socket", "pin_color"))
    cv.rect(sx, sy, sw, sh, sock, r=14)
    n = ini.getint("socket", "pins_per_side", fallback=9)
    cx, cy = sx + 60, sy + 60
    for k in range(n):  # contacts on all four sides, leading in to the chip
        px = cx + (k + 1) * cw // (n + 1) - 7; py = cy + (k + 1) * ch // (n + 1) - 7
        cv.rect(px, sy + 14, 14, 50, pin, r=3); cv.rect(px, sy + sh - 64, 14, 50, pin, r=3)
        cv.rect(sx + 14, py, 50, 14, pin, r=3); cv.rect(sx + sw - 64, py, 50, 14, pin, r=3)
    cv.rect(cx, cy, cw, ch, rgb(ini.get("chip", "color")), r=6)
    lx, ly = cx + 30, cy + 30
    cv.rect(lx, ly, lw, lh, rgb(ini.get("label", "color")), r=10)
    ink = rgb(ini.get("label", "text_color"))
    cv.bitmap(lx + 24, ly + (lh - mh * ms) // 2, mw, mh, mbits, ink, ms)
    ty = ly + (lh - len(lines) * 16 * fs) // 2
    for k, l in enumerate(lines):
        cv.text(lx + 24 + mw * ms + 24, ty + k * 16 * fs, l, font, ink, fs)

    out = a.out or os.path.join(ROOT, ini.get("output", "file"))
    fmt = ini.get("output", "format", fallback="bmp").strip()
    if fmt not in ("bmp", "stock-bootlogo"):
        sys.exit("[output] format must be bmp or stock-bootlogo, got %r" % fmt)
    if fmt == "stock-bootlogo" and (W, H) != (STOCK_W, STOCK_H):
        sys.exit("stock-bootlogo needs [screen] %dx%d, got %dx%d" % (STOCK_W, STOCK_H, W, H))
    cv.save_bmp(out)
    if fmt == "stock-bootlogo":
        why = check_stock(open(out, "rb").read())
        if why: sys.exit("%s: not stock format: %s" % (out, why))
    print("wrote %s (%dx%d, %s)" % (out, W, H, fmt))

if __name__ == "__main__":
    main()
