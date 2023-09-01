#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "hp48.h"
#include "hp48emu.h"
#include "timer.h"
#include "x48.h"

saturn_t saturn;

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

ann_struct_t ann_tbl[] = {
    { ANN_LEFT, 16, 4, ann_left_width, ann_left_height, ann_left_bitmap },
    { ANN_RIGHT, 61, 4, ann_right_width, ann_right_height, ann_right_bitmap },
    { ANN_ALPHA, 106, 4, ann_alpha_width, ann_alpha_height, ann_alpha_bitmap },
    { ANN_BATTERY, 151, 4, ann_battery_width, ann_battery_height,
      ann_battery_bitmap },
    { ANN_BUSY, 196, 4, ann_busy_width, ann_busy_height, ann_busy_bitmap },
    { ANN_IO, 241, 4, ann_io_width, ann_io_height, ann_io_bitmap },
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
}

static inline void emit_nibble( int c, int r, int val ) {
	for (int i = 0; i < 4; i++) {
		int x = c * 4;
		int y = r;
		int bit = (val & (1 << i));
		printf("%d %d %d ", x, y, bit);
	}
	printf("\n");
}

static inline void draw_nibble( int c, int r, int val ) {
    int x, y;

    x=r;
    y=c;
    val &= 0x0f;
    if ( val != lcd_buffer[ r ][ c ] ) {
        lcd_buffer[ r ][ c ] = val;
	emit_nibble( x, y, val );
    }
}

void draw_annunc( void ) {
}

void redraw_annunc( void ) {
    last_annunc_state = -1;
    draw_annunc();
}

void update_display( void ) {}
void redraw_display( void ) {}

void disp_draw_nibble( word_20 addr, word_4 val ) {
    long offset;
    int x, y;

    offset = ( addr - display.disp_start );
    x = offset % NIBBLES_PER_ROW;
    if ( x < 0 || x > 35 )
        return;
    y = offset / NIBBLES_PER_ROW;
    if ( y < 0 || y > 63 )
        return;
    if ( val != disp_buf[ y ][ x ] ) {
	disp_buf[ y ][ x ] = val;
	draw_nibble( x, y, val );
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
