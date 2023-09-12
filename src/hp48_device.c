#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#if defined( GUI_IS_X11 )
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#include "hp48.h"
#include "hp48emu.h"
#include "timer.h"
#include "x48.h" /* adjust_contrast(); ann_struct_t; DISP_ROWS; NIBS_PER_BUFFER_ROW */

saturn_t saturn;

extern int device_check;

device_t device;

/*******/
/* LCD */
/*******/

static int last_annunc_state = -1;

display_t display;

unsigned char disp_buf[ DISP_ROWS ][ NIBS_PER_BUFFER_ROW ];
unsigned char lcd_buffer[ DISP_ROWS ][ NIBS_PER_BUFFER_ROW ];

ann_struct_t ann_tbl[] = {
    { ANN_LEFT, 16, 4, ann_left_width, ann_left_height, ann_left_bitmap },
    { ANN_RIGHT, 61, 4, ann_right_width, ann_right_height, ann_right_bitmap },
    { ANN_ALPHA, 106, 4, ann_alpha_width, ann_alpha_height, ann_alpha_bitmap },
    { ANN_BATTERY, 151, 4, ann_battery_width, ann_battery_height,
      ann_battery_bitmap },
    { ANN_BUSY, 196, 4, ann_busy_width, ann_busy_height, ann_busy_bitmap },
    { ANN_IO, 241, 4, ann_io_width, ann_io_height, ann_io_bitmap },
    { 0 } };

#if defined( GUI_IS_X11 )
Pixmap nibble_maps[ 16 ];

unsigned char nibbles[ 16 ][ 2 ] = {
    { 0x00, 0x00 }, /* ---- */
    { 0x03, 0x03 }, /* *--- */
    { 0x0c, 0x0c }, /* -*-- */
    { 0x0f, 0x0f }, /* **-- */
    { 0x30, 0x30 }, /* --*- */
    { 0x33, 0x33 }, /* *-*- */
    { 0x3c, 0x3c }, /* -**- */
    { 0x3f, 0x3f }, /* ***- */
    { 0xc0, 0xc0 }, /* ---* */
    { 0xc3, 0xc3 }, /* *--* */
    { 0xcc, 0xcc }, /* -*-* */
    { 0xcf, 0xcf }, /* **-* */
    { 0xf0, 0xf0 }, /* --** */
    { 0xf3, 0xf3 }, /* *-** */
    { 0xfc, 0xfc }, /* -*** */
    { 0xff, 0xff }  /* **** */
};

static unsigned char nibble_bitmap[ 16 ];

static inline void init_nibble_maps( void ) {
    int i;

    for ( i = 0; i < 16; i++ ) {
        nibble_maps[ i ] =
            XCreateBitmapFromData( dpy, disp.win, ( char* )nibbles[ i ], 8, 2 );
    }

    if ( shm_flag ) {
        if ( disp.disp_image->bitmap_bit_order == MSBFirst ) {
            nibble_bitmap[ 0x0 ] = 0x00; /* ---- */
            nibble_bitmap[ 0x1 ] = 0xc0; /* *--- */
            nibble_bitmap[ 0x2 ] = 0x30; /* -*-- */
            nibble_bitmap[ 0x3 ] = 0xf0; /* **-- */
            nibble_bitmap[ 0x4 ] = 0x0c; /* --*- */
            nibble_bitmap[ 0x5 ] = 0xcc; /* *-*- */
            nibble_bitmap[ 0x6 ] = 0x3c; /* -**- */
            nibble_bitmap[ 0x7 ] = 0xfc; /* ***- */
            nibble_bitmap[ 0x8 ] = 0x03; /* ---* */
            nibble_bitmap[ 0x9 ] = 0xc3; /* *--* */
            nibble_bitmap[ 0xa ] = 0x33; /* -*-* */
            nibble_bitmap[ 0xb ] = 0xf3; /* **-* */
            nibble_bitmap[ 0xc ] = 0x0f; /* --** */
            nibble_bitmap[ 0xd ] = 0xcf; /* *-** */
            nibble_bitmap[ 0xe ] = 0x3f; /* -*** */
            nibble_bitmap[ 0xf ] = 0xff; /* **** */
        } else {
            nibble_bitmap[ 0x0 ] = 0x00; /* ---- */
            nibble_bitmap[ 0x1 ] = 0x03; /* *--- */
            nibble_bitmap[ 0x2 ] = 0x0c; /* -*-- */
            nibble_bitmap[ 0x3 ] = 0x0f; /* **-- */
            nibble_bitmap[ 0x4 ] = 0x30; /* --*- */
            nibble_bitmap[ 0x5 ] = 0x33; /* *-*- */
            nibble_bitmap[ 0x6 ] = 0x3c; /* -**- */
            nibble_bitmap[ 0x7 ] = 0x3f; /* ***- */
            nibble_bitmap[ 0x8 ] = 0xc0; /* ---* */
            nibble_bitmap[ 0x9 ] = 0xc3; /* *--* */
            nibble_bitmap[ 0xa ] = 0xcc; /* -*-* */
            nibble_bitmap[ 0xb ] = 0xcf; /* **-* */
            nibble_bitmap[ 0xc ] = 0xf0; /* --** */
            nibble_bitmap[ 0xd ] = 0xf3; /* *-** */
            nibble_bitmap[ 0xe ] = 0xfc; /* -*** */
            nibble_bitmap[ 0xf ] = 0xff; /* **** */
        }
    }
}

