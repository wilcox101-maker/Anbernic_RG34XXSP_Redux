// SPDX-License-Identifier: GPL-2.0-only
#define _POSIX_C_SOURCE 200809L
#include "osk_sys.h"

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

static int sys_open(const char *path, int flags) { return open(path, flags | O_CLOEXEC); }
static int sys_close(int fd) { return close(fd); }
static int sys_ioctl(int fd, unsigned long req, void *arg) { return ioctl(fd, req, arg); }
static ssize_t sys_read(int fd, void *buf, size_t n) { return read(fd, buf, n); }
static ssize_t sys_write(int fd, const void *buf, size_t n) { return write(fd, buf, n); }

static void *sys_mmap_rw(size_t len, int fd)
{
	void *p = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	return p == MAP_FAILED ? NULL : p;
}

static int sys_munmap(void *p, size_t len) { return munmap(p, len); }

static void sys_sleep_ms(unsigned ms)
{
	struct timespec ts;
	ts.tv_sec = (time_t)(ms / 1000u);
	ts.tv_nsec = (long)(ms % 1000u) * 1000000L;
	while (nanosleep(&ts, &ts) != 0 && errno == EINTR) {
	}
}

static const osk_sys_t real_sys = {
	sys_open, sys_close, sys_ioctl, sys_read, sys_write, sys_mmap_rw, sys_munmap, sys_sleep_ms,
};

const osk_sys_t *osk_sys_default(void) { return &real_sys; }
