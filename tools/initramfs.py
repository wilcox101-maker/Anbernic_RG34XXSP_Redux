#!/usr/bin/env python3
"""Unpack / repack a gzip'd newc cpio initramfs (the `ramdisk` from bootimg.py).

  python3 initramfs.py unpack ramdisk  rootdir/
  python3 initramfs.py pack   rootdir/ ramdisk.new

Unpacking keeps a manifest (.initramfs-manifest.json) of each entry's mode,
uid, gid and symlink target, because extracting as a normal user (or on
Windows) loses them. `pack` restores those from the manifest so the rebuilt
archive matches the original apart from your edits. Files you add that are not
in the manifest are packed as root-owned, keeping their on-disk mode.
Device nodes are recorded in the manifest and recreated on pack.
Pure standard library, no dependencies.
"""
import argparse
import gzip
import json
import os
import stat
import sys

MANIFEST = ".initramfs-manifest.json"
FIELDS = ["ino", "mode", "uid", "gid", "nlink", "mtime", "filesize",
          "devmajor", "devminor", "rdevmajor", "rdevminor", "namesize", "check"]


def unpack(ramdisk, root):
    raw = open(ramdisk, "rb").read()
    data = gzip.decompress(raw) if raw[:2] == b"\x1f\x8b" else raw
    os.makedirs(root, exist_ok=True)
    i, manifest = 0, []
    while True:
        if data[i:i + 6] not in (b"070701", b"070702"):
            sys.exit(f"bad cpio header at offset {i}")
        h = dict(zip(FIELDS, (int(data[i + 6 + 8 * k:i + 14 + 8 * k], 16) for k in range(13))))
        name = data[i + 110:i + 110 + h["namesize"] - 1].decode()
        j = (i + 110 + h["namesize"] + 3) & ~3
        body = data[j:j + h["filesize"]]
        i = (j + h["filesize"] + 3) & ~3
        if name == "TRAILER!!!":
            break
        mode = h["mode"]
        entry = {"name": name, "mode": mode, "uid": h["uid"], "gid": h["gid"],
                 "mtime": h["mtime"]}
        path = os.path.join(root, name)
        if name in (".", ""):
            manifest.append(entry); continue
        os.makedirs(os.path.dirname(path) or ".", exist_ok=True)
        if stat.S_ISDIR(mode):
            os.makedirs(path, exist_ok=True)
        elif stat.S_ISREG(mode):
            open(path, "wb").write(body)
            os.chmod(path, (mode & 0o777) | 0o600)
        elif stat.S_ISLNK(mode):
            entry["target"] = body.decode()
            if os.path.lexists(path):
                os.remove(path)
            try:
                os.symlink(entry["target"], path)
            except OSError:  # e.g. Windows without symlink rights
                open(path, "w").write(entry["target"])
        else:  # device nodes, fifos: manifest only
            entry["rdev"] = [h["rdevmajor"], h["rdevminor"]]
        manifest.append(entry)
    json.dump(manifest, open(os.path.join(root, MANIFEST), "w"), indent=0)
    print(f"unpacked {len(manifest)} entries into {root}")


def _rec(name, mode, uid, gid, mtime, body=b"", rdev=(0, 0), ino=0):
    namez = name.encode() + b"\0"
    vals = [ino, mode, uid, gid, 1, mtime, len(body), 0, 0, rdev[0], rdev[1], len(namez), 0]
    hdr = b"070701" + b"".join(b"%08X" % v for v in vals)
    out = hdr + namez
    out += b"\0" * ((4 - len(out) % 4) % 4)
    out += body
    out += b"\0" * ((4 - len(out) % 4) % 4)
    return out


def pack(root, out_path):
    mpath = os.path.join(root, MANIFEST)
    manifest = json.load(open(mpath)) if os.path.exists(mpath) else []
    known = {e["name"]: e for e in manifest}
    names = [e["name"] for e in manifest]
    for dp, dns, fns in os.walk(root):
        dns.sort(); fns.sort()
        for n in dns + fns:
            rel = os.path.relpath(os.path.join(dp, n), root).replace(os.sep, "/")
            if rel != MANIFEST and rel not in known:
                names.append(rel)
    blob, ino = bytearray(), 1
    for name in names:
        path = os.path.join(root, name)
        e = known.get(name)
        if e and not os.path.lexists(path) and not ("rdev" in e or name in (".", "")):
            continue  # deleted by the user
        if e:
            mode, uid, gid, mtime = e["mode"], e["uid"], e["gid"], e["mtime"]
        else:
            st = os.lstat(path)
            mode, uid, gid, mtime = st.st_mode, 0, 0, int(st.st_mtime)
        body, rdev = b"", (0, 0)
        if stat.S_ISREG(mode):
            body = open(path, "rb").read()
        elif stat.S_ISLNK(mode):
            body = (e["target"] if e else os.readlink(path)).encode()
        elif e and "rdev" in e:
            rdev = tuple(e["rdev"])
        blob += _rec(name, mode, uid, gid, mtime, body, rdev, ino)
        ino += 1
    blob += _rec("TRAILER!!!", 0, 0, 0, 0)
    blob += b"\0" * ((512 - len(blob) % 512) % 512)
    open(out_path, "wb").write(gzip.compress(bytes(blob), 9, mtime=0))
    print(f"packed {ino - 1} entries -> {out_path}")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    u = sub.add_parser("unpack"); u.add_argument("ramdisk"); u.add_argument("rootdir")
    p = sub.add_parser("pack"); p.add_argument("rootdir"); p.add_argument("output")
    a = ap.parse_args()
    unpack(a.ramdisk, a.rootdir) if a.cmd == "unpack" else pack(a.rootdir, a.output)


if __name__ == "__main__":
    main()
