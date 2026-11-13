#!/usr/bin/env python3
"""Checks every note in notes/ against the format in notes/README.md. Runs in CI; run it locally too.

    python tools/check_notes.py

Exit 0 = clean, 1 = a problem. Python 3 standard library only, runs on Windows.
Notes dated before 2026-11-13 are grandfathered (notes are append-only, so old ones can't be fixed).
Checks: file name pattern, front matter present, required keys, allowed values, and that every
note STATUS.md lists under "Open notes" exists.
"""
import os
import re
import sys

NOTES = "notes"
SKIP = {"README.md", "STATUS.md", "ACTIVITY.md"}
WHO = {"claude", "grok", "dale"}
EVENTS = {"phase-start", "milestone", "phase-gate", "handoff", "review-request", "review",
          "question", "decision", "blocker", "hw-result", "correction", "message"}
NAME = re.compile(r"^\d{4}-\d{2}-\d{2}-(claude|grok|dale)-([a-z0-9-]+)\.md$")
STATUS = re.compile(r"^(open|acknowledged|closed)\b")
SINCE = "2026-11-13"   # first date the check applies to


def front_matter(text):
    if not text.startswith("---"):
        return None
    end = text.find("\n---", 3)
    if end < 0:
        return None
    keys = {}
    for line in text[3:end].splitlines():
        line = line.split("#", 1)[0].strip()
        if ":" in line:
            k, v = line.split(":", 1)
            keys[k.strip()] = v.strip()
    return keys


def main():
    problems = []
    for name in sorted(os.listdir(NOTES)):
        path = os.path.join(NOTES, name)
        if not name.endswith(".md") or name in SKIP or not os.path.isfile(path):
            continue
        if name.startswith("handoff-"):
            if not re.match(r"^handoff-\d{4}-\d{2}-\d{2}(-\d+)?\.md$", name):
                problems.append(f"{name}: handoff files are named handoff-YYYY-MM-DD.md")
            continue
        m = NAME.match(name)
        if name[:10] < SINCE:
            continue
        if not m:
            problems.append(f"{name}: name is not YYYY-MM-DD-<from>-<event>-<topic>.md")
            continue
        with open(path, encoding="utf-8") as f:
            fm = front_matter(f.read())
        if fm is None:
            problems.append(f"{name}: no front matter (--- block at the top)")
            continue
        fm.setdefault("event", fm.get("type", ""))   # some notes say type:
        if fm.get("from"):
            fm["from"] = fm["from"].split()[0]          # "dale (recorded by claude)" -> dale
        for key in ("from", "to", "event", "status"):
            if not fm.get(key):
                problems.append(f"{name}: missing '{key}'")
        if fm.get("from") and fm["from"] not in WHO:
            problems.append(f"{name}: from '{fm['from']}' is not one of {sorted(WHO)}")
        if fm.get("from") and fm["from"] != m.group(1):
            problems.append(f"{name}: file name says {m.group(1)}, front matter says {fm['from']}")
        if fm.get("event") and fm["event"] not in EVENTS:
            problems.append(f"{name}: unknown event '{fm['event']}'")
        if fm.get("status") and not STATUS.match(fm["status"]):
            problems.append(f"{name}: status must start with open, acknowledged or closed")

    status = os.path.join(NOTES, "STATUS.md")
    if os.path.isfile(status):
        with open(status, encoding="utf-8") as f:
            for target in re.findall(r"\]\(([^)#]+\.md)\)", f.read()):
                if "/" not in target and not os.path.isfile(os.path.join(NOTES, target)):
                    problems.append(f"STATUS.md links to missing note {target}")

    for p in problems:
        print(p)
    print(f"check_notes: {len(problems)} problem(s)")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
