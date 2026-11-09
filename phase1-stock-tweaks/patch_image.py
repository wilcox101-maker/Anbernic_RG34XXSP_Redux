#!/usr/bin/env python3
"""Write a boot image into the `boot` partition of a COPY of the stock RG34XXSP.img.

  python3 patch_image.py RG34XXSP-redux.img boot-redux-p1.img          # apply
  python3 patch_image.py RG34XXSP-redux.img 3.boot.img --restore       # roll back to stock

Works on Windows: patch the image file, then flash it with Rufus / balenaEtcher.
No raw partition access is needed.

Safety checks before writing anything:
  - the disk image has a valid GPT with exactly one partition named "boot"
  - that partition is 64 MiB, and the new boot image is exactly that size
  - (apply) the partition currently holds the known stock boot.img, which proves
    this is the original-revision firmware and hasn't already been patched
After writing, it reads the partition back and checks the SHA-256.
"""
import argparse
import hashlib
import struct
import sys

SECTOR = 512
BOOT_SIZE = 64 * 1024 * 1024
STOCK_BOOT_SHA256 = "87913826e15dab962b060703a0552270793b9e14cc303e5f8ec0f38e179ef50b"


def sha256_region(f, off, size):
    h = hashlib.sha256()
    f.seek(off)
    left = size
    while left:
        chunk = f.read(min(left, 4 << 20))
        if not chunk:
            sys.exit("image ends before the partition does (truncated?)")
        h.update(chunk); left -= len(chunk)
    return h.hexdigest()


def find_partition(f, want):
    f.seek(SECTOR)
    hdr = f.read(92)
    if hdr[:8] != b"EFI PART":
        sys.exit("no GPT header at LBA 1. Is this the stock RG34XXSP.img?")
    entries_lba, n_entries, entry_size = struct.unpack_from("<QII", hdr, 72)
    f.seek(entries_lba * SECTOR)
    table = f.read(n_entries * entry_size)
    hits = []
    for k in range(n_entries):
        e = table[k * entry_size:(k + 1) * entry_size]
        if e[:16] == b"\0" * 16:
            continue
        first, last = struct.unpack_from("<QQ", e, 32)
        name = e[56:128].decode("utf-16-le").rstrip("\0")
        if name == want:
            hits.append((k + 1, first * SECTOR, (last - first + 1) * SECTOR))
    if len(hits) != 1:
        sys.exit(f"expected exactly one '{want}' partition, found {len(hits)}")
    return hits[0]


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("disk_image", help="a COPY of RG34XXSP.img (modified in place)")
    ap.add_argument("boot_image", help="64 MiB boot image to write")
    ap.add_argument("--restore", action="store_true",
                    help="write the stock boot.img back (skips the 'currently stock' check)")
    a = ap.parse_args()

    new = open(a.boot_image, "rb").read()
    if len(new) != BOOT_SIZE:
        sys.exit(f"{a.boot_image} is {len(new)} bytes; expected {BOOT_SIZE}")
    new_sha = hashlib.sha256(new).hexdigest()
    if a.restore and new_sha != STOCK_BOOT_SHA256:
        sys.exit("--restore expects the stock 3.boot.img (sha256 mismatch)")

    with open(a.disk_image, "r+b") as f:
        num, off, size = find_partition(f, "boot")
        print(f"boot partition: p{num}, offset {off}, size {size}")
        if size != BOOT_SIZE:
            sys.exit(f"boot partition is {size} bytes, expected {BOOT_SIZE}; unknown layout")
        cur = sha256_region(f, off, size)
        if cur == new_sha:
            print("partition already contains this image; nothing to do"); return
        if not a.restore and cur != STOCK_BOOT_SHA256:
            sys.exit("boot partition is not the known stock image (different hardware revision "
                     "or already modified). Refusing.")
        f.seek(off); f.write(new); f.flush()
        if sha256_region(f, off, size) != new_sha:
            sys.exit("VERIFY FAILED: read-back does not match. Do not flash this image.")
    print(f"ok: p{num} now holds {a.boot_image} (sha256 {new_sha[:16]}...)")


if __name__ == "__main__":
    main()
