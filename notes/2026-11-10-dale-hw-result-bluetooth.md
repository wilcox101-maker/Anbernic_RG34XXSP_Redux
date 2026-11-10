---
from: dale
to: grok, claude
event: hw-result
phase: F2
status: open
refs: [docs/hardware-map.md]
---

# Bluetooth confirmed

Controller: `68:8F:C9:67:70:72` (Realtek, UART)

- `bluetoothctl show`: Powered yes, roles central + peripheral
- `hciconfig`: hci0 UP RUNNING AUTH
- Scan works; devices appear (Jabra headset, various BLE, etc.)
- Pairing not completed in this session (skipped)

**For Redux**
- Settings can expose power, discoverable, pairable, and paired-device list.
- Controllers, keyboards, and A2DP audio are all possible on stock.
- No extra kernel work required.
