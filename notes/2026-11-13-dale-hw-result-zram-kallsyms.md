---
from: dale
to: all
event: hw-result
phase: F6
status: closed
refs: [issue #109, notes/2026-11-13-dale-decision-zram-v1x.md]
---

# Dale hardware result, 2026-11-13 10:57: zsmalloc and LZ4 are built into the stock kernel

Answers the open check in the [ZRAM v1.x decision](2026-11-13-dale-decision-zram-v1x.md)
("Not verified on device": `/proc/kallsyms`). Stock kernel 4.9.170, Dale's 2 GB unit, as root:

```
root@ANBERNIC:~# grep -E " zs_create_pool| lz4_compress" /proc/kallsyms
ffffff80081ad518 T zs_create_pool
ffffff800837bbb8 t lz4_compress_crypto
ffffff80083cc7d0 T lz4_compress
```

## What it means
- `zs_create_pool` (T): **zsmalloc is built in** and exported.
- `lz4_compress` (T) and `lz4_compress_crypto` (t): **LZ4 is built in**, crypto API wrapper included.
- So the stale `modules.order` entries (`mm/zsmalloc.ko`, `crypto/lz4.ko`) are leftovers; nothing
  is missing there.
- Unchanged: `CONFIG_ZRAM` is still not set in stock, so there is **no zram on the stock kernel**.
  The decision stands: zram comes with the mainline (Phase 1 / Beta) kernel as a config change; no
  out-of-tree `zram.ko`.

## Still not verified on device
- Peak memory of PPSSPP and DraStic on the 2 GB unit (input for the 2 GB default).
