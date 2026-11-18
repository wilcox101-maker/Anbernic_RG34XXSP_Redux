// SPDX-License-Identifier: GPL-2.0-only
#define _POSIX_C_SOURCE 200809L
#include "osk_in.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/input.h>

#define OSK_IN_QUEUE 8

struct osk_in {
	const osk_sys_t *sys;
	int             fd, own_fd;
	osk_in_keymap_t km;
	uint8_t         held[KEY_MAX / 8 + 1];  /* EV_KEY state */
	int             hat_x, hat_y;           /* ABS_HAT0X/Y, -1/0/1 */
	int             want_grab, grabbed;
	osk_in_event_t  q[OSK_IN_QUEUE];
	int             qn;
};

void osk_in_default_keymap(osk_in_keymap_t *km)
{
	km->menu = BTN_TL2;
	km->chord = BTN_TL;         /* Select (Dale 12:47: Menu + Select; Menu + X pauses RetroArch) */
	km->press = BTN_SOUTH;
	km->close = BTN_EAST;
	km->shift = 0;              /* none: Shift is a key on the keyboard (Select is the chord) */
	km->page_prev = BTN_WEST;   /* L1 (Redux remap_device.c, device 2026-11-12) */
	km->page_next = BTN_Z;      /* R1 */
}

static int is_held(const osk_in_t *in, unsigned code)
{
	return code <= KEY_MAX && (in->held[code / 8] >> (code % 8)) & 1;
}

static void set_held(osk_in_t *in, unsigned code, int on)
{
	if (code > KEY_MAX)
		return;
	if (on)
		in->held[code / 8] |= (uint8_t)(1u << (code % 8));
	else
		in->held[code / 8] &= (uint8_t)~(1u << (code % 8));
}

static int any_held(const osk_in_t *in)
{
	size_t i;
	if (in->hat_x || in->hat_y)
		return 1;
	for (i = 0; i < sizeof in->held; i++)
		if (in->held[i])
			return 1;
	return 0;
}

static void apply_grab(osk_in_t *in)
{
	int r;
	if (in->want_grab == in->grabbed || any_held(in))
		return;
	r = in->sys->ioctl(in->fd, EVIOCGRAB, (void *)(long)(in->want_grab ? 1 : 0));
	if (r == 0 || !in->want_grab)
		in->grabbed = in->want_grab;    /* on failure to grab (EBUSY) stay ungrabbed; the caller can check */
}

static void push(osk_in_t *in, osk_in_kind_t k, int repeat)
{
	if (in->qn < OSK_IN_QUEUE) {
		in->q[in->qn].kind = k;
		in->q[in->qn].repeat = repeat;
		in->qn++;
	}
}

static int pop(osk_in_t *in, osk_in_event_t *ev)
{
	if (in->qn == 0)
		return 0;
	*ev = in->q[0];
	memmove(&in->q[0], &in->q[1], (size_t)(in->qn - 1) * sizeof in->q[0]);
	in->qn--;
	return 1;
}

static void handle(osk_in_t *in, const struct input_event *e)
{
	int open = in->want_grab;                /* the OSK counts as open from the moment the caller asks */

	if (e->type == EV_KEY) {
		unsigned c = e->code;
		int v = e->value;               /* 0 up, 1 down, 2 auto-repeat */
		if (v == 1 && c == in->km.chord && is_held(in, in->km.menu))
			push(in, OSK_IN_TOGGLE, 0);
		else if (open && in->grabbed && v == 1) {
			if (c == in->km.press)
				push(in, OSK_IN_PRESS, 0);
			else if (c == in->km.close)
				push(in, OSK_IN_CLOSE, 0);
			else if (in->km.shift && c == in->km.shift)
				push(in, OSK_IN_SHIFT, 0);
			else if (in->km.page_prev && c == in->km.page_prev)
				push(in, OSK_IN_PAGE_PREV, 0);
			else if (in->km.page_next && c == in->km.page_next)
				push(in, OSK_IN_PAGE_NEXT, 0);
		}
		if (open && in->grabbed && (v == 1 || v == 2)) {
			if (c == KEY_UP)
				push(in, OSK_IN_UP, v == 2);
			else if (c == KEY_DOWN)
				push(in, OSK_IN_DOWN, v == 2);
			else if (c == KEY_LEFT)
				push(in, OSK_IN_LEFT, v == 2);
			else if (c == KEY_RIGHT)
				push(in, OSK_IN_RIGHT, v == 2);
		}
		if (v == 0 || v == 1)
			set_held(in, c, v);
	} else if (e->type == EV_ABS && (e->code == ABS_HAT0X || e->code == ABS_HAT0Y)) {
		int v = e->value > 0 ? 1 : (e->value < 0 ? -1 : 0);
		int *axis = e->code == ABS_HAT0X ? &in->hat_x : &in->hat_y;
		if (open && in->grabbed && v != 0 && v != *axis) {
			if (e->code == ABS_HAT0X)
				push(in, v < 0 ? OSK_IN_LEFT : OSK_IN_RIGHT, 0);
			else
				push(in, v < 0 ? OSK_IN_UP : OSK_IN_DOWN, 0);
		}
		*axis = v;
	}
	apply_grab(in);
}

