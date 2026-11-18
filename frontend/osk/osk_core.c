/* SPDX-License-Identifier: GPL-2.0-only */
/* Global OSK decisions. See osk_core.h. */
#include "osk_core.h"
#include "osk_kbd.h"
#include <string.h>

void osk_core_init(osk_core_t *c) { memset(c, 0, sizeof *c); osk_init(&c->k); }

static osk_act_t act(osk_act_kind_t kind) { osk_act_t a = { kind, NULL, 0 }; return a; }

osk_act_t osk_core_force_close(osk_core_t *c) {
    if (!c->k.open) return act(OSK_ACT_NONE);
    osk_toggle(&c->k);
    return act(OSK_ACT_CLOSE);
}

osk_act_t osk_core_event(osk_core_t *c, const osk_in_event_t *ev) {
    if (!ev) return act(OSK_ACT_NONE);
    if (!c->k.open) {
        if (ev->kind != OSK_IN_TOGGLE) return act(OSK_ACT_NONE);
        if (c->menu_up || c->retroarch) return act(OSK_ACT_REFUSED);
        osk_toggle(&c->k);
        return act(OSK_ACT_OPEN);
    }
    switch (ev->kind) {
    case OSK_IN_TOGGLE:
    case OSK_IN_CLOSE:
        return osk_core_force_close(c);
    case OSK_IN_UP: osk_move(&c->k, -1, 0); return act(OSK_ACT_REDRAW);
    case OSK_IN_DOWN: osk_move(&c->k, 1, 0); return act(OSK_ACT_REDRAW);
    case OSK_IN_LEFT: osk_move(&c->k, 0, -1); return act(OSK_ACT_REDRAW);
    case OSK_IN_RIGHT: osk_move(&c->k, 0, 1); return act(OSK_ACT_REDRAW);
    case OSK_IN_PAGE_PREV: osk_page(&c->k, -1); return act(OSK_ACT_REDRAW);
    case OSK_IN_PAGE_NEXT: osk_page(&c->k, 1); return act(OSK_ACT_REDRAW);
    case OSK_IN_SHIFT: c->k.shift = !c->k.shift; return act(OSK_ACT_REDRAW);
    case OSK_IN_PRESS: {
        const osk_key_t *key = osk_cur(&c->k);
        if (!key) return act(OSK_ACT_NONE);
        if (key->mode) { char dummy[4]; osk_press(&c->k, dummy, sizeof dummy); return act(OSK_ACT_REDRAW); }
        osk_act_t a = { OSK_ACT_TYPE, key, (c->k.shift ? OSK_MOD_SHIFT : 0u) | (c->k.ctrl ? OSK_MOD_CTRL : 0u) };
        c->k.ctrl = 0;   /* Ctrl applies to one key */
        return a;
    }
    default:
        return act(OSK_ACT_NONE);
    }
}
