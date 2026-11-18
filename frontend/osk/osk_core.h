/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef REDUX_OSK_CORE_H
#define REDUX_OSK_CORE_H

/* The global OSK's decisions, with no device access (redux_osk.c does the I/O), so the host tests can
   drive it. Dale 2026-11-18: chord Menu + Select; never opens over the Redux menu or while RetroArch
   runs; layout and look are keyboard UI A (osk.c, osk_draw.c). */

#include "osk.h"
#include "osk_in.h"

typedef struct {
    osk_t k;
    int menu_up;           /* the Redux menu is on screen (/tmp/redux-menu.up) */
    int retroarch;         /* RetroArch is running */
} osk_core_t;

typedef enum {
    OSK_ACT_NONE = 0,
    OSK_ACT_OPEN,          /* grab the pad, draw, show */
    OSK_ACT_CLOSE,         /* hide, release the grab */
    OSK_ACT_REDRAW,        /* highlight, page or a latch changed */
    OSK_ACT_TYPE,          /* type key with mods, then redraw (a one-shot Ctrl was used) */
    OSK_ACT_REFUSED,       /* the chord came while the menu or RetroArch is up: nothing shown */
} osk_act_kind_t;

typedef struct {
    osk_act_kind_t kind;
    const osk_key_t *key;  /* OSK_ACT_TYPE */
    unsigned mods;         /* OSK_MOD_SHIFT / OSK_MOD_CTRL (osk_kbd.h) */
} osk_act_t;

void osk_core_init(osk_core_t *c);
/* One input event in, one action out. */
osk_act_t osk_core_event(osk_core_t *c, const osk_in_event_t *ev);
/* Lid close, power-key sleep, HDMI change, the menu coming up or RetroArch starting: close if open. */
osk_act_t osk_core_force_close(osk_core_t *c);

#endif
