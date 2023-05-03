#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "debugger.h"
#include "device.h"
#include "hp48.h"
#include "hp48_emu.h"
#include "romio.h"
#include "timer.h"
#include "x48.h"

static int interrupt_called = 0;
extern long nibble_masks[ 16 ];

int got_alarm;

int first_press = 1; // PATCH

int conf_bank1 = 0x00000;
int conf_bank2 = 0x00000;

void do_in( void ) {
    int i, in, out;

    out = 0;
    for ( i = 2; i >= 0; i-- ) {
        out <<= 4;
        out |= saturn.OUT[ i ];
    }
    in = 0;
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
        first_press = 1;
    } else
        first_press = 0;

    // FIN PATCH

    for ( i = 0; i < 4; i++ ) {
        saturn.IN[ i ] = in & 0xf;
        in >>= 4;
    }
}

void clear_program_stat( int n ) { saturn.PSTAT[ n ] = 0; }

void set_program_stat( int n ) { saturn.PSTAT[ n ] = 1; }

int get_program_stat( int n ) { return saturn.PSTAT[ n ]; }

void register_to_status( unsigned char* r ) {
    int i;

    for ( i = 0; i < 12; i++ ) {
        saturn.PSTAT[ i ] = ( r[ i / 4 ] >> ( i % 4 ) ) & 1;
    }
}

void status_to_register( unsigned char* r ) {
    int i;

    for ( i = 0; i < 12; i++ ) {
        if ( saturn.PSTAT[ i ] ) {
            r[ i / 4 ] |= 1 << ( i % 4 );
        } else {
            r[ i / 4 ] &= ~( 1 << ( i % 4 ) ) & 0xf;
        }
    }
}

void swap_register_status( unsigned char* r ) {
    int i, tmp;

    for ( i = 0; i < 12; i++ ) {
        tmp = saturn.PSTAT[ i ];
        saturn.PSTAT[ i ] = ( r[ i / 4 ] >> ( i % 4 ) ) & 1;
        if ( tmp ) {
            r[ i / 4 ] |= 1 << ( i % 4 );
        } else {
            r[ i / 4 ] &= ~( 1 << ( i % 4 ) ) & 0xf;
        }
    }
}

void clear_status( void ) {
    int i;

    for ( i = 0; i < 12; i++ ) {
        saturn.PSTAT[ i ] = 0;
    }
}

void set_register_nibble( unsigned char* reg, int n, unsigned char val ) {
    reg[ n ] = val;
}

unsigned char get_register_nibble( unsigned char* reg, int n ) {
    return reg[ n ];
}

void set_register_bit( unsigned char* reg, int n ) {
    reg[ n / 4 ] |= ( 1 << ( n % 4 ) );
}

void clear_register_bit( unsigned char* reg, int n ) {
    reg[ n / 4 ] &= ~( 1 << ( n % 4 ) );
}

int get_register_bit( unsigned char* reg, int n ) {
    return ( ( int )( reg[ n / 4 ] & ( 1 << ( n % 4 ) ) ) > 0 ) ? 1 : 0;
}

short conf_tab_sx[] = { 1, 2, 2, 2, 2, 0 };
short conf_tab_gx[] = { 1, 2, 2, 2, 2, 0 };

void do_reset( void ) {
    int i;

    for ( i = 0; i < 6; i++ ) {
        if ( opt_gx )
            saturn.mem_cntl[ i ].unconfigured = conf_tab_gx[ i ];
        else
            saturn.mem_cntl[ i ].unconfigured = conf_tab_sx[ i ];
        saturn.mem_cntl[ i ].config[ 0 ] = 0x0;
        saturn.mem_cntl[ i ].config[ 1 ] = 0x0;
    }
}

void do_inton( void ) { saturn.kbd_ien = 1; }

void do_intoff( void ) { saturn.kbd_ien = 0; }

void do_return_interupt( void ) {
    if ( saturn.int_pending ) {
        saturn.int_pending = 0;
        saturn.intenable = 0;
        saturn.PC = 0xf;
    } else {
        saturn.PC = pop_return_addr();
        saturn.intenable = 1;

        if ( adj_time_pending ) {
            schedule_event = 0;
            sched_adjtime = 0;
        }
    }
}

void do_interupt( void ) {
    interrupt_called = 1;
    if ( saturn.intenable ) {
        push_return_addr( saturn.PC );
        saturn.PC = 0xf;
        saturn.intenable = 0;
    }
}

void do_kbd_int( void ) {
    interrupt_called = 1;
    if ( saturn.intenable ) {
        push_return_addr( saturn.PC );
        saturn.PC = 0xf;
        saturn.intenable = 0;
    } else {
        saturn.int_pending = 1;
    }
}

void do_reset_interrupt_system( void ) {
    int i, gen_intr;

    saturn.kbd_ien = 1;
    gen_intr = 0;
    for ( i = 0; i < 9; i++ ) {
        if ( saturn.keybuf.rows[ i ] != 0 ) {
            gen_intr = 1;
            break;
        }
    }
    if ( gen_intr ) {
        do_kbd_int();
    }
}

