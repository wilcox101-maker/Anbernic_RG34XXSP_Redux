#!/usr/bin/env python3
"""Install Redux on a stock RG34XXSP card, from your PC, over SSH (Dale's decision A, 2026-11-14).

    python tools/install.py --check <device-ip>
    python tools/install.py <device-ip> [--binary dmenu.bin] [--bootlogo bootlogo.bmp]
    python tools/install.py --uninstall <device-ip>
    python tools/install.py --print-script install [--binary ...]   # show what would be sent

One SSH session per run, so the root password is asked once. The device side is
installer/redux_install.sh, sent with the binary (base64) on the session's standard input; nothing
is copied with scp and nothing is left on the device but Redux itself.

What it changes on the handheld: /mnt/vendor/redux/ (the Redux binary, and the boot logo if you pass
--bootlogo), one line of /mnt/vendor/ctrl/dmenu_ln (backed up once to dmenu_ln.bak), and a log in
/mnt/data/redux/install.log. Never the Roms card, never the boot partition. --uninstall puts the
stock dmenu_ln back. See docs/install.md.

Needs: Python 3 (standard library only) and the ssh command (built into Windows 10 and 11); SSH on
the handheld (StockOS Mod setting, global.ssh=1).
"""

import argparse
import base64
import hashlib
import os
import subprocess
import sys

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
# In the repo: installer/redux_install.sh. From the dev release: next to install.py.
DEVICE_SCRIPTS = (os.path.join(ROOT_DIR, "installer", "redux_install.sh"),
                  os.path.join(os.path.dirname(os.path.abspath(__file__)), "redux_install.sh"))
LOGO_BYTES = 921654


def is_aarch64_elf(data):
    """AArch64 ELF: magic, then e_machine 0xB7 at offset 18 (little-endian)."""
    return len(data) >= 20 and data[:4] == b"\x7fELF" and data[18] == 0xB7 and data[19] == 0


def is_stock_bootlogo(data):
    """Same check as tools/make_bootlogo.py --check: 640x480, 24-bit BMP, 921654 bytes."""
    import struct
    if len(data) != LOGO_BYTES or data[:2] != b"BM":
        return False
    fsize, _, _, off = struct.unpack_from("<IHHI", data, 2)
    hdr, w, h, planes, bpp, comp = struct.unpack_from("<IiiHHI", data, 14)
    return (fsize, off, hdr, w, h, planes, bpp, comp) == (LOGO_BYTES, 54, 40, 640, 480, 1, 24, 0)


def _b64_lines(data):
    enc = base64.b64encode(data).decode("ascii")
    return "\n".join(enc[i:i + 76] for i in range(0, len(enc), 76))


def build_stream(mode, binary=None, logo=None, script_text=None):
    """Return the bytes sent to `sh -s` on the device: settings, payload functions, the script.

    Line endings are forced to LF: a Windows checkout with core.autocrlf would otherwise send
    CRLF, which sh on the device can't run.
    """
    if script_text is None:
        path = next((p for p in DEVICE_SCRIPTS if os.path.isfile(p)), None)
        if path is None:
            sys.exit("redux_install.sh not found: keep it in the same folder as install.py")
        with open(path, "rb") as f:
            script_text = f.read().decode("utf-8")
    script_text = script_text.replace("\r\n", "\n")
    head = [f"REDUX_MODE='{mode}'"]
    if binary is not None:
        head.append(f"REDUX_SHA256='{hashlib.sha256(binary).hexdigest()}'")
        head.append(f"REDUX_SIZE='{len(binary)}'")
        head.append("redux_payload() {\nbase64 -d <<'REDUX_PAYLOAD_END'\n"
                    + _b64_lines(binary) + "\nREDUX_PAYLOAD_END\n}")
    if logo is not None:
        head.append(f"REDUX_LOGO_SHA256='{hashlib.sha256(logo).hexdigest()}'")
        head.append("redux_payload_logo() {\nbase64 -d <<'REDUX_LOGO_END'\n"
                    + _b64_lines(logo) + "\nREDUX_LOGO_END\n}")
    return ("\n".join(head) + "\n" + script_text).encode("utf-8")


def read_file(path, what):
    try:
        with open(path, "rb") as f:
            return f.read()
    except OSError as e:
        sys.exit(f"cannot read {what} {path}: {e}")


def main(argv=None):
    ap = argparse.ArgumentParser(description="Install, check or remove Redux on a stock RG34XXSP card over SSH.")
    ap.add_argument("ip", nargs="?", help="the handheld's IP address (Settings > Network on the device)")
    g = ap.add_mutually_exclusive_group()
    g.add_argument("--check", action="store_true", help="report what is installed; writes nothing")
    g.add_argument("--uninstall", action="store_true", help="put the stock launcher back")
    g.add_argument("--print-script", metavar="MODE", choices=["check", "install", "uninstall"],
                   help="print what would be sent for MODE (check|install|uninstall) and exit")
    ap.add_argument("--binary", default="dmenu.bin",
                    help="the Redux dmenu.bin from the dev release (default: ./dmenu.bin)")
    ap.add_argument("--bootlogo", help="also copy this boot logo (bootlogo.bmp from the dev release)")
    a = ap.parse_args(argv)

    mode = a.print_script or ("check" if a.check else "uninstall" if a.uninstall else "install")
    binary = logo = None
    if mode == "install":
        binary = read_file(a.binary, "Redux binary")
        if not is_aarch64_elf(binary):
            sys.exit(f"refusing {a.binary}: not an aarch64 ELF (use dmenu.bin from the dev release)")
        if a.bootlogo:
            logo = read_file(a.bootlogo, "boot logo")
            if not is_stock_bootlogo(logo):
                sys.exit(f"refusing {a.bootlogo}: not a stock-format boot logo (640x480, 24-bit BMP)")
    stream = build_stream(mode, binary, logo)

    if a.print_script:
        sys.stdout.write(stream.decode("utf-8"))
        return 0
    if not a.ip:
        ap.error("the handheld's IP address is needed")
    if binary is not None:
        print(f"Redux binary: {a.binary}, {len(binary)} bytes, sha256 {hashlib.sha256(binary).hexdigest()}")
    print(f"Connecting to root@{a.ip} ({mode}). Enter the handheld's root password when asked.")
    cmd = ["ssh", "-o", "ConnectTimeout=10", f"root@{a.ip}", "sh -s"]
    try:
        rc = subprocess.run(cmd, input=stream).returncode
    except FileNotFoundError:
        sys.exit("the ssh command was not found (Windows: Settings > Apps > Optional features > OpenSSH Client)")
    if rc != 0:
        print(f"\nStopped (exit {rc}). If it says STOPPED above, nothing after that point was changed.",
              file=sys.stderr)
    return rc


if __name__ == "__main__":
    sys.exit(main())
