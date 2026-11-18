/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * osk_kbd (KR-3, output side): a uinput keyboard ("redux-osk") that types the keys of Redux's on-screen layout
 * (frontend/term/osk.c, Redux dd65506; not changed here). Each layout key's bytes (or shifted bytes) are turned
 * into a Linux key code plus Shift for a US layout (osk_keymap_lookup); Ctrl becomes a real KEY_LEFTCTRL press.
 *
 * RetroArch rule (Dale 2026-11-18 11:23): while RetroArch runs, Esc (quits the emulator) and F1 (opens its menu) are
 * left off the keyboard: osk_kbd_set_retroarch(k, 1) makes osk_kbd_type refuse them, and osk_key_hidden() tells the
 * drawer to leave them out. There is no hold guard.
 * Return values: 0/1 on success, -errno on failure.
 */
#ifndef OSK_KBD_H
#define OSK_KBD_H

#include "osk.h"        /* Redux frontend/term/osk.h (or its split frontend/osk/osk.h) */
#include "osk_sys.h"

#define OSK_MOD_SHIFT 1u
#define OSK_MOD_CTRL  2u

typedef struct osk_kbd osk_kbd_t;

typedef struct {
	unsigned short code;    /* Linux KEY_* */
	unsigned char  shift;   /* 1 = needs Shift on a US layout */
} osk_keystroke_t;

/* Bytes of one layout key -> keystroke. Returns 1 when known, 0 when not (the key is not typed). */
int  osk_keymap_lookup(const char *bytes, osk_keystroke_t *out);
/* 1 if the key is Esc or F1 and RetroArch runs (leave it out of the drawn layout). */
int  osk_key_hidden(const osk_key_t *key, int retroarch_running);

int  osk_kbd_open(osk_kbd_t **out, const osk_sys_t *sys);
void osk_kbd_set_retroarch(osk_kbd_t *k, int running);
/* Key-down/up hold in ms (default 30, so frame-polling games see the key). */
void osk_kbd_set_hold_ms(osk_kbd_t *k, unsigned ms);
/* Type one layout key. mods: OSK_MOD_SHIFT (the layout's Shift latch), OSK_MOD_CTRL (the Ctrl latch).
   Mode keys (Shift/Ctrl) are not typed: the caller flips its latch (osk_press does that). */
int  osk_kbd_type(osk_kbd_t *k, const osk_key_t *key, unsigned mods);
void osk_kbd_close(osk_kbd_t *k);

#endif
