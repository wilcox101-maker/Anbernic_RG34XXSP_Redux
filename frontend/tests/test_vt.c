/* Redux Terminal screen model tests (no SDL, no pty). Exit 0 = pass. */
#include "vt.h"
#include <stdio.h>
#include <string.h>

static int fails;
static vt_t t;

static void expect(int ok, const char *what) {
    printf("%s  %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) fails++;
}

static void feed(const char *s) { vt_feed(&t, (const unsigned char *)s, strlen(s)); }

/* Row r of the screen as ASCII, trailing spaces trimmed. */
static const char *row(int r) {
    static char line[VT_COLS + 1];
    for (int c = 0; c < VT_COLS; c++) {
        unsigned char ch = t.cells[r][c].ch;
        line[c] = (ch >= 0x20 && ch < 0x7F) ? (char)ch : '?';
    }
    int n = VT_COLS;
    while (n > 0 && line[n - 1] == ' ') n--;
    line[n] = 0;
    return line;
}

int main(void) {
    vt_init(&t);
    expect(t.row == 0 && t.col == 0 && t.cursor_visible && t.bot == VT_ROWS - 1, "init: cursor home, full scroll region");

    feed("hello\r\nworld");
    expect(!strcmp(row(0), "hello") && !strcmp(row(1), "world") && t.row == 1 && t.col == 5, "plain text, CR LF");

    /* Autowrap: 90 columns, the 91st glyph lands on the next row. */
    vt_init(&t);
    for (int i = 0; i < 90; i++) feed("x");
    expect(t.col == 89 && t.wrap_pending && t.row == 0, "wrap is pending at the last column");
    feed("y");
    expect(t.row == 1 && t.col == 1 && t.cells[1][0].ch == 'y', "the next glyph wraps");
    feed("\r");
    for (int i = 0; i < 90; i++) feed("x");
    feed("\r\n");   /* CR LF right after the last column must not add an empty line */
    expect(t.row == 2 && !strcmp(row(2), ""), "CR LF after a full row moves one row, not two");

    /* Cursor movement, absolute and relative. */
    vt_init(&t);
    feed("\x1b[5;10Habc");
    expect(t.row == 4 && t.col == 12 && t.cells[4][9].ch == 'a', "CUP is 1-based");
    feed("\x1b[2A\x1b[3D");
    expect(t.row == 2 && t.col == 9, "CUU and CUB");
    feed("\x1b[99B\x1b[99C");
    expect(t.row == VT_ROWS - 1 && t.col == VT_COLS - 1, "moves clamp at the edges");
    feed("\x1b[H");
    expect(t.row == 0 && t.col == 0, "CUP without parameters is home");

    /* Erase. */
    vt_init(&t);
    feed("abcdef\x1b[3D\x1b[K");
    expect(!strcmp(row(0), "abc"), "EL 0 erases to the end of the line");
    feed("\r\n0123456789\x1b[5D\x1b[1K");
    expect(!strcmp(row(1), "      6789"), "EL 1 erases to the cursor inclusive");
    feed("\x1b[2J");
    expect(!strcmp(row(0), "") && !strcmp(row(1), ""), "ED 2 clears the screen");
    feed("\x1b[1;1Hline0\r\nline1\r\nline2\x1b[2;1H\x1b[J");
    expect(!strcmp(row(0), "line0") && !strcmp(row(1), "") && !strcmp(row(2), ""), "ED 0 erases from the cursor down");

    /* Scrolling at the bottom. */
    vt_init(&t);
    for (int i = 0; i < 31; i++) { char b[8]; snprintf(b, sizeof b, "L%d\r\n", i); feed(b); }
    expect(!strcmp(row(0), "L2") && !strcmp(row(VT_ROWS - 2), "L30") && t.row == VT_ROWS - 1, "LF at the bottom scrolls up");

    /* Scroll region: ESC [ 2 ; 5 r keeps the title row. */
    vt_init(&t);
    feed("TITLE\x1b[2;5r");
    expect(t.top == 1 && t.bot == 4 && t.row == 0, "DECSTBM sets the region and homes the cursor");
    feed("\x1b[5;1Hbottom\n");
    expect(!strcmp(row(0), "TITLE") && !strcmp(row(3), "bottom") && !strcmp(row(4), ""), "LF at the region bottom scrolls only the region");
    feed("\x1b[2;1H\x1bM");
    expect(!strcmp(row(1), "") && !strcmp(row(4), "bottom"), "RI at the region top scrolls the region down");
    feed("\x1b[r");
    expect(t.top == 0 && t.bot == VT_ROWS - 1, "DECSTBM without parameters is the whole screen");

    /* Insert / delete lines and characters. */
    vt_init(&t);
    feed("one\r\ntwo\r\nthree\x1b[2;1H\x1b[L");
    expect(!strcmp(row(1), "") && !strcmp(row(2), "two") && !strcmp(row(3), "three"), "IL inserts a blank line");
    feed("\x1b[M");
    expect(!strcmp(row(1), "two") && !strcmp(row(2), "three") && !strcmp(row(3), ""), "DL deletes it again");
    feed("\x1b[1;1H\x1b[P");
    expect(!strcmp(row(0), "ne"), "DCH deletes a character");
    feed("\x1b[2@");
    expect(!strcmp(row(0), "  ne"), "ICH inserts blanks");
    feed("\x1b[4h\x1b[1;1HX\x1b[4l");
    expect(!strcmp(row(0), "X  ne"), "IRM inserts instead of overwriting");

    /* SGR: colours and attributes land in the cells. */
    vt_init(&t);
    feed("\x1b[1;31;44mA\x1b[0mB\x1b[7mC\x1b[27m\x1b[92mD\x1b[38;5;196mE\x1b[48;2;0;0;255mF");
    expect(t.cells[0][0].fg == 1 && t.cells[0][0].bg == 4 && (t.cells[0][0].attr & VT_BOLD), "bold red on blue");
    expect(t.cells[0][1].fg == VT_DEFAULT_FG && t.cells[0][1].bg == VT_DEFAULT_BG && t.cells[0][1].attr == 0, "SGR 0 resets");
    expect(t.cells[0][2].attr & VT_INVERSE, "inverse");
    expect(t.cells[0][3].fg == 10 && !(t.cells[0][3].attr & VT_INVERSE), "bright green, inverse off");
    expect(t.cells[0][4].fg == 9, "256-colour 196 maps to bright red");
    expect(t.cells[0][5].bg == 4, "RGB blue maps to blue");

    /* Erase uses the current background (bce), as curses expects. */
    vt_init(&t);
    feed("\x1b[44m\x1b[2J");
    expect(t.cells[5][5].bg == 4 && t.cells[5][5].ch == ' ', "ED paints the background colour");

    /* UTF-8 and DEC line drawing both reach CP437 box characters. */
    vt_init(&t);
    feed("\xe2\x94\x8c\xe2\x94\x80\xe2\x94\x90");          /* ┌─┐ */
    expect(t.cells[0][0].ch == 0xDA && t.cells[0][1].ch == 0xC4 && t.cells[0][2].ch == 0xBF && t.col == 3, "UTF-8 box drawing");
    feed("\x1b(0lqk\x1b(B");
    expect(t.cells[0][3].ch == 0xDA && t.cells[0][4].ch == 0xC4 && t.cells[0][5].ch == 0xBF, "DEC special graphics");
    feed("lqk");
    expect(t.cells[0][6].ch == 'l', "back to ASCII after ESC ( B");
    feed("\xe6\x97\xa5");                                     /* 日: no glyph */
    expect(t.cells[0][9].ch == '?', "a glyph the fonts lack shows as ?");
    feed("\xc3\xa9");                                         /* é */
    expect(t.cells[0][10].ch == 0x82, "Latin-1 letters map to CP437");
    feed("\x0e" "q" "\x0f" "q");                              /* SO selects G1 = ASCII by default */
    expect(t.cells[0][11].ch == 'q' && t.cells[0][12].ch == 'q', "SO/SI with G1 ASCII leaves text alone");
    feed("\x1b)0\x0e" "q" "\x0f");
    expect(t.cells[0][13].ch == 0xC4, "SO with G1 line drawing draws lines");

    /* Alternate screen (curses programs) restores the shell's screen on exit. */
    vt_init(&t);
    feed("shell prompt\x1b[?1049h");
    expect(!strcmp(row(0), "") && t.alt_active, "?1049h switches to a blank alternate screen");
    feed("full-screen app\x1b[?1049l");
    expect(!strcmp(row(0), "shell prompt") && !t.alt_active && t.col == 12, "?1049l brings the main screen and cursor back");

    /* Modes and replies. */
    vt_init(&t);
    char out[32];
    expect(vt_key_bytes(&t, VTK_UP, out, sizeof out) == 3 && out[1] == '[' && out[2] == 'A', "arrows send CSI by default");
    feed("\x1b[?1h");
    expect(vt_key_bytes(&t, VTK_UP, out, sizeof out) == 3 && out[1] == 'O', "DECCKM: arrows send SS3");
    feed("\x1b[?25l");
    expect(!t.cursor_visible, "?25l hides the cursor");
    feed("\x1b[?1000h\x1b[?1006h");
    expect(t.mouse_mode == 1000 && t.mouse_sgr, "mouse modes are remembered for the program");
    feed("\x1b[3;7H\x1b[6n");
    int n = vt_take_reply(&t, out, sizeof out);
    out[n] = 0;
    expect(!strcmp(out, "\x1b[3;7R"), "DSR 6 reports the cursor position");
    expect(vt_take_reply(&t, out, sizeof out) == 0, "the reply buffer empties");
    feed("\x1b[c");
    n = vt_take_reply(&t, out, sizeof out); out[n] = 0;
    expect(!strcmp(out, "\x1b[?1;2c"), "DA answers as a VT100");

    /* OSC title and unknown sequences are swallowed, not drawn. */
    vt_init(&t);
    feed("\x1b]0;my title\x07" "A\x1b]2;x\x1b\\" "B\x1b[>c" "C\x1b[?2004h" "D\x1bP junk\x1b\\" "E\x1b[1 q" "F");
    expect(!strcmp(row(0), "ABCDEF") && t.title_changed, "OSC, DCS and unknown CSI leave no text behind");

    /* Tabs and backspace. */
    vt_init(&t);
    feed("a\tb\b\b" "c");
    expect(t.cells[0][8].ch == 'b' && t.cells[0][7].ch == 'c' && t.col == 8, "TAB to column 8, BS back");

    /* Save / restore cursor keeps attributes too. */
    vt_init(&t);
    feed("\x1b[31m\x1b" "7\x1b[5;5H\x1b[0m\x1b" "8X");
    expect(t.cells[0][0].ch == 'X' && t.cells[0][0].fg == 1, "DECSC/DECRC restore position and colour");

    /* Bell is flagged for the program. */
    vt_init(&t);
    feed("\x07");
    expect(t.bell, "BEL sets the bell flag");

    /* Dump shape. */
    static char dump[VT_ROWS * (VT_COLS + 1) + 1];
    size_t len = vt_dump(&t, dump, sizeof dump);
    expect(len == (size_t)VT_ROWS * (VT_COLS + 1) && dump[VT_COLS] == '\n', "dump is 30 lines of 90 characters");

    printf("%s\n", fails ? "test_vt: FAILED" : "test_vt: all passed");
    return fails ? 1 : 0;
}
