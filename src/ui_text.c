#include <ctype.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdbool.h>
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

#include "runtime_options.h" /* mono, gray, small, tiny, progname */
#include "ui.h"              /* last_annunc_state, greyscale_lcd_buffer, DISP_ROWS, DISP_COLS */
#include "ui_inner.h"        /* generate_greyscale_lcd(); ui_init_LCD(); */

#define LCD_OFFSET_X 1
#define LCD_OFFSET_Y 1
#define LCD_BOTTOM LCD_OFFSET_Y + ( small ? ( DISP_ROWS / 2 ) : tiny ? ( DISP_ROWS / 4 ) : DISP_ROWS )
#define LCD_RIGHT LCD_OFFSET_X + ( ( small || tiny ) ? ( DISP_COLS / 2 ) + 1 : DISP_COLS )

#define LCD_COLOR_BG 48
#define LCD_COLOR_FG 49

#define LCD_PIXEL_ON 1
#define LCD_PIXEL_OFF 2
#define LCD_COLORS_PAIR 3

/****************************/
/* functions implementation */
/****************************/

static inline wchar_t eight_bits_to_braille_char( unsigned char b1, unsigned char b2, unsigned char b3, unsigned char b4, unsigned char b5,
                                                  unsigned char b6, unsigned char b7, unsigned char b8 )
{
    /*********/
    /* b1 b4 */
    /* b2 b5 */
    /* b3 b6 */
    /* b7 b8 */
    /*********/
    wchar_t chr = 0x2800;

    if ( b1 > 0 )
        chr |= 0b0000000000000001;
    if ( b2 > 0 )
        chr |= 0b0000000000000010;
    if ( b3 > 0 )
        chr |= 0b0000000000000100;
    if ( b4 > 0 )
        chr |= 0b0000000000001000;
    if ( b5 > 0 )
        chr |= 0b0000000000010000;
    if ( b6 > 0 )
        chr |= 0b0000000000100000;
    if ( b7 > 0 )
        chr |= 0b0000000001000000;
    if ( b8 > 0 )
        chr |= 0b0000000010000000;

    return chr;
}

static inline void ncurses_draw_greyscale_lcd_tiny( void )
{
    unsigned char b1, b2, b3, b4, b5, b6, b7, b8;
    int step_x = 2;
    int step_y = 4;

    wchar_t line[ 66 ]; /* ( DISP_COLS / step_x ) + 1 */

    if ( !mono && has_colors() )
        attron( COLOR_PAIR( LCD_COLORS_PAIR ) );

    for ( int y = 0; y < DISP_ROWS; y += step_y ) {
        wcscpy( line, L"" );

        for ( int x = 0; x < DISP_COLS; x += step_x ) {
            b1 = greyscale_lcd_buffer[ y ][ x ];
            b4 = ( ( x + 1 ) == DISP_COLS ) ? 0 : greyscale_lcd_buffer[ y ][ x + 1 ];

            b2 = greyscale_lcd_buffer[ y + 1 ][ x ];
            b5 = ( ( x + 1 ) == DISP_COLS ) ? 0 : greyscale_lcd_buffer[ y + 1 ][ x + 1 ];

            b3 = greyscale_lcd_buffer[ y + 2 ][ x ];
            b6 = ( ( x + 1 ) == DISP_COLS ) ? 0 : greyscale_lcd_buffer[ y + 2 ][ x + 1 ];

            b7 = greyscale_lcd_buffer[ y + 3 ][ x ];
            b8 = ( ( x + 1 ) == DISP_COLS ) ? 0 : greyscale_lcd_buffer[ y + 3 ][ x + 1 ];

            wchar_t pixels = eight_bits_to_braille_char( b1, b2, b3, b4, b5, b6, b7, b8 );
            wcsncat( line, &pixels, 1 );
        }
        mvaddwstr( LCD_OFFSET_Y + ( y / step_y ), LCD_OFFSET_X, line );
    }

    if ( !mono && has_colors() )
        attroff( COLOR_PAIR( LCD_COLORS_PAIR ) );

    wrefresh( stdscr );
}

