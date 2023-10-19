#include <ctype.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <locale.h>
#include <wchar.h>

#include <ncursesw/curses.h>

#include "emulator.h"
#include "romio.h"
#include "runtime_options.h"
#include "ui.h"
#include "ui_inner.h"

#define LCD_WIDTH 131
#define LCD_HEIGHT 64
#define LCD_OFFSET_X 1
#define LCD_OFFSET_Y 1
#define LCD_BOTTOM LCD_OFFSET_Y + ( small ? ( LCD_HEIGHT / 2 ) : tiny ? ( LCD_HEIGHT / 4 ) : LCD_HEIGHT )
#define LCD_RIGHT LCD_OFFSET_X + ( ( small || tiny ) ? ( LCD_WIDTH / 2 ) + 1 : LCD_WIDTH )

#define LCD_COLOR_BG 48
#define LCD_COLOR_FG 49

#define LCD_PIXEL_ON 1
#define LCD_PIXEL_OFF 2
#define LCD_COLORS_PAIR 3

/************************/
/* functions prototypes */
/************************/
void text_update_LCD( void );

/****************************/
/* functions implementation */
/****************************/

static inline void ncurses_draw_annunciators( void )
{
    wchar_t* annunciators_icons[ 6 ] = { L"↰", L"↱", L"α", L"🪫", L"⌛", L"⇄" };
    int val = display.annunc;

    if ( val == last_annunc_state )
        return;

    last_annunc_state = val;

    for ( int i = 0; i < NB_ANNUNCIATORS; i++ )
        mvaddwstr( 0, 4 + ( i * 4 ), ( ( annunciators_bits[ i ] & val ) == annunciators_bits[ i ] ) ? annunciators_icons[ i ] : L" " );
}

static inline wchar_t eight_bits_to_braille_char( bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7, bool b8 )
{
    /*********/
    /* b1 b4 */
    /* b2 b5 */
    /* b3 b6 */
    /* b7 b8 */
    /*********/
    wchar_t chr = 0x2800;

    if ( b1 )
        chr |= 0b0000000000000001;
    if ( b2 )
        chr |= 0b0000000000000010;
    if ( b3 )
        chr |= 0b0000000000000100;
    if ( b4 )
        chr |= 0b0000000000001000;
    if ( b5 )
        chr |= 0b0000000000010000;
    if ( b6 )
        chr |= 0b0000000000100000;
    if ( b7 )
        chr |= 0b0000000001000000;
    if ( b8 )
        chr |= 0b0000000010000000;

    return chr;
}

static inline void ncurses_draw_lcd_tiny( void )
{
    bool b1, b2, b3, b4, b5, b6, b7, b8;
    int nibble_top, nibble_middle_top, nibble_middle_bottom, nibble_bottom;
    int step_x = 2;
    int step_y = 4;

    wchar_t line[ 66 ]; /* ( LCD_WIDTH / step_x ) + 1 */

    if ( !mono && has_colors() )
        attron( COLOR_PAIR( LCD_COLORS_PAIR ) );

    for ( int y = 0; y < LCD_HEIGHT; y += step_y ) {
        wcscpy( line, L"" );

        for ( int nibble_x = 0; nibble_x < NIBBLES_PER_ROW - 1; ++nibble_x ) {
            nibble_top = lcd_nibbles_buffer[ y ][ nibble_x ];
            nibble_top &= 0x0f;

            nibble_middle_top = lcd_nibbles_buffer[ y + 1 ][ nibble_x ];
            nibble_middle_top &= 0x0f;

            nibble_middle_bottom = lcd_nibbles_buffer[ y + 2 ][ nibble_x ];
            nibble_middle_bottom &= 0x0f;

            nibble_bottom = lcd_nibbles_buffer[ y + 3 ][ nibble_x ];
            nibble_bottom &= 0x0f;

            for ( int bit_x = 0; bit_x < NIBBLES_NB_BITS; bit_x += step_x ) {
                b1 = 0 != ( nibble_top & ( 1 << ( bit_x & 3 ) ) );
                b4 = 0 != ( nibble_top & ( 1 << ( ( bit_x + 1 ) & 3 ) ) );

                b2 = 0 != ( nibble_middle_top & ( 1 << ( bit_x & 3 ) ) );
                b5 = 0 != ( nibble_middle_top & ( 1 << ( ( bit_x + 1 ) & 3 ) ) );

                b3 = 0 != ( nibble_middle_bottom & ( 1 << ( bit_x & 3 ) ) );
                b6 = 0 != ( nibble_middle_bottom & ( 1 << ( ( bit_x + 1 ) & 3 ) ) );

                b7 = 0 != ( nibble_bottom & ( 1 << ( bit_x & 3 ) ) );
                b8 = 0 != ( nibble_bottom & ( 1 << ( ( bit_x + 1 ) & 3 ) ) );

                wchar_t pixels = eight_bits_to_braille_char( b1, b2, b3, b4, b5, b6, b7, b8 );
                wcsncat( line, &pixels, 1 );
            }
        }
        mvaddwstr( LCD_OFFSET_Y + ( y / step_y ), LCD_OFFSET_X, line );
    }

    if ( !mono && has_colors() )
        attroff( COLOR_PAIR( LCD_COLORS_PAIR ) );

    wrefresh( stdscr );
}

