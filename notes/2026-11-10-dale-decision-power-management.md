---
type: decision
from: dale (recorded by claude)
to: grok, claude
date: 2026-11-10
re: Settings → Power & Lid, menu power management (docs/ui-spec.md, frontend)
status: open
---

# Decision: menu power management (dimming, screen off, Wi-Fi off by default)

Context: on Dale's unit, 100% → 87% in ~30 min idle in the Redux menu. Redux itself was ~0% CPU
(`top`), CPU at 480 MHz. The backlight at full and Wi-Fi left on are the main idle costs, plus the
fuel gauge settling after unplugging. Dale: power management is needed.

## Settings → Power & Lid (defaults; all user-changeable)
| Setting | Default | Notes |
|---|---|---|
| Dim after idle | 60 s, to 30 % brightness | any button restores |
| Screen off after idle | 3 min | backlight off, menu keeps running; any button wakes |
| Sleep after idle | 10 min | stock path (`pwr_new.sh auto`, as lid/timeout) |
| Brightness | remembered across boots | hotkey Menu + Up/Down |
| Wi-Fi | **off unless needed** | on while Internet/Network is in use, or when the user turns it on |
| Lid close | sleep | matches stock |

- These timers run only in the menu; emulators own the screen while a game runs.
- Respect the stock `system.cfg` keys where they exist (`power.lock`, `power.key`, `power.hdmi`)
  rather than inventing parallel ones.
- Unverified: the backlight sysfs path and range (stock DT: PWM0, max 255) and how Wi-Fi is switched
  on stock (rfkill vs. unloading `8821cs.ko`). Read both on the device before coding, and note them in
  `docs/hardware-map.md`.

## Order for Grok
After the F2 items (ROM list first, then the Battery/CPU readings, the tabs/back stack/Internet menu,
then text size). One PR for the Power & Lid page plus dim/screen-off/sleep timers; Wi-Fi switching in
its own PR (it touches drivers/rfkill). Evidence per AGENTS.md.

Phase 2 (mainline kernel), not now: CPU idle states and Wi-Fi driver power saving.
