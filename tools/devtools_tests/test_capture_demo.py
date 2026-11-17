"""Tests for tools/capture_demo.py: framebuffer conversion and the GIF writer (no device, no SSH)."""
import gzip
import io
import os
import struct
import sys
import tarfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import capture_demo as cd  # noqa: E402


def lzw_decode(data, min_code):
    """Reference GIF LZW decoder, written independently of the encoder."""
    clear, end = 1 << min_code, (1 << min_code) + 1
    size = min_code + 1
    table = [bytes([i]) for i in range(clear)] + [b"", b""]
    pos, bits, nbits, prev, out = 0, 0, 0, None, bytearray()
    while True:
        while nbits < size:
            bits |= data[pos] << nbits
            pos += 1
            nbits += 8
        code = bits & ((1 << size) - 1)
        bits >>= size
        nbits -= size
        if code == clear:
            table = table[:clear + 2]
            size, prev = min_code + 1, None
            continue
        if code == end:
            return bytes(out)
        if code < len(table):
            entry = table[code]
            if prev is not None:
                table.append(prev + entry[:1])
        else:
            entry = prev + prev[:1]
            table.append(entry)
        out += entry
        prev = entry
        if len(table) == (1 << size) and size < 12:
            size += 1


def frames_from_gif(gif):
    """Decode every image's indices from a GIF written by capture_demo."""
    assert gif[:6] == b"GIF89a"
    w, h, flags = struct.unpack("<HHB", gif[6:11])
    pos = 13 + 3 * (2 << (flags & 7))
    frames = []
    while gif[pos] != 0x3B:
        if gif[pos] == 0x21:
            pos += 2
            while gif[pos]:
                pos += gif[pos] + 1
            pos += 1
        elif gif[pos] == 0x2C:
            pos += 10
            min_code = gif[pos]
            pos += 1
            data = bytearray()
            while gif[pos]:
                data += gif[pos + 1:pos + 1 + gif[pos]]
                pos += gif[pos] + 1
            pos += 1
            frames.append(lzw_decode(bytes(data), min_code))
        else:
            raise AssertionError("bad block %02x" % gif[pos])
    return w, h, frames


class Gif(unittest.TestCase):
    def test_round_trip_small_and_long_runs(self):
        a = bytes(range(256)) * 3 + bytes(1000)            # every colour, then a long run
        b = bytes((i * 7) % 256 for i in range(5000))     # many table entries (forces a reset)
        gif = cd.write_gif([a[:40 * 50], b[:40 * 50]], 40, 50, 20)
        w, h, frames = frames_from_gif(gif)
        self.assertEqual((w, h), (40, 50))
        self.assertEqual(frames, [a[:2000], b[:2000]])

    def test_table_full_reset(self):
        import random
        rnd = random.Random(1)
        data = bytes(rnd.randrange(256) for _ in range(120 * 100))
        _, _, frames = frames_from_gif(cd.write_gif([data], 120, 100, 10))
        self.assertEqual(frames[0], data)


class Framebuffer(unittest.TestCase):
    def test_bgra_visible_half_of_page_flipped_buffer(self):
        w, vh = 4, 6                                    # second page starts at y=3
        raw = bytearray()
        for y in range(vh):
            for x in range(w):
                raw += bytes((x, y, 200, 255))          # B=x, G=y, R=200
        rgb = cd.to_rgb(bytes(raw), w, 3, 32, w * 4, 3)
        self.assertEqual(rgb[:3], bytes((200, 3, 0)))   # R, G=y(3), B=x(0)
        self.assertEqual(cd.to_rgb(bytes(raw), w, 3, 32, w * 4, 0, swap_rb=True)[:3], bytes((0, 0, 200)))

    def test_rgb565(self):
        raw = struct.pack("<2H", 0xF800, 0x001F)        # red, blue
        self.assertEqual(cd.to_rgb(raw, 2, 1, 16, 4, 0), bytes((255, 0, 0, 0, 0, 255)))

    def test_screen_size(self):
        self.assertEqual(cd.screen_size(720, 960, "U:720x480p-59"), (720, 480))
        self.assertEqual(cd.screen_size(720, 960, ""), (720, 480))
        self.assertEqual(cd.screen_size(720, 480, ""), (720, 480))

    def test_quantize_and_scale(self):
        self.assertEqual(cd.quantize(bytes((255, 255, 255, 0, 0, 0))), bytes((255, 0)))
        rgb, w, h = cd.scale_rgb(bytes(range(12)), 2, 2, 0.5)
        self.assertEqual((rgb, w, h), (bytes((0, 1, 2)), 1, 1))


class Stream(unittest.TestCase):
    def make_stream(self, mode="U:2x2p-59"):
        buf = io.BytesIO()
        with tarfile.open(fileobj=buf, mode="w") as t:
            for name, data in (("./0000.raw", bytes(2 * 4 * 4)), ("./0000.pan", b"0,2\n")):
                info = tarfile.TarInfo(name)
                info.size = len(data)
                t.addfile(info, io.BytesIO(data))
        return (b"META 2,4 32 8\nMODE " + mode.encode() + cd.MARK + gzip.compress(buf.getvalue()))

    def test_parse_and_build(self):
        meta, files = cd.parse_stream(self.make_stream())
        self.assertEqual(meta, (2, 4, 32, 8, "U:2x2p-59"))
        self.assertEqual(sorted(files), ["0000.pan", "0000.raw"])
        w, h, frames = frames_from_gif(cd.build_gif(meta, files, 5, 1, False))
        self.assertEqual((w, h, frames), (2, 2, [bytes(4)]))

    def test_device_error_is_reported(self):
        with self.assertRaises(RuntimeError) as e:
            cd.parse_stream(b"ERR cannot create /tmp/redux-capture\n")
        self.assertIn("cannot create", str(e.exception))


if __name__ == "__main__":
    unittest.main()
