/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * osklib: system-call table. Every device access in osk_disp/osk_in/osk_kbd goes through one of these, so the host
 * unit tests can replace them with fakes. osk_sys_default() returns the real calls.
 */
#ifndef OSK_SYS_H
#define OSK_SYS_H

#include <stddef.h>
#include <sys/types.h>

typedef struct osk_sys {
	int (*open)(const char *path, int flags);
	int (*close)(int fd);
	int (*ioctl)(int fd, unsigned long req, void *arg);
	ssize_t (*read)(int fd, void *buf, size_t n);
	ssize_t (*write)(int fd, const void *buf, size_t n);
	void *(*mmap_rw)(size_t len, int fd);           /* MAP_SHARED, PROT_READ|PROT_WRITE; NULL on failure */
	int (*munmap)(void *p, size_t len);
	void (*sleep_ms)(unsigned ms);
} osk_sys_t;

const osk_sys_t *osk_sys_default(void);

/* C99 compile-time check */
#define OSK_ASSERT(name, cond) typedef char osk_assert_##name[(cond) ? 1 : -1]

#endif
