// SPDX-License-Identifier: GPL-2.0-only
/* Host tests for osk_in (KR-3 input): chord, deferred grab, navigation, hat, repeat. */
#include <stdio.h>
#include <string.h>
#include "fake_sys.h"
#include "osk_in.h"

static struct input_event ev(unsigned short type, unsigned short code, int value)
{
	struct input_event e;
	memset(&e, 0, sizeof e);
	e.type = type;
	e.code = code;
	e.value = value;
	return e;
}

static int feed(osk_in_t *in, unsigned short type, unsigned short code, int value, osk_in_event_t *out)
{
	struct input_event e = ev(type, code, value);
	return osk_in_feed(in, &e, out);
}

static int t_chord_only_with_menu(void)
{
	osk_in_t *in;
	osk_in_event_t o;
	fake_reset();
	CHECK(osk_in_open(&in, NULL, NULL, fake_sys()) == 0);
	CHECK(osk_in_poll_fd(in) == 13);
	CHECK(feed(in, EV_KEY, BTN_TL, 1, &o) == 0);   /* Select alone: nothing */
	CHECK(feed(in, EV_KEY, BTN_TL, 0, &o) == 0);
	CHECK(feed(in, EV_KEY, BTN_TL2, 1, &o) == 0);     /* Menu down */
	CHECK(feed(in, EV_KEY, BTN_SOUTH, 1, &o) == 0);   /* Menu + A: not ours (closed) */
	CHECK(feed(in, EV_KEY, BTN_SOUTH, 0, &o) == 0);
	CHECK(feed(in, EV_KEY, BTN_TL, 1, &o) == 1 && o.kind == OSK_IN_TOGGLE);
	CHECK(feed(in, EV_KEY, BTN_TL, 2, &o) == 0);   /* repeat of Select: no second toggle */
	osk_in_close(in);
	CHECK(fake.open_fds == 0);
	return 0;
}

static int t_grab_waits_for_release(void)
{
	osk_in_t *in;
	osk_in_event_t o;
	fake_reset();
	CHECK(osk_in_open(&in, NULL, NULL, fake_sys()) == 0);
	feed(in, EV_KEY, BTN_TL2, 1, &o);
	CHECK(feed(in, EV_KEY, BTN_TL, 1, &o) == 1 && o.kind == OSK_IN_TOGGLE);
	CHECK(osk_in_grab(in, 1) == 0);                   /* keys still down: pending */
	CHECK(fake_count(EVIOCGRAB) == 0 && !osk_in_is_grabbed(in));
	feed(in, EV_KEY, BTN_TL, 0, &o);
	CHECK(fake_count(EVIOCGRAB) == 0);                /* Menu still down */
	feed(in, EV_KEY, BTN_TL2, 0, &o);
	CHECK(fake_count(EVIOCGRAB) == 1 && fake_last(EVIOCGRAB)->int_arg == 1 && osk_in_is_grabbed(in));
	/* open: navigation */
	CHECK(feed(in, EV_ABS, ABS_HAT0X, 1, &o) == 1 && o.kind == OSK_IN_RIGHT);
	CHECK(feed(in, EV_ABS, ABS_HAT0X, 1, &o) == 0);   /* same value again: nothing */
	CHECK(feed(in, EV_ABS, ABS_HAT0X, 0, &o) == 0);
	CHECK(feed(in, EV_ABS, ABS_HAT0Y, -1, &o) == 1 && o.kind == OSK_IN_UP);
	CHECK(feed(in, EV_ABS, ABS_HAT0Y, 0, &o) == 0);
	CHECK(feed(in, EV_KEY, KEY_DOWN, 1, &o) == 1 && o.kind == OSK_IN_DOWN && !o.repeat);
	CHECK(feed(in, EV_KEY, KEY_DOWN, 2, &o) == 1 && o.kind == OSK_IN_DOWN && o.repeat);
	CHECK(feed(in, EV_KEY, KEY_DOWN, 0, &o) == 0);
	CHECK(feed(in, EV_KEY, BTN_SOUTH, 1, &o) == 1 && o.kind == OSK_IN_PRESS);
	feed(in, EV_KEY, BTN_SOUTH, 0, &o);
	CHECK(feed(in, EV_KEY, BTN_WEST, 1, &o) == 1 && o.kind == OSK_IN_PAGE_PREV);   /* L1 */
	feed(in, EV_KEY, BTN_WEST, 0, &o);
	CHECK(feed(in, EV_KEY, BTN_Z, 1, &o) == 1 && o.kind == OSK_IN_PAGE_NEXT);      /* R1 */
	feed(in, EV_KEY, BTN_Z, 0, &o);
	CHECK(feed(in, EV_KEY, BTN_TL, 1, &o) == 0);       /* Select alone (no Menu): no Shift any more */
	feed(in, EV_KEY, BTN_TL, 0, &o);
	/* close with B: ungrab waits for B to come up */
	CHECK(feed(in, EV_KEY, BTN_EAST, 1, &o) == 1 && o.kind == OSK_IN_CLOSE);
	CHECK(osk_in_grab(in, 0) == 0 && osk_in_is_grabbed(in));
	feed(in, EV_KEY, BTN_EAST, 0, &o);
	CHECK(!osk_in_is_grabbed(in) && fake_last(EVIOCGRAB)->int_arg == 0);
	/* closed again: A is the app's */
	CHECK(feed(in, EV_KEY, BTN_SOUTH, 1, &o) == 0);
	feed(in, EV_KEY, BTN_SOUTH, 0, &o);
	osk_in_close(in);
	return 0;
}

