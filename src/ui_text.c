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
typedef struct tui_ann_struct_t {
    int bit;
    int x;
    int y;
    char icon;
} tui_ann_struct_t;

typedef struct tui_button_t {
    const char* name;
    short pressed;

    int code;
    /* int x; */
    /* int y; */
    /* unsigned int w; */
    /* unsigned int h; */

    /* int lc; */
    /* const char* label; */
    /* short font_size; */
    /* unsigned int lw; */
    /* unsigned int lh; */
    /* unsigned char* lb; */

    /* const char* letter; */

    /* const char* left; */
    /* short is_menu; */
    /* const char* right; */
    /* const char* sub; */
} tui_button_t;

/*************/
/* variables */
/*************/
static tui_ann_struct_t ann_tbl[] = {
    {ANN_LEFT,     16,  4, '<'}, /* '↰' */
    { ANN_RIGHT,   61,  4, '>'}, /* '↱ */
    { ANN_ALPHA,   106, 4, 'a'}, /* 'α' */
    { ANN_BATTERY, 151, 4, 'B'}, /* '🪫' */
    { ANN_BUSY,    196, 4, '*'}, /* '⌛' */
    { ANN_IO,      241, 4, '^'}, /* '☃' */
  /* { 0 } */
};

static tui_button_t* buttons = 0;

static tui_button_t buttons_sx[] = {
    {"A",       0, 0x14  },
    { "B",      0, 0x84  },
    { "C",      0, 0x83  },
    { "D",      0, 0x82  },
    { "E",      0, 0x81  },
    { "F",      0, 0x80  },

    { "MTH",    0, 0x24  },
    { "PRG",    0, 0x74  },
    { "CST",    0, 0x73  },
    { "VAR",    0, 0x72  },
    { "UP",     0, 0x71  },
    { "NXT",    0, 0x70  },

    { "COLON",  0, 0x04  },
    { "STO",    0, 0x64  },
    { "EVAL",   0, 0x63  },
    { "LEFT",   0, 0x62  },
    { "DOWN",   0, 0x61  },
    { "RIGHT",  0, 0x60  },

    { "SIN",    0, 0x34  },
    { "COS",    0, 0x54  },
    { "TAN",    0, 0x53  },
    { "SQRT",   0, 0x52  },
    { "POWER",  0, 0x51  },
    { "INV",    0, 0x50  },

    { "ENTER",  0, 0x44  },
    { "NEG",    0, 0x43  },
    { "EEX",    0, 0x42  },
    { "DEL",    0, 0x41  },
    { "BS",     0, 0x40  },

    { "ALPHA",  0, 0x35  },
    { "7",      0, 0x33  },
    { "8",      0, 0x32  },
    { "9",      0, 0x31  },
    { "DIV",    0, 0x30  },

    { "SHL",    0, 0x25  },
    { "4",      0, 0x23  },
    { "5",      0, 0x22  },
    { "6",      0, 0x21  },
    { "MUL",    0, 0x20  },

    { "SHR",    0, 0x15  },
    { "1",      0, 0x13  },
    { "2",      0, 0x12  },
    { "3",      0, 0x11  },
    { "MINUS",  0, 0x10  },

    { "ON",     0, 0x8000},
    { "0",      0, 0x03  },
    { "PERIOD", 0, 0x02  },
    { "SPC",    0, 0x01  },
    { "PLUS",   0, 0x00  },
 /* { 0 } */
};

