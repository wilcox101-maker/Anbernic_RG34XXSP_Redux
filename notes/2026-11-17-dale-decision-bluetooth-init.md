---
type: decision
from: dale
to: all
date: 2026-11-17
re: Bluetooth chip never started under Redux (Redux Manager finding, passed on by Dale 15:35; "A" 15:36)
status: open
---

# Decision: Bluetooth chip init at start, on by default with a switch (A)

**Finding (Redux Manager, 15:35, relayed by Dale):** under Redux there is no `hci0` adapter and no
`rtk_hciattach`; the Realtek chip on `ttyS1` only comes up after
`/mnt/vendor/ctrl/setBluetooth.sh init`, which the stock menu runs at boot and Redux never did.
The 2026-11-10 Bluetooth result passed with the stock menu running. Source named: the H700 repo's
`docs/stock-re-boot-and-userspace.md` lines 529, 631, 787. Checked here: nothing in the Redux
repo runs that script. Dale's `strings` of stock `dmenu.bin` (15:38) lists the scripts it calls;
`setBluetooth.sh` is the only boot-time one Redux dropped (setHostapd/setSSH are switches, mmc*.sh
card hot-plug, no NTP in dmenu.bin); `cpu_setting.sh` still to read.

**A (chosen):** Redux runs `setBluetooth.sh init` once per boot when `hci0` is missing (in the
background), like stock, and **Settings > Bluetooth Radio** On/Off (default On; Off = `hciconfig
hci0 down` now and no init at the next start; saved in `/mnt/data/redux/bluetooth`). B (off by
default) and C (always on, no switch) not taken.

Until a Redux build with this is installed, the same script works by hand over SSH:
`/mnt/vendor/ctrl/setBluetooth.sh init; sleep 3; hciconfig hci0` (lasts until restart).
