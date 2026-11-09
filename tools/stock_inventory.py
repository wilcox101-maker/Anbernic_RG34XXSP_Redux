#!/usr/bin/env python3
"""Inventory the stock RG34XXSP.img without mounting anything.

  python3 stock_inventory.py RG34XXSP.img [-o stock-inventory]

Read-only: the image is opened "rb" and never written. Pure standard library,
so it runs the same on Windows, WSL and Linux (Windows can't mount ext4).

What it produces in the output folder:
  bootloader-36MiB.bin   raw copy of the first 36 MiB (boot0, u-boot/TOC1, GPT)
  inventory.zip          everything below, in one file small enough to hand over:
    summary.txt          GPT table, ext4 superblock facts, SHA-256 of the 36 MiB cut
    list-<part>.tsv      every file: path, type, mode, uid, gid, size, link target
    modules.txt          every *.ko and every module-load config found
    hits.tsv             files (text OR binary) that mention CPU governor / cpufreq,
                         module loading, or power-key / lid / suspend handling,
                         with the printable strings around each hit
    files/<part>/...     copies of small text files: configs, scripts, units,
                         udev rules, and anything that produced a hit

Partitions read: rootfs, appfs, UDISK. The Roms partition is never read:
its name is matched and skipped before any of its sectors are touched.
"""
import argparse
import hashlib
import io
import os
import re
import stat
import struct
import sys
import zipfile

SECTOR = 512
BOOTLOADER_BYTES = 36 * 1024 * 1024
WANT = ("rootfs", "appfs", "UDISK")
TEXT_MAX = 512 * 1024          # copy text files up to this size
SCAN_MAX = 64 * 1024 * 1024    # keyword-scan files up to this size

# Directories under which every small text file is copied (configs, init, scripts).
COPY_DIRS = ("etc/", "lib/systemd/", "usr/lib/systemd/", "lib/udev/", "usr/lib/udev/",
             "usr/local/", "root/", "home/", "opt/", "emuelec/", "mnt/", "userdata/",
             "lib/modules-load.d/", "usr/lib/modules-load.d/", "var/spool/cron/")
COPY_EXT = (".sh", ".service", ".target", ".rules", ".conf", ".py", ".ini", ".cfg",
            ".json", ".txt", ".timer", ".path", ".socket", ".desktop")
# Noise that never matters here and costs time.
SKIP_SCAN = ("usr/share/doc/", "usr/share/man/", "usr/share/locale/", "usr/include/",
             "usr/src/", "usr/share/i18n/", "usr/share/zoneinfo/", "var/lib/apt/lists/",
             "usr/share/info/")

KEYWORDS = {
    "governor": [rb"scaling_governor", rb"scaling_(?:max|min|set)_?(?:freq|speed)",
                 rb"cpufreq", rb"\bperformance\b(?=.{0,40}governor)", rb"schedutil",
                 rb"ondemand", rb"userspace(?=.{0,40}governor)"],
    "modules":  [rb"\binsmod\b", rb"\bmodprobe\b", rb"\brmmod\b", rb"/lib/modules/",
                 rb"[\w\-]+\.ko\b"],
    "power":    [rb"KEY_POWER", rb"SW_LID", rb"HandleLidSwitch", rb"HandlePowerKey",
                 rb"/sys/power/state", rb"mem_sleep", rb"systemctl (?:suspend|poweroff)",
                 rb"\bpoweroff\b", rb"\bsuspend\b", rb"\bhall\b", rb"\blid\b",
                 rb"axp2202|axp22|axp-?pek|power[_-]?key|powerkey|pwrkey",
                 rb"gpio[-_]keys", rb"/dev/input/(?:event\d|by-)", rb"bl_power|fb0/blank"],
}
KW_RE = {k: re.compile(b"|".join(v), re.I) for k, v in KEYWORDS.items()}
PRINTABLE = re.compile(rb"[\x20-\x7e\t]{4,}")
STRINGS = re.compile(rb"[\x20-\x7e\t]{6,}")


# ---------------------------------------------------------------- GPT

