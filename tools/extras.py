#!/usr/bin/env python3
"""Redux extras: tick the Ubuntu packages you want, get the SSH command that installs them.

    python tools/extras.py                      # checklist, then prints the commands
    python tools/extras.py --select htop,btop   # no checklist
    python tools/extras.py --ip 172.16.3.178    # device address (default 192.168.1.13)
    python tools/extras.py --run                # also runs the install over ssh, after a y/N
    python tools/extras.py --check              # prints the functional check only
    python tools/extras.py --check --run        # runs the functional check over ssh

The install never upgrades anything. StockOS is a full Ubuntu, but an upgrade can break the
library paths RetroArch needs (cbepx-me, StockOS-Modification issue #300). So the generated
command simulates first and refuses if any installed package would change version.
Catalog: tools/extras.ini. Python 3 standard library only, runs on Windows.
"""
import argparse
import configparser
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
CATALOG = os.path.join(HERE, "extras.ini")
DEFAULT_IP = "192.168.1.13"
NAME = re.compile(r"^[a-z0-9][a-z0-9.+-]*$")
HOST = re.compile(r"^[A-Za-z0-9._-]+$")
APT_FLAGS = "--no-upgrade --no-install-recommends"


def load(path=CATALOG):
    cp = configparser.ConfigParser(inline_comment_prefixes=None)
    cp.read(path, encoding="utf-8")
    items = []
    for sec in cp.sections():
        if sec == "stock":
            continue
        s = cp[sec]
        if not NAME.match(sec) or not NAME.match(s.get("binary", "")):
            raise ValueError("bad package or binary name in [%s]" % sec)
        if not re.match(r"^-{1,2}[A-Za-z]+$", s.get("check", "--version")):
            raise ValueError("bad check argument in [%s]" % sec)
        items.append({
            "package": sec,
            "binary": s.get("binary"),
            "check": s.get("check", "--version"),
            "default": s.getboolean("default", False),
            "desc": s.get("desc", ""),
            "verified": s.get("verified", "").strip(),
        })
    stock = cp.get("stock", "binaries", fallback="").split()
    for b in stock:
        if not NAME.match(b):
            raise ValueError("bad stock binary %r" % b)
    return items, stock


def install_command(packages):
    """Remote shell text: update, simulate, refuse on any upgrade, then install."""
    if not packages:
        raise ValueError("nothing selected")
    for p in packages:
        if not NAME.match(p):
            raise ValueError("bad package name %r" % p)
    pk = " ".join(packages)
    sim = "apt-get -s install %s %s" % (APT_FLAGS, pk)
    return ("export LC_ALL=C; apt-get update -qq || exit 2; "
            "up=$(%s | grep \"^Inst [^ ]* \\[\"); "
            "if [ -n \"$up\" ]; then echo \"REFUSED, this would upgrade installed packages:\"; "
            "echo \"$up\"; exit 3; fi; "
            "apt-get install -y %s %s" % (sim, APT_FLAGS, pk))


def check_command(items, stock):
    """Remote shell text: for each program, path, missing libraries and a version line."""
    parts = []
    for it in items:
        parts.append("c %s %s" % (it["binary"], it["check"]))
    for b in stock:
        parts.append("c %s --version" % b)
    fn = ("c() { p=$(command -v $1); if [ -z \"$p\" ]; then echo \"$1: MISSING\"; return; fi; "
          "m=$(ldd \"$p\" 2>/dev/null | grep -c \"not found\"); "
          "v=$(LC_ALL=C timeout 3 \"$p\" $2 </dev/null 2>&1 | head -1); "
          "echo \"$1 | $p | missing-libs=$m | $v\"; }; ")
    return fn + "; ".join(parts)


def install_then_check(packages, items):
    """One remote session: install (in a subshell, so its exits stay local), then the check
    only if the install succeeded. Exit code is the install's. One ssh, one password prompt."""
    return "( %s ); rc=$?; if [ $rc -eq 0 ]; then %s; fi; exit $rc" % (
        install_command(packages), check_command(items, []))


def ssh_line(ip, remote):
    if not HOST.match(ip):
        raise ValueError("bad device address %r" % ip)
    if "'" in remote:
        raise ValueError("remote command may not contain a single quote")
    return "ssh root@%s '%s'" % (ip, remote)


def checklist(items, inp=input, out=print):
    ticked = [it["default"] for it in items]
    while True:
        out("")
        for i, it in enumerate(items):
            v = "verified: " + it["verified"] if it["verified"] else "not verified on a device"
            out("  %2d [%s] %-8s %s (%s)" % (i + 1, "x" if ticked[i] else " ", it["package"], it["desc"], v))
        ans = inp("Numbers to tick/untick (e.g. 2 4), Enter when done: ").strip()
        if not ans:
            return [it["package"] for i, it in enumerate(items) if ticked[i]]
        for tok in ans.replace(",", " ").split():
            if tok.isdigit() and 1 <= int(tok) <= len(items):
                ticked[int(tok) - 1] = not ticked[int(tok) - 1]


def main(argv=None):
    ap = argparse.ArgumentParser(description="Install Redux extras on the device over SSH.")
    ap.add_argument("--ip", default=DEFAULT_IP)
    ap.add_argument("--select", help="comma list of packages, skips the checklist")
    ap.add_argument("--run", action="store_true", help="run over ssh: the install after a y/N, or with --check the check")
    ap.add_argument("--check", action="store_true", help="print the functional check only")
    a = ap.parse_args(argv)
    items, stock = load()
    by_name = {it["package"]: it for it in items}
    if a.check:
        line = ssh_line(a.ip, check_command(items, stock))
        if not a.run:
            print(line)
            return 0
        return subprocess.run(["ssh", "root@" + a.ip, check_command(items, stock)]).returncode
    if a.select:
        chosen = [p.strip() for p in a.select.split(",") if p.strip()]
        unknown = [p for p in chosen if p not in by_name]
        if unknown:
            print("not in tools/extras.ini: " + ", ".join(unknown), file=sys.stderr)
            return 2
    else:
        chosen = checklist(items)
    if not chosen:
        print("Nothing ticked.")
        return 0
    install = ssh_line(a.ip, install_command(chosen))
    check = ssh_line(a.ip, check_command([by_name[p] for p in chosen], []))
    print("\nNeeds: SSH on the device (StockOS Mod setting), Wi-Fi with internet, the root password.")
    print("Never run apt upgrade on the device: it can break RetroArch.\n")
    print("1. Install (simulates first, refuses if anything installed would be upgraded):")
    print(install)
    print("\n2. Check they run:")
    print(check)
    if a.run:
        if input("\nRun steps 1 and 2 now on %s (one ssh session)? [y/N] " % a.ip).strip().lower() != "y":
            return 0
        remote = install_then_check(chosen, [by_name[p] for p in chosen])
        return subprocess.run(["ssh", "root@" + a.ip, remote]).returncode
    return 0


if __name__ == "__main__":
    sys.exit(main())
