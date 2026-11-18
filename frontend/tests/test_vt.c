/* Redux Terminal screen model tests (no SDL, no pty). Exit 0 = pass. */
#include "vt.h"
#include "osk.h"
#include "cjk.h"
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
        unsigned char ch = t.cells[r][c].wide ? '?' : t.cells[r][c].ch;
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
    feed("\xd0\x96");                                         /* Ж: no glyph, narrow */
    expect(t.cells[0][9].ch == '?' && !t.cells[0][9].wide, "a narrow glyph the fonts lack shows as ?");
    feed("\xc3\xa9");                                         /* é */
    expect(t.cells[0][10].ch == 0x82, "Latin-1 letters map to CP437");
    feed("\x0e" "q" "\x0f" "q");                              /* SO selects G1 = ASCII by default */
    expect(t.cells[0][11].ch == 'q' && t.cells[0][12].ch == 'q', "SO/SI with G1 ASCII leaves text alone");
    feed("\x1b)0\x0e" "q" "\x0f");
    expect(t.cells[0][13].ch == 0xC4, "SO with G1 line drawing draws lines");

    /* East Asian wide glyphs take two cells (CJK decision A, 2026-11-17). */
    vt_init(&t);
    feed("a\xe4\xb8\xad\xe6\x96\x87" "b");        /* a中文b */
    expect(t.cells[0][1].wide == 1 && t.cells[0][1].cp == 0x4E2D && t.cells[0][2].wide == 2
           && t.cells[0][3].wide == 1 && t.cells[0][3].cp == 0x6587 && t.cells[0][5].ch == 'b' && t.col == 6,
           "a CJK glyph is two cells with its code point in the first");
    expect(!strcmp(row(0), "a????b"), "the dump shows a wide glyph as ??");
    expect(vt_is_wide(0x3042) && vt_is_wide(0xAC00) && vt_is_wide(0xFF21) && !vt_is_wide(0x00E9) && !vt_is_wide('a'),
           "kana, Hangul and fullwidth are wide; Latin is not");
    feed("\x1b[1;2H" "X");
    expect(t.cells[0][1].ch == 'X' && !t.cells[0][1].wide && t.cells[0][2].ch == ' ' && !t.cells[0][2].wide,
           "overwriting the first half blanks the second");
    feed("\x1b[1;5H" "Y");
    expect(t.cells[0][4].ch == 'Y' && t.cells[0][3].ch == ' ' && !t.cells[0][3].wide, "overwriting the second half blanks the first");
    feed("\x1b[2;90H" "\xe4\xb8\xad");                 /* no room in the last column: wraps first */
    expect(t.cells[1][89].ch == ' ' && t.cells[2][0].wide == 1 && t.cells[2][1].wide == 2 && t.col == 2,
           "a wide glyph at the last column wraps to the next row");
    feed("\x1b[3;89H" "\xe4\xb8\xad");                 /* exactly fits columns 89-90 */
    expect(t.cells[2][88].wide == 1 && t.cells[2][89].wide == 2 && t.wrap_pending, "a wide glyph that just fits sets wrap pending");
    feed("\xcc\x81");                                     /* combining acute: ignored */
    expect(t.wrap_pending && t.row == 2, "a combining mark draws nothing");

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

    /* Mouse reports (PR 2): nothing unless the program asked; X10 and SGR encodings. */
    vt_init(&t);
    expect(vt_mouse_bytes(&t, 0, 0, 0, 5, 5, out, sizeof out) == 0, "no mouse report when the program did not ask");
    feed("\x1b[?1000h");
    n = vt_mouse_bytes(&t, 0, 0, 0, 10, 3, out, sizeof out);
    expect(n == 6 && out[2] == 'M' && out[3] == 32 && out[4] == 32 + 11 && out[5] == 32 + 4, "X10 left press at column 11, row 4");
    n = vt_mouse_bytes(&t, 0, 1, 0, 10, 3, out, sizeof out);
    expect(n == 6 && out[3] == 32 + 3, "X10 release is button 3");
    expect(vt_mouse_bytes(&t, 0, 0, 1, 10, 3, out, sizeof out) == 0, "mode 1000 sends no motion");
    feed("\x1b[?1002h\x1b[?1006h");
    n = vt_mouse_bytes(&t, 2, 0, 0, 89, 29, out, sizeof out); out[n] = 0;
    expect(!strcmp(out, "\x1b[<2;90;30M"), "SGR right press at the last cell");
    n = vt_mouse_bytes(&t, 0, 0, 1, 0, 0, out, sizeof out); out[n] = 0;
    expect(!strcmp(out, "\x1b[<32;1;1M"), "SGR drag with the left button held");
    n = vt_mouse_bytes(&t, 0, 1, 0, 0, 0, out, sizeof out); out[n] = 0;
    expect(!strcmp(out, "\x1b[<0;1;1m"), "SGR release ends in m");
    expect(vt_mouse_bytes(&t, -1, 0, 1, 0, 0, out, sizeof out) == 0, "mode 1002 sends no motion without a button");
    feed("\x1b[?1003h");
    n = vt_mouse_bytes(&t, -1, 0, 1, 4, 4, out, sizeof out); out[n] = 0;
    expect(!strcmp(out, "\x1b[<35;5;5M"), "mode 1003 reports plain motion");
    feed("\x1b[?1000l");
    expect(t.mouse_mode == 0 && vt_mouse_bytes(&t, 0, 0, 0, 0, 0, out, sizeof out) == 0, "?1000l switches reports off");

    /* On-screen keys: the shared layout (mock-up A, three pages). */
    osk_t k;
    osk_init(&k);
    expect(!k.open && k.page == 0 && osk_cur(&k) && !strcmp(osk_cur(&k)->label, "q"), "the picker starts on q (page abc), closed");
    {
        int wide = 0;
        for (int p = 0; p < OSK_PAGES; p++)
            for (int r = 0; r < OSK_ROWS; r++) {
                int n = osk_cols(p, r), last = n - 1;
                if (n > OSK_MAX_COLS || osk_key_x2(p, r, last) + osk_key(p, r, last)->units2 != OSK_ROW_UNITS2) wide = 1;
            }
        expect(!wide, "every row on every page is exactly 10 key units wide");
    }
    expect(osk_press(&k, out, sizeof out) == 1 && out[0] == 'q', "A on q types q");
    osk_move(&k, 2, 0);   /* two rows down from q: Shift sits under it */
    expect(k.row == 2 && !strcmp(osk_cur(&k)->label, "Shift"), "moving down lands on the key below");
    expect(osk_press(&k, out, sizeof out) == 0 && k.shift, "pressing Shift sets Shift and types nothing");
    osk_move(&k, 0, 1);
    expect(!strcmp(osk_cur(&k)->label, "z") && osk_press(&k, out, sizeof out) == 1 && out[0] == 'Z' && k.shift,
           "Shift types upper-case and stays on");
    osk_page(&k, 1);
    expect(k.page == 1 && k.shift && k.row == 2, "R1: page 123, same row, Shift still on");
    k.row = 0; k.col = 1;
    expect(osk_press(&k, out, sizeof out) == 1 && out[0] == '@', "Shift on the digit row types the symbol");
    k.shift = 0;
    osk_page(&k, -1);
    k.row = 3; k.col = 0;
    expect(k.page == 0 && !strcmp(osk_cur(&k)->label, "Ctrl") && osk_press(&k, out, sizeof out) == 0 && k.ctrl, "Ctrl is one-shot");
    osk_move(&k, -1, 0);   /* up from Ctrl: Shift */
    k.col = 3;             /* c */
    expect(osk_press(&k, out, sizeof out) == 1 && out[0] == 3 && !k.ctrl, "Ctrl-C sends byte 3 and clears Ctrl");
    osk_page(&k, -1);
    expect(k.page == 2, "L1 from abc wraps to Fn");
    k.row = 0; k.col = 0;
    expect(osk_press(&k, out, sizeof out) == 3 && out[0] == 0x1B && out[2] == 'P', "F1 sends SS3 P");
    osk_move(&k, 0, -1);
    expect(!strcmp(osk_cur(&k)->label, "F10"), "left from the first key wraps to the last");
    osk_move(&k, -1, 0);
    expect(k.row == OSK_ROWS - 1, "up from the first row wraps to the last");
    k.col = 7;
    expect(osk_press(&k, out, sizeof out) == 3 && !memcmp(out, "\x1b[D", 3), "the left arrow key sends CSI D");
    expect(!strcmp(osk_page_name(0), "abc") && !strcmp(osk_page_name(1), "123") && !strcmp(osk_page_name(2), "Fn"),
           "page names abc, 123, Fn");

    /* CJK glyph table (CJK PR 2): Unifont 16x16 for 中, kana and fullwidth; nothing for Latin. */
    {
        const uint8_t *g = cjk_glyph(0x4E2D);   /* 中 */
        expect(g != NULL && g[8] == 0x3F && g[9] == 0xF8, "中 has its Unifont bitmap (row 4 = 3FF8)");
        expect(cjk_glyph(0x3042) != NULL && cjk_glyph(0xFF21) != NULL, "hiragana あ and fullwidth A have glyphs");
        expect(cjk_glyph('a') == NULL && cjk_glyph(0xAC00) == NULL, "Latin and Hangul (not in the table yet) have none");
        static const unsigned char bad[] = "RDXCJK1\0\x05";
        expect(cjk_set_blob(bad, sizeof bad - 1) < 0 && cjk_glyph(0x4E2D) == NULL, "a truncated blob is refused");
        expect(cjk_set_blob(NULL, 0) == 3 && cjk_glyph(0x4E2D) != NULL, "the embedded blob has three ranges");
    }

    printf("%s\n", fails ? "test_vt: FAILED" : "test_vt: all passed");
    return fails ? 1 : 0;
}