static inline wchar_t four_bits_to_quadrant_char( bool top_left, bool top_right, bool bottom_left, bool bottom_right )
{
    if ( top_left ) {
        if ( top_right ) {
            if ( bottom_left )
                return bottom_right ? L'█' : L'▛'; /* 0x2588 0x2598 */
            else
                return bottom_right ? L'▜' : L'▀'; /* 0x259C 0x2580 */
        } else {
            if ( bottom_left )
                return bottom_right ? L'▙' : L'▌';
            else
                return bottom_right ? L'▚' : L'▘';
        }
    } else {
        if ( top_right ) {
            if ( bottom_left )
                return bottom_right ? L'▟' : L'▞';
            else
                return bottom_right ? L'▐' : L'▝';
        } else {
            if ( bottom_left )
                return bottom_right ? L'▄' : L'▖';
            else
                return bottom_right ? L'▗' : L' ';
        }
    }
}

static inline void ncurses_draw_lcd_small( void )
{
    bool top_left, top_right, bottom_left, bottom_right;
    int nibble_top, nibble_bottom;
    int step_x = 2;
    int step_y = 2;

    wchar_t line[ 66 ]; /* ( LCD_WIDTH / step_x ) + 1 */

    if ( !mono && has_colors() )
        attron( COLOR_PAIR( LCD_COLORS_PAIR ) );

    for ( int y = 0; y < LCD_HEIGHT; y += step_y ) {
        wcscpy( line, L"" );

        for ( int nibble_x = 0; nibble_x < NIBBLES_PER_ROW - 1; ++nibble_x ) {
            nibble_top = lcd_nibbles_buffer[ y ][ nibble_x ];
            nibble_top &= 0x0f;
            nibble_bottom = lcd_nibbles_buffer[ y + 1 ][ nibble_x ];
            nibble_bottom &= 0x0f;

            for ( int bit_x = 0; bit_x < NIBBLES_NB_BITS; bit_x += step_x ) {
                top_left = 0 != ( nibble_top & ( 1 << ( bit_x & 3 ) ) );
                top_right = 0 != ( nibble_top & ( 1 << ( ( bit_x + 1 ) & 3 ) ) );

                bottom_left = 0 != ( nibble_bottom & ( 1 << ( bit_x & 3 ) ) );
                bottom_right = 0 != ( nibble_bottom & ( 1 << ( ( bit_x + 1 ) & 3 ) ) );

                wchar_t pixels = four_bits_to_quadrant_char( top_left, top_right, bottom_left, bottom_right );
                wcsncat( line, &pixels, 1 );
            }
        }
        mvaddwstr( LCD_OFFSET_Y + ( y / step_y ), LCD_OFFSET_X, line );
    }

    if ( !mono && has_colors() )
        attroff( COLOR_PAIR( LCD_COLORS_PAIR ) );

    wrefresh( stdscr );
}

