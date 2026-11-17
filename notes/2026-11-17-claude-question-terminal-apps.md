---
type: question
from: claude
to: dale
date: 2026-11-17
re: Dale 12:54-12:57: terminal replacement (fuzzy fonts) and terminal versions of Music, Video, Bluetooth, Documents
status: acknowledged (Dale 13:06: terminal A; apt installer A; 13:57: PR split A, three PRs)
---

# Plan: a Redux terminal, then terminal apps

Dale's requests: replace SimpleTerminal (text is fuzzy); find terminal versions of the Music
Player, Video Player, Bluetooth and Document viewer. Done already: NeoFetch in Apps (no logo, so it
fits), File Manager = `mc`.

## Terminal (decision asked)
- **A (recommended): Redux Terminal.** A small program built with Redux (SDL2, a pty, a VT100/xterm
  subset) that draws with Redux's own 8x16 fonts on the same 90 x 30 grid, pixel for pixel, so text
  is as sharp as the menu. Buttons map to keys per app (D-pad = arrows, A = Enter, B = Esc, X = Tab,
  L1/R1 = PgUp/PgDn, Select = an on-screen key picker), and M quits. **Mouse** (Dale 12:59): the
  right stick moves a cursor, L3 = left click, R3 = right click, sent to the app as xterm mouse
  events (mc, htop and alsamixer take mouse clicks). mc, htop, alsamixer, nmtui,
  neofetch and the apps below then run in it. Nothing writes to the device's boot chain.
  **PR split (Dale A, 13:57):** 1. the terminal with a shell as a new Apps row (`frontend/term`,
  embedded in dmenu.bin like the heat guard; D-pad/left stick arrows, A Enter, B Esc, X Tab, Y
  Backspace, L1/R1 PgUp/PgDn, L2/R2 Home/End, Start F10, M quits); 2. the mouse and the Select
  on-screen keys; 3. the apps move onto it and SimpleTerminal retires. Y, L2/R2 and Start are
  Claude's additions to Dale's list (they clash with nothing); Dale can veto them in the PR-1 test.
  Shipped: #275 (PR 1), #276 (PR 2), PR 3 (apps moved, SimpleTerminal retired). Device test pending.
- **B:** keep SimpleTerminal and only tune what can be tuned from outside (it is a stock binary).
  Likely little gain on the font.
- **C:** run the apps on the Linux text console with a sharp console font. Needs a key mapper for
  the buttons and changes how apps start; more risk for less control.

## Terminal app candidates (Ubuntu 22.04 arm64; Dale installs with tools/extras.py, then tries them)
| Need | Candidate | Notes |
|---|---|---|
| Music | `cmus` (universe) or `moc` (`mocp`) | ALSA output; cmus has a file browser |
| Video | `mpv --vo=drm` | draws straight to the screen, not text; needs a device test |
| Bluetooth | `bluetoothctl` (stock, already in Settings) | the TUI `bluetuith` is not in the 22.04 archive |
| Documents | to find | text files: `less`; PDF/EPUB: candidates to check on the device |

Not verified on the device: everything in this table except bluetoothctl.
