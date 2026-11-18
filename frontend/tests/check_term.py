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


LOG = None


def run(term, cmd, hold_ms=None, extra=(), script=None, env_add=None):
    global LOG
    with tempfile.NamedTemporaryFile(suffix=".dump", delete=False) as f:
        dump = f.name
    log = dump + ".log"
    env = dict(os.environ, SDL_VIDEODRIVER="dummy", REDUX_TERM_DUMP=dump, REDUX_TERM_LOG=log)
    if hold_ms is not None:
        env["REDUX_TERM_HOLD_MS"] = str(hold_ms)
    if script:
        env["REDUX_TERM_SCRIPT"] = script
    if env_add:
        env.update(env_add)
    t0 = time.time()
    r = subprocess.run([term, *extra, "-e", "sh", "-c", cmd], env=env, timeout=60)
    with open(dump, encoding="utf-8", errors="replace") as f:
        text = f.read()
    with open(log, encoding="utf-8", errors="replace") as f:
        LOG = f.read()
    os.unlink(dump)
    os.unlink(log)
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

    # Stock starts the frontend with a Chinese locale; the terminal's programs get English (C.UTF-8).
    rc, text, took = run(term, 'echo "L=$LANG A=${LC_ALL:-none} G=${LANGUAGE:-none} M=${LC_MESSAGES:-none}"; sleep 2.2',
                         env_add={"LANG": "zh_CN.UTF-8", "LC_ALL": "zh_CN.UTF-8", "LANGUAGE": "zh_CN:zh",
                                  "LC_MESSAGES": "zh_CN.UTF-8"})
    check(text.startswith("L=C.UTF-8 A=none G=none M=none"), "a Chinese locale from the frontend becomes C.UTF-8")

    # A failing program keeps its screen and the footer until a button (here: the hold timeout).
    rc, text, took = run(term, 'echo oops >&2; exit 3', hold_ms=500)
    check(rc == 0 and "oops" in text, "stderr of a failed program is shown")
    check(0.4 < took < 10, "a failed program holds the screen (%.1f s)" % took)

    # --hold keeps a clean, long run on screen until a button (the hold timeout here).
    rc, text, took = run(term, 'echo kept; sleep 2.2', hold_ms=500, extra=["--hold"])
    check(rc == 0 and text.startswith("kept") and 2.6 < took < 10, "--hold keeps a finished program on screen (%.1f s)" % took)

    # A curses-style program: alternate screen, scroll region, line drawing via ESC ( 0.
    rc, text, took = run(term, r'printf "\033[?1049h\033[2;29r\033[2;1H\033(0lqqk\033(B box\033[?1049l"; echo back; sleep 2.2')
    check(text.split("--- other screen ---")[0].split("\n")[0].startswith("back"), "leaving the alternate screen restores the shell's screen")
    check("???? box" in text.split("--- other screen ---")[1], "line drawing landed on the alternate screen")

    # Scripted buttons: Select opens the picker, A types the highlighted key (q) into the shell, B
    # closes it; the event log records each step (what Dale reads back from the device).
    rc, text, took = run(term, 'read x; echo "got:$x"; sleep 2.2',
                         script="300 SELECT 1;400 SELECT 0;600 A 1;700 A 0;900 B 1;1000 B 0;1200 A 1;1300 A 0")
    check("picker opened" in LOG and "picker closed" in LOG and "frame osk=1" in LOG, "the log shows the picker opening, a frame with it, and closing")
    check("got:q" in text, "A on the picker typed q, and A outside it sent Enter")

    print("check_term: " + ("FAILED" if fails else "all passed"))
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