static tui_button_t buttons_gx[] = {
    {"A",       0, 0x14  },
    { "B",      0, 0x84  },
    { "C",      0, 0x83  },
    { "D",      0, 0x82  },
    { "E",      0, 0x81  },
    { "F",      0, 0x80  },

    { "MTH",    0, 0x24  },
    { "PRG",    0, 0x74  },
    { "CST",    0, 0x73  },
    { "VAR",    0, 0x72  },
    { "UP",     0, 0x71  },
    { "NXT",    0, 0x70  },

    { "COLON",  0, 0x04  },
    { "STO",    0, 0x64  },
    { "EVAL",   0, 0x63  },
    { "LEFT",   0, 0x62  },
    { "DOWN",   0, 0x61  },
    { "RIGHT",  0, 0x60  },

    { "SIN",    0, 0x34  },
    { "COS",    0, 0x54  },
    { "TAN",    0, 0x53  },
    { "SQRT",   0, 0x52  },
    { "POWER",  0, 0x51  },
    { "INV",    0, 0x50  },

    { "ENTER",  0, 0x44  },
    { "NEG",    0, 0x43  },
    { "EEX",    0, 0x42  },
    { "DEL",    0, 0x41  },
    { "BS",     0, 0x40  },

    { "ALPHA",  0, 0x35  },
    { "7",      0, 0x33  },
    { "8",      0, 0x32  },
    { "9",      0, 0x31  },
    { "DIV",    0, 0x30  },

    { "SHL",    0, 0x25  },
    { "4",      0, 0x23  },
    { "5",      0, 0x22  },
    { "6",      0, 0x21  },
    { "MUL",    0, 0x20  },

    { "SHR",    0, 0x15  },
    { "1",      0, 0x13  },
    { "2",      0, 0x12  },
    { "3",      0, 0x11  },
    { "MINUS",  0, 0x10  },

    { "ON",     0, 0x8000},
    { "0",      0, 0x03  },
    { "PERIOD", 0, 0x02  },
    { "SPC",    0, 0x01  },
    { "PLUS",   0, 0x00  },
 /* { 0 } */
};

/****************************/
/* functions implementation */
/****************************/
static inline void tui_draw_nibble( int nx, int ny, int val )
{
    for ( int x = 0; x < 4; x++ ) {
        // Check if bit is on
        // char c = lcd_buffer[y/2][x>>2];		// The 4 lower
        // bits in a byte are used (1 nibble per byte)
        if ( nx + x >= LCD_WIDTH ) // Clip at 131 pixels (some nibble writes may
                                   // go beyond the range, but are not visible)
            break;

        short bit = val & ( 1 << ( x & 3 ) );
        chtype pixel;

        if ( !mono && has_colors() )
            pixel = ' ' | COLOR_PAIR( bit ? LCD_PIXEL_ON : LCD_PIXEL_OFF );
        else
            pixel = bit ? ACS_BLOCK : ' ';

        mvaddch( ny + LCD_OFFSET_Y, nx + x + LCD_OFFSET_X, pixel );
    }
}

static inline void draw_nibble( int c, int r, int val )
{
    int x = ( c * 4 ), // x: start in pixels,
        y = r;         // y: start in pixels

    if ( r <= display.lines )
        x -= 2 * display.offset;

    val &= 0x0f;
    if ( val != lcd_buffer[ r ][ c ] ) {
        lcd_buffer[ r ][ c ] = val;

        tui_draw_nibble( x, y, val );
    }
}

static inline void draw_row( long addr, int row )
{
    int v;
    int line_length = NIBBLES_PER_ROW;

    if ( ( display.offset > 3 ) && ( row <= display.lines ) )
        line_length += 2;

    for ( int i = 0; i < line_length; i++ ) {
        v = read_nibble( addr + i );
        if ( v != disp_buf[ row ][ i ] ) {
            disp_buf[ row ][ i ] = v;
            draw_nibble( i, row, v );
        }
    }
}

static void tui_press_button( int b )
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

static void tui_release_button( int b )
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

static void tui_release_all_buttons( void )
{
    for ( int b = FIRST_BUTTON; b <= LAST_BUTTON; b++ )
        if ( buttons[ b ].pressed )
            tui_release_button( b );
}

/**********/
/* public */
/**********/
int text_get_event( void )
{
    int hpkey = -1;
    uint32_t k;

    /* Start fresh and mark all keys as released */
    tui_release_all_buttons();

    /* Iterate over all currently pressed keys and mark it as pressed */
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
                hpkey = BUTTON_SHL;
                break;
            case ']':
                hpkey = BUTTON_SHR;
                break;
            case ';':
                hpkey = BUTTON_ALPHA;
                break;
            case '\\':
                hpkey = BUTTON_ON;
                break;

            case '|': /* Shift+\ */
                nodelay( stdscr, FALSE );
                echo();

                endwin();
                exit_emulator();
                exit( 0 );
                break;
        }

        if ( !buttons[ hpkey ].pressed )
            tui_press_button( hpkey );
    }

    text_update_LCD();

    return 1;
}

void text_adjust_contrast() { text_update_LCD(); }

