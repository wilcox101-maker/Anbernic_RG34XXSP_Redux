#!/usr/bin/env python3
"""Mechanical checks for the AGENTS.md hard rules. Runs in CI on every PR; run it locally too.

    python tools/check_rules.py            # checks every tracked file (git ls-files)
    python tools/check_rules.py FILE...    # checks the given files

Exit code 0 = clean, 1 = a rule is broken. Python 3.10+ standard library only, runs on Windows.

A line may opt out of one check with a trailing comment naming it, e.g. `# rules: allow mkfs`
(`// rules: allow mkfs` in C). An opt-out must be explained in the PR.
"""
import ast
import os
import re
import subprocess
import sys

# Rule 1: never write to the Roms partition (p1, mounted at /mnt/mmc; /mnt/sdcard on debug units).
ROMS = re.compile(r"/mnt/(mmc|sdcard)\b")
# Scripts that run on the host or the device: any Roms path at all is suspect.
SCRIPT_DIRS = ("tools/", "phase1-", "phase2-")
SCRIPT_EXT = (".py", ".sh")
# Frontend C: a Roms path is fine to read (ROM lists, launch arguments), never to open for writing.
C_WRITE = re.compile(r"""fopen\s*\([^;]*/mnt/(mmc|sdcard)[^;]*,\s*"[wa]""")

# Rule 2: never format or repartition automatically.
FORMAT = re.compile(r"\b(mkfs(\.\w+)?|mke2fs|mkdosfs|wipefs|sfdisk|sgdisk|parted|fdisk)\b")
CODE_EXT = (".py", ".sh", ".c", ".h", ".cmake", ".txt")

# Rule 3: no binaries, ROMs, BIOS or images in the repo.
BINARY_EXT = {".img", ".bin", ".so", ".dge", ".elf", ".zip", ".7z", ".gz", ".xz", ".tar",
              ".gba", ".gb", ".gbc", ".nes", ".sfc", ".smc", ".md", ".iso", ".chd", ".cue",
              ".nds", ".pbp", ".fat", ".ext4", ".squashfs", ".ko", ".dtb"}
TEXT_MD_OK = (".md",)          # .md is Markdown here, not Mega Drive
MAX_BYTES = 1_000_000
MEDIA_EXT = {".png", ".jpg", ".jpeg", ".gif", ".svg", ".webp"}   # our own screenshots, allowed

# Rule 4: host tools are Python stdlib only.
NO_HOST_SCRIPTS = re.compile(r"^tools/.*\.(sh|bash|bat|cmd|ps1)$")

def tracked_files():
    out = subprocess.run(["git", "ls-files"], capture_output=True, text=True, check=True).stdout
    return [line for line in out.splitlines() if line]

def allowed(line, name):
    return re.search(r"rules:\s*allow\s+" + re.escape(name) + r"\b", line) is not None

SELF = "tools/check_rules.py"   # its own patterns and docs would match; skip only the line checks

def check_file(path, problems):
    p = path.replace("\\", "/")
    ext = os.path.splitext(p)[1].lower()
    try:
        data = open(path, "rb").read()
    except OSError as e:
        problems.append(f"{p}: cannot read ({e})")
        return

    # Rule 3
    if data[:4] == b"\x7fELF":
        problems.append(f"{p}: ELF binary committed (rule: no binaries)")
    if ext in BINARY_EXT and ext not in TEXT_MD_OK:
        problems.append(f"{p}: binary/ROM/image extension {ext} (rule: no binaries)")
    if len(data) > MAX_BYTES and not p.startswith("reference/"):
        problems.append(f"{p}: {len(data)} bytes, over {MAX_BYTES} (rule: no binaries; ask Dale)")
    if ext in MEDIA_EXT:
        return                 # our own screenshots/diagrams (size limit above still applies)
    if b"\0" in data[:8192]:
        problems.append(f"{p}: looks binary (NUL bytes)")
        return

    # Rule 4
    if NO_HOST_SCRIPTS.match(p):
        problems.append(f"{p}: host tools must be Python (rule: Python stdlib, runs on Windows)")
    if p.startswith("tools/") and ext == ".py":
        try:
            tree = ast.parse(data.decode("utf-8"), filename=p)
        except (SyntaxError, UnicodeDecodeError) as e:
            problems.append(f"{p}: does not parse ({e})")
            tree = None
        if tree is not None:
            for node in ast.walk(tree):
                names = []
                if isinstance(node, ast.Import):
                    names = [a.name for a in node.names]
                elif isinstance(node, ast.ImportFrom) and node.level == 0 and node.module:
                    names = [node.module]
                for n in names:
                    top = n.split(".")[0]
                    if top not in sys.stdlib_module_names:
                        problems.append(f"{p}:{node.lineno}: imports '{top}', not stdlib (rule: stdlib only)")

    if p == SELF:
        return
    text = data.decode("utf-8", errors="replace")
    in_script = p.startswith(SCRIPT_DIRS) and ext in SCRIPT_EXT or ext == ".sh"
    for i, line in enumerate(text.splitlines(), 1):
        # Rule 1
        if in_script and ROMS.search(line) and not allowed(line, "roms"):
            problems.append(f"{p}:{i}: Roms partition path in a script (rule: never touch Roms)")
        if ext in (".c", ".h") and C_WRITE.search(line) and not allowed(line, "roms"):
            problems.append(f"{p}:{i}: opens a Roms path for writing (rule: never touch Roms)")
        # Rule 2
        if ext in CODE_EXT and FORMAT.search(line) and not allowed(line, "mkfs"):
            problems.append(f"{p}:{i}: partition format/repartition command (rule: no auto-format)")

def main(argv):
    files = argv or tracked_files()
    problems = []
    for f in files:
        if os.path.isfile(f):
            check_file(f, problems)
    for line in problems:
        print("FAIL " + line)
    print(f"check_rules: {len(files)} files, {len(problems)} problem(s)")
    return 1 if problems else 0

if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
