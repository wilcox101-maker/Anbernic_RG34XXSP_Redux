#!/usr/bin/env python3
"""Print the button -> GPIO -> Linux event map from a decompiled sunxi BSP device tree.

  python3 dt_keys.py reference/stock-202505/board.dts

Reads the `gpio-keys-polled` node ("ANBERNIC-keys") and decodes each child's
sunxi-style GPIO spec <&pio bank pin mux pull drv data> into a pin name such
as PA6, along with the event type and code Linux will report.
"""
import re
import sys

BANKS = "ABCDEFGHIJKLMN"
EV = {1: "EV_KEY", 3: "EV_ABS", 5: "EV_SW"}
CODES = {
    0x10: "ABS_HAT0X", 0x11: "ABS_HAT0Y",
    0x130: "BTN_SOUTH", 0x131: "BTN_EAST", 0x132: "BTN_C", 0x133: "BTN_NORTH",
    0x134: "BTN_WEST", 0x135: "BTN_Z", 0x136: "BTN_TL", 0x137: "BTN_TR",
    0x138: "BTN_TL2", 0x139: "BTN_TR2", 0x13A: "BTN_SELECT", 0x13B: "BTN_START",
    0x13C: "BTN_MODE", 0x13D: "BTN_THUMBL", 0x13E: "BTN_THUMBR",
    0x72: "KEY_VOLUMEDOWN", 0x73: "KEY_VOLUMEUP", 0x74: "KEY_POWER",
}


def node_block(s, marker):
    i = s.find(marker)
    if i < 0:
        sys.exit(f"node containing {marker!r} not found")
    j = s.rfind("{", 0, i)
    depth = 0
    for k in range(j, len(s)):
        depth += {"{": 1, "}": -1}.get(s[k], 0)
        if depth == 0:
            return s[j:k + 1]


def main(path):
    s = open(path).read()
    blk = node_block(s, 'label = "ANBERNIC-keys"')
    poll = re.search(r"poll-interval = <(0x\w+)>", blk)
    if poll:
        print(f"poll-interval: {int(poll.group(1), 16)} ms\n")
    print(f"{'node':12} {'pin':5} {'type':7} {'code':6} {'linux name':14} value")
    for m in re.finditer(r"(\w+) \{([^{}]*)\}", blk):
        b = m.group(2)
        g = re.search(r"gpios = <([^>]*)>", b)
        if not g:
            continue
        a = [int(x, 16) for x in g.group(1).split()]
        code = int(re.search(r"linux,code = <(0x\w+)>", b).group(1), 16)
        t = re.search(r"linux,input-type = <(0x\w+)>", b)
        v = re.search(r"linux,input-value = <(0x\w+)>", b)
        typ = int(t.group(1), 16) if t else 1
        val = int(v.group(1), 16) if v else None
        val = "" if val is None else (-1 if val == 0xFFFFFFFF else val)
        print(f"{m.group(1):12} P{BANKS[a[1]]}{a[2]:<3} {EV.get(typ, typ):7} "
              f"{code:#06x} {CODES.get(code, '?'):14} {val}")


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "reference/stock-202505/board.dts")
