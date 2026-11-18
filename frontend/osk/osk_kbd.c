// SPDX-License-Identifier: GPL-2.0-only
#define _POSIX_C_SOURCE 200809L
#include "osk_kbd.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <linux/uinput.h>

struct osk_kbd {
	const osk_sys_t *sys;
	int              fd;
	int              retroarch;
	unsigned         hold_ms;
};

/* Multi-byte sequences the layout sends (xterm style), matched whole. */
static const struct { const char *seq; unsigned short code; } seq_map[] = {
	{ "\x1bOP", KEY_F1 }, { "\x1bOQ", KEY_F2 }, { "\x1bOR", KEY_F3 }, { "\x1bOS", KEY_F4 },
	{ "\x1b[15~", KEY_F5 }, { "\x1b[17~", KEY_F6 }, { "\x1b[18~", KEY_F7 }, { "\x1b[19~", KEY_F8 },
	{ "\x1b[20~", KEY_F9 }, { "\x1b[21~", KEY_F10 },
	{ "\x1b[2~", KEY_INSERT }, { "\x1b[3~", KEY_DELETE }, { "\x1b[H", KEY_HOME }, { "\x1b[F", KEY_END },
	{ "\x1b[5~", KEY_PAGEUP }, { "\x1b[6~", KEY_PAGEDOWN },
	{ "\x1b[A", KEY_UP }, { "\x1b[B", KEY_DOWN }, { "\x1b[C", KEY_RIGHT }, { "\x1b[D", KEY_LEFT },
};

/* Single characters, US layout: { char, code, shift } */
static const struct { char c; unsigned short code; unsigned char shift; } chr_map[] = {
	{ '\x1b', KEY_ESC, 0 }, { '\t', KEY_TAB, 0 }, { '\r', KEY_ENTER, 0 }, { '\n', KEY_ENTER, 0 },
	{ '\x7f', KEY_BACKSPACE, 0 }, { '\b', KEY_BACKSPACE, 0 }, { ' ', KEY_SPACE, 0 },
	{ '1', KEY_1, 0 }, { '2', KEY_2, 0 }, { '3', KEY_3, 0 }, { '4', KEY_4, 0 }, { '5', KEY_5, 0 },
	{ '6', KEY_6, 0 }, { '7', KEY_7, 0 }, { '8', KEY_8, 0 }, { '9', KEY_9, 0 }, { '0', KEY_0, 0 },
	{ '!', KEY_1, 1 }, { '@', KEY_2, 1 }, { '#', KEY_3, 1 }, { '$', KEY_4, 1 }, { '%', KEY_5, 1 },
	{ '^', KEY_6, 1 }, { '&', KEY_7, 1 }, { '*', KEY_8, 1 }, { '(', KEY_9, 1 }, { ')', KEY_0, 1 },
	{ '-', KEY_MINUS, 0 }, { '_', KEY_MINUS, 1 }, { '=', KEY_EQUAL, 0 }, { '+', KEY_EQUAL, 1 },
	{ '[', KEY_LEFTBRACE, 0 }, { '{', KEY_LEFTBRACE, 1 }, { ']', KEY_RIGHTBRACE, 0 }, { '}', KEY_RIGHTBRACE, 1 },
	{ '\\', KEY_BACKSLASH, 0 }, { '|', KEY_BACKSLASH, 1 }, { ';', KEY_SEMICOLON, 0 }, { ':', KEY_SEMICOLON, 1 },
	{ '\'', KEY_APOSTROPHE, 0 }, { '"', KEY_APOSTROPHE, 1 }, { '`', KEY_GRAVE, 0 }, { '~', KEY_GRAVE, 1 },
	{ ',', KEY_COMMA, 0 }, { '<', KEY_COMMA, 1 }, { '.', KEY_DOT, 0 }, { '>', KEY_DOT, 1 },
	{ '/', KEY_SLASH, 0 }, { '?', KEY_SLASH, 1 },
};

static const unsigned short letter_code[26] = {
	KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M,
	KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
};

int osk_keymap_lookup(const char *bytes, osk_keystroke_t *out)
{
	size_t i;
	if (!bytes || !bytes[0] || !out)
		return 0;
	for (i = 0; i < sizeof seq_map / sizeof seq_map[0]; i++)
		if (strcmp(bytes, seq_map[i].seq) == 0) {
			out->code = seq_map[i].code;
			out->shift = 0;
			return 1;
		}
	if (bytes[1] != '\0')
		return 0;
	if (bytes[0] >= 'a' && bytes[0] <= 'z') {
		out->code = letter_code[bytes[0] - 'a'];
		out->shift = 0;
		return 1;
	}
	if (bytes[0] >= 'A' && bytes[0] <= 'Z') {
		out->code = letter_code[bytes[0] - 'A'];
		out->shift = 1;
		return 1;
	}
	for (i = 0; i < sizeof chr_map / sizeof chr_map[0]; i++)
		if (chr_map[i].c == bytes[0]) {
			out->code = chr_map[i].code;
			out->shift = chr_map[i].shift;
			return 1;
		}
	return 0;
}

