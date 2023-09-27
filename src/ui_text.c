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

#include <ncurses.h>

#include "emulator.h"
#include "romio.h"
#include "runtime_options.h"
#include "ui.h"
#include "ui_inner.h"

#define LCD_WIDTH 131
#define LCD_HEIGHT 64
#define LCD_OFFSET_X 1
#define LCD_OFFSET_Y 2
#define LCD_BOTTOM LCD_HEIGHT + LCD_OFFSET_Y
#define LCD_RIGHT LCD_WIDTH + LCD_OFFSET_X

#define LCD_COLOR_BG 48
#define LCD_COLOR_FG 49

#define LCD_PIXEL_ON 1
#define LCD_PIXEL_OFF 2

/***********/
/* typedef */
/***********/
typedef struct tui_button_t {
    int code;
    short pressed;
} tui_button_t;

/*************/
/* variables */
/*************/
static int annunciators_bits[ 6 ] = { ANN_LEFT, ANN_RIGHT, ANN_ALPHA, ANN_BATTERY, ANN_BUSY, ANN_IO };

static tui_button_t buttons[] = {
  /* From top left to bottom right */
    {0x14,    0},
    { 0x84,   0},
    { 0x83,   0},
    { 0x82,   0},
    { 0x81,   0},
    { 0x80,   0},

    { 0x24,   0},
    { 0x74,   0},
    { 0x73,   0},
    { 0x72,   0},
    { 0x71,   0},
    { 0x70,   0},

    { 0x04,   0},
    { 0x64,   0},
    { 0x63,   0},
    { 0x62,   0},
    { 0x61,   0},
    { 0x60,   0},

    { 0x34,   0},
    { 0x54,   0},
    { 0x53,   0},
    { 0x52,   0},
    { 0x51,   0},
    { 0x50,   0},

    { 0x44,   0},
    { 0x43,   0},
    { 0x42,   0},
    { 0x41,   0},
    { 0x40,   0},

    { 0x35,   0},
    { 0x33,   0},
    { 0x32,   0},
    { 0x31,   0},
    { 0x30,   0},

    { 0x25,   0},
    { 0x23,   0},
    { 0x22,   0},
    { 0x21,   0},
    { 0x20,   0},

    { 0x15,   0},
    { 0x13,   0},
    { 0x12,   0},
    { 0x11,   0},
    { 0x10,   0},

    { 0x8000, 0},
    { 0x03,   0},
    { 0x02,   0},
    { 0x01,   0},
    { 0x00,   0},
};

/* the actual pixels buffer */
static short lcd_pixels_buffer[ LCD_WIDTH ][ LCD_HEIGHT ];

/****************************/
/* functions implementation */
/****************************/
/* TODO: not specific to tui  */
static inline void press_button( int b )
{
    // Check not already pressed (may be important: avoids a useless do_kbd_int)
    if ( buttons[ b ].pressed == 1 )
        return;

    buttons[ b ].pressed = 1;

    int code = buttons[ b ].code;
    if ( code == 0x8000 ) {
        for ( int i = 0; i < 9; i++ )
            saturn.keybuf.rows[ i ] |= 0x8000;
        do_kbd_int();
    } else {
        int r = code >> 4;
        int c = 1 << ( code & 0xf );
        if ( ( saturn.keybuf.rows[ r ] & c ) == 0 ) {
            if ( saturn.kbd_ien )
                do_kbd_int();
            if ( ( saturn.keybuf.rows[ r ] & c ) )
                fprintf( stderr, "bug\n" );

            saturn.keybuf.rows[ r ] |= c;
        }
    }
}

/* TODO: not specific to tui  */
static inline void release_button( int b )
{
    // Check not already released (not critical)
    if ( buttons[ b ].pressed == 0 )
        return;

    buttons[ b ].pressed = 0;

    int code = buttons[ b ].code;
    if ( code == 0x8000 ) {
        for ( int i = 0; i < 9; i++ )
            saturn.keybuf.rows[ i ] &= ~0x8000;
    } else {
        int r = code >> 4;
        int c = 1 << ( code & 0xf );
        saturn.keybuf.rows[ r ] &= ~c;
    }
}

