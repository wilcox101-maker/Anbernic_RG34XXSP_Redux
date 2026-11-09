#!/usr/bin/env python3
"""Unpack / repack Android boot images (header v0-v2), as used by the RG34XXSP stock firmware.

The stock 3.boot.img is header v2: kernel + gzip initramfs + device tree blob,
page size 2048, padded out to the 64 MiB partition size.

  python3 bootimg.py unpack boot.img outdir/
  python3 bootimg.py pack   outdir/  new-boot.img [--pad-to 67108864]

`unpack` writes kernel, ramdisk, second, recovery_dtbo, dtb (whichever exist)
plus header.json with every header field. `pack` rebuilds the image from the
same directory, recomputing sizes and the SHA-1 id, so you can swap in a new
dtb or ramdisk and repack. Pure standard library, no dependencies.
"""
import argparse
import hashlib
import json
import os
import struct
import sys

MAGIC = b"ANDROID!"
# v0 header: magic, kernel_size, kernel_addr, ramdisk_size, ramdisk_addr,
# second_size, second_addr, tags_addr, page_size, header_version, os_version,
# name[16], cmdline[512], id[32], extra_cmdline[1024]
V0_FMT = "<8s10I16s512s32s1024s"
V0_SIZE = struct.calcsize(V0_FMT)  # 1632
V1_FMT = "<IQI"   # recovery_dtbo_size, recovery_dtbo_offset, header_size
V2_FMT = "<IQ"    # dtb_size, dtb_addr
BLOBS = ["kernel", "ramdisk", "second", "recovery_dtbo", "dtb"]


def pages(n, ps):
    return (n + ps - 1) // ps


def unpack(img_path, out):
    data = open(img_path, "rb").read()
    if data[:8] != MAGIC:
        sys.exit(f"{img_path}: not an Android boot image (bad magic)")
    f = struct.unpack_from(V0_FMT, data, 0)
    hdr = dict(zip(
        ["magic", "kernel_size", "kernel_addr", "ramdisk_size", "ramdisk_addr",
         "second_size", "second_addr", "tags_addr", "page_size", "header_version",
         "os_version", "name", "cmdline", "id", "extra_cmdline"], f))
    hv = hdr["header_version"]
    if hv > 2:
        sys.exit(f"header v{hv} not supported (this tool handles v0-v2)")
    if hv >= 1:
        hdr["recovery_dtbo_size"], hdr["recovery_dtbo_offset"], hdr["header_size"] = \
            struct.unpack_from(V1_FMT, data, V0_SIZE)
    if hv >= 2:
        hdr["dtb_size"], hdr["dtb_addr"] = struct.unpack_from(V2_FMT, data, V0_SIZE + 16)

    ps = hdr["page_size"]
    os.makedirs(out, exist_ok=True)
    off = ps  # header occupies the first page
    for name in BLOBS:
        size = hdr.get(f"{name}_size", 0)
        if size:
            open(os.path.join(out, name), "wb").write(data[off:off + size])
        off += pages(size, ps) * ps

    blob_sizes = {f"{b}_size" for b in BLOBS}  # recomputed on pack
    meta = {k: v for k, v in hdr.items() if k not in blob_sizes and k != "magic"}
    for k in ("name", "cmdline", "extra_cmdline"):
        meta[k] = meta[k].rstrip(b"\0").decode("ascii", "replace")
    meta["id"] = meta["id"].hex()
    meta["image_size"] = len(data)
    json.dump(meta, open(os.path.join(out, "header.json"), "w"), indent=2)
    print(f"header v{hv}, page {ps}, image {len(data)} bytes")
    for name in BLOBS:
        p = os.path.join(out, name)
        if os.path.exists(p):
            print(f"  {name:14} {os.path.getsize(p):>10} bytes")


def pack(src, img_path, pad_to=None):
    meta = json.load(open(os.path.join(src, "header.json")))
    hv, ps = meta["header_version"], meta["page_size"]
    blobs = {}
    for name in BLOBS:
        p = os.path.join(src, name)
        blobs[name] = open(p, "rb").read() if os.path.exists(p) else b""
    if hv < 2 and blobs["dtb"]:
        sys.exit("dtb present but header_version < 2")

    sha = hashlib.sha1()
    order = ["kernel", "ramdisk", "second"] + (["recovery_dtbo"] if hv >= 1 else []) + \
            (["dtb"] if hv >= 2 else [])
    for name in order:
        sha.update(blobs[name])
        sha.update(struct.pack("<I", len(blobs[name])))
    digest = sha.digest().ljust(32, b"\0")

    hdr = struct.pack(
        V0_FMT, MAGIC,
        len(blobs["kernel"]), meta["kernel_addr"],
        len(blobs["ramdisk"]), meta["ramdisk_addr"],
        len(blobs["second"]), meta["second_addr"],
        meta["tags_addr"], ps, hv, meta["os_version"],
        meta["name"].encode(), meta["cmdline"].encode(), digest,
        meta["extra_cmdline"].encode())
    if hv >= 1:
        hdr += struct.pack(V1_FMT, len(blobs["recovery_dtbo"]),
                           meta.get("recovery_dtbo_offset", 0), meta.get("header_size", 0))
    if hv >= 2:
        hdr += struct.pack(V2_FMT, len(blobs["dtb"]), meta["dtb_addr"])

    out = bytearray(hdr.ljust(ps, b"\0"))
    for name in BLOBS:
        b = blobs[name]
        out += b + b"\0" * (pages(len(b), ps) * ps - len(b))
    if pad_to:
        if len(out) > pad_to:
            sys.exit(f"image is {len(out)} bytes, larger than --pad-to {pad_to}")
        # Stock images fill the unused partition tail with 0xFF (flash-erased style)
        out += b"\xff" * (pad_to - len(out))
    open(img_path, "wb").write(out)
    print(f"wrote {img_path}: {len(out)} bytes, header v{hv}")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    u = sub.add_parser("unpack"); u.add_argument("image"); u.add_argument("outdir")
    p = sub.add_parser("pack"); p.add_argument("srcdir"); p.add_argument("image")
    p.add_argument("--pad-to", type=lambda s: int(s, 0), default=None,
                   help="pad output to this many bytes (stock partition: 67108864)")
    a = ap.parse_args()
    if a.cmd == "unpack":
        unpack(a.image, a.outdir)
    else:
        pack(a.srcdir, a.image, a.pad_to)


if __name__ == "__main__":
    main()
