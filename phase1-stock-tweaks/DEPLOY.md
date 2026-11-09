# Phase 1 deployment and test plan

**Target:** the original 2 GB RG34XXSP, firmware `rg34xxsp-fw-sd_202505`.
**Risk:** low. Only the 64 MiB `boot` partition on a *spare* card changes, and the stock card is never touched.

## 0. What you need

- A **spare microSD card at least as large as the stock image.** `RG34XXSP.img` includes the ~44 GiB
  ROM partition, so 64 GB is the minimum
- `RG34XXSP.img` (stock, already downloaded) and `boot-redux-p1.img`
- Python 3 on Windows (or WSL), plus Rufus or balenaEtcher
- About 2.5 hours, mostly unattended battery tests

## 1. Check the inputs

```powershell
certutil -hashfile boot-redux-p1.img SHA256
# expect 3d176f7f796722aeedfa1d9f18ff89ddf0789ad95a75c3086b00f6670b238982
```

## 2. Baseline on stock, using the same spare card

Testing stock and patched on the **same card** isolates the change from card speed.

1. Flash the unmodified `RG34XXSP.img` to the spare card with Rufus or Etcher.
2. Boot it in the device (TF1 slot). Charge to 100%.
3. Record the **Stock** column in section 5.

## 3. Make the patched image (no raw disk access needed)

```powershell
copy RG34XXSP.img RG34XXSP-redux.img
python phase1-stock-tweaks\patch_image.py RG34XXSP-redux.img boot-redux-p1.img
```

The patcher checks for exactly one `boot` partition of 64 MiB, confirms it currently holds the
**known stock** boot image, writes the new image, then reads it back and verifies the hash. If any
check fails, it stops without writing.

## 4. Flash and boot

1. Flash `RG34XXSP-redux.img` to the same spare card.
2. Insert it and power on.

| What you see | Meaning | Action |
|---|---|---|
| Normal boot logo, then menu | ✅ | Go to section 5 |
| Boot logo, then a stall longer than 60 s | The kernel or init is not reaching userspace | Power off and go to section 6 |
| Nothing at all (no LED, no logo) | Not expected: the bootloader is untouched | Re-flash the card. If that doesn't help, boot the **stock card** to confirm the device is fine |

## 5. Measurements

Charge to 100% before each battery test, use the same brightness and volume, and turn Wi-Fi off.

| # | Test | How | Stock | Redux |
|---|---|---|---|---|
| 1 | Boot time | power-on → menu usable, stopwatch, average of 5 | | |
| 2 | Idle drain | 60 min sitting in the menu, screen on, % lost | | |
| 3 | Game drain | 60 min of one GBA game, % lost | | |
| 4 | Warmth | back of the device after test 3: cool / warm / hot | | |
| 5 | Button feel | D-pad and A in a precise platformer; note any difference | | |
| 6 | Button latency *(optional)* | 240 fps phone video: frames from press to on-screen response, median of 10 | | |
| 7 | Lid close/open | 10 cycles: does sleep and wake still behave exactly like stock? | | |
| 8 | Stability | 30 min of normal use: any freeze or crash? | | |

**How to read the results:**
- Tests 2 and 3 show the **governor** change. The Redux column should drain less, especially at idle.
  If idle drain is the **same** as stock, Anbernic's userspace is probably setting `performance`
  again after boot. That's useful to know, and the fix moves into the rootfs as Phase 1b.
- Tests 5 and 6 show the **poll-interval** change. Expect up to about 15 ms less worst-case lag.
- Test 7 must match stock. Phase 1 doesn't touch the lid.

**Optional shell check**, if you can get a terminal (stock "Modify System Tools", or SSH via the StockOS-Mod):
```sh
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor   # expect: schedutil
grep " / " /proc/mounts                                      # expect: barrier=1 (or no barrier=0)
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq   # should drop at idle
```

## 6. Rollback

Any of these works:
- **Quickest:** use the stock card. It was never modified.
- **Same card:** flash the original `RG34XXSP.img` again.
- **Patch back:** `python patch_image.py RG34XXSP-redux.img 3.boot.img --restore`, then flash.

## 7. Report back

Fill in the table above and add any notes. Commit it as `phase1-stock-tweaks/RESULTS.md`, or paste it
into the chat. The results decide whether we try a 1–2 ms poll interval and whether a Phase 1b
(rootfs-level governor) is needed before moving to Phase 2.