void do_unconfigure( void ) {
    int i;
    unsigned int conf;

    conf = 0;
    for ( i = 4; i >= 0; i-- ) {
        conf <<= 4;
        conf |= saturn.C[ i ];
    }

    for ( i = 0; i < 6; i++ ) {
        if ( saturn.mem_cntl[ i ].config[ 0 ] == conf ) {
            if ( opt_gx )
                saturn.mem_cntl[ i ].unconfigured = conf_tab_gx[ i ];
            else
                saturn.mem_cntl[ i ].unconfigured = conf_tab_sx[ i ];
            saturn.mem_cntl[ i ].config[ 0 ] = 0x0;
            saturn.mem_cntl[ i ].config[ 1 ] = 0x0;
            break;
        }
    }
}

void do_configure( void ) {
    int i;
    unsigned long conf;

    conf = 0;
    for ( i = 4; i >= 0; i-- ) {
        conf <<= 4;
        conf |= saturn.C[ i ];
    }

    for ( i = 0; i < 6; i++ ) {
        if ( saturn.mem_cntl[ i ].unconfigured ) {
            saturn.mem_cntl[ i ].unconfigured--;
            saturn.mem_cntl[ i ].config[ saturn.mem_cntl[ i ].unconfigured ] =
                conf;
            break;
        }
    }
}

int get_identification( void ) {
    int i;
    static int chip_id[] = { 0,    0,    0,    0,    0x05, 0xf6,
                             0x07, 0xf8, 0x01, 0xf2, 0,    0 };
    int id;

    for ( i = 0; i < 6; i++ ) {
        if ( saturn.mem_cntl[ i ].unconfigured )
            break;
    }
    if ( i < 6 )
        id = chip_id[ 2 * i + ( 2 - saturn.mem_cntl[ i ].unconfigured ) ];
    else
        id = 0;

    for ( i = 0; i < 3; i++ ) {
        saturn.C[ i ] = id & 0x0f;
        id >>= 4;
    }
    return 0;
}

void do_shutdown( void ) {
    int wake, alarms;
    t1_t2_ticks ticks;

    if ( device.display_touched ) {
        device.display_touched = 0;
        update_display();
#if defined( GUI_IS_X11 )
        if ( disp.display_update )
            refresh_display();
#endif
    }

    stop_timer( RUN_TIMER );
    start_timer( IDLE_TIMER );

    if ( is_zero_register( saturn.OUT, OUT_FIELD ) ) {
        saturn.intenable = 1;
        saturn.int_pending = 0;
    }

    if ( in_debugger )
        wake = 1;
    else
        wake = 0;

    alarms = 0;

    do {

        pause();

        if ( got_alarm ) {
            got_alarm = 0;

#if defined( GUI_IS_X11 )
            if ( disp.display_update )
                refresh_display();
#endif

            ticks = get_t1_t2();
            if ( saturn.t2_ctrl & 0x01 ) {
                saturn.timer2 = ticks.t2_ticks;
            }
            saturn.timer1 = set_t1 - ticks.t1_ticks;
            set_t1 = ticks.t1_ticks;

            interrupt_called = 0;
            if ( get_ui_event() ) {
                if ( interrupt_called )
                    wake = 1;
            }

            if ( saturn.timer2 <= 0 ) {
                if ( saturn.t2_ctrl & 0x04 ) {
                    wake = 1;
                }
                if ( saturn.t2_ctrl & 0x02 ) {
                    wake = 1;
                    saturn.t2_ctrl |= 0x08;
                    do_interupt();
                }
            }

            if ( saturn.timer1 <= 0 ) {
                saturn.timer1 &= 0x0f;
                if ( saturn.t1_ctrl & 0x04 ) {
                    wake = 1;
                }
                if ( saturn.t1_ctrl & 0x03 ) {
                    wake = 1;
                    saturn.t1_ctrl |= 0x08;
                    do_interupt();
                }
            }

            if ( wake == 0 ) {
                interrupt_called = 0;
                receive_char();
                if ( interrupt_called )
                    wake = 1;
            }

            alarms++;
        }

        if ( enter_debugger ) {
            wake = 1;
        }
    } while ( wake == 0 );

    stop_timer( IDLE_TIMER );
    start_timer( RUN_TIMER );
}

void set_hardware_stat( int op ) {
    if ( op & 1 )
        saturn.XM = 1;
    if ( op & 2 )
        saturn.SB = 1;
    if ( op & 4 )
        saturn.SR = 1;
    if ( op & 8 )
        saturn.MP = 1;
}

void clear_hardware_stat( int op ) {
    if ( op & 1 )
        saturn.XM = 0;
    if ( op & 2 )
        saturn.SB = 0;
    if ( op & 4 )
        saturn.SR = 0;
    if ( op & 8 )
        saturn.MP = 0;
}