static inline void draw_nibble( int c, int r, int val ) {
    int x, y;

    x = ( c * 8 ) + 5;

    if ( r <= display.lines )
        x -= disp.offset;
    y = ( r * 2 ) + 20;

    val &= 0x0f;
    if ( val != lcd_buffer[ r ][ c ] ) {
        XCopyPlane( dpy, nibble_maps[ val ], disp.win, disp.gc, 0, 0, 8, 2, x,
                    y, 1 );
        lcd_buffer[ r ][ c ] = val;
    }
    if ( val != lcd_buffer[ r ][ c ] )
        lcd_buffer[ r ][ c ] = val;
}
#elif defined( GUI_IS_SDL1 )
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
#endif

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

#if defined( GUI_IS_X11 )
void update_display( void ) {
    int i, j;
    long addr;
    static int old_offset = -1;
    static int old_lines = -1;
    int addr_pad;
    int val, line_pad, line_length;
    word_20 data_addr, data_addr_2;

    if ( !disp.mapped ) {
        refresh_icon();
        return;
    }
    if ( display.on ) {
        addr = display.disp_start;
        if ( shm_flag ) {
            data_addr = 0;
            data_addr_2 = disp.disp_image->bytes_per_line;
            line_length = NIBBLES_PER_ROW;
            if ( display.offset > 3 )
                line_length += 2;
            line_pad = 2 * disp.disp_image->bytes_per_line - line_length;
            addr_pad = display.nibs_per_line - line_length;
            for ( i = 0; i <= display.lines; i++ ) {
                for ( j = 0; j < line_length; j++ ) {
                    val = read_nibble( addr++ );
                    disp.disp_image->data[ data_addr++ ] = nibble_bitmap[ val ];
                    disp.disp_image->data[ data_addr_2++ ] =
                        nibble_bitmap[ val ];
                }
                addr += addr_pad;
                data_addr += line_pad;
                data_addr_2 += line_pad;
            }
            disp.display_update |= UPDATE_DISP;
        } else {
            if ( display.offset != old_offset ) {
                memset(
                    disp_buf, 0xf0,
                    ( size_t )( ( display.lines + 1 ) * NIBS_PER_BUFFER_ROW ) );
                memset(
                    lcd_buffer, 0xf0,
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
        }

        if ( i < DISP_ROWS ) {
            addr = display.menu_start;
            if ( shm_flag ) {
                data_addr = 0;
                data_addr_2 = disp.menu_image->bytes_per_line;
                line_pad =
                    2 * disp.menu_image->bytes_per_line - NIBBLES_PER_ROW;
                for ( ; i < DISP_ROWS; i++ ) {
                    for ( j = 0; j < NIBBLES_PER_ROW; j++ ) {
                        val = read_nibble( addr++ );
                        disp.menu_image->data[ data_addr++ ] =
                            nibble_bitmap[ val ];
                        disp.menu_image->data[ data_addr_2++ ] =
                            nibble_bitmap[ val ];
                    }
                    data_addr += line_pad;
                    data_addr_2 += line_pad;
                }
                disp.display_update |= UPDATE_MENU;
            } else {
                for ( ; i < DISP_ROWS; i++ ) {
                    draw_row( addr, i );
                    addr += NIBBLES_PER_ROW;
                }
            }
        }
    } else {
        if ( shm_flag ) {
            memset( disp.disp_image->data, 0,
                    ( size_t )( disp.disp_image->bytes_per_line *
                                disp.disp_image->height ) );
            memset( disp.menu_image->data, 0,
                    ( size_t )( disp.menu_image->bytes_per_line *
                                disp.menu_image->height ) );
            disp.display_update = UPDATE_DISP | UPDATE_MENU;
        } else {
            memset( disp_buf, 0xf0, sizeof( disp_buf ) );
            for ( i = 0; i < 64; i++ ) {
                for ( j = 0; j < NIBBLES_PER_ROW; j++ ) {
                    draw_nibble( j, i, 0x00 );
                }
            }
        }
    }
}

void redraw_display( void ) {
    XClearWindow( dpy, disp.win );

    memset( disp_buf, 0, sizeof( disp_buf ) );
    memset( lcd_buffer, 0, sizeof( lcd_buffer ) );
    update_display();
}

void disp_draw_nibble( word_20 addr, word_4 val ) {
    long offset;
    int shm_addr;
    int x, y;

    offset = ( addr - display.disp_start );
    x = offset % display.nibs_per_line;
    if ( x < 0 || x > 35 )
        return;
    if ( display.nibs_per_line != 0 ) {
        y = offset / display.nibs_per_line;
        if ( y < 0 || y > 63 )
            return;
        if ( shm_flag ) {
            shm_addr = ( 2 * y * disp.disp_image->bytes_per_line ) + x;
            disp.disp_image->data[ shm_addr ] = nibble_bitmap[ val ];
            disp.disp_image
                ->data[ shm_addr + disp.disp_image->bytes_per_line ] =
                nibble_bitmap[ val ];
            disp.display_update |= UPDATE_DISP;
        } else {
            if ( val != disp_buf[ y ][ x ] ) {
                disp_buf[ y ][ x ] = val;
                draw_nibble( x, y, val );
            }
        }
    } else {
        if ( shm_flag ) {
            shm_addr = x;
            for ( y = 0; y < display.lines; y++ ) {
                disp.disp_image->data[ shm_addr ] = nibble_bitmap[ val ];
                shm_addr += disp.disp_image->bytes_per_line;
                disp.disp_image->data[ shm_addr ] = nibble_bitmap[ val ];
                shm_addr += disp.disp_image->bytes_per_line;
            }
            disp.display_update |= UPDATE_DISP;
        } else {
            for ( y = 0; y < display.lines; y++ ) {
                if ( val != disp_buf[ y ][ x ] ) {
                    disp_buf[ y ][ x ] = val;
                    draw_nibble( x, y, val );
                }
            }
        }
    }
}

void menu_draw_nibble( word_20 addr, word_4 val ) {
    long offset;
    int shm_addr;
    int x, y;

    offset = ( addr - display.menu_start );
    if ( shm_flag ) {
        shm_addr =
            2 * ( offset / NIBBLES_PER_ROW ) * disp.menu_image->bytes_per_line +
            ( offset % NIBBLES_PER_ROW );
        disp.menu_image->data[ shm_addr ] = nibble_bitmap[ val ];
        disp.menu_image->data[ shm_addr + disp.menu_image->bytes_per_line ] =
            nibble_bitmap[ val ];
        disp.display_update |= UPDATE_MENU;
    } else {
        x = offset % NIBBLES_PER_ROW;
        y = display.lines + ( offset / NIBBLES_PER_ROW ) + 1;
        if ( val != disp_buf[ y ][ x ] ) {
            disp_buf[ y ][ x ] = val;
            draw_nibble( x, y, val );
        }
    }
}

void draw_annunc( void ) {
    int val;

    val = display.annunc;

    if ( val == last_annunc_state )
        return;
    last_annunc_state = val;

    for ( int i = 0; ann_tbl[ i ].bit; i++ ) {
        if ( ( ann_tbl[ i ].bit & val ) == ann_tbl[ i ].bit )
            XCopyPlane( dpy, ann_tbl[ i ].pixmap, disp.win, disp.gc, 0, 0,
                        ann_tbl[ i ].width, ann_tbl[ i ].height, ann_tbl[ i ].x,
                        ann_tbl[ i ].y, 1 );
        else
            XClearArea( dpy, disp.win, ann_tbl[ i ].x, ann_tbl[ i ].y,
                        ann_tbl[ i ].width, ann_tbl[ i ].height, False );
    }

    refresh_icon();
}

void init_annunc( void ) {
    for ( int i = 0; ann_tbl[ i ].bit; i++ )
        ann_tbl[ i ].pixmap =
            XCreateBitmapFromData( dpy, disp.win, ( char* )ann_tbl[ i ].bits,
                                   ann_tbl[ i ].width, ann_tbl[ i ].height );
}

#elif defined( GUI_IS_SDL1 )

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
#endif

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
