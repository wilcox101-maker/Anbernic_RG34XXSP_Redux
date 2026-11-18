# frontend/osk: the Redux on-screen keyboard

| File | What |
|---|---|
| `osk_draw.{c,h}` | Keyboard UI A drawing (framed caps, BIOS/DOS panel) into an ARGB8888 buffer; used by Redux Terminal and `redux-osk` |
| `osk_disp`, `osk_in`, `osk_kbd`, `osk_sys`, `sunxi_disp2_abi.h` | osklib from H700 Kernel Research KR-2/KR-3 (H700 PR #63, `e4a6516`, GPL-2.0; the ABI header follows Allwinner's GPL-2.0 BSP `sunxi_display2.h`). Redux changes: chord Menu + Select, no Shift button, L1/R1 page events |
| `osk_core.{c,h}` | The daemon's decisions (open, refuse over the Redux menu or RetroArch, close, type) without device access |
| `redux_osk.c` | `redux-osk`: the background program (`--off` clears the layer) |

The layout itself is `frontend/term/osk.c` (three pages: abc, 123, Fn). Evidence for the display and input
paths: H700 `docs/r26-global-osk-stock-disp2.md` and `docs/dr-osk-desk-checks-2026-11-18.md`. Nothing here has
run on the device yet; the K24-K28 probe session checks the assumptions.
