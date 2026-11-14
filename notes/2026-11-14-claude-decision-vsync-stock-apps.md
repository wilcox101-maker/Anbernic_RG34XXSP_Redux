---
type: decision
from: claude
to: dale
date: 2026-11-14
re: backlog, tearing in stock SimpleTerminal and File Manager (#41 2026-11-11, #144)
status: open
---

# Decision: what to do about tearing in SimpleTerminal and File Manager

Asked by Dale via the Manager relay (`MGR for DALE: DO backlog …`, 2026-11-14 08:24). Plan only; no code.

## What we know
- **Symptom:** SimpleTerminal tears when moving between keyboard rows; File Manager tears while
  loading (Dale, 2026-11-11 and 2026-11-13). Both pass otherwise.
- **Cause:** the menu's SDL is stock `/usr/lib/libSDL2-2.0.so.0` **2.0.12** (Anbernic's build, first
  in `LD_LIBRARY_PATH`), which presents without waiting for vsync. `SDL_RENDER_VSYNC=1` made no
  difference on the device. Redux avoids it by redrawing and presenting each frame 3 times
  (`PRESENT_BUFFERS`); the stock apps don't.
- **Both apps are open source:** SimpleTerminal comes from the StockOS Mod (`/mnt/mod/ctrl`) and is
  st-sdl (MIT). File Manager is `dinguxCommand_en.dge`, DinguxCommander (GPL-3.0).
- **Not known yet:** whether the display driver answers `FBIO_WAITFORVSYNC` on `/dev/fb0`, and
  whether stock SDL draws through fbdev or the Mali driver. A one-minute read-only check answers the first.

## Options

**A. Rebuild both from source with vsync-safe presenting.**
Port st-sdl and DinguxCommander, build them in CI for arm64, present each frame the way Redux does,
install as Redux's own copies under `/mnt/vendor/redux/apps/` (stock files untouched).
- For: fixes the cause; we control the apps afterwards (keys, fonts, colours).
- Against: two ports to own and keep working (DinguxCommander is a few thousand lines); GPL-3 for the
  file manager; each needs its own device testing. Days of work for a cosmetic fault.

**B. A small vsync shim, loaded only when Redux launches these two apps.**
A tiny library (`LD_PRELOAD`) that wraps `SDL_RenderPresent` and waits for the next vblank
(`FBIO_WAITFORVSYNC`) before passing the frame on. Redux sets it only on the SimpleTerminal and
File Manager launch lines; stock's own menu is unaffected.
- For: one short C file, no stock code changed, off again by removing one launch variable.
- Against: works only if the driver supports the vblank wait (the check above); may cost a few ms of
  input delay per frame; a shim inside a stock binary is less predictable than our own build.

**C. Leave it as stock.**
Record it as a known Stock OS cosmetic bug. It goes away on the mainline kernel (1.0), where KMS gives
real vsync to every app.
- For: no work, no risk, nothing to maintain before Beta.
- Against: the tearing stays until mainline.

## Recommendation: B, time-boxed, with C as the fallback
Run the read-only vblank check in the next device session. If `FBIO_WAITFORVSYNC` works, build the
shim (one PR, one device test). If it doesn't, or the shim causes lag, drop to **C** and close the
item. **A** is too much work before Beta for a cosmetic fault; it only makes sense later if we want
our own terminal or file manager for other reasons.

## Not verified on device
Everything here beyond Dale's two tearing reports.