/* TODO: not specific to tui  */
static inline void release_all_buttons( void )
{
    for ( int b = FIRST_BUTTON; b <= LAST_BUTTON; b++ )
        if ( buttons[ b ].pressed )
            release_button( b );
}

static inline void tranlate_nibble_into_lcd_pixels_buffer( int nibble, int initial_column, int row )
{
    for ( int x = 0; x < 4; x++ ) {
        // bits in a byte are used (1 nibble per byte)
        if ( initial_column + x >= LCD_WIDTH ) // Clip at 131 pixels
            break;

        lcd_pixels_buffer[ initial_column + x ][ row ] = nibble & ( 1 << ( x & 3 ) );
    }
}

static inline void ncurses_draw_annunciators( void )
{
    wchar_t* annunciators_icons[ 6 ] = { L"\u21b0", L"\u21b1", L"\u03b1", L"\u1faab", L"\u231b", L"\u21c4" };
    int val = display.annunc;

    if ( val == last_annunc_state )
        return;

    last_annunc_state = val;

    for ( int i = 0; i < 6; i++ )
        mvaddwstr( 0, 4 + ( i * 4 ), ( ( annunciators_bits[ i ] & val ) == annunciators_bits[ i ] ) ? annunciators_icons[ i ] : L" " );
}

static inline void ncurses_draw_lcd_pixels_buffer( void )
{
    for ( int x = 0; x < LCD_WIDTH; ++x ) {
        for ( int y = 0; y < LCD_HEIGHT; ++y ) {
            chtype pixel;

            pixel = lcd_pixels_buffer[ x ][ y ] ? ACS_BLOCK : ' ';
            if ( !mono && has_colors() )
                pixel |= COLOR_PAIR( lcd_pixels_buffer[ x ][ y ] ? LCD_PIXEL_ON : LCD_PIXEL_OFF );

            mvaddch( y + LCD_OFFSET_Y, x + LCD_OFFSET_X, pixel );
        }
    }

    refresh();
}