static inline void ncurses_draw_lcd( void )
{
    bool bit;
    int nibble;
    int bit_stop;
    int init_x;

    wchar_t line[ LCD_WIDTH ];

    if ( !mono && has_colors() )
        attron( COLOR_PAIR( LCD_COLORS_PAIR ) );

    for ( int y = 0; y < LCD_HEIGHT; ++y ) {
        wcscpy( line, L"" );

        for ( int nibble_x = 0; nibble_x < NIBBLES_PER_ROW; ++nibble_x ) {
            nibble = lcd_nibbles_buffer[ y ][ nibble_x ];
            nibble &= 0x0f;

            init_x = nibble_x * NIBBLES_NB_BITS;
            bit_stop = ( ( init_x + NIBBLES_NB_BITS >= LCD_WIDTH ) ? LCD_WIDTH - init_x : 4 );

            for ( int bit_x = 0; bit_x < bit_stop; bit_x++ ) {
                bit = 0 != ( nibble & ( 1 << ( bit_x & 3 ) ) );

                wchar_t pixel = bit ? L'█' : L' ';
                wcsncat( line, &pixel, 1 );
            }
        }
        mvaddwstr( LCD_OFFSET_Y + y, LCD_OFFSET_X, line );
    }

    if ( !mono && has_colors() )
        attroff( COLOR_PAIR( LCD_COLORS_PAIR ) );

    wrefresh( stdscr );
}

static inline int ncurses_get_event( void )
{
    int hpkey = -1;
    uint32_t k;

    /* Start fresh and mark all keys as released */
    release_all_keys();

    /* Iterate over all currently pressed keys and mark them as pressed */
    while ( ( k = getch() ) ) {
        if ( k == ( uint32_t )ERR )
            break;

        switch ( k ) {
            case '0':
                hpkey = HPKEY_0;
                break;
            case '1':
                hpkey = HPKEY_1;
                break;
            case '2':
                hpkey = HPKEY_2;
                break;
            case '3':
                hpkey = HPKEY_3;
                break;
            case '4':
                hpkey = HPKEY_4;
                break;
            case '5':
                hpkey = HPKEY_5;
                break;
            case '6':
                hpkey = HPKEY_6;
                break;
            case '7':
                hpkey = HPKEY_7;
                break;
            case '8':
                hpkey = HPKEY_8;
                break;
            case '9':
                hpkey = HPKEY_9;
                break;
            case 'a':
                hpkey = HPKEY_A;
                break;
            case 'b':
                hpkey = HPKEY_B;
                break;
            case 'c':
                hpkey = HPKEY_C;
                break;
            case 'd':
                hpkey = HPKEY_D;
                break;
            case 'e':
                hpkey = HPKEY_E;
                break;
            case 'f':
                hpkey = HPKEY_F;
                break;
            case 'g':
                hpkey = HPKEY_MTH;
                break;
            case 'h':
                hpkey = HPKEY_PRG;
                break;
            case 'i':
                hpkey = HPKEY_CST;
                break;
            case 'j':
                hpkey = HPKEY_VAR;
                break;
            case 'k':
                hpkey = HPKEY_UP;
                break;
            case KEY_UP:
                hpkey = HPKEY_UP;
                break;
            case 'l':
                hpkey = HPKEY_NXT;
                break;
            case 'm':
                hpkey = HPKEY_COLON;
                break;
            case 'n':
                hpkey = HPKEY_STO;
                break;
            case 'o':
                hpkey = HPKEY_EVAL;
                break;
            case 'p':
                hpkey = HPKEY_LEFT;
                break;
            case KEY_LEFT:
                hpkey = HPKEY_LEFT;
                break;
            case 'q':
                hpkey = HPKEY_DOWN;
                break;
            case KEY_DOWN:
                hpkey = HPKEY_DOWN;
                break;
            case 'r':
                hpkey = HPKEY_RIGHT;
                break;
            case KEY_RIGHT:
                hpkey = HPKEY_RIGHT;
                break;
            case 's':
                hpkey = HPKEY_SIN;
                break;
            case 't':
                hpkey = HPKEY_COS;
                break;
            case 'u':
                hpkey = HPKEY_TAN;
                break;
            case 'v':
                hpkey = HPKEY_SQRT;
                break;
            case 'w':
                hpkey = HPKEY_POWER;
                break;
            case 'x':
                hpkey = HPKEY_INV;
                break;
            case 'y':
                hpkey = HPKEY_NEG;
                break;
            case 'z':
                hpkey = HPKEY_EEX;
                break;
            case ' ':
                hpkey = HPKEY_SPC;
                break;
            case KEY_ENTER:
            case '\n':
            case ',':
                hpkey = HPKEY_ENTER;
                break;
            case KEY_BACKSPACE:
            case 127:
            case '\b':
                hpkey = HPKEY_BS;
                break;
            case KEY_DC:
                hpkey = HPKEY_DEL;
                break;
            case '.':
                hpkey = HPKEY_PERIOD;
                break;
            case '+':
                hpkey = HPKEY_PLUS;
                break;
            case '-':
                hpkey = HPKEY_MINUS;
                break;
            case '*':
                hpkey = HPKEY_MUL;
                break;
            case '/':
                hpkey = HPKEY_DIV;
                break;

            case '[':
            case 339: /* PgUp */
            case KEY_F( 5 ):
                hpkey = HPKEY_SHL;
                break;
            case ']':
            case 338: /* PgDn */
            case KEY_F( 6 ):
                hpkey = HPKEY_SHR;
                break;
            case ';':
            case KEY_IC: /* Ins */
            case KEY_F( 7 ):
            case KEY_F( 8 ):
                hpkey = HPKEY_ALPHA;
                break;
            case '\\':
                /* case KEY_ESC: */
            case 27:  /* Esc */
            case 262: /* Home */
            case KEY_F( 4 ):
                hpkey = HPKEY_ON;
                break;

            case '|':      /* Shift+\ */
            case KEY_SEND: /* Shift+End */
            case KEY_F( 1 ):
            case KEY_F( 10 ):
                nodelay( stdscr, FALSE );
                echo();

                endwin();
                exit_emulator();
                exit( 0 );
                break;
        }

        if ( !keyboard[ hpkey ].pressed )
            press_key( hpkey );
    }

    text_update_LCD();

    return 1;
}