int osk_in_feed(osk_in_t *in, const struct input_event *e, osk_in_event_t *ev)
{
	if (!in || !e || !ev)
		return -EINVAL;
	handle(in, e);
	return pop(in, ev);
}

int osk_in_wrap_fd(osk_in_t **out, int fd, int own_fd, const osk_in_keymap_t *km, const osk_sys_t *sys)
{
	osk_in_t *in;
	if (!out || fd < 0)
		return -EINVAL;
	in = calloc(1, sizeof *in);
	if (!in)
		return -ENOMEM;
	in->sys = sys ? sys : osk_sys_default();
	in->fd = fd;
	in->own_fd = own_fd;
	if (km)
		in->km = *km;
	else
		osk_in_default_keymap(&in->km);
	*out = in;
	return 0;
}

int osk_in_open(osk_in_t **out, const char *name, const osk_in_keymap_t *km, const osk_sys_t *sys)
{
	const osk_sys_t *s = sys ? sys : osk_sys_default();
	const char *want = name ? name : "ANBERNIC-keys";
	int i, r;

	if (!out)
		return -EINVAL;
	*out = NULL;
	for (i = 0; i < 32; i++) {
		char path[32], dev[128];
		int fd;
		snprintf(path, sizeof path, "/dev/input/event%d", i);
		fd = s->open(path, O_RDONLY | O_NONBLOCK);
		if (fd < 0)
			continue;
		memset(dev, 0, sizeof dev);
		if (s->ioctl(fd, EVIOCGNAME(sizeof dev - 1), dev) >= 0 && strcmp(dev, want) == 0) {
			r = osk_in_wrap_fd(out, fd, 1, km, s);
			if (r < 0)
				s->close(fd);
			return r;
		}
		s->close(fd);
	}
	return -ENODEV;
}

int osk_in_poll_fd(const osk_in_t *in) { return in ? in->fd : -1; }

int osk_in_next_event(osk_in_t *in, osk_in_event_t *ev)
{
	struct input_event e;
	if (!in || !ev)
		return -EINVAL;
	if (pop(in, ev))
		return 1;
	for (;;) {
		ssize_t n = in->sys->read(in->fd, &e, sizeof e);
		if (n != (ssize_t)sizeof e) {
			if (n < 0 && errno != EAGAIN && errno != EINTR)
				return -errno;
			return 0;
		}
		handle(in, &e);
		if (pop(in, ev))
			return 1;
	}
}

int osk_in_grab(osk_in_t *in, int on)
{
	if (!in)
		return -EINVAL;
	in->want_grab = on ? 1 : 0;
	apply_grab(in);
	return in->grabbed == in->want_grab;    /* 1 = applied now, 0 = waiting for keys to be released */
}

int osk_in_is_grabbed(const osk_in_t *in) { return in ? in->grabbed : 0; }

void osk_in_close(osk_in_t *in)
{
	if (!in)
		return;
	if (in->grabbed)
		in->sys->ioctl(in->fd, EVIOCGRAB, (void *)0);
	if (in->own_fd)
		in->sys->close(in->fd);
	free(in);
}