static inline int ncurses_get_event( void )
{
    int hpkey = -1;
    uint32_t k;

    /* Start fresh and mark all keys as released */
    release_all_buttons();

    /* Iterate over all currently pressed keys and mark them as pressed */
    while ( ( k = getch() ) ) {
        if ( k == ( uint32_t )ERR )
            break;

        switch ( k ) {
            case '0':
                hpkey = BUTTON_0;
                break;
            case '1':
                hpkey = BUTTON_1;
                break;
            case '2':
                hpkey = BUTTON_2;
                break;
            case '3':
                hpkey = BUTTON_3;
                break;
            case '4':
                hpkey = BUTTON_4;
                break;
            case '5':
                hpkey = BUTTON_5;
                break;
            case '6':
                hpkey = BUTTON_6;
                break;
            case '7':
                hpkey = BUTTON_7;
                break;
            case '8':
                hpkey = BUTTON_8;
                break;
            case '9':
                hpkey = BUTTON_9;
                break;
            case 'a':
                hpkey = BUTTON_A;
                break;
            case 'b':
                hpkey = BUTTON_B;
                break;
            case 'c':
                hpkey = BUTTON_C;
                break;
            case 'd':
                hpkey = BUTTON_D;
                break;
            case 'e':
                hpkey = BUTTON_E;
                break;
            case 'f':
                hpkey = BUTTON_F;
                break;
            case 'g':
                hpkey = BUTTON_MTH;
                break;
            case 'h':
                hpkey = BUTTON_PRG;
                break;
            case 'i':
                hpkey = BUTTON_CST;
                break;
            case 'j':
                hpkey = BUTTON_VAR;
                break;
            case 'k':
                hpkey = BUTTON_UP;
                break;
            case KEY_UP:
                hpkey = BUTTON_UP;
                break;
            case 'l':
                hpkey = BUTTON_NXT;
                break;
            case 'm':
                hpkey = BUTTON_COLON;
                break;
            case 'n':
                hpkey = BUTTON_STO;
                break;
            case 'o':
                hpkey = BUTTON_EVAL;
                break;
            case 'p':
                hpkey = BUTTON_LEFT;
                break;
            case KEY_LEFT:
                hpkey = BUTTON_LEFT;
                break;
            case 'q':
                hpkey = BUTTON_DOWN;
                break;
            case KEY_DOWN:
                hpkey = BUTTON_DOWN;
                break;
            case 'r':
                hpkey = BUTTON_RIGHT;
                break;
            case KEY_RIGHT:
                hpkey = BUTTON_RIGHT;
                break;
            case 's':
                hpkey = BUTTON_SIN;
                break;
            case 't':
                hpkey = BUTTON_COS;
                break;
            case 'u':
                hpkey = BUTTON_TAN;
                break;
            case 'v':
                hpkey = BUTTON_SQRT;
                break;
            case 'w':
                hpkey = BUTTON_POWER;
                break;
            case 'x':
                hpkey = BUTTON_INV;
                break;
            case 'y':
                hpkey = BUTTON_NEG;
                break;
            case 'z':
                hpkey = BUTTON_EEX;
                break;
            case ' ':
                hpkey = BUTTON_SPC;
                break;
            case KEY_ENTER:
            case '\n':
            case ',':
                hpkey = BUTTON_ENTER;
                break;
            case KEY_BACKSPACE:
            case 127:
            case '\b':
                hpkey = BUTTON_BS;
                break;
            case KEY_DC:
                hpkey = BUTTON_DEL;
                break;
            case '.':
                hpkey = BUTTON_PERIOD;
                break;
            case '+':
                hpkey = BUTTON_PLUS;
                break;
            case '-':
                hpkey = BUTTON_MINUS;
                break;
            case '*':
                hpkey = BUTTON_MUL;
                break;
            case '/':
                hpkey = BUTTON_DIV;
                break;

            case '[':
            case 339: /* PgUp */
            case KEY_F( 5 ):
                hpkey = BUTTON_SHL;
                break;
            case ']':
            case 338: /* PgDn */
            case KEY_F( 6 ):
                hpkey = BUTTON_SHR;
                break;
            case ';':
            case KEY_IC: /* Ins */
            case KEY_F( 7 ):
            case KEY_F( 8 ):
                hpkey = BUTTON_ALPHA;
                break;
            case '\\':
                /* case KEY_ESC: */
            case 27:  /* Esc */
            case 262: /* Home */
            case KEY_F( 4 ):
                hpkey = BUTTON_ON;
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

        if ( !buttons[ hpkey ].pressed )
            press_button( hpkey );
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
static inline void draw_nibble( int col, int row, int val )
{
    int c = ( col * 4 ), // c: start in pixels,
        r = row;         // r: start in pixels

    if ( row <= display.lines )
        c -= 2 * display.offset;

    val &= 0x0f;

    tranlate_nibble_into_lcd_pixels_buffer( val, c, r );
}

/* TODO: not specific to tui  */
static inline void draw_row( long addr, int row )
{
    int v;
    int line_length = NIBBLES_PER_ROW;

    if ( ( display.offset > 3 ) && ( row <= display.lines ) )
        line_length += 2;

    for ( int i = 0; i < line_length; i++ ) {
        v = read_nibble( addr + i );
        if ( v == lcd_nibbles_buffer[ row ][ i ] )
            break;

        lcd_nibbles_buffer[ row ][ i ] = v;
        draw_nibble( i, row, v );
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
    memset( lcd_pixels_buffer, 0xf0, sizeof( lcd_pixels_buffer ) );
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
    } else {
        memset( lcd_nibbles_buffer, 0xf0, sizeof( lcd_nibbles_buffer ) );
        memset( lcd_pixels_buffer, 0xf0, sizeof( lcd_pixels_buffer ) );
    }

    ncurses_draw_lcd_pixels_buffer();
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
        draw_nibble( x, y, val );
    } else {
        for ( y = 0; y < display.lines; y++ ) {
            if ( val == lcd_nibbles_buffer[ y ][ x ] )
                break;

            lcd_nibbles_buffer[ y ][ x ] = val;
            draw_nibble( x, y, val );
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
    draw_nibble( x, y, val );
}

void text_draw_annunc( void ) { ncurses_draw_annunciators(); }

void init_text_ui( int argc, char** argv )
{
    text_init_LCD();

    ncurses_init_ui();
}
