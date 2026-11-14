---
from: claude
to: all
event: correction
phase: F3
status: open
refs: [notes/2026-11-14-dale-decision-vsync-shim.md, PR #179, FROM-REDUX-READONLY-2026-11-14-e]
---

# Correction: the vsync library as merged in #179 could not work; it now covers SimpleTerminal only

Found by the read-only chat's pre-test review (FROM-REDUX-READONLY-2026-11-14-e, checked against
Dale's V1.0.6 appfs and the StockOS-Mod repo; read only):

1. **Wrong architecture.** SimpleTerminal and File Manager are 32-bit ARM (armhf, SDL 1.2). #179 embedded
   an aarch64 library, which the loader skips ("wrong ELF class"), so it never loaded. Now the device
   build embeds an **armhf** copy (CI arm64 job, `REDUX_VSYNC_EMBED`; `docs/build-arm.md`).
2. **File Manager can't be helped this way.** DinguxCommander never calls `SDL_Flip` / `SDL_UpdateRect`;
   it draws straight into the video surface. It is dropped from the preload and stays stock bug 9f.
   The [decision](2026-11-14-dale-decision-vsync-shim.md) said "SimpleTerminal and File Manager";
   that part is withdrawn.
3. **Inherited preload.** The library now unsets `LD_PRELOAD` when it loads, so bash, htop, alsamixer
   and the other 64-bit programs started from the terminal don't print loader errors.
4. **Leak after a failed launch.** `apps_launch()` clears the one-shot preload when the launch fails,
   so it can't end up on the next `.dge` or app launch (test in `test_ui_nav`).

## Not verified on device
- Whether the armhf library loads into SimpleTerminal: it needs armhf glibc 2.34 or newer (it uses
  `dlsym@GLIBC_2.34`). Check: `file /tmp/redux-vsync.so /mnt/mod/ctrl/SimpleTerminal` and
  `ls /lib/arm-linux-gnueabihf/libc.so.6`.
- Whether `/dev/fb0` answers `FBIO_WAITFORVSYNC`, and whether the tearing goes away.