static int is_esc_or_f1(const char *bytes)
{
	return bytes && (strcmp(bytes, "\x1b") == 0 || strcmp(bytes, "\x1bOP") == 0);
}

int osk_key_hidden(const osk_key_t *key, int retroarch_running)
{
	return key && retroarch_running && key->mode == 0 && is_esc_or_f1(key->bytes);
}

static void emit(osk_kbd_t *k, unsigned short type, unsigned short code, int value)
{
	struct input_event ev;
	memset(&ev, 0, sizeof ev);
	ev.type = type;
	ev.code = code;
	ev.value = value;
	(void)k->sys->write(k->fd, &ev, sizeof ev);
}

static void key(osk_kbd_t *k, unsigned short code, int down)
{
	emit(k, EV_KEY, code, down);
	emit(k, EV_SYN, SYN_REPORT, 0);
}

int osk_kbd_open(osk_kbd_t **out, const osk_sys_t *sys)
{
	const osk_sys_t *s = sys ? sys : osk_sys_default();
	struct uinput_user_dev ud;
	osk_kbd_t *k;
	size_t i;
	int fd, r;

	if (!out)
		return -EINVAL;
	*out = NULL;
	errno = 0;
	fd = s->open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (fd < 0)
		return errno ? -errno : -EIO;
	s->ioctl(fd, UI_SET_EVBIT, (void *)(long)EV_KEY);
	s->ioctl(fd, UI_SET_KEYBIT, (void *)(long)KEY_LEFTSHIFT);
	s->ioctl(fd, UI_SET_KEYBIT, (void *)(long)KEY_LEFTCTRL);
	for (i = 0; i < 26; i++)
		s->ioctl(fd, UI_SET_KEYBIT, (void *)(long)letter_code[i]);
	for (i = 0; i < sizeof chr_map / sizeof chr_map[0]; i++)
		s->ioctl(fd, UI_SET_KEYBIT, (void *)(long)chr_map[i].code);
	for (i = 0; i < sizeof seq_map / sizeof seq_map[0]; i++)
		s->ioctl(fd, UI_SET_KEYBIT, (void *)(long)seq_map[i].code);
	memset(&ud, 0, sizeof ud);
	snprintf(ud.name, sizeof ud.name, "redux-osk");
	ud.id.bustype = BUS_VIRTUAL;
	ud.id.vendor = 0x0001;
	ud.id.product = 0x0002;
	ud.id.version = 1;
	errno = 0;
	if (s->write(fd, &ud, sizeof ud) != (ssize_t)sizeof ud || s->ioctl(fd, UI_DEV_CREATE, NULL) < 0) {
		r = errno ? -errno : -EIO;
		s->close(fd);
		return r;
	}
	k = calloc(1, sizeof *k);
	if (!k) {
		s->ioctl(fd, UI_DEV_DESTROY, NULL);
		s->close(fd);
		return -ENOMEM;
	}
	k->sys = s;
	k->fd = fd;
	k->hold_ms = 30;
	*out = k;
	return 0;
}

void osk_kbd_set_retroarch(osk_kbd_t *k, int running)
{
	if (k)
		k->retroarch = running ? 1 : 0;
}

void osk_kbd_set_hold_ms(osk_kbd_t *k, unsigned ms)
{
	if (k)
		k->hold_ms = ms > 500 ? 500 : ms;
}

int osk_kbd_type(osk_kbd_t *k, const osk_key_t *kk, unsigned mods)
{
	const char *b;
	osk_keystroke_t ks;
	int shift, ctrl;

	if (!k || !kk)
		return -EINVAL;
	if (kk->mode != 0 || !kk->bytes)
		return 0;                       /* Shift/Ctrl: the caller's latch */
	b = kk->bytes;
	if ((mods & OSK_MOD_SHIFT) && kk->shifted)
		b = kk->shifted;
	if (k->retroarch && is_esc_or_f1(b))
		return -EPERM;
	if (!osk_keymap_lookup(b, &ks))
		return -ENOENT;
	/* Shift: the key's own need (e.g. "!"), or the latch on a letter (osk_press upper-cases letters). */
	shift = ks.shift || ((mods & OSK_MOD_SHIFT) && b[0] >= 'a' && b[0] <= 'z' && b[1] == '\0');
	ctrl = (mods & OSK_MOD_CTRL) != 0;
	if (ctrl)
		key(k, KEY_LEFTCTRL, 1);
	if (shift)
		key(k, KEY_LEFTSHIFT, 1);
	key(k, ks.code, 1);
	if (k->hold_ms)
		k->sys->sleep_ms(k->hold_ms);
	key(k, ks.code, 0);
	if (shift)
		key(k, KEY_LEFTSHIFT, 0);
	if (ctrl)
		key(k, KEY_LEFTCTRL, 0);
	return 1;
}

void osk_kbd_close(osk_kbd_t *k)
{
	if (!k)
		return;
	k->sys->ioctl(k->fd, UI_DEV_DESTROY, NULL);
	k->sys->close(k->fd);
	free(k);
}