static inline wchar_t four_bits_to_quadrant_char( unsigned char top_left, unsigned char top_right, unsigned char bottom_left,
                                                  unsigned char bottom_right )
{
    if ( top_left > 0 ) {
        if ( top_right > 0 ) {
            if ( bottom_left > 0 )
                return bottom_right ? L'█' : L'▛'; /* 0x2588 0x2598 */
            else
                return bottom_right ? L'▜' : L'▀'; /* 0x259C 0x2580 */
        } else {
            if ( bottom_left > 0 )
                return bottom_right ? L'▙' : L'▌';
            else
                return bottom_right ? L'▚' : L'▘';
        }
    } else {
        if ( top_right > 0 ) {
            if ( bottom_left > 0 )
                return bottom_right ? L'▟' : L'▞';
            else
                return bottom_right ? L'▐' : L'▝';
        } else {
            if ( bottom_left > 0 )
                return bottom_right ? L'▄' : L'▖';
            else
                return bottom_right ? L'▗' : L' ';
        }
    }
}

static inline void ncurses_draw_greyscale_lcd_small( void )
{
    unsigned char top_left, top_right, bottom_left, bottom_right;
    int step_x = 2;
    int step_y = 2;

    wchar_t line[ 66 ]; /* ( DISP_COLS / step_x ) + 1 */

    if ( !mono && has_colors() )
        attron( COLOR_PAIR( LCD_COLORS_PAIR ) );

    for ( int y = 0; y < DISP_ROWS; y += step_y ) {
        wcscpy( line, L"" );

        for ( int x = 0; x < DISP_COLS; x += step_x ) {
            top_left = greyscale_lcd_buffer[ y ][ x ];
            top_right = ( ( x + 1 ) == DISP_COLS ) ? 0 : greyscale_lcd_buffer[ y ][ x + 1 ];
            bottom_left = greyscale_lcd_buffer[ y + 1 ][ x ];
            bottom_right = ( ( x + 1 ) == DISP_COLS ) ? 0 : greyscale_lcd_buffer[ y + 1 ][ x + 1 ];

            wchar_t pixels = four_bits_to_quadrant_char( top_left, top_right, bottom_left, bottom_right );
            wcsncat( line, &pixels, 1 );
        }
        mvaddwstr( LCD_OFFSET_Y + ( y / step_y ), LCD_OFFSET_X, line );
    }

    if ( !mono && has_colors() )
        attroff( COLOR_PAIR( LCD_COLORS_PAIR ) );

    wrefresh( stdscr );
}

static inline void ncurses_draw_greyscale_lcd_fullsize( void )
{
    wchar_t line[ DISP_COLS ];

    if ( !mono && has_colors() )
        attron( COLOR_PAIR( LCD_COLORS_PAIR ) );

    for ( int y = 0; y < DISP_ROWS; ++y ) {
        wcscpy( line, L"" );

        for ( int x = 0; x < DISP_COLS; ++x ) {
            wchar_t pixel;

            switch ( greyscale_lcd_buffer[ y ][ x ] ) {
                case 3:
                    pixel = L'█'; /* ▓ */
                    break;
                case 2:
                    pixel = L'▒';
                    break;
                case 1:
                    pixel = L'░';
                    break;
                case 0:
                default:
                    pixel = L' ';
                    break;
            }

            wcsncat( line, &pixel, 1 );
        }
        mvaddwstr( LCD_OFFSET_Y + y, LCD_OFFSET_X, line );
    }

    if ( !mono && has_colors() )
        attroff( COLOR_PAIR( LCD_COLORS_PAIR ) );

    wrefresh( stdscr );
}

static inline void ncurses_draw_lcd( void )
{
    generate_greyscale_lcd();

    if ( small )
        ncurses_draw_greyscale_lcd_small(); /* no greyscale */
    else if ( tiny )
        ncurses_draw_greyscale_lcd_tiny(); /* no greyscale */
    else
        ncurses_draw_greyscale_lcd_fullsize();
}

/* TODO: duplicate of ui_sdl.c:draw_row()  */
static inline void draw_row( long addr, int row )
{
    int nibble;
    int line_length = NIBBLES_PER_ROW;

    if ( ( display.offset > 3 ) && ( row <= display.lines ) )
        line_length += 2;

    for ( int i = 0; i < line_length; i++ ) {
        nibble = read_nibble( addr + i );
        if ( nibble == lcd_nibbles_buffer_0[ row ][ i ] )
            continue;

        lcd_nibbles_buffer_0[ row ][ i ] = nibble;
    }
}

