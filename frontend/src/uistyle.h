#ifndef REDUX_UISTYLE_H
#define REDUX_UISTYLE_H

/* Settings > UI text sizes and colours (Dale 2026-11-17, plan A:
 * notes/2026-11-17-dale-decision-ui-text-sizes.md).
 *   Header, footer: Default or Large (twice as tall, same width).
 *   Window (body):  Default, Large (twice as tall), Largest (twice as tall and wide).
 *   Colours: menu text, header/footer text, window background, from the 16 standard VGA colours.
 * A size is a request: grid_present falls back to the largest size the screen's content fits.
 * Saved in /mnt/data/redux/uistyle (desktop: /tmp/redux.uistyle). No stock files are touched. */

#include <stdint.h>

enum { UIS_HEADER, UIS_FOOTER, UIS_WINDOW, UIS_TEXT, UIS_HFTEXT, UIS_WINCOLOR, UIS_COUNT };

#define UIS_COLORS 16
#define UIS_BLACK   0
#define UIS_WHITE  15

typedef struct { const char *name; uint8_t r, g, b; } uis_color_t;
extern const uis_color_t uis_colors[UIS_COLORS];

int  uistyle_get(int item);
/* Next or previous value (dir +1 / -1), saved. Sizes stop at the ends; colours wrap and skip a
 * colour that would hide text (menu text = window colour; header/footer text = black). */
int  uistyle_step(int item, int dir);
const char *uistyle_value_name(int item);   /* "Default", "Large", "Largest", or a colour name */

void uistyle_load(void);
void uistyle_reset_fonts(void);   /* the three sizes back to Default, saved */
void uistyle_reset(void);         /* Reset All: forget the file, everything back to default */
void uistyle_set_path(const char *path);   /* tests */

/* Push the current settings to the grid. */
void uistyle_apply(void);

#endif
