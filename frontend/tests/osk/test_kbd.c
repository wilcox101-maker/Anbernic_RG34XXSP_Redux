// SPDX-License-Identifier: GPL-2.0-only
/* Host tests for osk_kbd (KR-3 output): every key of Redux's layout maps, Shift/Ctrl, the RetroArch rule. */
#include <stdio.h>
#include <string.h>
#include "fake_sys.h"
#include "osk_kbd.h"

/* Collapse the recorded writes into "D<code>"/"U<code>" steps (SYN dropped). */
static int steps(int *codes, int *downs, int max)
{
	int i, n = 0;
	for (i = 0; i < fake.n_writes && n < max; i++)
		if (fake.writes[i].type == EV_KEY) {
			codes[n] = fake.writes[i].code;
			downs[n] = fake.writes[i].value;
			n++;
		}
	return n;
}

static int t_every_layout_key_maps(void)
{
	int p, r, c, n = 0, mode = 0;
	for (p = 0; p < OSK_PAGES; p++)
	for (r = 0; r < OSK_ROWS; r++)
		for (c = 0; c < osk_cols(p, r); c++) {
			const osk_key_t *k = osk_key(p, r, c);
			osk_keystroke_t ks;
			if (k->mode) {
				mode++;
				continue;
			}
			if (!osk_keymap_lookup(k->bytes, &ks)) {
				fprintf(stderr, "unmapped key row %d col %d label '%s'\n", r, c, k->label);
				return 1;
			}
			if (k->shifted && !osk_keymap_lookup(k->shifted, &ks)) {
				fprintf(stderr, "unmapped shifted key row %d col %d label '%s'\n", r, c, k->label);
				return 1;
			}
			n++;
		}
	CHECK(mode == 3 && n > 90);   /* Shift once, Ctrl on two pages; 96 typing keys */
	return 0;
}

static int t_type_sequences(void)
{
	osk_kbd_t *k;
	int codes[16], downs[16], n;
	const osk_key_t *q = osk_key(0, 0, 0), *one = osk_key(1, 0, 0), *bang = osk_key(1, 2, 0), *f1 = osk_key(2, 0, 0),
			*esc = osk_key(2, 1, 0), *shift = osk_key(0, 2, 0);
	fake_reset();
	CHECK(strcmp(q->label, "q") == 0 && strcmp(one->label, "1") == 0 && strcmp(bang->label, "!") == 0);
	CHECK(strcmp(f1->label, "F1") == 0 && strcmp(esc->label, "Esc") == 0 && shift->mode == 1);
	CHECK(osk_kbd_open(&k, fake_sys()) == 0 && fake.uinput_created == 1);
	/* q */
	fake.n_writes = 0;
	CHECK(osk_kbd_type(k, q, 0) == 1);
	n = steps(codes, downs, 16);
	CHECK(n == 2 && codes[0] == KEY_Q && downs[0] == 1 && codes[1] == KEY_Q && downs[1] == 0);
	CHECK(fake.sleeps == 1);
	/* Q (Shift latch on a letter) */
	fake.n_writes = 0;
	CHECK(osk_kbd_type(k, q, OSK_MOD_SHIFT) == 1);
	n = steps(codes, downs, 16);
	CHECK(n == 4 && codes[0] == KEY_LEFTSHIFT && downs[0] == 1 && codes[1] == KEY_Q && codes[3] == KEY_LEFTSHIFT && downs[3] == 0);
	/* "1" with Shift latch = "!" via the key's shifted bytes */
	fake.n_writes = 0;
	CHECK(osk_kbd_type(k, one, OSK_MOD_SHIFT) == 1);
	n = steps(codes, downs, 16);
	CHECK(n == 4 && codes[0] == KEY_LEFTSHIFT && codes[1] == KEY_1);
	/* "!" key of row 5 without the latch still needs Shift */
	fake.n_writes = 0;
	CHECK(osk_kbd_type(k, bang, 0) == 1);
	n = steps(codes, downs, 16);
	CHECK(n == 4 && codes[0] == KEY_LEFTSHIFT && codes[1] == KEY_1);
	/* Ctrl + c: real Ctrl press around the key */
	fake.n_writes = 0;
	CHECK(osk_kbd_type(k, osk_key(0, 2, 3), OSK_MOD_CTRL) == 1);
	n = steps(codes, downs, 16);
	CHECK(n == 4 && codes[0] == KEY_LEFTCTRL && downs[0] == 1 && codes[1] == KEY_C && codes[3] == KEY_LEFTCTRL && downs[3] == 0);
	/* F1 and Esc type normally... */
	fake.n_writes = 0;
	CHECK(osk_kbd_type(k, f1, 0) == 1);
	n = steps(codes, downs, 16);
	CHECK(n == 2 && codes[0] == KEY_F1);
	CHECK(osk_kbd_type(k, esc, 0) == 1);
	/* ...but not while RetroArch runs, and the drawer is told to hide them */
	osk_kbd_set_retroarch(k, 1);
	fake.n_writes = 0;
	CHECK(osk_kbd_type(k, f1, 0) < 0 && osk_kbd_type(k, esc, 0) < 0 && fake.n_writes == 0);
	CHECK(osk_key_hidden(f1, 1) && osk_key_hidden(esc, 1) && !osk_key_hidden(f1, 0));
	CHECK(!osk_key_hidden(osk_key(2, 0, 1), 1));          /* F2 stays */
	CHECK(osk_kbd_type(k, osk_key(2, 0, 1), 0) == 1);
	osk_kbd_set_retroarch(k, 0);
	CHECK(osk_kbd_type(k, esc, 0) == 1);
	/* mode keys are not typed */
	fake.n_writes = 0;
	CHECK(osk_kbd_type(k, shift, 0) == 0 && fake.n_writes == 0);
	osk_kbd_close(k);
	CHECK(fake.uinput_destroyed == 1 && fake.open_fds == 0);
	return 0;
}

static int t_specials(void)
{
	osk_keystroke_t ks;
	CHECK(osk_keymap_lookup("\r", &ks) && ks.code == KEY_ENTER);
	CHECK(osk_keymap_lookup("\x7f", &ks) && ks.code == KEY_BACKSPACE);
	CHECK(osk_keymap_lookup("\t", &ks) && ks.code == KEY_TAB);
	CHECK(osk_keymap_lookup("\x1b[6~", &ks) && ks.code == KEY_PAGEDOWN);
	CHECK(osk_keymap_lookup("\x1b[21~", &ks) && ks.code == KEY_F10);
	CHECK(osk_keymap_lookup("~", &ks) && ks.code == KEY_GRAVE && ks.shift);
	CHECK(osk_keymap_lookup("\"", &ks) && ks.code == KEY_APOSTROPHE && ks.shift);
	CHECK(!osk_keymap_lookup("\x1b[99~", &ks) && !osk_keymap_lookup("", &ks));
	return 0;
}

int main(void)
{
	int fails = 0;
	fails += t_every_layout_key_maps();
	fails += t_type_sequences();
	fails += t_specials();
	printf("test_kbd: %s\n", fails ? "FAIL" : "ok");
	return fails != 0;
}
