#!/usr/bin/env python3
"""Deploy the Redux frontend to the RG34XXSP.

DANGER: writes to the device. Run only after Dale has reviewed.
Writes only to appfs (/mnt/vendor). Never touches the Roms partition.

Usage:
  python tools/deploy_frontend.py <device-ip> [binary-path]
  python tools/deploy_frontend.py --restore <device-ip>

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

def run(cmd):
    print("+" + " ".join(cmd))
    return subprocess.run(cmd).returncode

def main():
    args = sys.argv[1:]
    restore = False
    if args and args[0] == "--restore":
        restore = True
        args = args[1:]
    if not args:
        print("usage: deploy_frontend.py [--restore] <device-ip> [binary]", file=sys.stderr)
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

    # Never overwrite an existing stock backup.
    rc = run(["ssh", f"root@{ip}", "test ! -f /mnt/vendor/ctrl/dmenu_ln.bak"])
    if rc != 0:
        print("error: dmenu_ln.bak already exists; refusing to overwrite", file=sys.stderr)
        return 1

    run(["ssh", f"root@{ip}", "mkdir -p /mnt/vendor/redux"])
    rc = run(["scp", bin_path, f"root@{ip}:/mnt/vendor/redux/rg34xxsp-redux"])
    if rc != 0:
        return rc

    print()
    print("Copied. On the device, after reading /mnt/vendor/ctrl/dmenu_ln:")
    print("  cp /mnt/vendor/ctrl/dmenu_ln /mnt/vendor/ctrl/dmenu_ln.bak")
    print("  # edit the wrapper to call /mnt/vendor/redux/rg34xxsp-redux")
    print("  # do not use exec if pwr_new.sh kills by process name")
    print()
    print(f"To restore: python tools/deploy_frontend.py --restore {ip}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
