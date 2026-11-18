// SPDX-License-Identifier: GPL-2.0-only
#define _POSIX_C_SOURCE 200809L
#include "fake_sys.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/uinput.h>

fake_t fake;

enum { FD_DISP = 10, FD_ION = 11, FD_UINPUT = 12, FD_EVENT3 = 13, FD_DMABUF = 20 };

static int f_open(const char *path, int flags)
{
	(void)flags;
	if (strcmp(path, "/dev/disp") == 0) { fake.open_fds++; return FD_DISP; }
	if (strcmp(path, "/dev/ion") == 0) {
		if (fake.fail_open_path_ion) { errno = ENOENT; return -1; }
		fake.open_fds++;
		return FD_ION;
	}
	if (strcmp(path, "/dev/uinput") == 0) { fake.open_fds++; return FD_UINPUT; }
	if (strcmp(path, "/dev/input/event3") == 0) { fake.open_fds++; return FD_EVENT3; }
	if (strncmp(path, "/dev/input/event", 16) == 0) { errno = ENOENT; return -1; }
	errno = ENOENT;
	return -1;
}

static int f_close(int fd) { (void)fd; fake.open_fds--; return 0; }

static int f_ioctl(int fd, unsigned long req, void *arg)
{
	fake_ioctl_t *r = fake.n_ioctls < FAKE_MAX ? &fake.ioctls[fake.n_ioctls++] : NULL;
	if (r) {
		memset(r, 0, sizeof *r);
		r->req = req;
		r->fd = fd;
		r->int_arg = (long)arg;
	}
	if (fd == FD_DISP) {
		unsigned long *a = arg;
		if (req == DISP_GET_SCN_WIDTH) return fake.scn_w;
		if (req == DISP_GET_SCN_HEIGHT) return fake.scn_h;
		if (req == DISP_LAYER_SET_CONFIG2) {
			if (r) {
				memcpy(r->disp_arg, a, sizeof r->disp_arg);
				memcpy(&r->cfg, (void *)a[1], sizeof r->cfg);
			}
			return 0;
		}
		errno = ENOTTY;
		return -1;
	}
	if (fd == FD_ION) {
		if (req == _IOWR('I', 0, struct ion_allocation_data_legacy)) {
			if (fake.legacy_alloc_fails) { errno = ENOTTY; return -1; }
			((struct ion_allocation_data_legacy *)arg)->handle = 7;
			return 0;
		}
		if (req == _IOWR('I', 4, struct ion_fd_data_legacy)) {
			((struct ion_fd_data_legacy *)arg)->fd = FD_DMABUF;
			fake.open_fds++;
			return 0;
		}
		if (req == _IOWR('I', 1, struct ion_handle_data_legacy)) return 0;
		if (req == _IOWR('I', 0, struct ion_allocation_data_new)) {
			((struct ion_allocation_data_new *)arg)->fd = FD_DMABUF + 1;
			fake.open_fds++;
			return 0;
		}
		errno = ENOTTY;
		return -1;
	}
	if (fd == FD_EVENT3) {
		if (req == EVIOCGNAME(127)) {
			strcpy(arg, fake.input_name ? fake.input_name : "ANBERNIC-keys");
			return 0;
		}
		if (req == EVIOCGRAB) {
			if ((long)arg == 1 && fake.grab_busy) { errno = EBUSY; return -1; }
			return 0;
		}
	}
	if (fd == FD_UINPUT) {
		if (req == UI_DEV_CREATE) { fake.uinput_created++; return 0; }
		if (req == UI_DEV_DESTROY) { fake.uinput_destroyed++; return 0; }
		return 0;
	}
	errno = ENOTTY;
	return -1;
}

static ssize_t f_read(int fd, void *buf, size_t n)
{
	if (fd != FD_EVENT3 || n != sizeof(struct input_event) || fake.read_pos >= fake.n_reads) {
		errno = EAGAIN;
		return -1;
	}
	memcpy(buf, &fake.reads[fake.read_pos++], n);
	return (ssize_t)n;
}

static ssize_t f_write(int fd, const void *buf, size_t n)
{
	if (fd == FD_UINPUT && n == sizeof(struct input_event) && fake.n_writes < FAKE_MAX)
		memcpy(&fake.writes[fake.n_writes++], buf, n);
	return (ssize_t)n;
}

static void *f_mmap(size_t len, int fd) { (void)fd; return calloc(1, len); }
static int f_munmap(void *p, size_t len) { (void)len; free(p); return 0; }
static void f_sleep(unsigned ms) { (void)ms; fake.sleeps++; }

static const osk_sys_t fsys = { f_open, f_close, f_ioctl, f_read, f_write, f_mmap, f_munmap, f_sleep };

const osk_sys_t *fake_sys(void) { return &fsys; }

void fake_reset(void)
{
	memset(&fake, 0, sizeof fake);
	fake.scn_w = 720;
	fake.scn_h = 480;
}

int fake_count(unsigned long req)
{
	int i, n = 0;
	for (i = 0; i < fake.n_ioctls; i++)
		if (fake.ioctls[i].req == req)
			n++;
	return n;
}

const fake_ioctl_t *fake_last(unsigned long req)
{
	int i;
	for (i = fake.n_ioctls - 1; i >= 0; i--)
		if (fake.ioctls[i].req == req)
			return &fake.ioctls[i];
	return NULL;
}