static int t_chord_closes_and_busy_grab(void)
{
	osk_in_t *in;
	osk_in_event_t o;
	fake_reset();
	fake.grab_busy = 1;
	CHECK(osk_in_open(&in, NULL, NULL, fake_sys()) == 0);
	CHECK(osk_in_grab(in, 1) == 0 && !osk_in_is_grabbed(in));   /* EBUSY: another program holds it */
	osk_in_grab(in, 0);
	fake.grab_busy = 0;
	CHECK(osk_in_grab(in, 1) == 1 && osk_in_is_grabbed(in));
	feed(in, EV_KEY, BTN_TL2, 1, &o);
	CHECK(feed(in, EV_KEY, BTN_TL, 1, &o) == 1 && o.kind == OSK_IN_TOGGLE);   /* chord while open */
	osk_in_close(in);                                           /* releases the grab */
	CHECK(fake_last(EVIOCGRAB)->int_arg == 0);
	return 0;
}

static int t_open_by_name_and_read_path(void)
{
	osk_in_t *in;
	osk_in_event_t o;
	osk_in_keymap_t km;
	fake_reset();
	fake.input_name = "other-device";
	CHECK(osk_in_open(&in, NULL, NULL, fake_sys()) < 0);
	CHECK(fake.open_fds == 0);
	fake_reset();
	osk_in_default_keymap(&km);
	km.chord = BTN_C;                                           /* if K27 shows X is BTN_C */
	CHECK(osk_in_open(&in, "ANBERNIC-keys", &km, fake_sys()) == 0);
	fake.reads[fake.n_reads++] = ev(EV_KEY, BTN_TL2, 1);
	fake.reads[fake.n_reads++] = ev(EV_SYN, SYN_REPORT, 0);
	fake.reads[fake.n_reads++] = ev(EV_KEY, BTN_NORTH, 1);  /* not the chord any more */
	fake.reads[fake.n_reads++] = ev(EV_KEY, BTN_C, 1);
	CHECK(osk_in_next_event(in, &o) == 1 && o.kind == OSK_IN_TOGGLE);
	CHECK(osk_in_next_event(in, &o) == 0);
	osk_in_close(in);
	return 0;
}

int main(void)
{
	int fails = 0;
	fails += t_chord_only_with_menu();
	fails += t_grab_waits_for_release();
	fails += t_chord_closes_and_busy_grab();
	fails += t_open_by_name_and_read_path();
	printf("test_in: %s\n", fails ? "FAIL" : "ok");
	return fails != 0;
}