def read_gpt(f):
    f.seek(SECTOR)
    hdr = f.read(92)
    if hdr[:8] != b"EFI PART":
        sys.exit("no GPT header at LBA 1; is this RG34XXSP.img?")
    entries_lba, n, esz = struct.unpack_from("<QII", hdr, 72)
    f.seek(entries_lba * SECTOR)
    table = f.read(n * esz)
    parts = []
    for k in range(n):
        e = table[k * esz:(k + 1) * esz]
        if e[:16] == b"\0" * 16:
            continue
        first, last = struct.unpack_from("<QQ", e, 32)
        name = e[56:128].decode("utf-16-le").rstrip("\0")
        parts.append({"num": k + 1, "name": name, "off": first * SECTOR,
                      "size": (last - first + 1) * SECTOR})
    return parts


# ---------------------------------------------------------------- ext4

class Ext4:
    def __init__(self, f, off):
        self.f, self.off = f, off
        sb = self._read(1024, 1024)
        if struct.unpack_from("<H", sb, 0x38)[0] != 0xEF53:
            raise ValueError("no ext2/3/4 superblock")
        self.inodes_count, = struct.unpack_from("<I", sb, 0x00)
        self.first_data_block, log_bs = struct.unpack_from("<II", sb, 0x14)
        self.bs = 1024 << log_bs
        self.bpg, _, self.ipg = struct.unpack_from("<III", sb, 0x20)
        self.inode_size, = struct.unpack_from("<H", sb, 0x58)
        self.compat, self.incompat, self.ro_compat = struct.unpack_from("<III", sb, 0x5C)
        self.is64 = bool(self.incompat & 0x80)
        self.desc_size = struct.unpack_from("<H", sb, 0xFE)[0] if self.is64 else 32
        self.desc_size = self.desc_size or 32
        self.label = sb[0x78:0x88].rstrip(b"\0").decode(errors="replace")
        blo, = struct.unpack_from("<I", sb, 0x04)
        bhi = struct.unpack_from("<I", sb, 0x150)[0] if self.is64 else 0
        self.blocks = blo | bhi << 32
        free_lo, = struct.unpack_from("<I", sb, 0x0C)
        free_hi = struct.unpack_from("<I", sb, 0x158)[0] if self.is64 else 0
        self.free_blocks = free_lo | free_hi << 32
        self.free_inodes, = struct.unpack_from("<I", sb, 0x10)
        self.mount_opts = sb[0x200:0x240].rstrip(b"\0").decode(errors="replace")
        self.last_mounted = sb[0x88:0xC8].rstrip(b"\0").decode(errors="replace")
        self.state, = struct.unpack_from("<H", sb, 0x3A)
        self._gd_cache = {}

    def _read(self, pos, n):
        self.f.seek(self.off + pos)
        return self.f.read(n)

    def _inode_table(self, group):
        if group not in self._gd_cache:
            gdt = (self.first_data_block + 1) * self.bs
            d = self._read(gdt + group * self.desc_size, self.desc_size)
            lo, = struct.unpack_from("<I", d, 0x8)
            hi = struct.unpack_from("<I", d, 0x28)[0] if self.desc_size >= 64 else 0
            self._gd_cache[group] = lo | hi << 32
        return self._gd_cache[group]

    def inode(self, n):
        g, i = divmod(n - 1, self.ipg)
        raw = self._read(self._inode_table(g) * self.bs + i * self.inode_size, 160)
        mode, uid, size_lo = struct.unpack_from("<HHI", raw, 0)
        gid, = struct.unpack_from("<H", raw, 0x18)
        flags, = struct.unpack_from("<I", raw, 0x20)
        size_hi, = struct.unpack_from("<I", raw, 0x6C)
        uid_hi, gid_hi = struct.unpack_from("<HH", raw, 0x78)
        blocks_lo, = struct.unpack_from("<I", raw, 0x1C)
        return {"ino": n, "mode": mode, "uid": uid | uid_hi << 16, "gid": gid | gid_hi << 16,
                "size": size_lo | size_hi << 32, "flags": flags,
                "iblock": raw[0x28:0x28 + 60], "blocks": blocks_lo}

    def _extent_runs(self, node):
        magic, entries, _, depth = struct.unpack_from("<HHHH", node, 0)
        if magic != 0xF30A:
            raise ValueError("bad extent header")
        for k in range(entries):
            e = node[12 + 12 * k:24 + 12 * k]
            if depth == 0:
                lblk, ln, hi, lo = struct.unpack("<IHHI", e)
                uninit = ln > 32768
                if uninit:
                    ln -= 32768
                yield lblk, (hi << 32 | lo), ln, uninit
            else:
                _, lo, hi = struct.unpack_from("<IIH", e)
                yield from self._extent_runs(self._read((hi << 32 | lo) * self.bs, self.bs))

    def _indirect_runs(self, iblock):
        ptrs = struct.unpack("<15I", iblock)
        per = self.bs // 4
        lblk = 0

        def walk(blk, level):
            nonlocal lblk
            if level == 0:
                if blk:
                    yield lblk, blk, 1, False
                lblk += 1
                return
            span = per ** level
            if not blk:
                lblk += span
                return
            for p in struct.unpack(f"<{per}I", self._read(blk * self.bs, self.bs)):
                yield from walk(p, level - 1)

        for p in ptrs[:12]:
            yield from walk(p, 0)
        for lvl, p in ((1, ptrs[12]), (2, ptrs[13]), (3, ptrs[14])):
            yield from walk(p, lvl)

    def read(self, ino, limit=None):
        size = ino["size"] if limit is None else min(ino["size"], limit)
        if ino["flags"] & 0x10000000:          # inline data (first 60 bytes only)
            return ino["iblock"][:size]
        out = bytearray(size)
        runs = (self._extent_runs(ino["iblock"]) if ino["flags"] & 0x80000
                else self._indirect_runs(ino["iblock"]))
        for lblk, pblk, ln, uninit in runs:
            start = lblk * self.bs
            if start >= size:
                if ino["flags"] & 0x80000:
                    continue
                break
            n = min(ln * self.bs, size - start)
            if not uninit:
                out[start:start + n] = self._read(pblk * self.bs, n)
        return bytes(out)

    def symlink_target(self, ino):
        if ino["size"] < 60 and not ino["flags"] & 0x80000 and not ino["flags"] & 0x10000000:
            return ino["iblock"][:ino["size"]].decode(errors="replace")
        return self.read(ino).decode(errors="replace")

    def listdir(self, ino):
        data = self.read(ino)
        if ino["flags"] & 0x10000000:          # inline dir: 4-byte parent, then dirents
            data = data[4:]
        i = 0
        while i + 8 <= len(data):
            n, rec, nl, ft = struct.unpack_from("<IHBB", data, i)
            if rec < 8:
                break
            if n:
                name = data[i + 8:i + 8 + nl].decode("utf-8", "surrogateescape")
                if name not in (".", ".."):
                    yield name, n
            i += rec

    def walk(self):
        stack = [("", 2)]
        seen = set()
        while stack:
            path, n = stack.pop()
            if n in seen:
                continue
            seen.add(n)
            d = self.inode(n)
            for name, child in sorted(self.listdir(d)):
                ci = self.inode(child)
                p = path + name
                yield p, ci
                if stat.S_ISDIR(ci["mode"]):
                    stack.append((p + "/", child))


