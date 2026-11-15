#!/usr/bin/env python3
"""Append one line to notes/ACTIVITY.md on main and push it. The only direct-to-main push (AGENTS.md).

    python tools/log_activity.py --who claude --read "Dale HELLO" --did "took ownership"
    python tools/log_activity.py ... --trailer "Co-Authored-By: ..." --dry-run

- The time comes from the clock, in America/New_York (computed here, so no tzdata is needed on
  Windows): `- 2026-11-14 17:31 · claude · read: … · did: …`.
- It works from any branch and never touches your working tree or index: it fetches main, builds
  the commit on top of origin/main with a temporary index, and pushes that commit to main. If main
  moved in between, it fetches and tries once more.
Python 3 standard library only; runs on Windows. Needs git and push access.
"""
import argparse
import datetime as dt
import os
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PATH = "notes/ACTIVITY.md"


def eastern_now(utc=None):
    """US Eastern time: DST from the 2nd Sunday in March 02:00 to the 1st Sunday in November 02:00."""
    utc = utc or dt.datetime.now(dt.timezone.utc).replace(tzinfo=None)
    y = utc.year

    def sunday(month, nth):
        d = dt.datetime(y, month, 1)
        d += dt.timedelta(days=(6 - d.weekday()) % 7)
        return d + dt.timedelta(weeks=nth - 1)
    start = sunday(3, 2) + dt.timedelta(hours=7)    # 02:00 EST = 07:00 UTC
    end = sunday(11, 1) + dt.timedelta(hours=6)     # 02:00 EDT = 06:00 UTC
    offset = -4 if start <= utc < end else -5
    return utc + dt.timedelta(hours=offset)


def git(*args, env=None, input=None, check=True):
    r = subprocess.run(["git", *args], cwd=ROOT, capture_output=True, text=True, env=env, input=input)
    if check and r.returncode != 0:
        raise RuntimeError(f"git {' '.join(args)}: {r.stderr.strip()}")
    return r


def line_for(who, read, did, now=None):
    t = (now or eastern_now()).strftime("%Y-%m-%d %H:%M")
    return f"- {t} · {who} · read: {read} · did: {did}"


def commit_on_main(line, message, remote="origin"):
    git("fetch", "-q", "--depth=50", remote, "main")
    base = git("rev-parse", "FETCH_HEAD").stdout.strip()
    text = git("show", f"{base}:{PATH}").stdout
    if not text.endswith("\n"):
        text += "\n"
    blob = git("hash-object", "-w", "--stdin", input=text + line + "\n").stdout.strip()
    fd, index = tempfile.mkstemp(prefix="activity-index-")
    os.close(fd)
    try:
        env = dict(os.environ, GIT_INDEX_FILE=index)
        git("read-tree", base, env=env)
        git("update-index", "--add", "--cacheinfo", f"100644,{blob},{PATH}", env=env)
        tree = git("write-tree", env=env).stdout.strip()
    finally:
        os.remove(index)
    return git("commit-tree", tree, "-p", base, "-m", message).stdout.strip()


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--who", default="claude")
    ap.add_argument("--read", required=True, help="what you read (Dale's command, notes, PRs)")
    ap.add_argument("--did", required=True, help="what you did, or 'nothing'")
    ap.add_argument("--trailer", action="append", default=[], help="commit trailer line (repeatable)")
    ap.add_argument("--remote", default="origin")
    ap.add_argument("--dry-run", action="store_true", help="print the line and commit, push nothing")
    a = ap.parse_args(argv)
    line = line_for(a.who, a.read, a.did)
    message = "ACTIVITY: " + a.who + "\n" + ("\n" + "\n".join(a.trailer) if a.trailer else "")
    for attempt in (1, 2):
        sha = commit_on_main(line, message, a.remote)
        if a.dry_run:
            print(line)
            print(f"log_activity: dry run, commit {sha[:7]} not pushed")
            return 0
        r = git("push", "-q", a.remote, f"{sha}:refs/heads/main", check=False)
        if r.returncode == 0:
            print(line)
            print(f"log_activity: pushed {sha[:7]} to main")
            return 0
        if attempt == 2:
            print(f"log_activity: push failed: {r.stderr.strip()}", file=sys.stderr)
            return 1
    return 1


if __name__ == "__main__":
    sys.exit(main())
