# Field reports: revisions and known problems

Community reports collected 2026-11-09 from muOS forums, retrohandhelds.gg, Notebookcheck and
Anbernic's site. Treat everything here as **reported, not verified** until it's confirmed on hardware.

## Hardware revisions ⚠ most important

| Revision | When | RAM | Panel | Custom firmware |
|---|---|---|---|---|
| Original | May 2025 | 2 GB | original | muOS / Knulli / ROCKNIX boot |
| "Metallic Blue" era | ~May 2026 | reported as 1 GB on the product page; one teardown found a single **512 MB Samsung LPDDR3** chip (K4E4E324EE) | reportedly a different panel vendor and driver | muOS: black screen with menu sounds. Knulli and ROCKNIX: don't boot |

- Firmware `rg34xxsp-fw-sd_202505` (our reference) predates the change, so it targets the **original 2 GB** board.
- ROCKNIX's July 2026 H700 release ships separate **DDR3 and DDR4** images, which matches a
  memory-type change on some H700 boards. Mainline u-boot needs the DRAM type at build time.
- A later StockOS-Mod release (v3.9.0, Dec 2025) says the newer stock OS changed underlying drivers
  to support new panel models.
- An incompatible image can leave the device stuck in **FEL mode**, with no light, no screen and no
  sound. It recovers by flashing a compatible card.
- **Risk:** Knulli warned that running a CFW with the wrong panel driver on revised RG40XX units
  could damage the screen. **Our firmware must identify the panel before initializing it, and must
  never drive an unknown panel.**

### What this means for the project
- [ ] Get a stock image from a revised unit and diff its DTB, env and boot0 against `stock-202505`
- [ ] Find a runtime way to tell boards apart (DRAM size and type from boot0/u-boot, panel ID over the panel bus)
- [ ] Phase 1 patches only apply to the original board's `boot.img`, and must not be flashed to a revised unit

## Lid / sleep: the most common complaint

The stock kernel handles the hall sensor inside Anbernic's PMIC power-key driver. Stock boot logs show
`axp20x_pek_probe ... wakup_irq`, `hall_switch_isr` and `hall_press_work_func: state=1`.

Reported problems (mostly muOS, on the stock BSP kernel):
- Closing the lid stops triggering sleep after a reset, and the power button won't sleep the device while the lid is closed
- Opening the lid returns to the menu with a "reset button pressed" warning instead of resuming the game
- Wake-from-sleep failures specific to the 34XXSP (muOS 2601.1)
- Closing the lid during shutdown leaves the device half-on, with heavy battery drain
- Auto-save states load an older state after sleep then shutdown
- A faulty or misaligned hall sensor makes the device think the lid is closed at boot (it goes straight to sleep)
- muOS moved to real kernel suspend for power savings, but reportedly can't wake from lid-open

Stock behavior, per Anbernic: closing the lid sleeps, and **waking requires pressing power after
opening the lid**. Lid-open is not a wake source on stock.

### Design targets that follow
- Make lid-open a real wake source (PE7 interrupt as the wakeup IRQ), with no power-button press needed
- Suspend sequence: pause emulator → flush save/state → `sync` → suspend. Resume to the same frame
- Debounce the lid, and ignore it during boot and shutdown
- Fall back gracefully if the sensor reads "closed" at boot (likely a misaligned magnet)

## Other reports
- Battery percentage is unreliable after sleep (fuel-gauge reporting, not real drain, per muOS devs). Use the AXP2202 calibration data properly
- A faint flicker on some 1 GB units, "like 50 Hz on 60 Hz". That could be a panel timing mismatch on the new panel
- The screen is physically 4:3 behind the bezel (the visible area is 3:2)

## Dale's unit (first-hand)

- 2 GB RAM, confirmed in software (original revision)
- Otherwise stock: no hardware mods. All measurements and limits use stock hardware.

## Sources
- muOS community: lid/sleep threads 735, 1177, 1340, 1394, 213, 1036, 342; device quirks page
- retrohandhelds.gg: "RG34XXSP might've been quietly downgraded again" (May 2026) and the follow-up on CFW failures
- Notebookcheck: "Anbernic silent downgrade revealed for popular clamshell gaming handheld"
- Anbernic download page (sleep/wake note)
- ROCKNIX release 20260701 (DDR3/DDR4 H700 images)
