#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#if defined( GUI_IS_X11 )
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#include "device.h"
#include "hp48.h"
#include "hp48emu.h"
#include "timer.h"
#include "x48.h"

extern int device_check;

device_t device;

void check_devices( void ) {
    if ( device.display_touched > 0 && device.display_touched-- == 1 ) {
        device.display_touched = 0;
        update_display();
    }
    if ( device.display_touched > 0 ) {
        device_check = 1;
    }
    if ( device.contrast_touched ) {
        device.contrast_touched = 0;
        adjust_contrast( display.contrast );
    }
    if ( device.ann_touched ) {
        device.ann_touched = 0;
        draw_annunc();
    }
    if ( device.baud_touched ) {
        device.baud_touched = 0;
        serial_baud( saturn.baud );
    }
    if ( device.ioc_touched ) {
        device.ioc_touched = 0;
        if ( ( saturn.io_ctrl & 0x02 ) && ( saturn.rcs & 0x01 ) ) {
            do_interupt();
        }
    }
    if ( device.rbr_touched ) {
        device.rbr_touched = 0;
        receive_char();
    }
    if ( device.tbr_touched ) {
        device.tbr_touched = 0;
        transmit_char();
    }
    if ( device.t1_touched ) {
        saturn.t1_instr = 0;
        sched_timer1 = saturn.t1_tick;
        restart_timer( T1_TIMER );
        set_t1 = saturn.timer1;
        device.t1_touched = 0;
    }
    if ( device.t2_touched ) {
        saturn.t2_instr = 0;
        sched_timer2 = saturn.t2_tick;
        device.t2_touched = 0;
    }
    /*   if (device.disp_test_touched) { */
    /*     device.disp_test_touched = 0; */
    /*   } */
    /*   if (device.crc_touched) { */
    /*     device.crc_touched = 0; */
    /*   } */
    /*   if (device.power_status_touched) { */
    /*     device.power_status_touched = 0; */
    /*   } */
    /*   if (device.power_ctrl_touched) { */
    /*     device.power_ctrl_touched = 0; */
    /*   } */
    /*   if (device.mode_touched) { */
    /*     device.mode_touched = 0; */
    /*   } */
    /*   if (device.card_ctrl_touched) { */
    /*     device.card_ctrl_touched = 0; */
    /*   } */
    /*   if (device.card_status_touched) { */
    /*     device.card_status_touched = 0; */
    /*   } */
    /*   if (device.tcs_touched) { */
    /*     device.tcs_touched = 0; */
    /*   } */
    /*   if (device.rcs_touched) { */
    /*     device.rcs_touched = 0; */
    /*   } */
    /*   if (device.sreq_touched) { */
    /*     device.sreq_touched = 0; */
    /*   } */
    /*   if (device.ir_ctrl_touched) { */
    /*     device.ir_ctrl_touched = 0; */
    /*   } */
    /*   if (device.base_off_touched) { */
    /*     device.base_off_touched = 0; */
    /*   } */
    /*   if (device.lcr_touched) { */
    /*     device.lcr_touched = 0; */
    /*   } */
    /*   if (device.lbr_touched) { */
    /*     device.lbr_touched = 0; */
    /*   } */
    /*   if (device.scratch_touched) { */
    /*     device.scratch_touched = 0; */
    /*   } */
    /*   if (device.base_nibble_touched) { */
    /*     device.base_nibble_touched = 0; */
    /*   } */
    /*   if (device.unknown_touched) { */
    /*     device.unknown_touched = 0; */
    /*   } */
    /*   if (device.t1_ctrl_touched) { */
    /*     device.t1_ctrl_touched = 0; */
    /*   } */
    /*   if (device.t2_ctrl_touched) { */
    /*     device.t2_ctrl_touched = 0; */
    /*   } */
    /*   if (device.unknown2_touched) { */
    /*     device.unknown2_touched = 0; */
    /*   } */
}

/* #include <fcntl.h> */
/* #include <stdio.h> */
/* #include <stdlib.h> */
/* #include <unistd.h> */

/* void check_out_register(oid) { */
/*   static int au = -2; */
/*   unsigned char c[] = { 0xff, 0x00 }; */