int is_zero_hardware_stat( int op ) {
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

void push_return_addr( long addr ) {
    int i;

    if ( ++saturn.rstkp >= NR_RSTK ) {
        for ( i = 1; i < NR_RSTK; i++ )
            saturn.rstk[ i - 1 ] = saturn.rstk[ i ];
        saturn.rstkp--;
    }
    saturn.rstk[ saturn.rstkp ] = addr;
}

long pop_return_addr( void ) {
    if ( saturn.rstkp < 0 )
        return 0;
    return saturn.rstk[ saturn.rstkp-- ];
}

char* make_hexstr( long addr, int n ) {
    static char str[ 44 ];
    int i, t, trunc;

    trunc = 0;
    if ( n > 40 ) {
        n = 40;
        trunc = 1;
    }
    for ( i = 0; i < n; i++ ) {
        t = read_nibble( addr + i );
        if ( t <= 9 )
            str[ i ] = '0' + t;
        else
            str[ i ] = 'a' + ( t - 10 );
    }
    str[ n ] = '\0';
    if ( trunc ) {
        str[ n ] = '.';
        str[ n + 1 ] = '.';
        str[ n + 2 ] = '.';
        str[ n + 3 ] = '\0';
    }
    return str;
}

void load_constant( unsigned char* reg, int n, long addr ) {
    int i, p;

    p = saturn.P;
    for ( i = 0; i < n; i++ ) {
        reg[ p ] = read_nibble( addr + i );
        p = ( p + 1 ) & 0xf;
    }
}

void load_addr( word_20* dat, long addr, int n ) {
    int i;

    for ( i = 0; i < n; i++ ) {
        *dat &= ~nibble_masks[ i ];
        *dat |= read_nibble( addr + i ) << ( i * 4 );
    }
}

void load_address( unsigned char* reg, long addr, int n ) {
    int i;

    for ( i = 0; i < n; i++ ) {
        reg[ i ] = read_nibble( addr + i );
    }
}

void register_to_address( unsigned char* reg, word_20* dat, int s ) {
    int i, n;

    if ( s )
        n = 4;
    else
        n = 5;
    for ( i = 0; i < n; i++ ) {
        *dat &= ~nibble_masks[ i ];
        *dat |= ( reg[ i ] & 0x0f ) << ( i * 4 );
    }
}

void address_to_register( word_20 dat, unsigned char* reg, int s ) {
    int i, n;

    if ( s )
        n = 4;
    else
        n = 5;
    for ( i = 0; i < n; i++ ) {
        reg[ i ] = dat & 0x0f;
        dat >>= 4;
    }
}

long dat_to_addr( unsigned char* dat ) {
    int i;
    long addr;

    addr = 0;
    for ( i = 4; i >= 0; i-- ) {
        addr <<= 4;
        addr |= ( dat[ i ] & 0xf );
    }
    return addr;
}

void addr_to_dat( long addr, unsigned char* dat ) {
    int i;

    for ( i = 0; i < 5; i++ ) {
        dat[ i ] = ( addr & 0xf );
        addr >>= 4;
    }
}

void add_address( word_20* dat, int add ) {
    *dat += add;
    if ( *dat & ( word_20 )0xfff00000 ) {
        saturn.CARRY = 1;
    } else {
        saturn.CARRY = 0;
    }
    *dat &= 0xfffff;
}

static int start_fields[] = { -1, 0, 2,  0, 15, 3, 0, 0, -1, 0,
                              2,  0, 15, 3, 0,  0, 0, 0, 0 };

static int end_fields[] = { -1, -1, 2,  2,  15, 14, 1, 15, -1, -1,
                            2,  2,  15, 14, 1,  4,  3, 2,  0 };

static inline int get_start( int code ) {
    int s;

    if ( ( s = start_fields[ code ] ) == -1 ) {
        s = saturn.P;
    }
    return s;
}

static inline int get_end( int code ) {
    int e;

    if ( ( e = end_fields[ code ] ) == -1 ) {
        e = saturn.P;
    }
    return e;
}

void store( word_20 dat, unsigned char* reg, int code ) {
    int i, s, e;

    s = get_start( code );
    e = get_end( code );
    for ( i = s; i <= e; i++ ) {
        write_nibble( dat++, reg[ i ] );
    }
}

void store_n( word_20 dat, unsigned char* reg, int n ) {
    int i;

    for ( i = 0; i < n; i++ ) {
        write_nibble( dat++, reg[ i ] );
    }
}

void recall( unsigned char* reg, word_20 dat, int code ) {
    int i, s, e;

    s = get_start( code );
    e = get_end( code );
    for ( i = s; i <= e; i++ ) {
        reg[ i ] = read_nibble_crc( dat++ );
    }
}

void recall_n( unsigned char* reg, word_20 dat, int n ) {
    int i;

    for ( i = 0; i < n; i++ ) {
        reg[ i ] = read_nibble_crc( dat++ );
    }
}
