#!/usr/bin/env python3
"""Redux Terminal smoke test: run the built redux_term headless (SDL dummy video driver) around a
real shell on a real pty and check the screen it ends with. Covers what test_vt cannot: the pty,
the child's environment, the exit path and the hold-on-failure footer.

    python3 frontend/tests/check_term.py ./build/redux_term
Exit 0 = pass. Needs a Linux host with /bin/sh; no display and no device."""
import os
import subprocess
import sys
import tempfile
import time


def run(term, cmd, hold_ms=None):
    with tempfile.NamedTemporaryFile(suffix=".dump", delete=False) as f:
        dump = f.name
    env = dict(os.environ, SDL_VIDEODRIVER="dummy", REDUX_TERM_DUMP=dump)
    if hold_ms is not None:
        env["REDUX_TERM_HOLD_MS"] = str(hold_ms)
    t0 = time.time()
    r = subprocess.run([term, "-e", "sh", "-c", cmd], env=env, timeout=60)
    with open(dump, encoding="utf-8", errors="replace") as f:
        text = f.read()
    os.unlink(dump)
    return r.returncode, text, time.time() - t0


def main():
    term = sys.argv[1] if len(sys.argv) > 1 else "./build/redux_term"
    fails = 0

    def check(ok, what):
        nonlocal fails
        print(("ok   " if ok else "FAIL ") + what)
        fails += not ok

    # Plain output, colours, cursor moves and the environment the program sees.
    rc, text, took = run(term, r'printf "hello \033[1;32mworld\033[0m\n"; tput cup 5 10; echo "T=$TERM C=$COLUMNS L=$LINES";'
                               r' stty size; sleep 2.2')
    rows = text.split("\n")
    check(rc == 0, "clean exit after a run longer than two seconds closes the terminal")
    check(rows[0].startswith("hello world"), "text reaches the screen")
    check(rows[5][10:].startswith("T=xterm-256color C=90 L=30"), "cursor moves; TERM, COLUMNS and LINES are set")
    check(rows[6].startswith("30 90"), "the pty is 90 x 30")
    check(took < 10, "exits promptly (%.1f s)" % took)

    # A failing program keeps its screen and the footer until a button (here: the hold timeout).
    rc, text, took = run(term, 'echo oops >&2; exit 3', hold_ms=500)
    check(rc == 0 and "oops" in text, "stderr of a failed program is shown")
    check(0.4 < took < 10, "a failed program holds the screen (%.1f s)" % took)

    # A curses-style program: alternate screen, scroll region, line drawing via ESC ( 0.
    rc, text, took = run(term, r'printf "\033[?1049h\033[2;29r\033[2;1H\033(0lqqk\033(B box\033[?1049l"; echo back; sleep 2.2')
    check(text.split("--- other screen ---")[0].split("\n")[0].startswith("back"), "leaving the alternate screen restores the shell's screen")
    check("???? box" in text.split("--- other screen ---")[1], "line drawing landed on the alternate screen")

    print("check_term: " + ("FAILED" if fails else "all passed"))
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