/*   if (au == -2) */
/*     if ((au = open("/dev/audio", O_WRONLY)) < 0) */
/*   if (au < 0) */
/*     return; */
/*   if (saturn.OUT[2] & 0x8) */
/*     write(au, c, 1); */
/*   else */
/*     write(au, &c[1], 1); */
/* } */

/*******/
/* LCD */
/*******/

static int last_annunc_state = -1;

display_t display;

unsigned char disp_buf[ DISP_ROWS ][ NIBS_PER_BUFFER_ROW ];
unsigned char lcd_buffer[ DISP_ROWS ][ NIBS_PER_BUFFER_ROW ];

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

static unsigned char nibble_bits[ 16 ];

void init_nibble_maps( void ) {
    int i;

    for ( i = 0; i < 16; i++ ) {
        nibble_maps[ i ] =
            XCreateBitmapFromData( dpy, disp.win, ( char* )nibbles[ i ], 8, 2 );
    }

    if ( shm_flag ) {
        if ( disp.disp_image->bitmap_bit_order == MSBFirst ) {
            nibble_bits[ 0x0 ] = 0x00; /* ---- */
            nibble_bits[ 0x1 ] = 0xc0; /* *--- */
            nibble_bits[ 0x2 ] = 0x30; /* -*-- */
            nibble_bits[ 0x3 ] = 0xf0; /* **-- */
            nibble_bits[ 0x4 ] = 0x0c; /* --*- */
            nibble_bits[ 0x5 ] = 0xcc; /* *-*- */
            nibble_bits[ 0x6 ] = 0x3c; /* -**- */
            nibble_bits[ 0x7 ] = 0xfc; /* ***- */
            nibble_bits[ 0x8 ] = 0x03; /* ---* */
            nibble_bits[ 0x9 ] = 0xc3; /* *--* */
            nibble_bits[ 0xa ] = 0x33; /* -*-* */
            nibble_bits[ 0xb ] = 0xf3; /* **-* */
            nibble_bits[ 0xc ] = 0x0f; /* --** */
            nibble_bits[ 0xd ] = 0xcf; /* *-** */
            nibble_bits[ 0xe ] = 0x3f; /* -*** */
            nibble_bits[ 0xf ] = 0xff; /* **** */
        } else {
            nibble_bits[ 0x0 ] = 0x00; /* ---- */
            nibble_bits[ 0x1 ] = 0x03; /* *--- */
            nibble_bits[ 0x2 ] = 0x0c; /* -*-- */
            nibble_bits[ 0x3 ] = 0x0f; /* **-- */
            nibble_bits[ 0x4 ] = 0x30; /* --*- */
            nibble_bits[ 0x5 ] = 0x33; /* *-*- */
            nibble_bits[ 0x6 ] = 0x3c; /* -**- */
            nibble_bits[ 0x7 ] = 0x3f; /* ***- */
            nibble_bits[ 0x8 ] = 0xc0; /* ---* */
            nibble_bits[ 0x9 ] = 0xc3; /* *--* */
            nibble_bits[ 0xa ] = 0xcc; /* -*-* */
            nibble_bits[ 0xb ] = 0xcf; /* **-* */
            nibble_bits[ 0xc ] = 0xf0; /* --** */
            nibble_bits[ 0xd ] = 0xf3; /* *-** */
            nibble_bits[ 0xe ] = 0xfc; /* -*** */
            nibble_bits[ 0xf ] = 0xff; /* **** */
        }
    }
}
#endif

ann_struct_t ann_tbl[] = {
    { ANN_LEFT, 16, 4, ann_left_width, ann_left_height, ann_left_bits },
    { ANN_RIGHT, 61, 4, ann_right_width, ann_right_height, ann_right_bits },
    { ANN_ALPHA, 106, 4, ann_alpha_width, ann_alpha_height, ann_alpha_bits },
    { ANN_BATTERY, 151, 4, ann_battery_width, ann_battery_height,
      ann_battery_bits },
    { ANN_BUSY, 196, 4, ann_busy_width, ann_busy_height, ann_busy_bits },
    { ANN_IO, 241, 4, ann_io_width, ann_io_height, ann_io_bits },
    { 0 } };

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

#if defined( GUI_IS_X11 )
    init_nibble_maps();
#endif
}

