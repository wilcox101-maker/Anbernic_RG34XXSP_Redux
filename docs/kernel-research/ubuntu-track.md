# Ubuntu 24.04 userland track (parallel)

Status: **plan only, nothing built. Dale is reviewing it; not on the ROADMAP yet.** Manager draft
2026-11-13, revised the same day after the kernel research in this folder.

## Why
Stock is Ubuntu 22.04 (Jammy) on Allwinner's 4.9.170 BSP kernel. Jammy standard support ends
**spring 2027** (Canonical's table says April; the cut-off has been the end of May, as for 20.04 on 2025-05-31; ESM to 2032). 24.04 (Noble) runs to April 2029 (ESM 2034); 26.04 to 2031.
A newer userland brings current SDL2, Mesa (Panfrost for Phase 2), Python 3.12, NetworkManager, BlueZ.

## Where it fits (no change to Beta)
| Track | Kernel | Userland | Ships |
|---|---|---|---|
| Beta (v0.5) | stock 4.9.170 | stock 22.04, untouched | Redux replaces only the launcher |
| **U (this plan)** | stock 4.9.170 first, then our mainline build | **24.04, built by us** | experiment, then feeds 1.0 |
| 1.0 (Phase 2) | mainline (ROCKNIX runs 7.2 on this device) | RAM-resident system | per `docs/ram-system-design.md` |

Track U never touches a user's stock card, never gates Beta, and runs on a **separate bench card**
between Beta queue items.

## Ubuntu's kernels don't fit the device
| Release | Kernel | H700 support |
|---|---|---|
| 22.04 (stock userland) | 5.15 GA; stock runs Anbernic's 4.9 instead | none upstream |
| 24.04 GA / .1 | 6.8 | none (RG35XX trees arrived in 6.10) |
| 24.04.4 HWE (early 2026) | 6.17 | RG35XX 2024/Plus/H/SP trees; no display, no RG34XX-SP |
| 24.04.5 HWE (Jul 2026) | 7.0 | same |
| 26.04 GA (Apr 2026) | 7.0 | same |
So track U always uses our kernel. Ubuntu supplies only userland packages.

## Known risks
1. **systemd 255 vs. kernel 4.9.** Upstream calls kernels below 4.15 "not recommended" (significant
   gaps; `old-kernel` taint); 3.15 is the hard minimum. Expect some unit sandboxing and cgroup v2
   features to fail on 4.9. Disappears on mainline.
2. **Closed GPU stack on 4.9.** `mali_kbase.ko`, libmali and Anbernic's SDL2 can't be committed;
   the bench card copies them from Dale's own stock image at install time. On mainline, Panfrost +
   Noble's Mesa replace them.
3. **Stock apps are built for Jammy.** RetroArch 1.19.1 and cores, DraStic, PPSSPP, `.dge` apps,
   Pixel Reader, PortMaster, stock Python 3.10 scripts. Old binaries mostly run on newer glibc;
   Python 3.12 and soname changes (libssl, libfontconfig) will break some.
4. **Size vs. the Phase 2 design.** The RAM image is meant to be BusyBox-class; stock `/usr` is
   3.5 GiB. Full Ubuntu can't be the RAM image, least of all on 512 MB / 1 GB boards.
5. **Hard rules.** No Anbernic binaries in the repo; host tools stay Python stdlib on Windows.
   Images are built in CI on the arm64 runners.

## Steps
**U0. Decide (Dale, one `decision` note).**
- Release: 24.04 (as asked) or 26.04 (newer, supported to 2031). Manager view: plan for 24.04, keep
  26.04 as the fallback.
- Role of Ubuntu in 1.0: (a) full Ubuntu rootfs on the card; (b) Ubuntu packages feeding the
  read-only RAM image and payload squashfs; (c) Ubuntu only on the stock-kernel track. Manager
  recommends (b).
- Fable checkpoint trigger 3 applies (touches the RAM-resident system).

**U1. Inventory (desk).** Every userland piece Redux and the stock apps need: package, Jammy and
Noble versions, ABI risk. Output `docs/ubuntu-2404-inventory.md`.

**U2. Build pipeline (CI only).** Two proven patterns to choose from: `mmdebstrap` minbase (GregoryPletnev/rg35xxh-debian13,
small, simple) or Canonical's `livecd-rootfs` with the kernel packaged as Ubuntu kernel debs and a PPA
(Joshua-Riek/ubuntu-rockchip, closest to an official Ubuntu image, more upkeep; that project was archived in
2026-04 after years of single-maintainer work). Manager view: mmdebstrap, with our kernel as a plain .deb.
 A Noble arm64 minimal rootfs on `ubuntu-24.04-arm`,
package list in the repo, image as a CI artifact. A CI job builds the Redux frontend against Noble.
Exit: green twice.

**U3. Bench boot on the stock kernel (Dale, bench card).** Stock boot.img and modules; installer
copies `mali_kbase.ko`, Wi-Fi/BT modules and libmali from Dale's stock image. Checks: shell, systemd
units (log every 4.9 failure), Wi-Fi, audio, buttons, GPU via SDL2. Needs UART or SSH. Exit: hw-result note.

**U4. Redux and RetroArch on the bench card.** Frontend from U2; RetroArch built from source in CI.
Exit: the F2 launch-parity checklist.

**U5. Go / no-go (Dale).** Go: fold into Phase 2 per U0. No-go on 4.9: Noble only on mainline.

Checkpoints for Dale: U0, U3, U4, U5.

## Not in scope
Upgrading a stock card in place (`do-release-upgrade`): it replaces Anbernic's tuned packages,
breaks RetroArch (why `tools/extras.py` refuses upgrades) and can't be undone from the device.
