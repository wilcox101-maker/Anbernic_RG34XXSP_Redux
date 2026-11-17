#!/usr/bin/env python3
"""Record the handheld's screen to an animated GIF, over SSH, for the README demo.

    python tools/capture_demo.py 192.168.1.13                  # 15 s at 5 frames/s -> redux-demo.gif
    python tools/capture_demo.py 192.168.1.13 --seconds 20 --fps 4 --scale 0.5 --out demo.gif

While it records, use the handheld: menu -> launch a game -> close the lid. It reads the framebuffer
(/dev/fb0) into RAM on the handheld (/dev/shm, or /tmp), sends the frames back in one SSH session
(one password prompt), deletes them on the handheld and writes the GIF on the PC.

Writes nothing on the handheld except its own temporary folder, which it removes. Never touches the
ROM cards. Python 3 standard library only; runs on Windows (needs the ssh command).

Not run on the device yet: the framebuffer layout (32-bit BGRA, panning) is read from
/sys/class/graphics/fb0 at run time; if the colours come out wrong, try --rgb. A game drawn by the
GPU outside the framebuffer may show as black frames.
"""
import argparse
import io
import os
import re
import struct
import subprocess
import sys
import tarfile

MARK = b"\n--REDUX-CAPTURE-TAR--\n"

# Runs on the handheld (busybox sh). $1 frames, $2 delay in seconds.
DEVICE_SCRIPT = r"""
set -u
N=$1; DELAY=$2
D=/dev/shm/redux-capture; [ -d /dev/shm ] || D=/tmp/redux-capture
rm -rf "$D"; mkdir -p "$D" || { echo "ERR cannot create $D"; exit 1; }
F=/sys/class/graphics/fb0
VS=$(cat $F/virtual_size); BPP=$(cat $F/bits_per_pixel); STRIDE=$(cat $F/stride 2>/dev/null || echo 0)
echo "META $VS $BPP $STRIDE"
echo "MODE $(head -n 1 $F/modes 2>/dev/null)"
i=0
while [ $i -lt $N ]; do
  PAN=$(cat $F/pan 2>/dev/null || echo 0,0)
  echo "$PAN" > "$D/$(printf %04d $i).pan"
  cat /dev/fb0 > "$D/$(printf %04d $i).raw"
  i=$((i + 1))
  sleep "$DELAY"
done
printf '\n--REDUX-CAPTURE-TAR--\n'
tar cf - -C "$D" . | gzip -1
rm -rf "$D"
"""


