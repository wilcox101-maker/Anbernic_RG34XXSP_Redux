#!/usr/bin/env python3
"""Deploy the Redux frontend to the RG34XXSP.

DANGER: writes to the device. Run only after Dale has reviewed.
Writes only to appfs (/mnt/vendor). Never touches the Roms partition.

Usage:
  python tools/deploy_frontend.py <device-ip> [binary-path]
  python tools/deploy_frontend.py --restore <device-ip>
  python tools/deploy_frontend.py --bootlogo <bootlogo.bmp> <device-ip>

--bootlogo only copies the Redux boot logo to appfs (/mnt/vendor/redux/bootlogo.bmp), after
checking it is exactly the stock format. It does not touch the boot partition: installing it
is done on the handheld, in Settings > System > Boot Screen.

The binary must be an aarch64 ELF (checked from the header).
Uses Windows' built-in ssh and scp. Python standard library only.
"""

import sys
import subprocess
import os

def is_aarch64_elf(path):
    """Check the ELF header for AArch64."""
    try:
        with open(path, "rb") as f:
            hdr = f.read(20)
        if len(hdr) < 20 or hdr[:4] != b"\x7fELF":
            return False
        # e_machine at offset 18 (little-endian): 0xB7 = AArch64
        return hdr[18] == 0xB7 and hdr[19] == 0x00
    except OSError:
        return False

def is_stock_bootlogo(path):
    """Same check as tools/make_bootlogo.py --check: 640x480, 24-bit, 921654 bytes."""
    import struct
    try:
        d = open(path, "rb").read()
    except OSError:
        return False
    if len(d) != 921654 or d[:2] != b"BM":
        return False
    fsize, _, _, off = struct.unpack_from("<IHHI", d, 2)
    hdr, w, h, planes, bpp, comp = struct.unpack_from("<IiiHHI", d, 14)
    return (fsize, off, hdr, w, h, planes, bpp, comp) == (921654, 54, 40, 640, 480, 1, 24, 0)

def deploy_bootlogo(ip, path):
    if not is_stock_bootlogo(path):
        print(f"refusing {path}: not a stock-format boot logo "
              "(python tools/make_bootlogo.py --check)", file=sys.stderr)
        return 1
    if run(["ssh", f"root@{ip}", "mkdir -p /mnt/vendor/redux"]) != 0:
        return 1
    rc = run(["scp", path, f"root@{ip}:/mnt/vendor/redux/bootlogo.bmp.new"])
    if rc != 0:
        return rc
    rc = run(["ssh", f"root@{ip}",
              "mv /mnt/vendor/redux/bootlogo.bmp.new /mnt/vendor/redux/bootlogo.bmp && sync"])
    if rc == 0:
        print("Copied to /mnt/vendor/redux/bootlogo.bmp. Install it on the handheld:")
        print("  Settings > System > Boot Screen (3 s hold).")
    return rc

def run(cmd):
    print("+" + " ".join(cmd))
    return subprocess.run(cmd).returncode

def main():
    args = sys.argv[1:]
    restore = False
    if args and args[0] == "--restore":
        restore = True
        args = args[1:]
    if len(args) == 3 and args[0] == "--bootlogo":
        return deploy_bootlogo(args[2], args[1])
    if not args:
        print("usage: deploy_frontend.py [--restore] <device-ip> [binary]\n"
              "       deploy_frontend.py --bootlogo <bootlogo.bmp> <device-ip>", file=sys.stderr)
        return 2
    ip = args[0]
    bin_path = args[1] if len(args) > 1 else "frontend/build/rg34xxsp-redux"

    if restore:
        # Exit with error if no backup exists. Never overwrite a missing backup.
        rc = run(["ssh", f"root@{ip}", "test -f /mnt/vendor/ctrl/dmenu_ln.bak"])
        if rc != 0:
            print("error: no dmenu_ln.bak found; refusing to restore", file=sys.stderr)
            return 1
        return run(["ssh", f"root@{ip}",
                    "mv /mnt/vendor/ctrl/dmenu_ln.bak /mnt/vendor/ctrl/dmenu_ln"])

    if not os.path.isfile(bin_path):
        print(f"binary not found: {bin_path}", file=sys.stderr)
        return 1
    if not is_aarch64_elf(bin_path):
        print(f"refusing non-aarch64 ELF: {bin_path}", file=sys.stderr)
        print("build on the device or with an aarch64 toolchain + -DREDUX_PLATFORM=stock",
              file=sys.stderr)
        return 1

    # Copy to a temp name, then rename: overwriting a running binary in place fails
    # ("Text file busy") and a cut-off copy would leave a broken launcher.
    if run(["ssh", f"root@{ip}", "mkdir -p /mnt/vendor/redux"]) != 0:
        print("error: cannot reach the device or create /mnt/vendor/redux", file=sys.stderr)
        return 1
    rc = run(["scp", bin_path, f"root@{ip}:/mnt/vendor/redux/dmenu.bin.new"])
    if rc != 0:
        return rc
    rc = run(["ssh", f"root@{ip}",
              "chmod +x /mnt/vendor/redux/dmenu.bin.new && "
              "mv /mnt/vendor/redux/dmenu.bin.new /mnt/vendor/redux/dmenu.bin"])
    if rc != 0:
        return rc

    print()
    print("Copied to /mnt/vendor/redux/dmenu.bin (named dmenu.bin so pwr_new.sh's pidof check")
    print("still finds the menu). To switch the launcher, on the device:")
    print("  [ -e /mnt/vendor/ctrl/dmenu_ln.bak ] || cp /mnt/vendor/ctrl/dmenu_ln /mnt/vendor/ctrl/dmenu_ln.bak")
    print("""  sed -i 's|^CMD="/mnt/vendor/bin/dmenu.bin"|CMD="/mnt/vendor/redux/dmenu.bin"|' /mnt/vendor/ctrl/dmenu_ln""")
    print("  grep '^CMD=' /mnt/vendor/ctrl/dmenu_ln   # expect the redux path")
    print("See notes/2026-11-10-claude-finding-dmenu-ln.md.")
    print()
    print(f"To restore: python tools/deploy_frontend.py --restore {ip}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
