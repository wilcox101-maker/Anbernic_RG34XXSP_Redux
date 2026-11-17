#ifndef REDUX_VT_H
#define REDUX_VT_H
#include <stddef.h>
#include <stdint.h>

/* Redux Terminal, screen model (Dale 2026-11-17 A, notes/2026-11-17-claude-question-terminal-apps.md).
   A VT100/xterm subset on a fixed 90 x 30 grid of CP437 cells, enough for the curses programs Redux
   runs (mc, htop, alsamixer, nmtui, bluetoothctl, neofetch, a shell). Pure: bytes in, cells out, no
   SDL and no pty, so tests can drive it. UTF-8 in; anything the 8x16 fonts cannot show is '?'.
   Colours are the 16 ANSI colours (256-colour and RGB requests are mapped to the nearest). */

#define VT_COLS 90
#define VT_ROWS 30
#define VT_DEFAULT_FG 7     /* the app's default: white on black (Redux's own text colours come later) */
#define VT_DEFAULT_BG 0

#define VT_BOLD      0x01
#define VT_UNDERLINE 0x02
#define VT_INVERSE   0x04
#define VT_DIM       0x08

typedef struct {
    unsigned char ch;    /* CP437 code */
    uint8_t fg, bg;      /* 0..15 */
    uint8_t attr;        /* VT_* bits */
} vt_cell_t;

typedef struct {
    vt_cell_t cells[VT_ROWS][VT_COLS];
    vt_cell_t alt[VT_ROWS][VT_COLS];      /* the other screen (?1049 / ?47) */
    int row, col;                         /* cursor */
    int saved_row, saved_col;             /* DECSC */
    uint8_t saved_fg, saved_bg, saved_attr;
    int top, bot;                         /* scroll region, inclusive */
    uint8_t fg, bg, attr;                 /* current SGR */
    int cursor_visible;
    int autowrap;
    int app_cursor;                       /* DECCKM: arrows send ESC O x */
    int insert_mode;                      /* IRM */
    int origin_mode;                      /* DECOM */
    int alt_active;
    int mouse_mode;                       /* 0 off, 1000 clicks, 1002 drag, 1003 any; see vt_mouse_report */
    int mouse_sgr;                        /* ?1006 */
    int wrap_pending;                     /* cursor past the last column, wrap on the next glyph */
    int charset[2], shift;                /* G0/G1: 0 ASCII, 1 DEC line drawing; SO/SI picks */
    uint8_t tabs[VT_COLS];
    int bell;                             /* set by BEL; the program clears it */
    int title_changed;                    /* OSC 0/2 seen */

    /* Parser */
    int state;
    int params[16], nparams, priv;
    char inter;                           /* intermediate byte, e.g. '(' */
    uint32_t utf_cp; int utf_left;
    char osc[64]; int osc_len;

    /* Bytes the terminal must send back to the program (DSR, DA). */
    char reply[64]; int reply_len;
    int dirty;
} vt_t;

void vt_init(vt_t *t);
void vt_feed(vt_t *t, const unsigned char *buf, size_t n);

/* Replies queued by vt_feed; the caller writes them to the pty and the buffer empties. */
int vt_take_reply(vt_t *t, char *out, int cap);

/* UTF-8 to CP437 (0 if the fonts have no glyph for it; the caller shows '?'). */
unsigned char vt_cp437(uint32_t cp);

/* Bytes a key sends, honouring DECCKM. Returns the length. */
typedef enum {
    VTK_UP, VTK_DOWN, VTK_LEFT, VTK_RIGHT, VTK_ENTER, VTK_ESC, VTK_TAB, VTK_BACKSPACE,
    VTK_PGUP, VTK_PGDN, VTK_HOME, VTK_END, VTK_INSERT, VTK_DELETE, VTK_F1, VTK_F2, VTK_F3, VTK_F4,
    VTK_F5, VTK_F6, VTK_F7, VTK_F8, VTK_F9, VTK_F10, VTK_SPACE
} vt_key_t;
int vt_key_bytes(const vt_t *t, vt_key_t k, char *out, int cap);

/* xterm mouse report for a program that asked for one (mouse_mode != 0), or 0 bytes if none did.
   button: 0 left, 1 middle, 2 right; release = 1 for the button going up; motion = 1 for a move
   with a button held (sent only in mode 1002/1003) or with none (1003 only). col/row 0-based. */
int vt_mouse_bytes(const vt_t *t, int button, int release, int motion, int col, int row, char *out, int cap);

/* Text dump for tests: VT_ROWS lines of the cells as ASCII ('?' for non-ASCII), each ending in
   '\n'; trailing spaces kept. Returns bytes written. */
size_t vt_dump(const vt_t *t, char *buf, size_t len);

#endif