static inline void ncurses_init_ui( void )
{
    setlocale( LC_ALL, "" );
    initscr();              /* initialize the curses library */
    keypad( stdscr, TRUE ); /* enable keyboard mapping */
    nodelay( stdscr, TRUE );
    curs_set( 0 );
    cbreak(); /* take input chars one at a time, no wait for \n */
    noecho();
    nonl(); /* tell curses not to do NL->CR/NL on output */

    if ( !mono && has_colors() ) {
        start_color();

        if ( gray ) {
            init_color( LCD_COLOR_BG, 205, 205, 205 );
            init_color( LCD_COLOR_FG, 20, 20, 20 );
        } else {
            init_color( LCD_COLOR_BG, 202, 221, 92 );
            init_color( LCD_COLOR_FG, 0, 0, 128 );
        }

        init_pair( LCD_PIXEL_OFF, LCD_COLOR_BG, LCD_COLOR_BG );
        init_pair( LCD_PIXEL_ON, LCD_COLOR_FG, LCD_COLOR_FG );
        init_pair( LCD_COLORS_PAIR, LCD_COLOR_FG, LCD_COLOR_BG );
    }

    mvaddch( 0, 0, ACS_ULCORNER );
    mvaddch( LCD_BOTTOM, 0, ACS_LLCORNER );
    mvaddch( 0, LCD_RIGHT, ACS_URCORNER );
    mvaddch( LCD_BOTTOM, LCD_RIGHT, ACS_LRCORNER );
    mvhline( 0, 1, ACS_HLINE, LCD_RIGHT - 1 );
    mvhline( LCD_BOTTOM, 1, ACS_HLINE, LCD_RIGHT - 1 );
    mvvline( 1, 0, ACS_VLINE, LCD_BOTTOM - 1 );
    mvvline( 1, LCD_RIGHT, ACS_VLINE, LCD_BOTTOM - 1 );

    mvprintw( 0, 2, "[   |   |   |   |   |   ]" ); /* annunciators */

    mvprintw( LCD_BOTTOM, 2, "[ wire: %s ]-[ IR: %s ]", wire_name, ir_name );
}

