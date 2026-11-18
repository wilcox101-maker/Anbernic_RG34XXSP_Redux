/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * osk_in (KR-3, input side): reads the stock keypad (`ANBERNIC-keys`) without a grab, spots the OSK chord
 * (Menu held, then Select; Dale 2026-11-18 12:47), and while the OSK is open holds EVIOCGRAB and turns the pad into
 * OSK events (D-pad moves, A types, L1/R1 change page, B or the chord closes; Redux keyboard UI A).
 *
 * Grab timing: EVIOCGRAB is applied (or released) only when no key is held. Taking the grab while Menu/X are down
 * would leave those keys "pressed" in the app (it would never see the release), and releasing it while B is down
 * would hand the app a release it never saw pressed. So osk_in_grab() records the wish and the grab follows as soon
 * as all keys are up (normally a few tens of ms after the chord).
 *
 * Key codes: defaults from the stock launcher's key table (stock-re-boot-and-userspace.md section 1.2; inferred,
 * checked by K27). Pass a keymap to change them without a rebuild.
 * Return values: 0/1 or a positive count on success, -errno on failure.
 */
#ifndef OSK_IN_H
#define OSK_IN_H

#include <stdint.h>
#include "osk_sys.h"

struct input_event;

typedef enum {
	OSK_IN_NONE = 0,
	OSK_IN_TOGGLE,          /* chord: open when closed, close when open */
	OSK_IN_UP, OSK_IN_DOWN, OSK_IN_LEFT, OSK_IN_RIGHT,
	OSK_IN_PRESS,           /* A: type the highlighted key */
	OSK_IN_CLOSE,           /* B */
	OSK_IN_SHIFT,           /* the shift button, if the keymap names one (none by default) */
	OSK_IN_PAGE_PREV,       /* L1 */
	OSK_IN_PAGE_NEXT,       /* R1 */
} osk_in_kind_t;

typedef struct {
	osk_in_kind_t kind;
	int           repeat;   /* 1 for a kernel auto-repeat of a direction */
} osk_in_event_t;

typedef struct {
	uint16_t menu;          /* default BTN_TL2 (312): the launcher's MENU/FN key */
	uint16_t chord;         /* default BTN_TL (310): Select */
	uint16_t press;         /* default BTN_SOUTH (304): A */
	uint16_t close;         /* default BTN_EAST (305): B */
	uint16_t shift;         /* default 0: none */
	uint16_t page_prev;     /* default BTN_WEST (308): L1 */
	uint16_t page_next;     /* default BTN_Z (309): R1 */
} osk_in_keymap_t;

typedef struct osk_in osk_in_t;

void osk_in_default_keymap(osk_in_keymap_t *km);
/* Opens the input device named `name` (NULL = "ANBERNIC-keys") among /dev/input/event0..31; km NULL = defaults. */
int  osk_in_open(osk_in_t **out, const char *name, const osk_in_keymap_t *km, const osk_sys_t *sys);
int  osk_in_poll_fd(const osk_in_t *in);
/* Reads what the device has (non-blocking) and returns the next OSK event: 1 = *ev filled, 0 = none for now. */
int  osk_in_next_event(osk_in_t *in, osk_in_event_t *ev);
/* Want the grab on (OSK opened) or off (closed). Applied when no key is held (see above). */
int  osk_in_grab(osk_in_t *in, int on);
int  osk_in_is_grabbed(const osk_in_t *in);
void osk_in_close(osk_in_t *in);            /* releases a grab */

/* For tests and for callers with their own reader: feed one evdev event; returns 1 and fills *ev if it makes one. */
int  osk_in_feed(osk_in_t *in, const struct input_event *e, osk_in_event_t *ev);
/* Test hook: an osk_in on an already open fd (not closed by osk_in_close unless own_fd). */
int  osk_in_wrap_fd(osk_in_t **out, int fd, int own_fd, const osk_in_keymap_t *km, const osk_sys_t *sys);

#endif