def screen_size(virt_w, virt_h, mode=""):
    """Visible size from the mode line (e.g. 'U:720x480p-59'); without it, a buffer at least twice
    as tall as wide-to-panel ratio is taken as page-flipped and halved."""
    m = re.search(r"(\d+)x(\d+)", mode or "")
    if m:
        return int(m.group(1)), int(m.group(2))
    return virt_w, (virt_h // 2 if virt_h >= 2 * virt_w * 2 // 3 else virt_h)


def to_rgb(raw, width, height, bpp, stride, yoff, swap_rb=False):
    """Visible region of one framebuffer dump as RGB bytes (3 per pixel)."""
    bytes_pp = bpp // 8
    stride = stride or width * bytes_pp
    out = bytearray(width * height * 3)
    o = 0
    for y in range(height):
        row = (yoff + y) * stride
        line = raw[row:row + width * bytes_pp]
        if len(line) < width * bytes_pp:
            line = line + bytes(width * bytes_pp - len(line))
        if bytes_pp == 4:
            b, g, r = line[0::4], line[1::4], line[2::4]
        elif bytes_pp == 2:
            px = struct.unpack("<%dH" % width, line)
            r = bytes(((p >> 11) & 31) * 255 // 31 for p in px)
            g = bytes(((p >> 5) & 63) * 255 // 63 for p in px)
            b = bytes((p & 31) * 255 // 31 for p in px)
        else:
            raise ValueError("unsupported bits per pixel: %d" % bpp)
        if swap_rb:
            r, b = b, r
        out[o:o + width * 3:3] = r
        out[o + 1:o + width * 3:3] = g
        out[o + 2:o + width * 3:3] = b
        o += width * 3
    return bytes(out)


def scale_rgb(rgb, width, height, factor):
    """Nearest-neighbour scale; keeps pixel-art edges sharp."""
    if factor == 1:
        return rgb, width, height
    nw, nh = max(1, int(width * factor)), max(1, int(height * factor))
    out = bytearray(nw * nh * 3)
    xs = [min(width - 1, int(x / factor)) * 3 for x in range(nw)]
    for y in range(nh):
        sy = min(height - 1, int(y / factor)) * width * 3
        o = y * nw * 3
        for x, sx in enumerate(xs):
            out[o + 3 * x:o + 3 * x + 3] = rgb[sy + sx:sy + sx + 3]
    return bytes(out), nw, nh


def quantize(rgb):
    """RGB -> indices into the fixed 3-3-2 palette (256 colours)."""
    r, g, b = rgb[0::3], rgb[1::3], rgb[2::3]
    return bytes((rr & 0xE0) | ((gg >> 3) & 0x1C) | (bb >> 6) for rr, gg, bb in zip(r, g, b))


def palette332():
    pal = bytearray()
    for i in range(256):
        pal += bytes(((i >> 5) * 255 // 7, ((i >> 2) & 7) * 255 // 7, (i & 3) * 255 // 3))
    return bytes(pal)


def lzw_encode(indices, min_code=8):
    """GIF LZW: variable-width codes, packed LSB first, cut into sub-blocks."""
    clear, end = 1 << min_code, (1 << min_code) + 1
    table = {bytes([i]): i for i in range(clear)}
    size, nxt = min_code + 1, end + 1
    bits = nbits = 0
    out = bytearray()

    def emit(code):
        nonlocal bits, nbits
        bits |= code << nbits
        nbits += size
        while nbits >= 8:
            out.append(bits & 0xFF)
            bits >>= 8
            nbits -= 8

    emit(clear)
    w = b""
    for k in indices:
        wk = w + bytes([k])
        if wk in table:
            w = wk
            continue
        emit(table[w])
        if nxt < 4096:
            table[wk] = nxt
            nxt += 1
            if nxt > (1 << size) and size < 12:
                size += 1
        else:
            emit(clear)
            table = {bytes([i]): i for i in range(clear)}
            size, nxt = min_code + 1, end + 1
        w = bytes([k])
    if w:
        emit(table[w])
    emit(end)
    if nbits:
        out.append(bits & 0xFF)
    blocks = bytearray()
    for i in range(0, len(out), 255):
        chunk = out[i:i + 255]
        blocks += bytes([len(chunk)]) + chunk
    return bytes([min_code]) + bytes(blocks) + b"\x00"


def write_gif(frames, width, height, delay_cs):
    """frames: list of index bytes (3-3-2 palette). Loops forever."""
    gif = bytearray(b"GIF89a")
    gif += struct.pack("<HHBBB", width, height, 0xF7, 0, 0) + palette332()
    gif += b"\x21\xFF\x0BNETSCAPE2.0\x03\x01\x00\x00\x00"
    for idx in frames:
        gif += b"\x21\xF9\x04\x04" + struct.pack("<H", delay_cs) + b"\x00\x00"
        gif += b"\x2C" + struct.pack("<HHHHB", 0, 0, width, height, 0)
        gif += lzw_encode(idx)
    gif += b"\x3B"
    return bytes(gif)


def parse_stream(data):
    """Split the SSH output into (meta tuple, {name: bytes})."""
    if MARK not in data:
        tail = data.decode("utf-8", "replace").strip().splitlines()[-3:]
        raise RuntimeError("no frames came back: " + " | ".join(tail))
    head, tar_gz = data.split(MARK, 1)
    meta = None
    mode = ""
    for line in head.decode("utf-8", "replace").splitlines():
        if line.startswith("ERR "):
            raise RuntimeError(line[4:])
        if line.startswith("META "):
            vs, bpp, stride = line.split()[1:4]
            w, h = (int(v) for v in vs.split(","))
            meta = [w, h, int(bpp), int(stride)]
        if line.startswith("MODE "):
            mode = line[5:].strip()
    if meta is None:
        raise RuntimeError("the handheld did not report the framebuffer size")
    meta = tuple(meta + [mode])
    files = {}
    with tarfile.open(fileobj=io.BytesIO(tar_gz), mode="r:gz") as t:
        for m in t.getmembers():
            if m.isfile():
                files[os.path.basename(m.name)] = t.extractfile(m).read()
    return meta, files


def build_gif(meta, files, fps, scale, swap_rb):
    vw, vh, bpp, stride, mode = meta
    w, h = screen_size(vw, vh, mode)
    frames = []
    for name in sorted(n for n in files if n.endswith(".raw")):
        pan = files.get(name[:-4] + ".pan", b"0,0").decode().strip() or "0,0"
        yoff = int(pan.split(",")[1]) if "," in pan else 0
        rgb = to_rgb(files[name], w, h, bpp, stride, yoff, swap_rb)
        rgb, sw, sh = scale_rgb(rgb, w, h, scale)
        frames.append(quantize(rgb))
    if not frames:
        raise RuntimeError("no frames in the capture")
    return write_gif(frames, sw, sh, max(2, round(100 / fps)))


def main(argv=None):
    a = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    a.add_argument("ip")
    a.add_argument("--seconds", type=float, default=15)
    a.add_argument("--fps", type=float, default=5)
    a.add_argument("--scale", type=float, default=1.0, help="0.5 halves the size")
    a.add_argument("--rgb", action="store_true", help="swap red and blue if the colours are wrong")
    a.add_argument("--out", default="redux-demo.gif")
    a = a.parse_args(argv)
    n = max(1, int(a.seconds * a.fps))
    cmd = ["ssh", "-o", "ConnectTimeout=10", "root@" + a.ip, "sh -s %d %.3f" % (n, 1 / a.fps)]
    print("recording %d frames (%.0f s); use the handheld now" % (n, a.seconds))
    try:
        r = subprocess.run(cmd, input=DEVICE_SCRIPT.encode(), stdout=subprocess.PIPE)
    except FileNotFoundError:
        sys.exit("the ssh command was not found (Windows: Settings > Apps > Optional features > OpenSSH Client)")
    if r.returncode != 0:
        sys.exit("ssh failed (exit %d)" % r.returncode)
    meta, files = parse_stream(r.stdout)
    gif = build_gif(meta, files, a.fps, a.scale, a.rgb)
    with open(a.out, "wb") as f:
        f.write(gif)
    print("wrote %s (%d KiB)" % (a.out, len(gif) // 1024))


if __name__ == "__main__":
    main()