/* TODO: not specific to tui  */
static inline void draw_row( long addr, int row )
{
    int nibble;
    int line_length = NIBBLES_PER_ROW;

    if ( ( display.offset > 3 ) && ( row <= display.lines ) )
        line_length += 2;

    for ( int i = 0; i < line_length; i++ ) {
        nibble = read_nibble( addr + i );
        if ( nibble == lcd_nibbles_buffer[ row ][ i ] )
            continue;

        lcd_nibbles_buffer[ row ][ i ] = nibble;
    }
}

/**********/
/* public */
/**********/
int text_get_event( void ) { return ncurses_get_event(); }

void text_adjust_contrast() { text_update_LCD(); }

/* TODO: not specific to tui  */
void text_init_LCD( void )
{
    init_display();

    memset( lcd_nibbles_buffer, 0xf0, sizeof( lcd_nibbles_buffer ) );
}

/* TODO: not specific to tui  */
void text_update_LCD( void )
{
    if ( display.on ) {
        int i;
        long addr;
        static int old_offset = -1;
        static int old_lines = -1;

        addr = display.disp_start;
        if ( display.offset != old_offset ) {
            memset( lcd_nibbles_buffer, 0xf0, ( size_t )( ( display.lines + 1 ) * NIBS_PER_BUFFER_ROW ) );

            old_offset = display.offset;
        }
        if ( display.lines != old_lines ) {
            memset( &lcd_nibbles_buffer[ 56 ][ 0 ], 0xf0, ( size_t )( 8 * NIBS_PER_BUFFER_ROW ) );

            old_lines = display.lines;
        }
        for ( i = 0; i <= display.lines; i++ ) {
            draw_row( addr, i );
            addr += display.nibs_per_line;
        }
        if ( i < DISP_ROWS ) {
            addr = display.menu_start;
            for ( ; i < DISP_ROWS; i++ ) {
                draw_row( addr, i );
                addr += NIBBLES_PER_ROW;
            }
        }
    } else
        memset( lcd_nibbles_buffer, 0xf0, sizeof( lcd_nibbles_buffer ) );

    if ( small )
        ncurses_draw_lcd_small();
    else if ( tiny )
        ncurses_draw_lcd_tiny();
    else
        ncurses_draw_lcd();
}

void text_refresh_LCD( void ) {}

/* TODO: not specific to tui  */
void text_disp_draw_nibble( word_20 addr, word_4 val )
{
    long offset;
    int x, y;

    offset = ( addr - display.disp_start );
    x = offset % display.nibs_per_line;
    if ( x < 0 || x > 35 )
        return;
    if ( display.nibs_per_line != 0 ) {
        y = offset / display.nibs_per_line;
        if ( y < 0 || y > 63 )
            return;

        if ( val == lcd_nibbles_buffer[ y ][ x ] )
            return;

        lcd_nibbles_buffer[ y ][ x ] = val;
    } else {
        for ( y = 0; y < display.lines; y++ ) {
            if ( val == lcd_nibbles_buffer[ y ][ x ] )
                break;

            lcd_nibbles_buffer[ y ][ x ] = val;
        }
    }
}

/* TODO: not specific to tui  */
void text_menu_draw_nibble( word_20 addr, word_4 val )
{
    long offset;
    int x, y;

    offset = ( addr - display.menu_start );
    x = offset % NIBBLES_PER_ROW;
    y = display.lines + ( offset / NIBBLES_PER_ROW ) + 1;

    if ( val == lcd_nibbles_buffer[ y ][ x ] )
        return;

    lcd_nibbles_buffer[ y ][ x ] = val;
}

void text_draw_annunc( void ) { ncurses_draw_annunciators(); }

void init_text_ui( int argc, char** argv )
{
    /* Set public API to this UIs functions */
    ui_disp_draw_nibble = text_disp_draw_nibble;
    ui_menu_draw_nibble = text_menu_draw_nibble;
    ui_get_event = text_get_event;
    ui_update_LCD = text_update_LCD;
    ui_refresh_LCD = text_refresh_LCD;
    ui_adjust_contrast = text_adjust_contrast;
    ui_draw_annunc = text_draw_annunc;
    ui_init_LCD = text_init_LCD;

    text_init_LCD();

    ncurses_init_ui();
}
