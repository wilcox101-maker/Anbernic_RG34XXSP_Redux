/* Boot Screen manager against a temp folder standing in for the boot partition.
   Does not mount or touch a device. Run ./test_bootscreen, exit 0 = pass. */
#include "bootscreen.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define D "/tmp/redux-bootscreen-test"
#define TARGET D "/mnt/bootlogo.bmp"
#define REDUX  D "/redux.bmp"
#define BACKUP D "/stock-backup.bmp"
#define MARKER D "/marker"

static int fails;
static void expect(int ok, const char *what) {
    printf("%s %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) fails++;
}

static void le32(unsigned char *p, unsigned v) { p[0] = v; p[1] = v >> 8; p[2] = v >> 16; p[3] = v >> 24; }

/* A stock-format BMP filled with one byte value; w lets a test make a wrong header. */
static void make_bmp(const char *path, unsigned char fill, unsigned w, long size) {
    unsigned char *b = malloc((size_t)size);
    memset(b, fill, (size_t)size);
    memset(b, 0, 54);
    b[0] = 'B'; b[1] = 'M';
    le32(b + 2, (unsigned)BOOTSCREEN_SIZE); le32(b + 10, 54); le32(b + 14, 40);
    le32(b + 18, w); le32(b + 22, 480);
    b[26] = 1; b[28] = 24;
    FILE *f = fopen(path, "wb");
    fwrite(b, 1, (size_t)size, f);
    fclose(f);
    free(b);
}

/* The byte at offset 1000 tells the pictures apart; -1 if the file is missing. */
static int pixel(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    fseek(f, 1000, SEEK_SET);
    int c = fgetc(f);
    fclose(f);
    return c;
}

static int entries(const char *dir) {
    DIR *d = opendir(dir);
    int n = 0;
    struct dirent *de;
    while (d && (de = readdir(d)) != NULL) if (de->d_name[0] != '.') n++;
    if (d) closedir(d);
    return n;
}

static int mounts, umounts, mount_rc, umount_rc;
static int hook(const char *cmd) {
    if (strcmp(cmd, BOOTSCREEN_MOUNT_CMD) == 0) { mounts++; return mount_rc; }
    if (strcmp(cmd, BOOTSCREEN_UMOUNT_CMD) == 0) { umounts++; return umount_rc; }
    return -1;
}

static void reset(void) {
    mkdir(D, 0755); mkdir(D "/mnt", 0755);
    remove(BACKUP); remove(BACKUP ".new"); remove(MARKER);
    make_bmp(TARGET, 0x11, 640, BOOTSCREEN_SIZE);   /* "stock" */
    make_bmp(REDUX, 0x22, 640, BOOTSCREEN_SIZE);    /* "redux" */
    mounts = umounts = mount_rc = umount_rc = 0;
}

int main(void) {
    bootscreen_set_paths(D "/mnt", REDUX, BACKUP, MARKER);
    bootscreen_set_run(hook);

    reset();
    expect(bootscreen_valid(REDUX), "a 640x480 24-bit 921654-byte BMP is valid");
    make_bmp(D "/bad.bmp", 0x22, 720, BOOTSCREEN_SIZE);
    expect(!bootscreen_valid(D "/bad.bmp"), "a 720-wide header is not valid");
    make_bmp(D "/bad.bmp", 0x22, 640, BOOTSCREEN_SIZE - 1);
    expect(!bootscreen_valid(D "/bad.bmp"), "a file one byte short is not valid");
    expect(!bootscreen_valid(D "/missing.bmp"), "a missing file is not valid");
    remove(D "/bad.bmp");

    reset();
    remove(REDUX);
    expect(bootscreen_install() == BOOTSCREEN_E_SOURCE && mounts == 0, "no Redux logo: refused before mounting");

    reset();
    mount_rc = -1;
    expect(bootscreen_install() == BOOTSCREEN_E_MOUNT && pixel(TARGET) == 0x11 && pixel(BACKUP) == -1,
           "mount fails: nothing written, no backup");

    reset();
    make_bmp(TARGET, 0x11, 640, 500000);
    expect(bootscreen_install() == BOOTSCREEN_E_TARGET && pixel(BACKUP) == -1 && umounts == 1,
           "unexpected logo on the partition: refused, unmounted, no backup");
    {
        struct stat st;
        expect(stat(TARGET, &st) == 0 && st.st_size == 500000, "the unexpected file is left exactly as it was");
    }

    reset();
    {
        struct stat before, after;
        stat(TARGET, &before);
        expect(!bootscreen_is_redux(), "starts as Stock");
        expect(bootscreen_install() == BOOTSCREEN_OK, "install succeeds");
        stat(TARGET, &after);
        expect(pixel(TARGET) == 0x22 && bootscreen_valid(TARGET), "the partition now holds the Redux logo");
        expect(pixel(BACKUP) == 0x11 && bootscreen_valid(BACKUP), "the stock logo was saved first");
        expect(before.st_ino == after.st_ino && after.st_size == BOOTSCREEN_SIZE,
               "overwritten in place: same file, same size");
        expect(entries(D "/mnt") == 1, "nothing else was created on the partition");
        expect(mounts == 1 && umounts == 1, "mounted once, unmounted once");
        expect(bootscreen_is_redux(), "state is Redux");
    }
    make_bmp(REDUX, 0x33, 640, BOOTSCREEN_SIZE);     /* a newer Redux logo */
    expect(bootscreen_install() == BOOTSCREEN_OK && pixel(TARGET) == 0x33, "a second install updates the logo");
    expect(pixel(BACKUP) == 0x11, "the backup is still the first (stock) logo");

    expect(bootscreen_restore() == BOOTSCREEN_OK && pixel(TARGET) == 0x11, "restore puts the stock logo back");
    expect(!bootscreen_is_redux() && pixel(BACKUP) == 0x11, "state is Stock; the backup is kept");
    expect(entries(D "/mnt") == 1, "restore created nothing on the partition");

    reset();
    expect(bootscreen_restore() == BOOTSCREEN_E_SOURCE && mounts == 0, "restore without a backup is refused");

    reset();
    umount_rc = -1;
    expect(bootscreen_install() == BOOTSCREEN_E_UMOUNT && pixel(TARGET) == 0x22 && bootscreen_is_redux(),
           "unmount failure is reported; the logo was written");

    printf("%d failed\n", fails);
    return fails ? 1 : 0;
}