static inline void draw_nibble( int c, int r, int val ) {
    int x, y;

#if defined( GUI_IS_X11 )
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
#elif defined( GUI_IS_SDL1 )
    if ( val != lcd_buffer[ r ][ c ] )
        lcd_buffer[ r ][ c ] = val;

    x = ( c * 4 ); // x: start in pixels
    if ( r <= display.lines )
        x -= disp.offset; // Correct the pixels with display offset
    y = r;                // y: start in pixels
    SDLDrawNibble( x, y, val );
#endif
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
#if defined( GUI_IS_X11 )
    int addr_pad;
    int val, line_pad, line_length;
    word_20 data_addr, data_addr_2;
#endif

    if ( !disp.mapped ) {
#if defined( GUI_IS_X11 )
        refresh_icon();
#endif
        return;
    }
    if ( display.on ) {
        addr = display.disp_start;
#if defined( GUI_IS_X11 )
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
                    disp.disp_image->data[ data_addr++ ] = nibble_bits[ val ];
                    disp.disp_image->data[ data_addr_2++ ] = nibble_bits[ val ];
                }
                addr += addr_pad;
                data_addr += line_pad;
                data_addr_2 += line_pad;
            }
            disp.display_update |= UPDATE_DISP;
        } else {
#endif
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
#if defined( GUI_IS_X11 )
        }
#endif
        if ( i < DISP_ROWS ) {
            addr = display.menu_start;
#if defined( GUI_IS_X11 )
            if ( shm_flag ) {
                data_addr = 0;
                data_addr_2 = disp.menu_image->bytes_per_line;
                line_pad =
                    2 * disp.menu_image->bytes_per_line - NIBBLES_PER_ROW;
                for ( ; i < DISP_ROWS; i++ ) {
                    for ( j = 0; j < NIBBLES_PER_ROW; j++ ) {
                        val = read_nibble( addr++ );
                        disp.menu_image->data[ data_addr++ ] =
                            nibble_bits[ val ];
                        disp.menu_image->data[ data_addr_2++ ] =
                            nibble_bits[ val ];
                    }
                    data_addr += line_pad;
                    data_addr_2 += line_pad;
                }
                disp.display_update |= UPDATE_MENU;
            } else {
#endif
                for ( ; i < DISP_ROWS; i++ ) {
                    draw_row( addr, i );
                    addr += NIBBLES_PER_ROW;
                }
#if defined( GUI_IS_X11 )
            }
#endif
        }
    } else {
#if defined( GUI_IS_X11 )
        if ( shm_flag ) {
            memset( disp.disp_image->data, 0,
                    ( size_t )( disp.disp_image->bytes_per_line *
                                disp.disp_image->height ) );
            memset( disp.menu_image->data, 0,
                    ( size_t )( disp.menu_image->bytes_per_line *
                                disp.menu_image->height ) );
            disp.display_update = UPDATE_DISP | UPDATE_MENU;
        } else {
#endif
            memset( disp_buf, 0xf0, sizeof( disp_buf ) );
            for ( i = 0; i < 64; i++ ) {
                for ( j = 0; j < NIBBLES_PER_ROW; j++ ) {
                    draw_nibble( j, i, 0x00 );
                }
            }
#if defined( GUI_IS_X11 )
        }
#endif
    }
}

void redraw_display( void ) {
#if defined( GUI_IS_X11 )
    XClearWindow( dpy, disp.win );
#endif
    memset( disp_buf, 0, sizeof( disp_buf ) );
    memset( lcd_buffer, 0, sizeof( lcd_buffer ) );
    update_display();
}