void text_init_LCD( void )
{
    display.on = ( int )( saturn.disp_io & 0x8 ) >> 3;

    display.disp_start = ( saturn.disp_addr & 0xffffe );
    display.offset = ( saturn.disp_io & 0x7 );

    display.lines = ( saturn.line_count & 0x3f );
    if ( display.lines == 0 )
        display.lines = 63;

    if ( display.offset > 3 )
        display.nibs_per_line = ( NIBBLES_PER_ROW + saturn.line_offset + 2 ) & 0xfff;
    else
        display.nibs_per_line = ( NIBBLES_PER_ROW + saturn.line_offset ) & 0xfff;

    display.disp_end = display.disp_start + ( display.nibs_per_line * ( display.lines + 1 ) );

    display.menu_start = saturn.menu_addr;
    display.menu_end = saturn.menu_addr + 0x110;

    display.contrast = saturn.contrast_ctrl;
    display.contrast |= ( ( saturn.disp_test & 0x1 ) << 4 );

    display.annunc = saturn.annunc;

    memset( disp_buf, 0xf0, sizeof( disp_buf ) );
    memset( lcd_buffer, 0xf0, sizeof( lcd_buffer ) );
}

void text_update_LCD( void )
{
    int i, j;
    long addr;
    static int old_offset = -1;
    static int old_lines = -1;

    if ( display.on ) {
        addr = display.disp_start;
        if ( display.offset != old_offset ) {
            memset( disp_buf, 0xf0, ( size_t )( ( display.lines + 1 ) * NIBS_PER_BUFFER_ROW ) );
            memset( lcd_buffer, 0xf0, ( size_t )( ( display.lines + 1 ) * NIBS_PER_BUFFER_ROW ) );
            old_offset = display.offset;
        }
        if ( display.lines != old_lines ) {
            memset( &disp_buf[ 56 ][ 0 ], 0xf0, ( size_t )( 8 * NIBS_PER_BUFFER_ROW ) );
            memset( &lcd_buffer[ 56 ][ 0 ], 0xf0, ( size_t )( 8 * NIBS_PER_BUFFER_ROW ) );
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
        memset( disp_buf, 0xf0, sizeof( disp_buf ) );
        for ( i = 0; i < 64; i++ ) {
            for ( j = 0; j < NIBBLES_PER_ROW; j++ ) {
                draw_nibble( j, i, 0x00 );
            }
        }
    }

    refresh();
}

void text_refresh_LCD( void ) {}

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
        if ( val != disp_buf[ y ][ x ] ) {
            disp_buf[ y ][ x ] = val;
            draw_nibble( x, y, val );
        }
    } else {
        for ( y = 0; y < display.lines; y++ ) {
            if ( val != disp_buf[ y ][ x ] ) {
                disp_buf[ y ][ x ] = val;
                draw_nibble( x, y, val );
            }
        }
    }
}

void text_menu_draw_nibble( word_20 addr, word_4 val )
{
    long offset;
    int x, y;

    offset = ( addr - display.menu_start );
    x = offset % NIBBLES_PER_ROW;
    y = display.lines + ( offset / NIBBLES_PER_ROW ) + 1;
    if ( val != disp_buf[ y ][ x ] ) {
        disp_buf[ y ][ x ] = val;
        draw_nibble( x, y, val );
    }
}

void text_draw_annunc( void )
{
    int val;

    val = display.annunc;

    if ( val == last_annunc_state )
        return;

    last_annunc_state = val;

    for ( int i = 0; ann_tbl[ i ].bit; i++ )
        mvaddch( 1, 3 + ( i * 4 ), ( ( ann_tbl[ i ].bit & val ) == ann_tbl[ i ].bit ) ? ann_tbl[ i ].icon : ' ' );
}

void init_text_ui( int argc, char** argv )
{
    buttons = ( tui_button_t* )malloc( sizeof( buttons_gx ) );

    if ( opt_gx )
        memcpy( buttons, buttons_gx, sizeof( buttons_gx ) );
    else
        memcpy( buttons, buttons_sx, sizeof( buttons_sx ) );

    text_init_LCD();

    initscr();              /* initialize the curses library */
    keypad( stdscr, TRUE ); /* enable keyboard mapping */
    nodelay( stdscr, TRUE );
    curs_set( 0 );
    nonl();   /* tell curses not to do NL->CR/NL on output */
    cbreak(); /* take input chars one at a time, no wait for \n */
    noecho();

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

    mvprintw( 0, 1, "[   |   |   |   |   |   ]" ); /* annunciators */
}
