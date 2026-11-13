#ifndef REDUX_BOOTSCREEN_H
#define REDUX_BOOTSCREEN_H

/* Boot Screen manager (Settings > System > Boot Screen). Dale-only: it writes to the
   boot-resource partition (mmcblk0p2), which u-boot reads the boot logo from.

   The stock logo is /bootlogo.bmp there: 640x480, 24-bit, uncompressed, bottom-up, 54-byte
   header, 921654 bytes. That FAT16 filesystem claims 128 MiB inside a 32 MiB partition, so
   this code never creates, renames, truncates or deletes anything on it. It only overwrites
   the existing file IN PLACE with a file of exactly the same size: same clusters, no FAT or
   directory allocation. Both files start with the same 54 header bytes, so a write cut off by
   power loss still leaves a valid BMP (a mix of the two pictures).

   Order: check the new file's format -> mount -> check the file on the partition is the
   expected size and format (else refuse) -> first time only, save it as the backup and verify
   the copy -> overwrite in place, fsync -> read back and compare (on a mismatch, write the old
   bytes back) -> sync, unmount. Not run on the device yet. */

#define BOOTSCREEN_SIZE 921654L

#define BOOTSCREEN_MOUNT_CMD \
    "mkdir -p /tmp/redux-bootres && mount -t vfat -o rw,noatime /dev/mmcblk0p2 /tmp/redux-bootres"
#define BOOTSCREEN_UMOUNT_CMD "sync; umount /tmp/redux-bootres"

enum {
    BOOTSCREEN_OK = 0,
    BOOTSCREEN_E_MOUNT = -1,     /* could not mount the partition; nothing written */
    BOOTSCREEN_E_SOURCE = -2,    /* Redux logo not deployed / no backup, or wrong format */
    BOOTSCREEN_E_TARGET = -3,    /* the logo on the partition is not the expected file; refused */
    BOOTSCREEN_E_BACKUP = -4,    /* could not save or verify the backup; nothing written */
    BOOTSCREEN_E_WRITE = -5,     /* write or read-back failed; the old bytes were written back */
    BOOTSCREEN_E_UMOUNT = -6     /* written and verified, but the unmount failed */
};

typedef int (*bootscreen_run_fn)(const char *command);

/* Tests replace the mount/unmount exec. NULL = the real one (stock builds only; a desktop
   build without a hook fails at the mount step and touches nothing). */
void bootscreen_set_run(bootscreen_run_fn fn);

/* Tests only: mount point directory, the Redux logo, the backup file, the state marker.
   NULL keeps the current value. */
void bootscreen_set_paths(const char *mount_dir, const char *redux_logo,
                          const char *backup, const char *marker);

/* 1 if the file is exactly the stock boot-logo format. */
int bootscreen_valid(const char *path);

/* 1 if Redux last installed its own logo (the marker), 0 if stock. */
int bootscreen_is_redux(void);

/* Put the Redux logo / the saved stock logo on the partition. BOOTSCREEN_* result. */
int bootscreen_install(void);
int bootscreen_restore(void);

/* Short text for a result, for the screen. */
const char *bootscreen_message(int rc);

#endif
