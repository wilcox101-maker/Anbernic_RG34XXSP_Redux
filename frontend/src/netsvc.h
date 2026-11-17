#ifndef REDUX_NETSVC_H
#define REDUX_NETSVC_H

/* SSH and SMB on/off in Settings (Dale 2026-11-17 A; security item, Dale merges).
   The stock way: the StockOS Mod keeps the switches in /mnt/mod/ctrl/configs/system.cfg
   (global.ssh, global.samba) and its autostart applies them at boot with
   handle_service "global.samba" "smbd" / "nmbd" and handle_service "global.ssh" "ssh"
   (read on Dale's unit, 2026-11-17). Redux sets the same key, then starts or stops the same
   services at once, so the choice holds after a reboot too. Only that one line of system.cfg
   changes (written to a .new file, fsync, rename). */

enum { NETSVC_SSH, NETSVC_SMB, NETSVC_COUNT };

#define NETSVC_CFG "/mnt/mod/ctrl/configs/system.cfg"

/* 1 on, 0 off (key missing or not 1). */
int netsvc_get(int which);

/* Set the key, then start or stop the services. 0 ok; -1 system.cfg not changed (nothing run);
   -2 saved, but a service command failed. */
int netsvc_set(int which, int on);

const char *netsvc_name(int which);        /* "SSH", "SMB (Samba)" */

/* Tests: another system.cfg, and a runner instead of system(). NULL restores the defaults.
   Desktop builds never run systemctl: without a runner they only log the command. */
typedef int (*netsvc_run_fn)(const char *command);
void netsvc_set_path(const char *path);
void netsvc_set_run(netsvc_run_fn fn);

#endif
