---
from: dale
to: all
event: decision
phase: 1.x
status: open
refs: [ROADMAP.md, docs/ram-system-design.md, docs/kernel-strategy.md, docs/hardware-map.md]
---

# Dale decision, 2026-11-13: ZRAM is a v1.x requirement

Recorded from Dale's words to the Redux Manager chat (03:10 America/Detroit): "Roadmap for after
all current tasks. Consider it a v1.x.x requirement." This followed the Manager's ZRAM feasibility
review below.

## Decision

- ZRAM is **required for v1.x**, on the mainline kernel. It is scheduled **after every current queue
  item and the 1.0 milestone**. It does not gate Beta or 1.0, and nothing is added to the stock-kernel
  (Phase 1 / Beta) build.
- The 2 GB default (off, safety net, or toggle) is decided later, from measurements, in its own
  `decision` note.

## Feasibility findings (Manager, read-only)

- Stock 4.9.170: `CONFIG_ZRAM` is not set. `ZSMALLOC`, `CRYPTO_LZ4`/`LZO`, `SWAP` are built in;
  `MODULES`, `MODVERSIONS`, `MODULE_FORCE_LOAD` set, no module signing. An out-of-tree `zram.ko` from
  vanilla 4.9.170 is possible only if every symbol CRC matches; no matching BSP source exists, and a
  forced load risks crashes. Not pursued (stability first; `.ko` from the card is a known root hook).
- The 2025 stock rootfs's `/lib/modules/4.9.170` metadata is inconsistent: `modules.order` lists
  `mm/zsmalloc.ko` and `crypto/lz4.ko`, but neither file exists and neither is in `modules.builtin`.
  Not verified on device: `grep -E " zs_create_pool| lz4_compress" /proc/kallsyms`.
- Mainline: a config change only (`docs/kernel-strategy.md`: kernel config under our control).
- Use: high on boards under 1.5 GiB (already in `docs/ram-system-design.md`); low to medium on the
  2 GB unit (1979 MB measured; > 1.6 GiB budgeted for emulators) as a safety net against OOM kills.
  No swap on the card or p7.

## Not verified on device

- Whether `zsmalloc` and `lz4` are really built into the running stock kernel (`/proc/kallsyms` check above).
- Peak memory of PPSSPP and DraStic on the 2 GB unit.