/**********/
/* public */
/**********/
/* TODO: quasi-duplicate of ui_sdl.c:sdl_update_LCD()  */
void text_update_LCD( void )
{
    /* First populate lcd_nibbles_buffer */
    if ( display.on ) {
        int i;
        long addr = display.disp_start;
        static int old_offset = -1;
        static int old_lines = -1;

        if ( display.offset != old_offset ) {
            memset( lcd_nibbles_buffer_0, 0xf0, ( size_t )( ( display.lines + 1 ) * NIBS_PER_BUFFER_ROW ) );

            old_offset = display.offset;
        }
        if ( display.lines != old_lines ) {
            memset( &lcd_nibbles_buffer_0[ 56 ][ 0 ], 0xf0, ( size_t )( 8 * NIBS_PER_BUFFER_ROW ) );

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
        ui_init_LCD();

    /* Then actually draw from lcd_nibbles_buffer onto screen */
    ncurses_draw_lcd();
}

/* TODO: duplicate of ui_sdl.c:sdl_disp_draw_nibble()  */
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

        if ( val == lcd_nibbles_buffer_0[ y ][ x ] )
            return;

        lcd_nibbles_buffer_0[ y ][ x ] = val;
    } else {
        for ( y = 0; y < display.lines; y++ ) {
            if ( val == lcd_nibbles_buffer_0[ y ][ x ] )
                break;

            lcd_nibbles_buffer_0[ y ][ x ] = val;
        }
    }
}

/* TODO: duplicate of ui_sdl.c:sdl_menu_draw_nibble()  */
void text_menu_draw_nibble( word_20 addr, word_4 val )
{
    long offset;
    int x, y;

    offset = ( addr - display.menu_start );
    x = offset % NIBBLES_PER_ROW;
    y = display.lines + ( offset / NIBBLES_PER_ROW ) + 1;

    if ( val == lcd_nibbles_buffer_0[ y ][ x ] )
        return;

    lcd_nibbles_buffer_0[ y ][ x ] = val;
}

void text_draw_annunc( void )
{
    wchar_t* annunciators_icons[ 6 ] = { L"↰", L"↱", L"α", L"🪫", L"⌛", L"⇄" };
    int val = saturn.annunc;

    if ( val == last_annunc_state )
        return;

    last_annunc_state = val;

    for ( int i = 0; i < NB_ANNUNCIATORS; i++ )
        mvaddwstr( 0, 4 + ( i * 4 ), ( ( annunciators_bits[ i ] & val ) == annunciators_bits[ i ] ) ? annunciators_icons[ i ] : L" " );
}

void text_adjust_contrast( void ) { text_update_LCD(); }

int text_get_event( void )
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
            case KEY_LEFT:
                hpkey = HPKEY_LEFT;
                break;
            case 'q':
            case KEY_DOWN:
                hpkey = HPKEY_DOWN;
                break;
            case 'r':
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

            case KEY_F( 1 ):
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
            case KEY_F( 2 ):
            case '[':
            case 339: /* PgUp */
                hpkey = HPKEY_SHL;
                break;
            case KEY_F( 3 ):
            case ']':
            case 338: /* PgDn */
                hpkey = HPKEY_SHR;
                break;
            case KEY_F( 4 ):
            case ';':
            case KEY_IC: /* Ins */
                hpkey = HPKEY_ALPHA;
                break;
            case KEY_F( 5 ):
            case '\\':
            case 27:  /* Esc */
            case 262: /* Home */
                hpkey = HPKEY_ON;
                break;

            case KEY_F( 7 ):
            case '|':      /* Shift+\ */
            case KEY_SEND: /* Shift+End */
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

    return 1;
}

void init_text_ui( int argc, char** argv )
{
    /* Set public API to this UIs functions */
    ui_disp_draw_nibble = text_disp_draw_nibble;
    ui_menu_draw_nibble = text_menu_draw_nibble;
    ui_get_event = text_get_event;
    ui_update_LCD = text_update_LCD;
    ui_refresh_LCD = text_update_LCD;
    ui_adjust_contrast = text_adjust_contrast;
    ui_draw_annunc = text_draw_annunc;

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
    mvprintw( 0, LCD_RIGHT - 18, "< %s v%i.%i.%i >", progname, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );

    mvprintw( LCD_BOTTOM, 2, "[ wire: %s ]-[ IR: %s ]-[ contrast: %i ]", wire_name, ir_name, display.contrast );

    mvprintw( LCD_BOTTOM + 1, 0, "F1: Enter, F2: Left-Shift, F3: Right-Shift, F4: Alpha, F5: On, F7: Quit" );
}