# ---------------------------------------------------------------- inventory

def ftype(mode):
    for test, ch in ((stat.S_ISDIR, "d"), (stat.S_ISREG, "f"), (stat.S_ISLNK, "l"),
                     (stat.S_ISCHR, "c"), (stat.S_ISBLK, "b"), (stat.S_ISFIFO, "p"),
                     (stat.S_ISSOCK, "s")):
        if test(mode):
            return ch
    return "?"


def is_text(b):
    return b"\0" not in b[:8192]


def contexts(data, rx, limit=4):
    out = []
    for s in PRINTABLE.finditer(data):
        if rx.search(s.group()):
            t = s.group().decode("ascii", "replace").strip()
            out.append(t[:200])
            if len(out) >= limit:
                break
    return out


def inventory_part(fs, part, zf, hits, modules):
    rows = ["path\ttype\tmode\tuid\tgid\tsize\ttarget"]
    nfiles = copied = 0
    for path, ino in fs.walk():
        t = ftype(ino["mode"])
        target = ""
        if t == "l":
            try:
                target = fs.symlink_target(ino)
            except Exception as e:
                target = f"<unreadable: {e}>"
        rows.append(f"/{path}\t{t}\t{stat.S_IMODE(ino['mode']):04o}\t{ino['uid']}\t"
                    f"{ino['gid']}\t{ino['size']}\t{target}")
        if path.endswith(".ko") or "modules-load.d/" in path or path in (
                "etc/modules", "etc/modprobe.conf") or path.startswith("etc/modprobe.d/"):
            modules.append(f"{part}\t/{path}\t{t}\t{ino['size']}")
        if t != "f":
            continue
        nfiles += 1
        if ino["size"] > SCAN_MAX or path.startswith(SKIP_SCAN):
            continue
        try:
            data = fs.read(ino)
        except Exception as e:
            print(f"  warn: /{path}: {e}", file=sys.stderr)
            continue
        hit = False
        text = is_text(data)
        # In binaries, only look inside real strings (>= 6 printable chars) so
        # random bytes like "lId" in compressed data don't count as hits.
        scan = data if text else b"\n".join(STRINGS.findall(data))
        for cat, rx in KW_RE.items():
            m = rx.findall(scan)
            if m:
                hit = True
                hits.append(f"{part}\t/{path}\t{cat}\t{len(m)}\t"
                            f"{'text' if text else 'binary'}\t"
                            + " | ".join(contexts(scan, rx)).replace("\t", " "))
        want = text and ino["size"] <= TEXT_MAX and (
            hit or path.startswith(COPY_DIRS) or path.endswith(COPY_EXT)
            or part != "rootfs")
        if want:
            zf.writestr(f"files/{part}/{path}", data)
            copied += 1
    zf.writestr(f"list-{part}.tsv", "\n".join(rows) + "\n")
    return nfiles, copied


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("image", help="stock RG34XXSP.img (opened read-only)")
    ap.add_argument("-o", "--out", default="stock-inventory", help="output folder")
    a = ap.parse_args()
    os.makedirs(a.out, exist_ok=True)

    with open(a.image, "rb") as f:
        summary = [f"image: {os.path.basename(a.image)}  size: {os.path.getsize(a.image)} bytes", ""]

        # 1. bootloader region
        blob = f.read(BOOTLOADER_BYTES)
        if len(blob) != BOOTLOADER_BYTES:
            sys.exit("image is shorter than 36 MiB")
        bl_path = os.path.join(a.out, "bootloader-36MiB.bin")
        with open(bl_path, "wb") as o:
            o.write(blob)
        summary.append(f"bootloader-36MiB.bin sha256 {hashlib.sha256(blob).hexdigest()}")
        summary.append(f"boot0 magic at 8 KiB+4: {blob[8192 + 4:8192 + 12]!r}")
        del blob

        # 2. GPT
        parts = read_gpt(f)
        summary += ["", "GPT:", "num  name             start_MiB      size_MiB"]
        for p in parts:
            summary.append(f"p{p['num']:<3} {p['name']:<16} {p['off'] / 2**20:>10.2f} "
                           f"{p['size'] / 2**20:>13.2f}")
        first = min(p["off"] for p in parts)
        if first < BOOTLOADER_BYTES:
            summary.append(f"WARNING: a partition starts at {first}, inside the 36 MiB cut")
        print("\n".join(summary))

        # 3. ext4 partitions
        zpath = os.path.join(a.out, "inventory.zip")
        hits = ["part\tpath\tcategory\tcount\tkind\tcontext"]
        modules = ["part\tpath\ttype\tsize"]
        with zipfile.ZipFile(zpath, "w", zipfile.ZIP_DEFLATED) as zf:
            for p in parts:
                if "rom" in p["name"].lower():
                    summary.append(f"\np{p['num']} {p['name']}: skipped (never read)")
                    continue
                if p["name"] not in WANT:
                    continue
                print(f"\nreading p{p['num']} {p['name']} ...", flush=True)
                fs = Ext4(f, p["off"])
                feats = f"compat={fs.compat:#x} incompat={fs.incompat:#x} ro_compat={fs.ro_compat:#x}"
                used = (fs.blocks - fs.free_blocks) * fs.bs
                summary += ["", f"p{p['num']} {p['name']}: label={fs.label!r} block={fs.bs} "
                            f"blocks={fs.blocks} used={used / 2**20:.1f} MiB "
                            f"inodes={fs.inodes_count - fs.free_inodes}/{fs.inodes_count}",
                            f"   {feats} state={fs.state:#x} last_mounted={fs.last_mounted!r} "
                            f"default_mount_opts={fs.mount_opts!r}"]
                n, c = inventory_part(fs, p["name"], zf, hits, modules)
                summary.append(f"   regular files: {n}, text files copied: {c}")
                print(f"  {n} files, {c} copied")
            zf.writestr("hits.tsv", "\n".join(hits) + "\n")
            zf.writestr("modules.txt", "\n".join(modules) + "\n")
            zf.writestr("summary.txt", "\n".join(summary) + "\n")
    print(f"\ndone: {bl_path}\n      {zpath} ({os.path.getsize(zpath) / 2**20:.1f} MiB)")


if __name__ == "__main__":
    main()
