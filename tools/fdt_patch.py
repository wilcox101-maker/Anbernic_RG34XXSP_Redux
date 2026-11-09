#!/usr/bin/env python3
"""Read or overwrite a fixed-size property in a compiled device tree blob (.dtb), in place.

  python3 fdt_patch.py get board.dtb /soc@03000000/gpio_keys poll-interval
  python3 fdt_patch.py set board.dtb /soc@03000000/gpio_keys poll-interval 5 [-o out.dtb]

Only same-length u32 values are written, so the blob layout never changes. That
is the safest way to tweak a vendor DTB with no dtc round-trip. Pure standard
library, no dependencies.
"""
import argparse
import struct
import sys

FDT_MAGIC = 0xD00DFEED
BEGIN_NODE, END_NODE, PROP, NOP, END = 1, 2, 3, 4, 9


def find_prop(dtb, node_path, prop_name):
    """Return (value_offset, length) of prop_name in node_path, or None."""
    magic, totalsize, off_struct, off_strings = struct.unpack_from(">4I", dtb, 0)
    if magic != FDT_MAGIC:
        sys.exit("not a flattened device tree (bad magic)")
    want = [p for p in node_path.strip("/").split("/") if p]
    path, i = [], off_struct
    while True:
        (tok,) = struct.unpack_from(">I", dtb, i); i += 4
        if tok == BEGIN_NODE:
            end = dtb.index(b"\0", i)
            name = dtb[i:end].decode()
            if name:
                path.append(name)
            i = (end + 1 + 3) & ~3
        elif tok == END_NODE:
            if path:
                path.pop()
        elif tok == PROP:
            length, nameoff = struct.unpack_from(">2I", dtb, i); i += 8
            nend = dtb.index(b"\0", off_strings + nameoff)
            pname = dtb[off_strings + nameoff:nend].decode()
            if path == want and pname == prop_name:
                return i, length
            i = (i + length + 3) & ~3
        elif tok == NOP:
            continue
        elif tok == END:
            return None
        else:
            sys.exit(f"bad FDT token {tok:#x} at {i - 4}")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("cmd", choices=["get", "set"])
    ap.add_argument("dtb"); ap.add_argument("node"); ap.add_argument("prop")
    ap.add_argument("value", nargs="?", type=lambda s: int(s, 0))
    ap.add_argument("-o", "--output", help="write here instead of modifying dtb in place")
    a = ap.parse_args()

    dtb = bytearray(open(a.dtb, "rb").read())
    hit = find_prop(dtb, a.node, a.prop)
    if not hit:
        sys.exit(f"{a.node}:{a.prop} not found")
    off, length = hit
    if length != 4:
        sys.exit(f"{a.prop} is {length} bytes; only u32 properties are supported")
    (old,) = struct.unpack_from(">I", dtb, off)
    if a.cmd == "get":
        print(old); return
    if a.value is None:
        sys.exit("set needs a value")
    struct.pack_into(">I", dtb, off, a.value)
    open(a.output or a.dtb, "wb").write(dtb)
    print(f"{a.node}:{a.prop} {old} -> {a.value}")


if __name__ == "__main__":
    main()
