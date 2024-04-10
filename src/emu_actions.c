#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "runtime_options.h" /* only for inhibit_shutdown in do_shutdown() */
#include "emulator.h"
#include "emulator_inner.h"
#include "romio.h"
#include "ui.h" /* ui_get_event(); ui_update_LCD(); */

#include "debugger.h" /* in_debugger, enter_debugger */

static bool interrupt_called = false;
extern long nibble_masks[ 16 ];

bool sigalarm_triggered = false;

bool first_press = true; // PATCH

int conf_bank1 = 0x00000;
int conf_bank2 = 0x00000;

short conf_tab[] = { 1, 2, 2, 2, 2, 0 };

void do_in( void )
{
    int i, in = 0, out = 0;

    for ( i = 2; i >= 0; i-- ) {
        out <<= 4;
        out |= saturn.OUT[ i ];
    }

    for ( i = 0; i < 9; i++ )
        if ( out & ( 1 << i ) )
            in |= saturn.keybuf.rows[ i ];

    // PATCH
    // http://svn.berlios.de/wsvn/x48?op=comp&compare[]=/trunk@12&compare[]=/trunk@13
    // PAS TERRIBLE VISIBLEMENT

    if ( saturn.PC == 0x00E31 && !first_press &&
         ( ( out & 0x10 && in & 0x1 ) ||  // keys are Backspace
           ( out & 0x40 && in & 0x7 ) ||  // right, left & down
           ( out & 0x80 && in & 0x2 ) ) ) // up arrows
    {
        for ( i = 0; i < 9; i++ )
            if ( out & ( 1 << i ) )
                saturn.keybuf.rows[ i ] = 0;
        first_press = true;
    } else
        first_press = false;

    // FIN PATCH

    for ( i = 0; i < 4; i++ ) {
        saturn.IN[ i ] = in & 0xf;
        in >>= 4;
    }
}

void clear_program_stat( int n ) { saturn.PSTAT[ n ] = 0; }

void set_program_stat( int n ) { saturn.PSTAT[ n ] = 1; }

int get_program_stat( int n ) { return saturn.PSTAT[ n ]; }

void register_to_status( unsigned char* r )
{
    for ( int i = 0; i < 12; i++ ) {
        saturn.PSTAT[ i ] = ( r[ i / 4 ] >> ( i % 4 ) ) & 1;
    }
}

void status_to_register( unsigned char* r )
{
    for ( int i = 0; i < 12; i++ ) {
        if ( saturn.PSTAT[ i ] ) {
            r[ i / 4 ] |= 1 << ( i % 4 );
        } else {
            r[ i / 4 ] &= ~( 1 << ( i % 4 ) ) & 0xf;
        }
    }
}

void swap_register_status( unsigned char* r )
{
    int tmp;

    for ( int i = 0; i < 12; i++ ) {
        tmp = saturn.PSTAT[ i ];
        saturn.PSTAT[ i ] = ( r[ i / 4 ] >> ( i % 4 ) ) & 1;
        if ( tmp )
            r[ i / 4 ] |= 1 << ( i % 4 );
        else
            r[ i / 4 ] &= ~( 1 << ( i % 4 ) ) & 0xf;
    }
}

void clear_status( void )
{
    for ( int i = 0; i < 12; i++ )
        saturn.PSTAT[ i ] = 0;
}

void set_register_nibble( unsigned char* reg, int n, unsigned char val ) { reg[ n ] = val; }

unsigned char get_register_nibble( unsigned char* reg, int n ) { return reg[ n ]; }

void set_register_bit( unsigned char* reg, int n ) { reg[ n / 4 ] |= ( 1 << ( n % 4 ) ); }

void clear_register_bit( unsigned char* reg, int n ) { reg[ n / 4 ] &= ~( 1 << ( n % 4 ) ); }

int get_register_bit( unsigned char* reg, int n ) { return ( ( int )( reg[ n / 4 ] & ( 1 << ( n % 4 ) ) ) > 0 ) ? 1 : 0; }

void do_reset( void )
{
    for ( int i = 0; i < 6; i++ ) {
        saturn.mem_cntl[ i ].unconfigured = conf_tab[ i ];

        saturn.mem_cntl[ i ].config[ 0 ] = 0x0;
        saturn.mem_cntl[ i ].config[ 1 ] = 0x0;
    }
}

void do_inton( void ) { saturn.kbd_ien = true; }

void do_intoff( void ) { saturn.kbd_ien = false; }

void do_return_interupt( void )
{
    if ( saturn.int_pending ) {
        saturn.int_pending = false;
        saturn.interruptable = false;
        saturn.PC = 0xf;
    } else {
        saturn.PC = pop_return_addr();
        saturn.interruptable = true;

        if ( adj_time_pending ) {
            schedule_event = 0;
            sched_adjtime = 0;
        }
    }
}

