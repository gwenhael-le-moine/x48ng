#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "emulate.h"
#include "init.h"
#include "memory.h"
#include "../options.h"
#include "persistence.h"
#include "serial.h"

// bool please_exit = false;
bool save_before_exit = true;

static void init_display( void )
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
}

/**********/
/* public */
/**********/

void saturn_config_init( void )
{
    saturn.version[ 0 ] = VERSION_MAJOR;
    saturn.version[ 1 ] = VERSION_MINOR;
    saturn.version[ 2 ] = PATCHLEVEL;
    memset( &device, 0, sizeof( device ) );
    device.baud_touched = true;
    saturn.rcs = 0x0;
    saturn.tcs = 0x0;
    saturn.lbr = 0x0;
}

void init_saturn( void )
{
    memset( &saturn, 0, sizeof( saturn ) - 4 * sizeof( unsigned char* ) );
    saturn.pc = 0x00000;
    saturn.magic = X48_MAGIC;
    saturn.t1_tick = 8192;
    saturn.t2_tick = 16;
    saturn.i_per_s = 0;
    saturn.version[ 0 ] = VERSION_MAJOR;
    saturn.version[ 1 ] = VERSION_MINOR;
    saturn.version[ 2 ] = PATCHLEVEL;
    saturn.hexmode = HEX;
    saturn.rstk_ptr = -1;
    saturn.interruptable = true;
    saturn.int_pending = false;
    saturn.kbd_ien = true;
    saturn.timer1 = 0;
    saturn.timer2 = 0x2000;
    saturn.bank_switch = 0;
    for ( int i = 0; i < NB_MCTL; i++ ) {
        if ( i == 0 )
            saturn.mem_cntl[ i ].unconfigured = 1;
        else if ( i == 5 )
            saturn.mem_cntl[ i ].unconfigured = 0;
        else
            saturn.mem_cntl[ i ].unconfigured = 2;
        saturn.mem_cntl[ i ].config[ 0 ] = 0;
        saturn.mem_cntl[ i ].config[ 1 ] = 0;
    }
    dev_memory_init();
}

void start_emulator( void )
{
    // please_exit = false;
    save_before_exit = true;

    /* If files are successfully read => return and let's go */
    if ( read_files() ) {
        if ( config.resetOnStartup )
            saturn.pc = 0x00000;
    } else {
        /* if files were not readable => initialize */
        if ( config.verbose )
            fprintf( stderr, "initialization of %s\n", normalized_config_path );

        init_saturn();
        if ( !read_rom( normalized_rom_path ) )
            exit( 1 ); /* can't read ROM */
    }

    init_serial();
    init_display();
}

void stop_emulator( void )
{
    if ( save_before_exit )
        write_files();
}
