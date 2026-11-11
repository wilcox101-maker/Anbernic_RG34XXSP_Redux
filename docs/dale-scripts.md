# Dale's scripts (copy/paste)

Numbered so Claude or Grok can say "run S3". **S** = on the device (SSH session), **P** = on the PC
(PowerShell). Everything here is read-only unless it says **changes**. None of it touches the Roms
partition except S10, which only lists it.

Set your device IP once per PowerShell window: `$ip = "172.16.3.175"` (change to the current one).

---

## Connect and power

**P1: connect to the device**
```
ssh root@$ip
```

**S1: show the device IP** (also sets English messages for this session)
```
export LANG=C; hostname -I
```

**S2: reboot** (changes: restarts)
```
sync; reboot
```

**S3: shut down** (changes: powers off; hold power ~1.5 s to turn on)
```
sync; poweroff
```

---

## Redux menu

**P2: upload a new Redux build.** Run it in the folder holding the unzipped `dmenu.bin` from a CI run's Artifacts. Then run S4.
```
scp dmenu.bin root@${ip}:/mnt/vendor/redux/dmenu.bin.new
```
(WinSCP: upload `dmenu.bin` to `/mnt/vendor/redux/` and rename it to `dmenu.bin.new`.)

**S4: install the uploaded build and reboot** (changes: replaces the Redux binary)
```
chmod +x /mnt/vendor/redux/dmenu.bin.new && mv /mnt/vendor/redux/dmenu.bin.new /mnt/vendor/redux/dmenu.bin && sync && reboot
```

**S5: Redux log, last 30 lines**
```
tail -30 /mnt/vendor/redux/redux.log
```

**S6: clear the Redux log** (changes: empties the log)
```
: > /mnt/vendor/redux/redux.log
```

**S7: back to the STOCK menu** (changes: restores the stock launcher, reboots)
```
cp /mnt/vendor/ctrl/dmenu_ln.bak /mnt/vendor/ctrl/dmenu_ln && sync && reboot
```

**S8: back to the REDUX menu** (changes: switches the launcher to Redux with logging, reboots)
```
grep -q 'CMD="/mnt/vendor/redux/run.sh"' /mnt/vendor/ctrl/dmenu_ln || sed -i 's|^CMD="/mnt/vendor/bin/dmenu.bin"|CMD="/mnt/vendor/redux/run.sh"|' /mnt/vendor/ctrl/dmenu_ln; grep '^CMD=' /mnt/vendor/ctrl/dmenu_ln; sync; reboot
```

**S9: which menu is set to run?**
```
grep '^CMD=' /mnt/vendor/ctrl/dmenu_ln; ls -l /mnt/vendor/redux/
```

---

## Information for Claude and Grok (paste the output)

**S10: ROM folders** (lists only)
```
ls /mnt/mmc/Roms
```

**S11: full info bundle** (ROM folders, boot logo, backlight, Wi-Fi, compilers)
```
export LANG=C
echo "== ROM folders";    ls /mnt/mmc/Roms
echo "== boot logo";      mount | grep -i mmcblk; find / -xdev \( -iname "*logo*" -o -iname "*bootlogo*" \) 2>/dev/null | grep -v proc | head -20
echo "== boot partition"; ls -la /dev/mmcblk0p4 2>/dev/null; ls /boot 2>/dev/null | head
echo "== backlight";      for d in /sys/class/backlight/*; do echo $d; cat $d/max_brightness $d/brightness; done
echo "== wifi config";    ls -la /etc/wpa_supplicant* /etc/NetworkManager/system-connections 2>/dev/null; ls /mnt/data 2>/dev/null | head
echo "== wifi now";       ip -brief addr; iw dev 2>/dev/null | head -8
echo "== compilers";      which gcc cc cmake make pkg-config 2>&1; gcc --version 2>/dev/null | head -1
```

**S12: battery and temperature snapshot**
```
B=/sys/class/power_supply/axp2202-battery; echo "$(date +%H:%M) cap=$(cat $B/capacity)% mV=$(( $(cat $B/voltage_now)/1000 )) status=$(cat $B/status) soc=$(( $(cat /sys/class/thermal/thermal_zone0/temp)/1000 ))C"
```
Idle-drain test: run S12, leave the device 30 min, run S12 again, paste both lines.

**S13: CPU snapshot**
```
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor; for c in 0 1 2 3; do echo "cpu$c $(( $(cat /sys/devices/system/cpu/cpu$c/cpufreq/scaling_cur_freq)/1000 )) MHz"; done
```

**S14: what's using the CPU**
```
top -b -n 1 | head -15
```

**S15: lid sensor** (1 = open, 0 = closed)
```
cat /sys/class/power_supply/axp2202-battery/hallkey
```

**S16: is a button or stick sending events?** (press buttons during the 5 s; 0 = nothing)
```
timeout 5 cat /dev/input/event1 | wc -c
```

---

## PC side, optional

**P3: render the boot screen** (needs the repo and Python; writes `build/bootlogo.bmp`)
```
python tools/make_bootlogo.py
```

**P4: get or update the repo**
```
git clone https://github.com/wilcox101-maker/Anbernic_RG34XXSP_Redux; cd Anbernic_RG34XXSP_Redux; git pull
```

---

## Saving Claude usage: how we work

1. **Report by number:** "S10 + S12:" followed by the pasted text. Text costs far less than photos or videos. Use a
   photo only when the screen itself is the question (layout, colours).
2. **Batch:** run the scripts you were asked for, then paste everything in **one** message.
3. **No PING between steps.** GitHub wakes Claude when a PR opens, CI finishes, or someone
   comments on the inbox (#41). PING only when you've been waiting over 30 min, or before you stop for the day.
4. **CI is the first reviewer.** Claude reviews only green PRs; a red one goes back to Grok
   through the inbox automatically.
5. **Grok builds, Claude reviews.** Claude writes code only when that's cheaper than another round
   with Grok (small fixes, device emergencies).
6. **Decisions as letters:** Claude offers A/B/C; reply with the letter plus any change.
7. **Long or repeating output:** add `| tail -30`.
8. **Usage over 85% with work pending:** stop and wait for the reset. Use the free reset only for
   a device test that can't wait.
