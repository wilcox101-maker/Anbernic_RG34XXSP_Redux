#ifndef REDUX_CARDSCRIPTS_H
#define REDUX_CARDSCRIPTS_H

/* "Card Scripts" switch, Ports option B part 2 (notes/2026-11-13-dale-decision-ports.md).
   The explicit user opt-in the hard rule asks for before Redux runs code from the Roms card.
   Default Off. Turning it On takes a 3 s hold (ui.c); turning it Off is one press. */

int cardscripts_enabled(void);
void cardscripts_set(int on);

/* Tests only: where the setting is kept. */
void cardscripts_set_path(const char *path);

#endif