void do_interupt( void )
{
    interrupt_called = true;
    if ( saturn.interruptable ) {
        push_return_addr( saturn.PC );
        saturn.PC = 0xf;
        saturn.interruptable = false;
    }
}

void do_kbd_int( void )
{
    do_interupt();
    if ( !saturn.interruptable )
        saturn.int_pending = true;
}

void do_reset_interrupt_system( void )
{
    saturn.kbd_ien = true;
    int gen_intr = 0;
    for ( int i = 0; i < 9; i++ ) {
        if ( saturn.keybuf.rows[ i ] != 0 ) {
            gen_intr = 1;
            break;
        }
    }
    if ( gen_intr )
        do_kbd_int();
}

void do_unconfigure( void )
{
    int i;
    unsigned int conf = 0;

    for ( i = 4; i >= 0; i-- ) {
        conf <<= 4;
        conf |= saturn.C[ i ];
    }

    for ( i = 0; i < 6; i++ ) {
        if ( saturn.mem_cntl[ i ].config[ 0 ] == conf ) {
            saturn.mem_cntl[ i ].unconfigured = conf_tab[ i ];

            saturn.mem_cntl[ i ].config[ 0 ] = 0x0;
            saturn.mem_cntl[ i ].config[ 1 ] = 0x0;
            break;
        }
    }
}

void do_configure( void )
{
    int i;
    unsigned long conf = 0;

    for ( i = 4; i >= 0; i-- ) {
        conf <<= 4;
        conf |= saturn.C[ i ];
    }

    for ( i = 0; i < 6; i++ ) {
        if ( saturn.mem_cntl[ i ].unconfigured ) {
            saturn.mem_cntl[ i ].unconfigured--;
            saturn.mem_cntl[ i ].config[ saturn.mem_cntl[ i ].unconfigured ] = conf;
            break;
        }
    }
}

int get_identification( void )
{
    int i;
    static int chip_id[] = { 0, 0, 0, 0, 0x05, 0xf6, 0x07, 0xf8, 0x01, 0xf2, 0, 0 };

    for ( i = 0; i < 6; i++ )
        if ( saturn.mem_cntl[ i ].unconfigured )
            break;

    int id = ( i < 6 ) ? chip_id[ 2 * i + ( 2 - saturn.mem_cntl[ i ].unconfigured ) ] : 0;

    for ( i = 0; i < 3; i++ ) {
        saturn.C[ i ] = id & 0x0f;
        id >>= 4;
    }

    return 0;
}

void do_shutdown( void )
{
    if ( inhibit_shutdown )
        return;

    /***************************/
    /* hpemu/src/opcodes.c:367 */
    /***************************/
    /* static void op807( byte* opc ) // SHUTDN */
    /* { */
    /*     // TODO: Fix SHUTDN */
    /*     if ( !cpu.in[ 0 ] && !cpu.in[ 1 ] && !cpu.in[ 3 ] ) { */
    /*         cpu.shutdown = true; */
    /*     } */
    /*     cpu.pc += 3; */
    /*     cpu.cycles += 5; */
    /* } */

    /***********************************/
    /* saturn_bertolotti/src/cpu.c:364 */
    /***********************************/
    /* static void ExecSHUTDN( void ) */
    /* { */
    /*     debug1( DEBUG_C_TRACE, CPU_I_CALLED, "SHUTDN" ); */

    /* #ifdef CPU_SPIN_SHUTDN */
    /*     /\* If the CPU_SPIN_SHUTDN symbol is defined, the CPU module implements */
    /*        SHUTDN as a spin loop; the program counter is reset to the starting */
    /*        nibble of the SHUTDN opcode. */
    /*     *\/ */
    /*     cpu_status.PC -= 3; */
    /* #endif */

    /*     /\* Set shutdown flag *\/ */
    /*     cpu_status.shutdn = 1; */

    /* #ifndef CPU_SPIN_SHUTDN */
    /*     /\* If the CPU_SPIN_SHUTDN symbol is not defined, the CPU module implements */
    /*        SHUTDN signalling the condition CPU_I_SHUTDN */
    /*     *\/ */
    /*     ChfCondition CPU_I_SHUTDN, CHF_INFO ChfEnd; */
    /*     ChfSignal(); */
    /* #endif */
    /* } */

    if ( device.display_touched ) {
        device.display_touched = 0;
        ui_refresh_LCD();
    }

    stop_timer( RUN_TIMER );
    start_timer( IDLE_TIMER );

    if ( is_zero_register( saturn.OUT, OUT_FIELD ) ) {
        saturn.interruptable = true;
        saturn.int_pending = false;
    }

    bool wake = in_debugger;
    t1_t2_ticks ticks;

    do {
        pause();

        if ( sigalarm_triggered ) {
            sigalarm_triggered = false;

            ui_refresh_LCD();

            ticks = get_t1_t2();
            if ( saturn.t2_ctrl & 0x01 )
                saturn.timer2 = ticks.t2_ticks;

            saturn.timer1 = set_t1 - ticks.t1_ticks;
            set_t1 = ticks.t1_ticks;

            interrupt_called = false;
            ui_get_event();
            if ( interrupt_called )
                wake = true;

            if ( saturn.timer2 <= 0 ) {
                if ( saturn.t2_ctrl & 0x04 )
                    wake = true;

                if ( saturn.t2_ctrl & 0x02 ) {
                    wake = true;
                    saturn.t2_ctrl |= 0x08;
                    do_interupt();
                }
            }

            if ( saturn.timer1 <= 0 ) {
                saturn.timer1 &= 0x0f;
                if ( saturn.t1_ctrl & 0x04 )
                    wake = true;

                if ( saturn.t1_ctrl & 0x03 ) {
                    wake = true;
                    saturn.t1_ctrl |= 0x08;
                    do_interupt();
                }
            }

            if ( !wake ) {
                interrupt_called = false;
                receive_char();
                if ( interrupt_called )
                    wake = true;
            }
        }

        if ( enter_debugger )
            wake = true;
    } while ( !wake );

    stop_timer( IDLE_TIMER );
    start_timer( RUN_TIMER );
}

