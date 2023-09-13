#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "hp48.h"
#include "hp48emu.h"
#include "timer.h"
#include "x48.h" /* adjust_contrast(); ann_struct_t; DISP_ROWS; NIBS_PER_BUFFER_ROW */

/*******/
/* LCD */
/*******/

static int last_annunc_state = -1;

display_t display;

unsigned char disp_buf[ DISP_ROWS ][ NIBS_PER_BUFFER_ROW ];
unsigned char lcd_buffer[ DISP_ROWS ][ NIBS_PER_BUFFER_ROW ];

ann_struct_t ann_tbl[] = {
    { ANN_LEFT, 16, 4, ann_left_width, ann_left_height, ann_left_bitmap, 0, 0 },
    { ANN_RIGHT, 61, 4, ann_right_width, ann_right_height, ann_right_bitmap, 0,
      0 },
    { ANN_ALPHA, 106, 4, ann_alpha_width, ann_alpha_height, ann_alpha_bitmap, 0,
      0 },
    { ANN_BATTERY, 151, 4, ann_battery_width, ann_battery_height,
      ann_battery_bitmap, 0, 0 },
    { ANN_BUSY, 196, 4, ann_busy_width, ann_busy_height, ann_busy_bitmap, 0,
      0 },
    { ANN_IO, 241, 4, ann_io_width, ann_io_height, ann_io_bitmap, 0, 0 },
    { 0 } };

static inline void init_nibble_maps( void ) {}

static inline void draw_nibble( int c, int r, int val ) {
    int x, y;

    x = ( c * 4 ); // x: start in pixels

    if ( r <= display.lines )
        x -= disp.offset;
    y = r; // y: start in pixels

    val &= 0x0f;
    if ( val != lcd_buffer[ r ][ c ] ) {
        lcd_buffer[ r ][ c ] = val;
    }
    if ( val != lcd_buffer[ r ][ c ] )
        lcd_buffer[ r ][ c ] = val;

    SDLDrawNibble( x, y, val );
}

static inline void draw_row( long addr, int row ) {
    int i, v;
    int line_length;

    line_length = NIBBLES_PER_ROW;
    if ( ( display.offset > 3 ) && ( row <= display.lines ) )
        line_length += 2;
    for ( i = 0; i < line_length; i++ ) {
        v = read_nibble( addr + i );
        if ( v != disp_buf[ row ][ i ] ) {
            disp_buf[ row ][ i ] = v;
            draw_nibble( i, row, v );
        }
    }
}

void update_display( void ) {
    int i, j;
    long addr;
    static int old_offset = -1;
    static int old_lines = -1;

    if ( !disp.mapped )
        return;

    if ( display.on ) {
        addr = display.disp_start;
        if ( display.offset != old_offset ) {
            memset( disp_buf, 0xf0,
                    ( size_t )( ( display.lines + 1 ) * NIBS_PER_BUFFER_ROW ) );
            memset( lcd_buffer, 0xf0,
                    ( size_t )( ( display.lines + 1 ) * NIBS_PER_BUFFER_ROW ) );
            old_offset = display.offset;
        }
        if ( display.lines != old_lines ) {
            memset( &disp_buf[ 56 ][ 0 ], 0xf0,
                    ( size_t )( 8 * NIBS_PER_BUFFER_ROW ) );
            memset( &lcd_buffer[ 56 ][ 0 ], 0xf0,
                    ( size_t )( 8 * NIBS_PER_BUFFER_ROW ) );
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
}

void redraw_display( void ) {
    memset( disp_buf, 0, sizeof( disp_buf ) );
    memset( lcd_buffer, 0, sizeof( lcd_buffer ) );
    update_display();
}

void disp_draw_nibble( word_20 addr, word_4 val ) {
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

void menu_draw_nibble( word_20 addr, word_4 val ) {
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

void draw_annunc( void ) {
    int val;

    val = display.annunc;

    if ( val == last_annunc_state )
        return;
    last_annunc_state = val;

    char sdl_annuncstate[ 6 ];
    for ( int i = 0; ann_tbl[ i ].bit; i++ ) {
        sdl_annuncstate[ i ] =
            ( ( ann_tbl[ i ].bit & val ) == ann_tbl[ i ].bit ) ? 1 : 0;
    }

    SDLDrawAnnunc( sdl_annuncstate );
}

void redraw_annunc( void ) {
    last_annunc_state = -1;
    draw_annunc();
}

void init_display( void ) {
    display.on = ( int )( saturn.disp_io & 0x8 ) >> 3;

    display.disp_start = ( saturn.disp_addr & 0xffffe );
    display.offset = ( saturn.disp_io & 0x7 );
    disp.offset = 2 * display.offset;

    display.lines = ( saturn.line_count & 0x3f );
    if ( display.lines == 0 )
        display.lines = 63;
    disp.lines = 2 * display.lines;
    if ( disp.lines < 110 )
        disp.lines = 110;

    if ( display.offset > 3 )
        display.nibs_per_line =
            ( NIBBLES_PER_ROW + saturn.line_offset + 2 ) & 0xfff;
    else
        display.nibs_per_line =
            ( NIBBLES_PER_ROW + saturn.line_offset ) & 0xfff;

    display.disp_end =
        display.disp_start + ( display.nibs_per_line * ( display.lines + 1 ) );

    display.menu_start = saturn.menu_addr;
    display.menu_end = saturn.menu_addr + 0x110;

    display.contrast = saturn.contrast_ctrl;
    display.contrast |= ( ( saturn.disp_test & 0x1 ) << 4 );

    display.annunc = saturn.annunc;

    memset( disp_buf, 0xf0, sizeof( disp_buf ) );
    memset( lcd_buffer, 0xf0, sizeof( lcd_buffer ) );

    init_nibble_maps();
}
