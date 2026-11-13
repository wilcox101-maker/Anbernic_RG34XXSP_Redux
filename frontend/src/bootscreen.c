#define _POSIX_C_SOURCE 200809L
#include "bootscreen.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define PATH_N 256

static char g_mount[PATH_N]  = "/tmp/redux-bootres";
static char g_redux[PATH_N]  = "/mnt/vendor/redux/bootlogo.bmp";
static char g_backup[PATH_N] = "/mnt/vendor/redux/bootlogo.stock.bmp";
static char g_marker[PATH_N] = "/mnt/data/redux/bootscreen";
static bootscreen_run_fn g_run;

void bootscreen_set_run(bootscreen_run_fn fn) { g_run = fn; }

void bootscreen_set_paths(const char *mount_dir, const char *redux_logo,
                          const char *backup, const char *marker) {
    if (mount_dir)  snprintf(g_mount, sizeof g_mount, "%s", mount_dir);
    if (redux_logo) snprintf(g_redux, sizeof g_redux, "%s", redux_logo);
    if (backup)     snprintf(g_backup, sizeof g_backup, "%s", backup);
    if (marker)     snprintf(g_marker, sizeof g_marker, "%s", marker);
}

static int run(const char *cmd) {
    if (g_run) return g_run(cmd);
#ifdef REDUX_PLATFORM_STOCK
    pid_t pid = fork();
    if (pid < 0) return -1;
    if (pid == 0) {
        execlp("sh", "sh", "-c", cmd, (char *)NULL);
        _exit(127);
    }
    int st = 0;
    if (waitpid(pid, &st, 0) < 0) return -1;
    return (WIFEXITED(st) && WEXITSTATUS(st) == 0) ? 0 : -1;
#else
    (void)cmd;
    return -1;   /* a desktop build never mounts anything */
#endif
}

static unsigned rd32(const unsigned char *p) {
    return (unsigned)p[0] | (unsigned)p[1] << 8 | (unsigned)p[2] << 16 | (unsigned)p[3] << 24;
}
static unsigned rd16(const unsigned char *p) { return (unsigned)p[0] | (unsigned)p[1] << 8; }

/* The same test as tools/make_bootlogo.py check_stock(). */
static int valid_bytes(const unsigned char *d, long n) {
    if (n != BOOTSCREEN_SIZE) return 0;
    if (d[0] != 'B' || d[1] != 'M') return 0;
    if (rd32(d + 2) != (unsigned)BOOTSCREEN_SIZE || rd32(d + 10) != 54 || rd32(d + 14) != 40) return 0;
    if (rd32(d + 18) != 640 || rd32(d + 22) != 480) return 0;      /* positive height: bottom-up */
    if (rd16(d + 26) != 1 || rd16(d + 28) != 24 || rd32(d + 30) != 0) return 0;
    return 1;
}

/* Read a file that must be exactly BOOTSCREEN_SIZE bytes. Returns a malloc'd buffer or NULL. */
static unsigned char *load(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0 || !S_ISREG(st.st_mode) || st.st_size != BOOTSCREEN_SIZE) return NULL;
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    unsigned char *buf = malloc((size_t)BOOTSCREEN_SIZE);
    if (buf && fread(buf, 1, (size_t)BOOTSCREEN_SIZE, f) != (size_t)BOOTSCREEN_SIZE) {
        free(buf);
        buf = NULL;
    }
    fclose(f);
    return buf;
}

int bootscreen_valid(const char *path) {
    unsigned char *b = load(path);
    int ok = b && valid_bytes(b, BOOTSCREEN_SIZE);
    free(b);
    return ok;
}

/* Write the whole buffer to an open fd and flush it to the card. */
static int write_all(int fd, const unsigned char *buf) {
    long done = 0;
    while (done < BOOTSCREEN_SIZE) {
        ssize_t w = write(fd, buf + done, (size_t)(BOOTSCREEN_SIZE - done));
        if (w < 0 && errno == EINTR) continue;
        if (w <= 0) return -1;
        done += w;
    }
    return fsync(fd);
}

