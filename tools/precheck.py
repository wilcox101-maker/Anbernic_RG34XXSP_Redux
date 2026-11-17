#!/usr/bin/env python3
"""Run the CI checks that matter for what you changed, before you push. One command.

    python tools/precheck.py            # checks for files changed against origin/main
    python tools/precheck.py --all      # every check, as CI runs them
    python tools/precheck.py --list     # say what would run, run nothing

It mirrors .github/workflows/ci.yml and notes-lint.yml:
  always            tools/check_rules.py, tools/check_notes.py
  frontend/ (C, CMake), branding/, tests
                    desktop + stock CMake builds with -Werror, every build/test_*,
                    the boot-logo render and check, the mock-up exit test, the terminal smoke test
  tools/            the Python unit tests (bench, extras, install) and a compile of tools/
  installer/        the install tests
Python 3 standard library only; runs on Windows (the C builds need CMake and SDL2, so on Windows
they are skipped with a note, as in CI's windows-tools job).
Exit 0 = everything that ran passed.
"""
import argparse
import glob
import os
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PY = sys.executable or "python3"

PY_TESTS = [("bench/tests", "bench harness"), ("extras_tests", "extras installer"),
            ("install_tests", "installer"),
            ("devtools_tests", "precheck and log_activity")]


def changed_files():
    """Files changed against origin/main: committed on this branch, staged, unstaged, untracked."""
    out = set()
    for cmd in (["git", "diff", "--name-only", "origin/main...HEAD"],
                ["git", "diff", "--name-only", "HEAD"],
                ["git", "ls-files", "--others", "--exclude-standard"]):
        r = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
        if r.returncode == 0:
            out.update(x for x in r.stdout.split() if x)
    return sorted(out)


def plan(files, everything):
    def touched(*prefixes):
        return everything or any(f.startswith(prefixes) for f in files)
    steps = [("hard-rule checks", [PY, "tools/check_rules.py"], "."),
             ("notes format", [PY, "tools/check_notes.py"], ".")]
    c_side = touched("frontend/", "branding/", ".github/workflows/ci.yml")
    if c_side:
        if os.name == "nt" or shutil.which("cmake") is None:
            why = "Windows" if os.name == "nt" else "no cmake here"
            steps.append(("C builds", None, f"skipped ({why}): CI's linux job builds and tests them"))
        else:
            for name, plat, out in (("desktop build", "desktop", "build"),
                                    ("stock build", "stock", "build-stock")):
                steps.append((name + " (configure)",
                              ["cmake", "-S", "frontend", "-B", out, "-DREDUX_PLATFORM=" + plat,
                               "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_C_FLAGS=-Werror",
                               "-DREDUX_BUILD=local"], "."))
                steps.append((name, ["cmake", "--build", out], "."))
            steps.append(("C tests (every build/test_*)", "C_TESTS", "."))
            steps.append(("boot logo render", [PY, "tools/make_bootlogo.py", "--out", "build/bootlogo.bmp"], "."))
            steps.append(("boot logo check", [PY, "tools/make_bootlogo.py", "--check", "build/bootlogo.bmp"], "."))
            steps.append(("mock-up exit test", [PY, "frontend/tests/check_mockups.py",
                                                "./build/rg34xxsp-redux"], "."))
            if os.name != "nt":
                steps.append(("terminal smoke test", [PY, "frontend/tests/check_term.py",
                                                      "./build/redux_term"], "."))
    if touched("tools/", "installer/", "phase1-stock-tweaks/"):
        steps.append(("tools compile", [PY, "-m", "compileall", "-q", "tools", "phase1-stock-tweaks"], "."))
        for d, what in PY_TESTS:
            steps.append((what + " tests", [PY, "-B", "-m", "unittest", "discover", "-s", d, "-t", "."], "tools"))
    return steps


C_STEPS = ("desktop build", "stock build", "C tests", "mock-up")


def run(steps):
    failed = []
    for name, cmd, cwd in steps:
        if failed and name.startswith(C_STEPS) and any(f.startswith(C_STEPS) for f in failed):
            print(f"--  {name}: skipped (an earlier C step failed)")
            continue
        if cmd is None:
            print(f"--  {name}: {cwd}")
            continue
        if cmd == "C_TESTS":
            tests = sorted(t for t in glob.glob(os.path.join(ROOT, "build", "test_*"))
                           if os.access(t, os.X_OK) and not t.endswith((".c", ".o")))
            ok = all(subprocess.run([t], cwd=ROOT).returncode == 0 for t in tests)
            print(f"{'ok' if ok else 'FAIL'}  {name}: {len(tests)} test programs")
            if not ok:
                failed.append(name)
            continue
        r = subprocess.run(cmd, cwd=os.path.join(ROOT, cwd), capture_output=True, text=True)
        tail = (r.stdout + r.stderr).strip().splitlines()[-1:] or [""]
        print(f"{'ok' if r.returncode == 0 else 'FAIL'}  {name}: {tail[0]}")
        if r.returncode != 0:
            print((r.stdout + r.stderr)[-3000:])
            failed.append(name)
    return failed


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--all", action="store_true", help="run every check, not only those for changed files")
    ap.add_argument("--list", action="store_true", help="show the plan, run nothing")
    a = ap.parse_args(argv)
    files = changed_files()
    steps = plan(files, a.all)
    print(f"precheck: {len(files)} changed file(s); {len(steps)} step(s)")
    if a.list:
        for name, cmd, cwd in steps:
            print(f"  {name}")
        return 0
    failed = run(steps)
    print(f"precheck: {'all passed' if not failed else 'FAILED: ' + ', '.join(failed)}")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
