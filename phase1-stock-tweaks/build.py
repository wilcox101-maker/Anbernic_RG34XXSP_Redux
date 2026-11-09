#!/usr/bin/env python3
"""Build the Phase 1 boot image from the stock RG34XXSP boot.img.

  python3 phase1-stock-tweaks/build.py <stock 3.boot.img> <output boot-redux.img>

Changes (all inside the stock thermal and frequency envelope):
  1. DTB  gpio_keys poll-interval 20 ms -> 5 ms        (input latency)
  2. init set the CPU governor to schedutil              (stock pins it at max with 'performance')
  3. init mount the rootfs with write barriers on        (stock: barrier=0, risky on power loss)

Refuses to build unless the input matches the known stock image (firmware
rg34xxsp-fw-sd_202505, original 2 GB board), so it can't be applied to an
unknown hardware revision by accident.
"""
import hashlib
import os
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
TOOLS = os.path.join(HERE, "..", "tools")
KNOWN_STOCK = {
    # sha256 of 3.boot.img from rg34xxsp-fw-sd_202505 -> label
    "87913826e15dab962b060703a0552270793b9e14cc303e5f8ec0f38e179ef50b": "rg34xxsp-fw-sd_202505 (original 2 GB board)",
}
KEYS_NODE = "/soc@03000000/gpio_keys"
POLL_MS = 5

GOVERNOR_BLOCK = """
# --- RG34XXSP Redux phase 1: demand-based CPU governor (stock: performance) ---
for gov in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
	[ -w "$gov" ] && grep -qw schedutil "${gov%/*}/scaling_available_governors" \\
		&& echo schedutil > "$gov"
done
# --- end Redux ---
"""


def run(*args):
    subprocess.run([sys.executable, "-I", *args], check=True)


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    src, dst = sys.argv[1:]
    digest = hashlib.sha256(open(src, "rb").read()).hexdigest()
    if digest not in KNOWN_STOCK:
        sys.exit(f"refusing: {src} (sha256 {digest[:16]}...) is not a known stock boot.img")
    print(f"input: {KNOWN_STOCK[digest]}")

    work = tempfile.mkdtemp(prefix="redux-p1-")
    try:
        bi, rd = os.path.join(work, "boot"), os.path.join(work, "initramfs")
        run(os.path.join(TOOLS, "bootimg.py"), "unpack", src, bi)

        # 1. DTB: faster button polling
        run(os.path.join(TOOLS, "fdt_patch.py"), "set", os.path.join(bi, "dtb"),
            KEYS_NODE, "poll-interval", str(POLL_MS))

        # 2 + 3. initramfs /init
        run(os.path.join(TOOLS, "initramfs.py"), "unpack", os.path.join(bi, "ramdisk"), rd)
        init_path = os.path.join(rd, "init")
        init = open(init_path, newline="\n").read()
        old_mount = "noauto_da_alloc,barrier=0,data=ordered"
        handoff = "[ -x /mnt/init ] && mount -o noatime,move /dev /mnt/dev"
        if init.count(old_mount) != 1 or init.count(handoff) != 1:
            sys.exit("stock /init does not look as expected; not patching")
        init = init.replace(old_mount, "noauto_da_alloc,barrier=1,data=ordered")
        init = init.replace(handoff, GOVERNOR_BLOCK.lstrip("\n") + "\n" + handoff)
        open(init_path, "w", newline="\n").write(init)
        run(os.path.join(TOOLS, "initramfs.py"), "pack", rd, os.path.join(bi, "ramdisk"))

        run(os.path.join(TOOLS, "bootimg.py"), "pack", bi, dst, "--pad-to", "67108864")
    finally:
        shutil.rmtree(work, ignore_errors=True)
    print(f"done: {dst}\nsha256 {hashlib.sha256(open(dst, 'rb').read()).hexdigest()}")


if __name__ == "__main__":
    main()
