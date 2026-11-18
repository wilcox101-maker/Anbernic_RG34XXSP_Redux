---
type: question
from: claude
to: dale
date: 2026-11-18
re: MGR for DALE letter FROM-REDUX-MANAGER-2026-11-18-b, Dale 11:00 GO on the global on-screen keyboard (OSK); plan only
status: acknowledged (Dale 11:23: A A B B A A; decision 2026-11-18-dale-decision-global-osk.md)
---

# Plan: a global on-screen keyboard over any app (plan only, no code yet)

Source: H700 R26 [`docs/r26-global-osk-stock-disp2.md`](https://github.com/wilcox101-maker/Allwinner-H700-Custom-Kernel-Research/blob/152ed504730ebd01f88af3918057bcc6fedc2448/docs/r26-global-osk-stock-disp2.md)
(H700 PR #61 head `152ed50`, verdict GO, desk only). Dale's rule (10:50): this is the top Redux task, and all other
development and device testing waits for it. Once it is built, every open task and the device-test queue (incl. the
`52fd6ba`/`2a0e1a5` sheet) is re-checked against it and the changes reported. **Nothing below has run on the device.**
Each decision gives options; **A** is the recommendation.

## What gets built
`redux-osk`, a small background program. When the user opens it, it draws a keyboard on its own display layer (disp0
**ch3/l0**, zorder 30, ARGB8888, own ION system-heap buffer, `DISP_LAYER_SET_CONFIG2`) above whatever app is running,
reads the pad while open (`EVIOCGRAB` on `ANBERNIC-keys`, event1 only), and types into the app through a `/dev/uinput`
keyboard. While closed it holds no grab, commits nothing and sleeps in `poll()`. Hard rules: nothing on p1, no boot
chain, no kernel change, stock OPP untouched. It never writes `nds_esckey`.

## D1. Daemon: language and lifecycle
- **A (recommended): C, aarch64, embedded in `dmenu.bin`** like the heat guard (written to `/tmp/redux-osk`, double
  fork, outlives Redux between app launches; pid file `/tmp/redux-osk.pid`, so a Redux restart never starts a second
  copy). Built by the existing native arm64 CI job; the struct layout carries R26's `_Static_assert`s (224-byte
  `disp_layer_config2`), header from the GPL-2.0 BSP (Redux is GPL-2.0, so the licence fits).
  - **Start:** at Redux start, when the switch (D2) is On and no live pid.
  - **Stop:** SIGTERM/SIGINT/exit handlers hide the layer and release the grab. **One-shot "ch3 off":**
    `redux-osk --off` (a single `SET_CONFIG2` with `enable = 0` on ch3/l0, then exit), run (1) by Redux at every start,
    before the daemon check, which clears a layer left by a crash (`disp_release` does nothing, R26 §1.1); (2) when the
    switch goes Off; (3) before Restart / Power Off.
  - **Hides itself** on lid close / power-key sleep (event0, read without a grab, like the lid watcher), on an HDMI
    state change (then re-reads the screen size, ioctls 0x07/0x08, and recomputes the rectangle), and never opens over
    the Redux menu (Redux writes `/tmp/redux-menu.up` while its menu runs and removes it before launching an app).
- **B:** a shell script around a small helper; more moving parts, no gain.

## D2. Toggle
- **A (recommended): Settings > On-screen Keyboard: Off / On** (a row beside Bluetooth Radio), saved like the other
  switches. **Default Off** until the K26 probe passes, then default On in a follow-up PR.
- **B:** an Apps row that switches it (a second place to look; the Settings row can come later).
- **C:** both.

## D3. Opening it, and keys while open
- **A (recommended):** **Menu + Select held 0.5 s** opens and closes it, acting on the second key while Menu is held
  (R26 §3.1); **Menu + X** as the fallback if K27 shows a clash. While open: D-pad moves, A types, B or the chord closes,
  Select = Shift (one press). The rest of the pad is the app's again the moment it closes.
- **B:** Menu + X as the main chord.
- Known side effect: while it is open, Menu + Vol brightness and volume do nothing (the grab hides the pad from
  `portsCtrl`/`ndsCtrl`); power and lid keep working.

## D4. Layout and drawing (reuse)
- Layout, movement and the Shift/Ctrl latch come straight from `frontend/term/osk.c`; it is split into a shared part
  (`frontend/osk/`) used by both the terminal and the daemon. New: a label → (Linux keycode, Shift) table for a US
  layout, so each key becomes uinput events instead of pty bytes; Ctrl becomes a real `KEY_LEFTCTRL` press.
- Drawing: software into the ION buffer (no SDL; SDL would take fb0), with Redux's 8x16 fonts (`font.c`) and the saved
  accent / window colours from Settings > UI (`uistyle.c`). The bottom 720 x 200 band by default; commit only when the
  image changes, double buffer to avoid tearing.
- RetroArch danger keys (Esc quits the emulator, F1 opens its menu; R26 §3.4): **A:** drawn in red and need a 1 s hold
  while RetroArch runs; **B:** left off the keyboard while RetroArch runs.

## D5. First device step: the probe session (K24-K28), read-only, RAM-only
Before any daemon code: `redux-oskprobe`, an aarch64 static test tool built by CI (not embedded, not installed). Dale
copies it over SSH to `/tmp` (RAM, gone at reboot), runs it there and pastes the output. Nothing is written to any card
or partition; the only thing it changes is the ch3 layer, which `--off` clears.
| Step | Command | Answers |
|---|---|---|
| K24 | `oskprobe layers` while the menu, an SDL2 port, RetroArch, the video player and DraStic run (`/sys/class/disp/disp/attr/sys`, plus a list of that folder) | is ch3 (or ch2) free; ch0 for video |
| K25 | `oskprobe nodes` (`/dev/uinput`, `/dev/ion`, `/dev/disp`, `/sys/class/misc/uinput/dev`) | uinput present and openable |
| K26 | `oskprobe bar 10` over each app: a half-transparent bar on ch3/l0, zorder 30; during it lid close/open, power sleep/resume, Menu + Vol, HDMI in/out; then `oskprobe bar 60 &` + `kill -9` (bar should stay) and `oskprobe off` (bar gone) | the layer works and survives as R26 predicts |
| K27 | `oskprobe keys` (codes for Menu + Select, Menu + X, L3, R3); `oskprobe type redux` (a uinput keyboard types "redux" after 5 s) into an SDL2 port, RetroArch, Redux Terminal and an SDL 1.2 app; does Menu alone open RetroArch's menu | the chord and who receives uinput keys |
| K28 | `oskprobe grab` with each K24 app running (grab `ANBERNIC-keys`, release at once) | EBUSY = that app grabs the pad |
- **A (recommended):** the probe PR is the first PR; Dale merges it (it is the first Redux code that talks to
  `/dev/disp` directly, though it writes nothing to storage), runs one batched session, results go into an hw-result
  note, and the daemon is built on what it shows (ch2 if ch3 is taken).
- **B:** skip the probe and test the finished daemon (faster if all goes well; a failure then costs a rebuild).

## D6. Redux Terminal's Select picker (8a9c2b5 FAIL 4, picker flaky by app)
- **A (recommended): fold it in.** Inside Redux Terminal the global chord opens the global OSK, and the terminal's own
  Select picker is removed once K27 shows uinput keys reach the terminal (it already handles SDL key events, so typed
  keys arrive as keys). One keyboard everywhere; the flaky Select path goes away instead of being debugged. Until then
  the terminal picker stays as it is, and `/tmp/redux-term.log` remains the way to read a failed Select.
- **B:** keep both: fix the terminal picker separately and have the daemon stay closed while Redux Terminal runs.

## Order once A/B is answered
1. Probe PR (D5), Dale merges; 2. one device session K24-K28, hw-result note; 3. daemon PR (D1, D3, D4), Dale merges
(new background service with uinput); 4. Settings switch (D2) and the Redux menu flag; 5. terminal fold (D6); 6.
re-check every STATUS item and the device-test queue against the OSK and report the changes, as Dale asked.

**Done when:** the chord opens a keyboard over a game, a port, RetroArch and Redux Terminal; keys reach the app; it
closes on lid, sleep, HDMI change and daemon death (after the next Redux start); and hidden it costs no CPU (`top`, FPS
counter with and without it).
