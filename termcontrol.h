#ifndef __TERMCONTROL_H__
#define __TERMCONTROL_H__

// TODO: 
// code refactor
// new funcs - get term size in chars

#include <stdio.h>
#include <stdint.h>
#include <termios.h>
#include <string.h>
#include <sys/select.h>
#include <sys/ioctl.h>

#define KEY_ESCAPE  0x001b
#define KEY_ENTER   0x000a
#define KEY_UP      0x0105
#define KEY_DOWN    0x0106
#define KEY_LEFT    0x0107
#define KEY_RIGHT   0x0108

static struct termios term, oterm;

///< is any key was pressed
int kbhit(void) {
    static const int STDIN = 0;
    static int initialized = 0;

    if ( !initialized ) {
        // Use termios to turn off line buffering
        struct termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = 1;
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}

///< get input symbol
int getch(void) {
    int c = 0;

    tcgetattr(0, &oterm);
    memcpy(&term, &oterm, sizeof(term));
    term.c_lflag &= ~(ICANON | ECHO);
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &term);
    c = getchar();
    tcsetattr(0, TCSANOW, &oterm);
    return c;
}

// return maped value for escaped keys like arrows
int kbesc() {
    int c;

    if (!kbhit()) return KEY_ESCAPE;
    c = getch();
    if (c == '[') {
        switch ( getch() ) {
            case 'A':
                c = KEY_UP;
                break;
            case 'B':
                c = KEY_DOWN;
                break;
            case 'C':
                c = KEY_RIGHT;
                break;
            case 'D':
                c = KEY_LEFT;
                break;
            default:
                c = 0;
                break;
        }

    } else {
        c = 0;
    }

    if (c == 0) {
        while ( kbhit() ) {
            getch();
        }
    }

    return c;
}

int kbget() {
    int c;

    c = getch();
    return (c == KEY_ESCAPE) ? kbesc() : c;
}

#define CR "\r"      // cursor return 
#define NL "\n"      // new line
#define BS "\b"      // backspase

#define ESC "\033"   // start escape sequance
#define CSI "["      // start csi control group
#define CU "%uA"     // cursor up
#define CD "%uB"     // cursor down
#define CF "%uC"     // cursor forward
#define CB "%uD"     // cursor back
#define CNL "%uE"    // cursor next line
#define CPL "%uF"    // cursor prev line
#define CUP "%u;%uH" // cursor position
#define CHIDE "?25l" // cursor hide
#define CSHOW "?25h" // cursor show

#define SGR "%um" // set graphic rendition
/* MODES:
 * 0 - reset
 * 1 - Bold or increased intensity
 * 2 - Faint, decreased intensity, or dim
 * 3 - Italic
 * 4 - Underline
 * 5 - Slow blink
 * 6 - Rapid blink
 * 21 - Doubly underlined; or: not bold
 * 22 - Normal intensity 
 * 23 - Neither italic, nor blackletter
 * 24 - Not underlined
 * 25 - Not blinking
 * 39 - Default foreground color
 * 49 - Default background color
 * and other more from ANSI escape code
 */

#define SCR_CLR "%1uJ" // clear screen (0:cursor-end, 1:cursor-begin, 2:all, 3:all+scrollback buffer )

#define COLOR_24_FG "38;2;%hhu;%hhu;%hhum"
#define COLOR_24_BG "48;2;%hhu;%hhu;%hhum"
#define COLOR_8_FG "38;5;%hhum"
#define COLOR_8_BG "48;5;%hhum"

typedef struct {
    uint8_t r, g, b;
} color_24;

typedef struct {
    uint8_t i;
} color_8;

void set_color_fg_24bit(color_24 color) {
    printf(ESC CSI COLOR_24_FG, color.r, color.g, color.b);
}

void set_color_bg_24bit(color_24 color) {
    printf(ESC CSI COLOR_24_BG, color.r, color.g, color.b);
}

void set_color_fg_8bit(color_8 color) {
    printf(ESC CSI COLOR_8_FG, color.i);
}

void set_color_bg_8bit(color_8 color) {
    printf(ESC CSI COLOR_8_BG, color.i);
}

void cursor_set(uint32_t x, uint32_t y) {
    printf(ESC CSI CUP, y, x);
}

void cursor_hide() {
    printf(ESC CSI CHIDE);
}

void cursor_show() {
    printf(ESC CSI CSHOW);
}

void cursor_move_up(uint32_t n) {
    printf(ESC CSI CU, n);
}

void cursor_move_down(uint32_t n) {
    printf(ESC CSI CD, n);
}

void cursor_move_forward(uint32_t n) {
    printf(ESC CSI CF, n);
}

void cursor_move_backward(uint32_t n) {
    printf(ESC CSI CB, n);
}

void cursor_move_next(uint32_t n) {
    printf(ESC CSI CNL, n);
}

void cursor_move_prev(uint32_t n) {
    printf(ESC CSI CPL, n);
}

void screen_clear() {
    printf(ESC CSI SCR_CLR, 2);
}

// Dont work correct
// void screen_clear_all() {
//     printf(ESC CSI SCR_CLR, 3);
// }

void set_mode(uint8_t mode) {
    if ( mode > 107 ) {
        mode = 107;
    }

    printf(ESC CSI SGR, mode);
}

void reset_all() {
    printf(ESC CSI SGR, 0);
}

#endif /* __TERMCONTROL_H__ */
