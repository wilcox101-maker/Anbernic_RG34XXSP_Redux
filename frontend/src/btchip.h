#ifndef REDUX_BTCHIP_H
#define REDUX_BTCHIP_H

/* Bluetooth chip at start (Dale 2026-11-17 A, notes/2026-11-17-dale-decision-bluetooth-init.md).
   The Realtek controller hangs off a serial line and only exists as hci0 after
   /mnt/vendor/ctrl/setBluetooth.sh init, which the stock menu runs at boot and Redux did not
   (Redux Manager, 2026-11-17: no hci0, no rtk_hciattach under Redux). Redux runs the same script
   once per boot unless Settings > Bluetooth Radio is Off. The choice is Redux's own file
   (/mnt/data/redux/bluetooth: "0" off; missing = on, stock parity). Nothing else on the device
   changes. */

#define BTCHIP_INIT "/mnt/vendor/ctrl/setBluetooth.sh init"
#define BTCHIP_HCI  "/sys/class/bluetooth/hci0"

int  btchip_enabled(void);
/* Save On/Off. On: runs the init now if hci0 is missing. Off: hciconfig hci0 down now, and no
   init at the next start. 0 ok, -1 could not save, -2 saved but the command failed. */
int  btchip_set(int on);
/* At menu start: the init (in the background) when enabled and hci0 is missing. 1 if started. */
int  btchip_at_start(void);

/* Tests: setting file, hci0 probe path and a runner instead of system(). NULL = defaults. */
typedef int (*btchip_run_fn)(const char *command);
void btchip_set_paths(const char *setting, const char *hci);
void btchip_set_run(btchip_run_fn fn);

#endif
