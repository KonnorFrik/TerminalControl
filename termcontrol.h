#ifndef __TERMCONTROL_H__
#define __TERMCONTROL_H__

// TODO: 
// new funcs - get term size in chars
// preprocessing for controling which would be included
// make functions prototypes

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <termios.h>
#include <string.h>
#include <sys/select.h>
#include <sys/ioctl.h>

// Key mapping
#define KEY_ESCAPE  0x001b
#define KEY_ENTER   0x000a
#define KEY_UP      0x0105
#define KEY_DOWN    0x0106
#define KEY_LEFT    0x0107
#define KEY_RIGHT   0x0108

// Control sequances
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
#define CUP "%u;%uH" // cursor position y;x
#define CHIDE "?25l" // cursor hide
#define CSHOW "?25h" // cursor show

/** @brief MODES for SGR macros:
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
#define SGR "%um" // set graphic rendition

#define SCR_CLR "%1uJ" // clear screen (0:cursor-end, 1:cursor-begin, 2:all, 3:all+scrollback buffer )

#define COLOR_24_FG "38;2;%hhu;%hhu;%hhum"
#define COLOR_24_BG "48;2;%hhu;%hhu;%hhum"
#define COLOR_8_FG "38;5;%hhum"
#define COLOR_8_BG "48;5;%hhum"

// Misc macros
#define TOSTR(val) #val
#define MACROTOSTR(macro) TOSTR(macro)

#define _STDIN 0

/** @brief RGB components of color */
typedef struct {
    uint8_t r, g, b;
} color_24;

/** @brief Index to color table */
typedef struct {
    uint8_t i;
} color_8;

// Prototypes
void term_deinit();
void term_init();
void echo_off();
void echo_on();
void reset_all();
void set_mode(uint8_t mode);
void screen_clear();
void str_to(uint32_t x, uint32_t y, const char* s);
void symbol_to(uint32_t x, uint32_t y, char s);
void cursor_move_prev(uint32_t n);
void cursor_move_next(uint32_t n);
void cursor_move_backward(uint32_t n);
void cursor_move_forward(uint32_t n);
void cursor_move_down(uint32_t n);
void cursor_move_up(uint32_t n);
void cursor_show();
void cursor_hide();
void cursor_set(uint32_t x, uint32_t y);
void set_color_bg_8(color_8 color);
void set_color_fg_8(color_8 color);
void set_color_bg_24(color_24 color);
void set_color_fg_24(color_24 color);
char* str_get_from(uint32_t x, uint32_t y, size_t* size_out);
int kbget();
int kbesc();
int getch();
int kbhit();


// Input related functions

/** @brief Get status if keyboard key was pressed
 * @return result - Count of bytes in stdin
 * @return result - 0:NO_PRESSED 1+:PRESSED
 */
int kbhit() {
    int bytesWaiting;
    ioctl(_STDIN, FIONREAD, &bytesWaiting);

    return bytesWaiting;
}

/** @brief Return copy of symol from stdin
 * @return symbol copied from stdin
 */
int getch() {
    struct termios term = {0}, oterm = {0};
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

/** @brief Proccess pressed key if key is ESC
 * @return mapped_symbol For arrows, 0 otherwise
 */
int kbesc() {
    int c;

    if ( !kbhit() ) {
        return KEY_ESCAPE;
    }

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
        if ( kbhit() ) {
            fflush(stdin);
        }
    }

    return c;
}

/** @brief Get pressed key. Return mapped key if key is ESC
 * @return symbol
 */
int kbget() {
    int c = getch();

    return (c == KEY_ESCAPE) ? kbesc() : c;
}

/** @brief Get line from stdin 
 * @note Don't forget to free returned line
 * @param[in]  x Cursor position for x
 * @param[in]  y Cursor position for y
 * @param[out] size_out Size of returned line ( NOT lingth)
 * @return line Readed line, NULL otherwise
 */
char* str_get_from(uint32_t x, uint32_t y, size_t* size_out) {
    char* line = NULL;
    size_t line_size = 0;

    cursor_set(x, y);

    // use getline for get input
    if ( getline(&line, &line_size, stdin) == -1 ) {
        if ( line ) {
            free(line);
            line = NULL;
            line_size = 0;
        }
    }

    // write size if size_out not NULL
    if ( size_out ) {
        *size_out = line_size;
    }

    return line;
}

// Color related functions
void set_color_fg_24(color_24 color) {
    printf(ESC CSI COLOR_24_FG, color.r, color.g, color.b);
}

void set_color_bg_24(color_24 color) {
    printf(ESC CSI COLOR_24_BG, color.r, color.g, color.b);
}

void set_color_fg_8(color_8 color) {
    printf(ESC CSI COLOR_8_FG, color.i);
}

void set_color_bg_8(color_8 color) {
    printf(ESC CSI COLOR_8_BG, color.i);
}

// Cursor related functions
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

/** @brief Move cursor to position and print symbol
 * @param[in] x Position for x
 * @param[in] y Position for y
 * @param[in] s Symbol for print
 */
void symbol_to(uint32_t x, uint32_t y, char s) {
    printf(ESC CSI CUP "%c", y, x, s);
}

/** @brief Move cursor to position and print strings
 * @param[in] x Position for x
 * @param[in] y Position for y
 * @param[in] s String for print
 */
void str_to(uint32_t x, uint32_t y, const char* s) {
    printf(ESC CSI CUP "%s", y, x, s);
}

// Screen related functions
void screen_clear() {
    printf(ESC CSI SCR_CLR, 2);
}

// Dont work correct
// void screen_clear_all() {
//     printf(ESC CSI SCR_CLR, 3);
// }

// Misc functions
void set_mode(uint8_t mode) {
    if ( mode > 107 ) {
        mode = 107;
    }

    printf(ESC CSI SGR, mode);
}

void reset_all() {
    printf(ESC CSI SGR, 0);
}

void echo_on() {
    struct termios term;
    tcgetattr(_STDIN, &term);
    term.c_lflag |= ECHO;
    tcsetattr(_STDIN, TCSANOW, &term);
}

void echo_off() {
    struct termios term;
    tcgetattr(_STDIN, &term);
    term.c_lflag &= ~ECHO;
    tcsetattr(_STDIN, TCSANOW, &term);
}

void term_init() {
    struct termios term;
    tcgetattr(_STDIN, &term);
    term.c_lflag &= ~ICANON;
    tcsetattr(_STDIN, TCSANOW, &term);

    setbuf(stdin, NULL);
}

void term_deinit() {
    struct termios term;
    tcgetattr(_STDIN, &term);
    term.c_lflag |= ICANON;
    tcsetattr(_STDIN, TCSANOW, &term);
}


#endif /* __TERMCONTROL_H__ */
