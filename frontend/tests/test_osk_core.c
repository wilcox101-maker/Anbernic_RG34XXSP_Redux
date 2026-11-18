/* SPDX-License-Identifier: GPL-2.0-only */
/* Global OSK decisions (frontend/osk/osk_core.c): no device. Exit 0 = pass. */
#include "osk_core.h"
#include "osk_kbd.h"
#include <stdio.h>
#include <string.h>

static int fails;
static void expect(int ok, const char *what) { printf("%s %s\n", ok ? "ok  " : "FAIL", what); if (!ok) fails++; }

static osk_act_t send(osk_core_t *c, osk_in_kind_t k) { osk_in_event_t e = { k, 0 }; return osk_core_event(c, &e); }

int main(void) {
    osk_core_t c;
    osk_core_init(&c);
    expect(!c.k.open && send(&c, OSK_IN_PRESS).kind == OSK_ACT_NONE, "closed: A does nothing (the app keeps it)");

    c.menu_up = 1;
    expect(send(&c, OSK_IN_TOGGLE).kind == OSK_ACT_REFUSED && !c.k.open, "the chord over the Redux menu is refused");
    c.menu_up = 0; c.retroarch = 1;
    expect(send(&c, OSK_IN_TOGGLE).kind == OSK_ACT_REFUSED && !c.k.open, "the chord while RetroArch runs is refused");
    c.retroarch = 0;
    expect(send(&c, OSK_IN_TOGGLE).kind == OSK_ACT_OPEN && c.k.open, "the chord over another app opens it");

    osk_act_t a = send(&c, OSK_IN_PRESS);
    expect(a.kind == OSK_ACT_TYPE && !strcmp(a.key->label, "q") && a.mods == 0, "A types the highlighted key (q)");
    expect(send(&c, OSK_IN_RIGHT).kind == OSK_ACT_REDRAW && !strcmp(osk_cur(&c.k)->label, "w"), "Right moves and redraws");
    expect(send(&c, OSK_IN_PAGE_NEXT).kind == OSK_ACT_REDRAW && c.k.page == 1, "R1: page 123");
    expect(send(&c, OSK_IN_PAGE_PREV).kind == OSK_ACT_REDRAW && c.k.page == 0, "L1: back to abc");

    /* Shift latch from the Shift key, Ctrl one-shot from the Ctrl key. */
    c.k.row = 2; c.k.col = 0;
    expect(send(&c, OSK_IN_PRESS).kind == OSK_ACT_REDRAW && c.k.shift, "A on Shift latches it");
    c.k.row = 3; c.k.col = 0;
    expect(send(&c, OSK_IN_PRESS).kind == OSK_ACT_REDRAW && c.k.ctrl, "A on Ctrl arms it");
    c.k.row = 2; c.k.col = 3;   /* c */
    a = send(&c, OSK_IN_PRESS);
    expect(a.kind == OSK_ACT_TYPE && a.mods == (OSK_MOD_SHIFT | OSK_MOD_CTRL) && !c.k.ctrl && c.k.shift,
           "typed with Shift and Ctrl; Ctrl clears, Shift stays");

    expect(send(&c, OSK_IN_CLOSE).kind == OSK_ACT_CLOSE && !c.k.open, "B closes");
    send(&c, OSK_IN_TOGGLE);
    expect(send(&c, OSK_IN_TOGGLE).kind == OSK_ACT_CLOSE && !c.k.open, "the chord again closes");
    send(&c, OSK_IN_TOGGLE);
    expect(osk_core_force_close(&c).kind == OSK_ACT_CLOSE && !c.k.open, "lid/sleep/HDMI close it");
    expect(osk_core_force_close(&c).kind == OSK_ACT_NONE, "closing a closed keyboard does nothing");

    printf("%s\n", fails ? "test_osk_core: FAILED" : "test_osk_core: all passed");
    return fails ? 1 : 0;
}