/* Overwrite an existing same-size file in place: no O_CREAT, no O_TRUNC. */
static int overwrite_in_place(const char *path, const unsigned char *buf) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) return -1;
    int rc = write_all(fd, buf);
    if (close(fd) != 0) rc = -1;
    return rc;
}

static int same_as(const char *path, const unsigned char *buf) {
    unsigned char *got = load(path);
    int same = got && memcmp(got, buf, (size_t)BOOTSCREEN_SIZE) == 0;
    free(got);
    return same;
}

/* The backup lives on appfs (ext4), so it is written to a temp name and renamed. */
static int save_backup(const unsigned char *buf) {
    char tmp[PATH_N + 8];
    snprintf(tmp, sizeof tmp, "%s.new", g_backup);
    int fd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return -1;
    int rc = write_all(fd, buf);
    if (close(fd) != 0) rc = -1;
    if (rc != 0 || !same_as(tmp, buf) || rename(tmp, g_backup) != 0) {
        remove(tmp);
        return -1;
    }
    return 0;
}

static void set_marker(int redux) {
    if (!redux) { remove(g_marker); return; }
#ifdef REDUX_PLATFORM_STOCK
    if (mkdir("/mnt/data/redux", 0755) != 0 && errno != EEXIST) return;
#endif
    FILE *f = fopen(g_marker, "w");
    if (!f) return;
    fputs("redux\n", f);
    fclose(f);
}

int bootscreen_is_redux(void) {
    return access(g_marker, R_OK) == 0;
}

static int apply(const char *source, int make_backup) {
    unsigned char *want = load(source);
    if (!want || !valid_bytes(want, BOOTSCREEN_SIZE)) { free(want); return BOOTSCREEN_E_SOURCE; }
    if (run(BOOTSCREEN_MOUNT_CMD) != 0) { free(want); return BOOTSCREEN_E_MOUNT; }

    char target[PATH_N + 16];
    snprintf(target, sizeof target, "%s/bootlogo.bmp", g_mount);
    int rc = BOOTSCREEN_OK;
    unsigned char *old = load(target);
    if (!old || !valid_bytes(old, BOOTSCREEN_SIZE)) {
        rc = BOOTSCREEN_E_TARGET;
    } else if (make_backup && access(g_backup, F_OK) != 0 && save_backup(old) != 0) {
        rc = BOOTSCREEN_E_BACKUP;        /* the first logo found is kept; never overwritten later */
    } else if (memcmp(old, want, (size_t)BOOTSCREEN_SIZE) != 0) {
        if (overwrite_in_place(target, want) != 0 || !same_as(target, want)) {
            overwrite_in_place(target, old);   /* put the previous picture back */
            rc = BOOTSCREEN_E_WRITE;
        }
    }
    free(old);
    free(want);
    if (run(BOOTSCREEN_UMOUNT_CMD) != 0 && rc == BOOTSCREEN_OK) rc = BOOTSCREEN_E_UMOUNT;
    return rc;
}

int bootscreen_install(void) {
    int rc = apply(g_redux, 1);
    if (rc == BOOTSCREEN_OK || rc == BOOTSCREEN_E_UMOUNT) set_marker(1);
    return rc;
}

int bootscreen_restore(void) {
    int rc = apply(g_backup, 0);
    if (rc == BOOTSCREEN_OK || rc == BOOTSCREEN_E_UMOUNT) set_marker(0);
    return rc;
}

const char *bootscreen_message(int rc) {
    switch (rc) {
    case BOOTSCREEN_OK:       return "Done. Shows at the next power-on.";
    case BOOTSCREEN_E_MOUNT:  return "Could not open the boot partition. Nothing changed.";
    case BOOTSCREEN_E_SOURCE: return "Logo file missing or wrong format. Nothing changed.";
    case BOOTSCREEN_E_TARGET: return "Unexpected logo on the device; refused. Nothing changed.";
    case BOOTSCREEN_E_BACKUP: return "Could not save the stock logo first. Nothing changed.";
    case BOOTSCREEN_E_WRITE:  return "Write failed; the previous logo was put back.";
    case BOOTSCREEN_E_UMOUNT: return "Written. Use Restart before powering off.";
    default:                  return "failed";
    }
}