void clear_hardware_stat( int op )
{
    if ( op & 1 )
        saturn.XM = 0;
    if ( op & 2 )
        saturn.SB = 0;
    if ( op & 4 )
        saturn.SR = 0;
    if ( op & 8 )
        saturn.MP = 0;
}

int is_zero_hardware_stat( int op )
{
    if ( op & 1 )
        if ( saturn.XM != 0 )
            return 0;
    if ( op & 2 )
        if ( saturn.SB != 0 )
            return 0;
    if ( op & 4 )
        if ( saturn.SR != 0 )
            return 0;
    if ( op & 8 )
        if ( saturn.MP != 0 )
            return 0;

    return 1;
}

void push_return_addr( long addr )
{
    if ( ++saturn.rstkp >= NB_RSTK ) {
        for ( int i = 1; i < NB_RSTK; i++ )
            saturn.rstk[ i - 1 ] = saturn.rstk[ i ];

        saturn.rstkp--;
    }
    saturn.rstk[ saturn.rstkp ] = addr;
}

long pop_return_addr( void )
{
    if ( saturn.rstkp < 0 )
        return 0;

    return saturn.rstk[ saturn.rstkp-- ];
}

void load_constant( unsigned char* reg, int n, long addr )
{
    int p = saturn.P;

    for ( int i = 0; i < n; i++ ) {
        reg[ p ] = read_nibble( addr + i );
        p = ( p + 1 ) & 0xf;
    }
}

void load_addr( word_20* dat, long addr, int n )
{
    for ( int i = 0; i < n; i++ ) {
        *dat &= ~nibble_masks[ i ];
        *dat |= read_nibble( addr + i ) << ( i * 4 );
    }
}

void register_to_address( unsigned char* reg, word_20* dat, int s )
{
    int n = ( s ) ? 4 : 5;

    for ( int i = 0; i < n; i++ ) {
        *dat &= ~nibble_masks[ i ];
        *dat |= ( reg[ i ] & 0x0f ) << ( i * 4 );
    }
}

long dat_to_addr( unsigned char* dat )
{
    long addr = 0;

    for ( int i = 4; i >= 0; i-- ) {
        addr <<= 4;
        addr |= ( dat[ i ] & 0xf );
    }

    return addr;
}

void addr_to_dat( long addr, unsigned char* dat )
{
    for ( int i = 0; i < 5; i++ ) {
        dat[ i ] = ( addr & 0xf );
        addr >>= 4;
    }
}

void add_address( word_20* dat, int add )
{
    *dat += add;

    if ( *dat & ( word_20 )0xfff00000 )
        saturn.CARRY = 1;
    else
        saturn.CARRY = 0;

    *dat &= 0xfffff;
}

void store( word_20 dat, unsigned char* reg, int code )
{
    int s = get_start( code );
    int e = get_end( code );

    for ( int i = s; i <= e; i++ )
        write_nibble( dat++, reg[ i ] );
}

void store_n( word_20 dat, unsigned char* reg, int n )
{
    for ( int i = 0; i < n; i++ )
        write_nibble( dat++, reg[ i ] );
}

void recall( unsigned char* reg, word_20 dat, int code )
{
    int s = get_start( code );
    int e = get_end( code );

    for ( int i = s; i <= e; i++ )
        reg[ i ] = read_nibble_crc( dat++ );
}

void recall_n( unsigned char* reg, word_20 dat, int n )
{
    for ( int i = 0; i < n; i++ )
        reg[ i ] = read_nibble_crc( dat++ );
}

/************/
/* keyboard */
/************/