void disp_draw_nibble( word_20 addr, word_4 val ) {
    long offset;
#if defined( GUI_IS_X11 )
    int shm_addr;
#endif
    int x, y;

    offset = ( addr - display.disp_start );
    x = offset % display.nibs_per_line;
    if ( x < 0 || x > 35 )
        return;
    if ( display.nibs_per_line != 0 ) {
        y = offset / display.nibs_per_line;
        if ( y < 0 || y > 63 )
            return;
#if defined( GUI_IS_X11 )
        if ( shm_flag ) {
            shm_addr = ( 2 * y * disp.disp_image->bytes_per_line ) + x;
            disp.disp_image->data[ shm_addr ] = nibble_bits[ val ];
            disp.disp_image
                ->data[ shm_addr + disp.disp_image->bytes_per_line ] =
                nibble_bits[ val ];
            disp.display_update |= UPDATE_DISP;
        } else {
#endif
            if ( val != disp_buf[ y ][ x ] ) {
                disp_buf[ y ][ x ] = val;
                draw_nibble( x, y, val );
            }
#if defined( GUI_IS_X11 )
        }
#endif
    } else {
#if defined( GUI_IS_X11 )
        if ( shm_flag ) {
            shm_addr = x;
            for ( y = 0; y < display.lines; y++ ) {
                disp.disp_image->data[ shm_addr ] = nibble_bits[ val ];
                shm_addr += disp.disp_image->bytes_per_line;
                disp.disp_image->data[ shm_addr ] = nibble_bits[ val ];
                shm_addr += disp.disp_image->bytes_per_line;
            }
            disp.display_update |= UPDATE_DISP;
        } else {
#endif
            for ( y = 0; y < display.lines; y++ ) {
                if ( val != disp_buf[ y ][ x ] ) {
                    disp_buf[ y ][ x ] = val;
                    draw_nibble( x, y, val );
                }
            }
#if defined( GUI_IS_X11 )
        }
#endif
    }
}

void menu_draw_nibble( word_20 addr, word_4 val ) {
    long offset;
#if defined( GUI_IS_X11 )
    int shm_addr;
#endif
    int x, y;

    offset = ( addr - display.menu_start );
#if defined( GUI_IS_X11 )
    if ( shm_flag ) {
        shm_addr =
            2 * ( offset / NIBBLES_PER_ROW ) * disp.menu_image->bytes_per_line +
            ( offset % NIBBLES_PER_ROW );
        disp.menu_image->data[ shm_addr ] = nibble_bits[ val ];
        disp.menu_image->data[ shm_addr + disp.menu_image->bytes_per_line ] =
            nibble_bits[ val ];
        disp.display_update |= UPDATE_MENU;
    } else {
#endif
        x = offset % NIBBLES_PER_ROW;
        y = display.lines + ( offset / NIBBLES_PER_ROW ) + 1;
        if ( val != disp_buf[ y ][ x ] ) {
            disp_buf[ y ][ x ] = val;
            draw_nibble( x, y, val );
        }
#if defined( GUI_IS_X11 )
    }
#endif
}

void draw_annunc( void ) {
    int val;
    int i;

    val = display.annunc;

    if ( val == last_annunc_state )
        return;
    last_annunc_state = val;

#if defined( GUI_IS_SDL1 )
    char sdl_annuncstate[ 6 ];
#endif
    for ( i = 0; ann_tbl[ i ].bit; i++ ) {
#if defined( GUI_IS_X11 )
        if ( ( ann_tbl[ i ].bit & val ) == ann_tbl[ i ].bit ) {
            XCopyPlane( dpy, ann_tbl[ i ].pixmap, disp.win, disp.gc, 0, 0,
                        ann_tbl[ i ].width, ann_tbl[ i ].height, ann_tbl[ i ].x,
                        ann_tbl[ i ].y, 1 );
        } else {
            XClearArea( dpy, disp.win, ann_tbl[ i ].x, ann_tbl[ i ].y,
                        ann_tbl[ i ].width, ann_tbl[ i ].height, False );
        }
#elif defined( GUI_IS_SDL1 )
        if ( ( ann_tbl[ i ].bit & val ) == ann_tbl[ i ].bit )
            sdl_annuncstate[ i ] = 1;
        else
            sdl_annuncstate[ i ] = 0;
#endif
    }
#if defined( GUI_IS_X11 )
    refresh_icon();
#elif defined( GUI_IS_SDL1 )
    SDLDrawAnnunc( sdl_annuncstate );
#endif
}

#if defined( GUI_IS_X11 )
void init_annunc( void ) {
    int i;

    for ( i = 0; ann_tbl[ i ].bit; i++ ) {
        ann_tbl[ i ].pixmap =
            XCreateBitmapFromData( dpy, disp.win, ( char* )ann_tbl[ i ].bits,
                                   ann_tbl[ i ].width, ann_tbl[ i ].height );
    }
}
#endif

void redraw_annunc( void ) {
    last_annunc_state = -1;
    draw_annunc();
}
