#!/usr/bin/env python3
"""F1 exit test: every F0 mock-up in docs/ui-spec.md is reproduced cell for cell.

  python frontend/tests/check_mockups.py <path to rg34xxsp-redux binary>

Runs the binary with --dump (which renders the six screens with the clock fixed to
12:34) and compares each 90 x 30 screen with the matching mock-up. "▓▓" marks the
selected row in both. Standard library only; runs on Windows and Linux.
"""
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
SCREENS = ["main", "roms", "settings", "eng-cpu", "eng-battery", "internet", "loading"]


def spec_mockups():
    text = (ROOT / "docs" / "ui-spec.md").read_text(encoding="utf-8")
    blocks = [b for b in re.findall(r"```\n(.*?)```", text, re.S) if "┌" in b]
    if len(blocks) != len(SCREENS):
        sys.exit(f"expected {len(SCREENS)} mock-ups in ui-spec.md, found {len(blocks)}")
    return {name: block.rstrip("\n").split("\n") for name, block in zip(SCREENS, blocks)}


def rendered(binary):
    out = subprocess.run([binary, "--dump"], capture_output=True, check=True).stdout.decode("utf-8")
    screens, name = {}, None
    for line in out.split("\n"):
        m = re.fullmatch(r"=== ([\w-]+) ===", line)
        if m:
            name = m.group(1)
            screens[name] = []
        elif name is not None and len(screens[name]) < 30:
            screens[name].append(line)
    return screens


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    want, got = spec_mockups(), rendered(sys.argv[1])
    failures = 0
    for name in SCREENS:
        w, g = want[name], got.get(name, [])
        bad = [(i, a, b) for i, (a, b) in enumerate(zip(w, g)) if a.rstrip() != b.rstrip()]
        if len(w) != 30 or len(g) != 30:
            bad.append((-1, f"{len(w)} rows in spec", f"{len(g)} rows rendered"))
        if bad:
            failures += 1
            print(f"FAIL {name}: {len(bad)} row(s) differ")
            for i, a, b in bad[:5]:
                print(f"  row {i}\n    spec: {a!r}\n    got:  {b!r}")
        else:
            print(f"ok   {name}: 30 rows x 90 columns match")
    sys.exit(1 if failures else 0)


if __name__ == "__main__":
    main()
