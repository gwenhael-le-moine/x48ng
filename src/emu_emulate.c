#include <stdio.h>
#include <unistd.h>

#include <sys/time.h>

#include "debugger.h" /* enter_debugger, TRAP_INSTRUCTION, ILLEGAL_INSTRUCTION */
#include "emulator.h"
#include "emulator_inner.h"
#include "config.h" /* throttle */
#include "ui.h"     /* ui_get_event(); ui_adjust_contrast(); ui_update_LCD(); ui_draw_annunc(); */

#include "debugger.h" /* in_debugger, enter_debugger */

/* #define P_FIELD 0  /\* unused? *\/ */
/* #define WP_FIELD 1 /\* unused? *\/ */
/* #define XS_FIELD 2 /\* unused? *\/ */
/* #define X_FIELD 3  /\* unused? *\/ */
/* #define S_FIELD 4  /\* unused? *\/ */
/* #define M_FIELD 5  /\* unused? *\/ */
/* #define B_FIELD 6  /\* unused? *\/ */
#define W_FIELD 7
#define A_FIELD 15
#define IN_FIELD 16
#define OUTS_FIELD 18

#define SrvcIoStart 0x3c0
#define SrvcIoEnd 0x5ec

#define SCHED_INSTR_ROLLOVER 0x3fffffff
#define SCHED_RECEIVE 0x7ff
#define SCHED_ADJTIME 0x1ffe
#define SCHED_TIMER1 0x1e00
#define SCHED_TIMER2 0xf
#define SCHED_STATISTICS 0x7ffff
#define SCHED_NEVER 0x7fffffff

#define NB_SAMPLES 10

extern long nibble_masks[ 16 ];

saturn_t saturn;
device_t device;

bool sigalarm_triggered = false;
bool device_check = false;
bool adj_time_pending = false;

int set_t1;

long schedule_event = 0;
long sched_adjtime = SCHED_ADJTIME;
long sched_timer1 = SCHED_TIMER1;
long sched_timer2 = SCHED_TIMER2;

unsigned long t1_i_per_tick;
unsigned long t2_i_per_tick;

static bool interrupt_called = false;
static bool first_press = true; // PATCH

static unsigned long instructions = 0;

static unsigned long s_1 = 0;
static unsigned long s_16 = 0;
static unsigned long old_s_1 = 0;
static unsigned long old_s_16 = 0;
static unsigned long delta_t_1;
static unsigned long delta_t_16;
static unsigned long delta_i;

static long sched_instr_rollover = SCHED_INSTR_ROLLOVER;
static long sched_receive = SCHED_RECEIVE;
static long sched_statistics = SCHED_STATISTICS;
static long sched_display = SCHED_NEVER;

static inline void push_return_addr( long addr )
{
    if ( ++saturn.rstkp >= NB_RSTK ) {
        for ( int i = 1; i < NB_RSTK; i++ )
            saturn.RSTK[ i - 1 ] = saturn.RSTK[ i ];

        saturn.rstkp--;
    }
    saturn.RSTK[ saturn.rstkp ] = addr;
}

static inline long pop_return_addr( void )
{
    if ( saturn.rstkp < 0 )
        return 0;

    return saturn.RSTK[ saturn.rstkp-- ];
}

static inline void do_in( void )
{
    int i, in = 0, out = 0;

    for ( i = 2; i >= 0; i-- ) {
        out <<= 4;
        out |= saturn.OUT[ i ];
    }

    for ( i = 0; i < KEYS_BUFFER_SIZE; i++ )
        if ( out & ( 1 << i ) )
            in |= saturn.keybuf[ i ];

    // PATCH
    // http://svn.berlios.de/wsvn/x48?op=comp&compare[]=/trunk@12&compare[]=/trunk@13
    // PAS TERRIBLE VISIBLEMENT

    if ( saturn.PC == 0x00E31 && !first_press &&
         ( ( out & 0x10 && in & 0x1 ) ||  // keys are Backspace
           ( out & 0x40 && in & 0x7 ) ||  // right, left & down
           ( out & 0x80 && in & 0x2 ) ) ) // up arrows
    {
        for ( i = 0; i < KEYS_BUFFER_SIZE; i++ )
            if ( out & ( 1 << i ) )
                saturn.keybuf[ i ] = 0;
        first_press = true;
    } else
        first_press = false;

    // FIN PATCH

    for ( i = 0; i < 4; i++ ) {
        saturn.IN[ i ] = in & 0xf;
        in >>= 4;
    }
}

static inline int get_program_stat( int n ) { return saturn.PSTAT[ n ]; }

static inline void set_register_nibble( unsigned char* reg, int n, unsigned char val ) { reg[ n ] = val; }

static inline unsigned char get_register_nibble( unsigned char* reg, int n ) { return reg[ n ]; }

static inline void set_register_bit( unsigned char* reg, int n ) { reg[ n / 4 ] |= ( 1 << ( n % 4 ) ); }

static inline void clear_register_bit( unsigned char* reg, int n ) { reg[ n / 4 ] &= ~( 1 << ( n % 4 ) ); }

static inline int get_register_bit( unsigned char* reg, int n ) { return ( ( int )( reg[ n / 4 ] & ( 1 << ( n % 4 ) ) ) > 0 ) ? 1 : 0; }

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

static inline void load_constant( unsigned char* reg, int n, long addr )
{
    int p = saturn.P;

    for ( int i = 0; i < n; i++ ) {
        reg[ p ] = read_nibble( addr + i );
        p = ( p + 1 ) & 0xf;
    }
}

static inline void register_to_address( unsigned char* reg, word_20* dat, int s )
{
    int n = ( s ) ? 4 : 5;

    for ( int i = 0; i < n; i++ ) {
        *dat &= ~nibble_masks[ i ];
        *dat |= ( reg[ i ] & 0x0f ) << ( i * 4 );
    }
}

static inline long dat_to_addr( unsigned char* dat )
{
    long addr = 0;

    for ( int i = 4; i >= 0; i-- ) {
        addr <<= 4;
        addr |= ( dat[ i ] & 0xf );
    }

    return addr;
}

static inline void addr_to_dat( long addr, unsigned char* dat )
{
    for ( int i = 0; i < 5; i++ ) {
        dat[ i ] = ( addr & 0xf );
        addr >>= 4;
    }
}

static inline void add_address( word_20* dat, int add )
{
    *dat += add;

    if ( *dat & ( word_20 )0xfff00000 )
        saturn.CARRY = 1;
    else
        saturn.CARRY = 0;

    *dat &= 0xfffff;
}

static inline void store( word_20 dat, unsigned char* reg, int code )
{
    int s = get_start( code );
    int e = get_end( code );

    for ( int i = s; i <= e; i++ )
        write_nibble( dat++, reg[ i ] );
}

static inline void store_n( word_20 dat, unsigned char* reg, int n )
{
    for ( int i = 0; i < n; i++ )
        write_nibble( dat++, reg[ i ] );
}

static inline void recall( unsigned char* reg, word_20 dat, int code )
{
    int s = get_start( code );
    int e = get_end( code );

    for ( int i = s; i <= e; i++ )
        reg[ i ] = read_nibble_crc( dat++ );
}

static inline void recall_n( unsigned char* reg, word_20 dat, int n )
{
    for ( int i = 0; i < n; i++ )
        reg[ i ] = read_nibble_crc( dat++ );
}

void load_addr( word_20* dat, long addr, int n )
{
    for ( int i = 0; i < n; i++ ) {
        *dat &= ~nibble_masks[ i ];
        *dat |= read_nibble( addr + i ) << ( i * 4 );
    }
}

void step_instruction( void )
{
    word_20 jumpmasks[] = { 0xffffffff, 0xfffffff0, 0xffffff00, 0xfffff000, 0xffff0000, 0xfff00000, 0xff000000, 0xf0000000 };
    short conf_tab[] = { 1, 2, 2, 2, 2, 0 };

    bool illegal_instruction = false;
    int op0, op1, op2, op3, op4, op5;

    op0 = read_nibble( saturn.PC );
    switch ( op0 ) {
        case 0:
            op1 = read_nibble( saturn.PC + 1 );
            switch ( op1 ) {
                case 0: /* RTNSXM */
                    saturn.XM = 1;
                    saturn.PC = pop_return_addr();
                    break;
                case 1: /* RTN */
                    saturn.PC = pop_return_addr();
                    break;
                case 2: /* RTNSC */
                    saturn.CARRY = 1;
                    saturn.PC = pop_return_addr();
                    break;
                case 3: /* RTNCC */
                    saturn.CARRY = 0;
                    saturn.PC = pop_return_addr();
                    break;
                case 4: /* SETHEX */
                    saturn.PC += 2;
                    saturn.hexmode = HEX;
                    break;
                case 5: /* SETDEC */
                    saturn.PC += 2;
                    saturn.hexmode = DEC;
                    break;
                case 6: /* RSTK=C */
                    push_return_addr( dat_to_addr( saturn.C ) );
                    saturn.PC += 2;
                    break;
                case 7: /* C=RSTK */
                    saturn.PC += 2;
                    addr_to_dat( pop_return_addr(), saturn.C );
                    break;
                case 8: /* CLRST */
                    saturn.PC += 2;
                    for ( int i = 0; i < 12; i++ )
                        saturn.PSTAT[ i ] = 0;
                    break;
                case 9: /* C=ST */
                    saturn.PC += 2;
                    for ( int i = 0; i < 12; i++ )
                        if ( saturn.PSTAT[ i ] )
                            saturn.C[ i / 4 ] |= 1 << ( i % 4 );
                        else
                            saturn.C[ i / 4 ] &= ~( 1 << ( i % 4 ) ) & 0xf;
                    break;
                case 0xa: /* ST=C */
                    saturn.PC += 2;
                    for ( int i = 0; i < 12; i++ )
                        saturn.PSTAT[ i ] = ( saturn.C[ i / 4 ] >> ( i % 4 ) ) & 1;
                    break;
                case 0xb: /* CSTEX */
                    saturn.PC += 2;
                    {
                        int tmp;

                        for ( int i = 0; i < 12; i++ ) {
                            tmp = saturn.PSTAT[ i ];
                            saturn.PSTAT[ i ] = ( saturn.C[ i / 4 ] >> ( i % 4 ) ) & 1;
                            if ( tmp )
                                saturn.C[ i / 4 ] |= 1 << ( i % 4 );
                            else
                                saturn.C[ i / 4 ] &= ~( 1 << ( i % 4 ) ) & 0xf;
                        }
                    }
                    break;
                case 0xc: /* P=P+1 */
                    saturn.PC += 2;
                    if ( saturn.P == 0xf ) {
                        saturn.P = 0;
                        saturn.CARRY = 1;
                    } else {
                        saturn.P += 1;
                        saturn.CARRY = 0;
                    }
                    break;
                case 0xd: /* P=P-1 */
                    saturn.PC += 2;
                    if ( saturn.P == 0 ) {
                        saturn.P = 0xf;
                        saturn.CARRY = 1;
                    } else {
                        saturn.P -= 1;
                        saturn.CARRY = 0;
                    }
                    break;
                case 0xe:
                    op2 = read_nibble( saturn.PC + 2 );
                    op3 = read_nibble( saturn.PC + 3 );
                    switch ( op3 ) {
                        case 0: /* A=A&B */
                            saturn.PC += 4;
                            and_register( saturn.A, saturn.A, saturn.B, op2 );
                            break;
                        case 1: /* B=B&C */
                            saturn.PC += 4;
                            and_register( saturn.B, saturn.B, saturn.C, op2 );
                            break;
                        case 2: /* C=C&A */
                            saturn.PC += 4;
                            and_register( saturn.C, saturn.C, saturn.A, op2 );
                            break;
                        case 3: /* D=D&C */
                            saturn.PC += 4;
                            and_register( saturn.D, saturn.D, saturn.C, op2 );
                            break;
                        case 4: /* B=B&A */
                            saturn.PC += 4;
                            and_register( saturn.B, saturn.B, saturn.A, op2 );
                            break;
                        case 5: /* C=C&B */
                            saturn.PC += 4;
                            and_register( saturn.C, saturn.C, saturn.B, op2 );
                            break;
                        case 6: /* A=A&C */
                            saturn.PC += 4;
                            and_register( saturn.A, saturn.A, saturn.C, op2 );
                            break;
                        case 7: /* C=C&D */
                            saturn.PC += 4;
                            and_register( saturn.C, saturn.C, saturn.D, op2 );
                            break;
                        case 8: /* A=A!B */
                            saturn.PC += 4;
                            or_register( saturn.A, saturn.A, saturn.B, op2 );
                            break;
                        case 9: /* B=B!C */
                            saturn.PC += 4;
                            or_register( saturn.B, saturn.B, saturn.C, op2 );
                            break;
                        case 0xa: /* C=C!A */
                            saturn.PC += 4;
                            or_register( saturn.C, saturn.C, saturn.A, op2 );
                            break;
                        case 0xb: /* D=D!C */
                            saturn.PC += 4;
                            or_register( saturn.D, saturn.D, saturn.C, op2 );
                            break;
                        case 0xc: /* B=B!A */
                            saturn.PC += 4;
                            or_register( saturn.B, saturn.B, saturn.A, op2 );
                            break;
                        case 0xd: /* C=C!B */
                            saturn.PC += 4;
                            or_register( saturn.C, saturn.C, saturn.B, op2 );
                            break;
                        case 0xe: /* A=A!C */
                            saturn.PC += 4;
                            or_register( saturn.A, saturn.A, saturn.C, op2 );
                            break;
                        case 0xf: /* C=C!D */
                            saturn.PC += 4;
                            or_register( saturn.C, saturn.C, saturn.D, op2 );
                            break;
                        default:
                            illegal_instruction = true;
                    }
                    break;
                case 0xf: /* RTI */
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
                    break;
                default:
                    illegal_instruction = true;
            }
            break;
        case 1:
            op2 = read_nibble( saturn.PC + 1 );
            switch ( op2 ) {
                case 0:
                    op3 = read_nibble( saturn.PC + 2 );
                    switch ( op3 ) {
                        case 0: /* saturn.R0=A */
                            saturn.PC += 3;
                            copy_register( saturn.R0, saturn.A, W_FIELD );
                            break;
                        case 1: /* saturn.R1=A */
                        case 5:
                            saturn.PC += 3;
                            copy_register( saturn.R1, saturn.A, W_FIELD );
                            break;
                        case 2: /* saturn.R2=A */
                        case 6:
                            saturn.PC += 3;
                            copy_register( saturn.R2, saturn.A, W_FIELD );
                            break;
                        case 3: /* saturn.R3=A */
                        case 7:
                            saturn.PC += 3;
                            copy_register( saturn.R3, saturn.A, W_FIELD );
                            break;
                        case 4: /* saturn.R4=A */
                            saturn.PC += 3;
                            copy_register( saturn.R4, saturn.A, W_FIELD );
                            break;
                        case 8: /* saturn.R0=C */
                            saturn.PC += 3;
                            copy_register( saturn.R0, saturn.C, W_FIELD );
                            break;
                        case 9: /* saturn.R1=C */
                        case 0xd:
                            saturn.PC += 3;
                            copy_register( saturn.R1, saturn.C, W_FIELD );
                            break;
                        case 0xa: /* saturn.R2=C */
                        case 0xe:
                            saturn.PC += 3;
                            copy_register( saturn.R2, saturn.C, W_FIELD );
                            break;
                        case 0xb: /* saturn.R3=C */
                        case 0xf:
                            saturn.PC += 3;
                            copy_register( saturn.R3, saturn.C, W_FIELD );
                            break;
                        case 0xc: /* saturn.R4=C */
                            saturn.PC += 3;
                            copy_register( saturn.R4, saturn.C, W_FIELD );
                            break;
                        default:
                            illegal_instruction = true;
                    }
                    break;
                case 1:
                    op3 = read_nibble( saturn.PC + 2 );
                    switch ( op3 ) {
                        case 0: /* A=R0 */
                            saturn.PC += 3;
                            copy_register( saturn.A, saturn.R0, W_FIELD );
                            break;
                        case 1: /* A=R1 */
                        case 5:
                            saturn.PC += 3;
                            copy_register( saturn.A, saturn.R1, W_FIELD );
                            break;
                        case 2: /* A=R2 */
                        case 6:
                            saturn.PC += 3;
                            copy_register( saturn.A, saturn.R2, W_FIELD );
                            break;
                        case 3: /* A=R3 */
                        case 7:
                            saturn.PC += 3;
                            copy_register( saturn.A, saturn.R3, W_FIELD );
                            break;
                        case 4: /* A=R4 */
                            saturn.PC += 3;
                            copy_register( saturn.A, saturn.R4, W_FIELD );
                            break;
                        case 8: /* C=R0 */
                            saturn.PC += 3;
                            copy_register( saturn.C, saturn.R0, W_FIELD );
                            break;
                        case 9: /* C=R1 */
                        case 0xd:
                            saturn.PC += 3;
                            copy_register( saturn.C, saturn.R1, W_FIELD );
                            break;
                        case 0xa: /* C=R2 */
                        case 0xe:
                            saturn.PC += 3;
                            copy_register( saturn.C, saturn.R2, W_FIELD );
                            break;
                        case 0xb: /* C=R3 */
                        case 0xf:
                            saturn.PC += 3;
                            copy_register( saturn.C, saturn.R3, W_FIELD );
                            break;
                        case 0xc: /* C=R4 */
                            saturn.PC += 3;
                            copy_register( saturn.C, saturn.R4, W_FIELD );
                            break;
                        default:
                            illegal_instruction = true;
                    }
                    break;
                case 2:
                    op3 = read_nibble( saturn.PC + 2 );
                    switch ( op3 ) {
                        case 0: /* AR0EX */
                            saturn.PC += 3;
                            exchange_register( saturn.A, saturn.R0, W_FIELD );
                            break;
                        case 1: /* AR1EX */
                        case 5:
                            saturn.PC += 3;
                            exchange_register( saturn.A, saturn.R1, W_FIELD );
                            break;
                        case 2: /* AR2EX */
                        case 6:
                            saturn.PC += 3;
                            exchange_register( saturn.A, saturn.R2, W_FIELD );
                            break;
                        case 3: /* AR3EX */
                        case 7:
                            saturn.PC += 3;
                            exchange_register( saturn.A, saturn.R3, W_FIELD );
                            break;
                        case 4: /* AR4EX */
                            saturn.PC += 3;
                            exchange_register( saturn.A, saturn.R4, W_FIELD );
                            break;
                        case 8: /* CR0EX */
                            saturn.PC += 3;
                            exchange_register( saturn.C, saturn.R0, W_FIELD );
                            break;
                        case 9: /* CR1EX */
                        case 0xd:
                            saturn.PC += 3;
                            exchange_register( saturn.C, saturn.R1, W_FIELD );
                            break;
                        case 0xa: /* CR2EX */
                        case 0xe:
                            saturn.PC += 3;
                            exchange_register( saturn.C, saturn.R2, W_FIELD );
                            break;
                        case 0xb: /* CR3EX */
                        case 0xf:
                            saturn.PC += 3;
                            exchange_register( saturn.C, saturn.R3, W_FIELD );
                            break;
                        case 0xc: /* CR4EX */
                            saturn.PC += 3;
                            exchange_register( saturn.C, saturn.R4, W_FIELD );
                            break;
                        default:
                            illegal_instruction = true;
                    }
                    break;
                case 3:
                    op3 = read_nibble( saturn.PC + 2 );
                    switch ( op3 ) {
                        case 0: /* D0=A */
                            saturn.PC += 3;
                            register_to_address( saturn.A, &saturn.D0, 0 );
                            break;
                        case 1: /* D1=A */
                            saturn.PC += 3;
                            register_to_address( saturn.A, &saturn.D1, 0 );
                            break;
                        case 2: /* AD0EX */
                            saturn.PC += 3;
                            exchange_reg( saturn.A, &saturn.D0, A_FIELD );
                            break;
                        case 3: /* AD1EX */
                            saturn.PC += 3;
                            exchange_reg( saturn.A, &saturn.D1, A_FIELD );
                            break;
                        case 4: /* D0=C */
                            saturn.PC += 3;
                            register_to_address( saturn.C, &saturn.D0, 0 );
                            break;
                        case 5: /* D1=C */
                            saturn.PC += 3;
                            register_to_address( saturn.C, &saturn.D1, 0 );
                            break;
                        case 6: /* CD0EX */
                            saturn.PC += 3;
                            exchange_reg( saturn.C, &saturn.D0, A_FIELD );
                            break;
                        case 7: /* CD1EX */
                            saturn.PC += 3;
                            exchange_reg( saturn.C, &saturn.D1, A_FIELD );
                            break;
                        case 8: /* D0=AS */
                            saturn.PC += 3;
                            register_to_address( saturn.A, &saturn.D0, 1 );
                            break;
                        case 9: /* saturn.D1=AS */
                            saturn.PC += 3;
                            register_to_address( saturn.A, &saturn.D1, 1 );
                            break;
                        case 0xa: /* AD0XS */
                            saturn.PC += 3;
                            exchange_reg( saturn.A, &saturn.D0, IN_FIELD );
                            break;
                        case 0xb: /* AD1XS */
                            saturn.PC += 3;
                            exchange_reg( saturn.A, &saturn.D1, IN_FIELD );
                            break;
                        case 0xc: /* D0=CS */
                            saturn.PC += 3;
                            register_to_address( saturn.C, &saturn.D0, 1 );
                            break;
                        case 0xd: /* D1=CS */
                            saturn.PC += 3;
                            register_to_address( saturn.C, &saturn.D1, 1 );
                            break;
                        case 0xe: /* CD0XS */
                            saturn.PC += 3;
                            exchange_reg( saturn.C, &saturn.D0, IN_FIELD );
                            break;
                        case 0xf: /* CD1XS */
                            saturn.PC += 3;
                            exchange_reg( saturn.C, &saturn.D1, IN_FIELD );
                            break;
                        default:
                            illegal_instruction = true;
                    }
                    break;
                case 4:
                    op3 = read_nibble( saturn.PC + 2 );
                    {
                        int opX = op3 < 8 ? 0xf : 6;
                        switch ( op3 & 7 ) {
                            case 0: /* DAT0=A */
                                saturn.PC += 3;
                                store( saturn.D0, saturn.A, opX );
                                break;
                            case 1: /* DAT1=A */
                                saturn.PC += 3;
                                store( saturn.D1, saturn.A, opX );
                                break;
                            case 2: /* A=DAT0 */
                                saturn.PC += 3;
                                recall( saturn.A, saturn.D0, opX );
                                break;
                            case 3: /* A=DAT1 */
                                saturn.PC += 3;
                                recall( saturn.A, saturn.D1, opX );
                                break;
                            case 4: /* DAT0=C */
                                saturn.PC += 3;
                                store( saturn.D0, saturn.C, opX );
                                break;
                            case 5: /* DAT1=C */
                                saturn.PC += 3;
                                store( saturn.D1, saturn.C, opX );
                                break;
                            case 6: /* C=DAT0 */
                                saturn.PC += 3;
                                recall( saturn.C, saturn.D0, opX );
                                break;
                            case 7: /* C=DAT1 */
                                saturn.PC += 3;
                                recall( saturn.C, saturn.D1, opX );
                                break;
                            default:
                                illegal_instruction = true;
                        }
                    }
                    break;
                case 5:
                    op3 = read_nibble( saturn.PC + 2 );
                    op4 = read_nibble( saturn.PC + 3 );
                    if ( op3 >= 8 ) {
                        switch ( op3 & 7 ) {
                            case 0: /* DAT0=A */
                                saturn.PC += 4;
                                store_n( saturn.D0, saturn.A, op4 + 1 );
                                break;
                            case 1: /* DAT1=A */
                                saturn.PC += 4;
                                store_n( saturn.D1, saturn.A, op4 + 1 );
                                break;
                            case 2: /* A=DAT0 */
                                saturn.PC += 4;
                                recall_n( saturn.A, saturn.D0, op4 + 1 );
                                break;
                            case 3: /* A=DAT1 */
                                saturn.PC += 4;
                                recall_n( saturn.A, saturn.D1, op4 + 1 );
                                break;
                            case 4: /* DAT0=C */
                                saturn.PC += 4;
                                store_n( saturn.D0, saturn.C, op4 + 1 );
                                break;
                            case 5: /* DAT1=C */
                                saturn.PC += 4;
                                store_n( saturn.D1, saturn.C, op4 + 1 );
                                break;
                            case 6: /* C=DAT0 */
                                saturn.PC += 4;
                                recall_n( saturn.C, saturn.D0, op4 + 1 );
                                break;
                            case 7: /* C=DAT1 */
                                saturn.PC += 4;
                                recall_n( saturn.C, saturn.D1, op4 + 1 );
                                break;
                            default:
                                illegal_instruction = true;
                        }
                    } else {
                        switch ( op3 ) {
                            case 0: /* DAT0=A */
                                saturn.PC += 4;
                                store( saturn.D0, saturn.A, op4 );
                                break;
                            case 1: /* DAT1=A */
                                saturn.PC += 4;
                                store( saturn.D1, saturn.A, op4 );
                                break;
                            case 2: /* A=DAT0 */
                                saturn.PC += 4;
                                recall( saturn.A, saturn.D0, op4 );
                                break;
                            case 3: /* A=DAT1 */
                                saturn.PC += 4;
                                recall( saturn.A, saturn.D1, op4 );
                                break;
                            case 4: /* DAT0=C */
                                saturn.PC += 4;
                                store( saturn.D0, saturn.C, op4 );
                                break;
                            case 5: /* DAT1=C */
                                saturn.PC += 4;
                                store( saturn.D1, saturn.C, op4 );
                                break;
                            case 6: /* C=DAT0 */
                                saturn.PC += 4;
                                recall( saturn.C, saturn.D0, op4 );
                                break;
                            case 7: /* C=DAT1 */
                                saturn.PC += 4;
                                recall( saturn.C, saturn.D1, op4 );
                                break;
                            default:
                                illegal_instruction = true;
                        }
                    }
                    break;
                case 6:
                    op3 = read_nibble( saturn.PC + 2 );
                    saturn.PC += 3;
                    add_address( &saturn.D0, op3 + 1 );
                    break;
                case 7:
                    op3 = read_nibble( saturn.PC + 2 );
                    saturn.PC += 3;
                    add_address( &saturn.D1, op3 + 1 );
                    break;
                case 8:
                    op3 = read_nibble( saturn.PC + 2 );
                    saturn.PC += 3;
                    add_address( &saturn.D0, -( op3 + 1 ) );
                    break;
                case 9:
                    load_addr( &saturn.D0, saturn.PC + 2, 2 );
                    saturn.PC += 4;
                    break;
                case 0xa:
                    load_addr( &saturn.D0, saturn.PC + 2, 4 );
                    saturn.PC += 6;
                    break;
                case 0xb:
                    load_addr( &saturn.D0, saturn.PC + 2, 5 );
                    saturn.PC += 7;
                    break;
                case 0xc:
                    op3 = read_nibble( saturn.PC + 2 );
                    saturn.PC += 3;
                    add_address( &saturn.D1, -( op3 + 1 ) );
                    break;
                case 0xd:
                    load_addr( &saturn.D1, saturn.PC + 2, 2 );
                    saturn.PC += 4;
                    break;
                case 0xe:
                    load_addr( &saturn.D1, saturn.PC + 2, 4 );
                    saturn.PC += 6;
                    break;
                case 0xf:
                    load_addr( &saturn.D1, saturn.PC + 2, 5 );
                    saturn.PC += 7;
                    break;
                default:
                    illegal_instruction = true;
            }
            break;
        case 2:
            op2 = read_nibble( saturn.PC + 1 );
            saturn.PC += 2;
            saturn.P = op2;
            break;
        case 3:
            op2 = read_nibble( saturn.PC + 1 );
            load_constant( saturn.C, op2 + 1, saturn.PC + 2 );
            saturn.PC += 3 + op2;
            break;
        case 4:
            op2 = read_nibbles( saturn.PC + 1, 2 );
            if ( op2 == 0x02 ) {
                saturn.PC += 3;
            } else {
                if ( saturn.CARRY != 0 ) {
                    if ( op2 ) {
                        if ( op2 & 0x80 )
                            op2 |= jumpmasks[ 2 ];
                        saturn.PC = ( saturn.PC + op2 + 1 ) & 0xfffff;
                    } else
                        saturn.PC = pop_return_addr();
                } else
                    saturn.PC += 3;
            }
            break;
        case 5:
            if ( saturn.CARRY == 0 ) {
                op2 = read_nibbles( saturn.PC + 1, 2 );
                if ( op2 ) {
                    if ( op2 & 0x80 )
                        op2 |= jumpmasks[ 2 ];
                    saturn.PC = ( saturn.PC + op2 + 1 ) & 0xfffff;
                } else
                    saturn.PC = pop_return_addr();
            } else
                saturn.PC += 3;
            break;
        case 6:
            op2 = read_nibbles( saturn.PC + 1, 3 );
            if ( op2 == 0x003 )
                saturn.PC += 4;
            else {
                if ( op2 == 0x004 ) {
                    op3 = read_nibbles( saturn.PC + 4, 1 );
                    saturn.PC += 5;
                    if ( op3 != 0 ) {
                        enter_debugger |= TRAP_INSTRUCTION;
                        return;
                    }
                } else {
                    if ( op2 & 0x800 )
                        op2 |= jumpmasks[ 3 ];
                    saturn.PC = ( op2 + saturn.PC + 1 ) & 0xfffff;
                }
            }
            break;
        case 7:
            op2 = read_nibbles( saturn.PC + 1, 3 );
            if ( op2 & 0x800 )
                op2 |= jumpmasks[ 3 ];
            push_return_addr( saturn.PC + 4 );
            saturn.PC = ( op2 + saturn.PC + 4 ) & 0xfffff;
            break;
        case 8:
            op1 = read_nibble( saturn.PC + 1 );
            switch ( op1 ) {
                case 0:
                    op3 = read_nibble( saturn.PC + 2 );
                    switch ( op3 ) {
                        case 0: /* OUT=CS */
                            saturn.PC += 3;
                            copy_register( saturn.OUT, saturn.C, OUTS_FIELD );
                            break;
                        case 1: /* OUT=C */
                            saturn.PC += 3;
                            copy_register( saturn.OUT, saturn.C, OUT_FIELD );
                            break;
                        case 2: /* A=IN */
                            saturn.PC += 3;
                            do_in();
                            copy_register( saturn.A, saturn.IN, IN_FIELD );
                            break;
                        case 3: /* C=IN */
                            saturn.PC += 3;
                            do_in();
                            copy_register( saturn.C, saturn.IN, IN_FIELD );
                            break;
                        case 4: /* UNCNFG */
                            saturn.PC += 3;
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
                            break;
                        case 5: /* CONFIG */
                            saturn.PC += 3;
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
                            break;
                        case 6: /* C=ID */
                            saturn.PC += 3;
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
                            }
                            break;
                        case 7: /* SHUTDN */
                            saturn.PC += 3;
                            if ( config.inhibit_shutdown )
                                break;

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

                            {
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
                            break;
                        case 8:
                            op4 = read_nibble( saturn.PC + 3 );
                            switch ( op4 ) {
                                case 0: /* INTON */
                                    saturn.PC += 4;
                                    saturn.kbd_ien = true;
                                    break;
                                case 1: /* RSI... */
                                    op5 = read_nibble( saturn.PC + 4 );
                                    saturn.PC += 5;
                                    {
                                        saturn.kbd_ien = true;
                                        int gen_intr = 0;
                                        for ( int i = 0; i < KEYS_BUFFER_SIZE; i++ ) {
                                            if ( saturn.keybuf[ i ] != 0 ) {
                                                gen_intr = 1;
                                                break;
                                            }
                                        }
                                        if ( gen_intr )
                                            do_kbd_int();
                                    }
                                    break;
                                case 2: /* LA... */
                                    op5 = read_nibble( saturn.PC + 4 );
                                    load_constant( saturn.A, op5 + 1, saturn.PC + 5 );
                                    saturn.PC += 6 + op5;
                                    break;
                                case 3: /* BUSCB */
                                    saturn.PC += 4;
                                    break;
                                case 4: /* ABIT=0 */
                                    op5 = read_nibble( saturn.PC + 4 );
                                    saturn.PC += 5;
                                    clear_register_bit( saturn.A, op5 );
                                    break;
                                case 5: /* ABIT=1 */
                                    op5 = read_nibble( saturn.PC + 4 );
                                    saturn.PC += 5;
                                    set_register_bit( saturn.A, op5 );
                                    break;
                                case 8: /* CBIT=0 */
                                    op5 = read_nibble( saturn.PC + 4 );
                                    saturn.PC += 5;
                                    clear_register_bit( saturn.C, op5 );
                                    break;
                                case 9: /* CBIT=1 */
                                    op5 = read_nibble( saturn.PC + 4 );
                                    saturn.PC += 5;
                                    set_register_bit( saturn.C, op5 );
                                    break;
                                case 6:   /* ?ABIT=0 */
                                case 7:   /* ?ABIT=1 */
                                case 0xa: /* ?CBIT=0 */
                                case 0xb: /* ?CBIT=1 */
                                    op5 = read_nibble( saturn.PC + 4 );
                                    saturn.CARRY = ( get_register_bit( ( op4 < 8 ) ? saturn.A : saturn.C, op5 ) ==
                                                     ( ( op4 == 6 || op4 == 0xa ) ? 0 : 1 ) )
                                                       ? 1
                                                       : 0;
                                    if ( saturn.CARRY ) {
                                        saturn.PC += 5;
                                        int op6 = read_nibbles( saturn.PC, 2 );
                                        if ( op6 ) {
                                            if ( op6 & 0x80 )
                                                op6 |= jumpmasks[ 2 ];
                                            saturn.PC = ( saturn.PC + op6 ) & 0xfffff;
                                        } else
                                            saturn.PC = pop_return_addr();
                                    } else
                                        saturn.PC += 7;
                                    break;
                                case 0xc: /* PC=(A) */
                                    saturn.PC = read_nibbles( dat_to_addr( saturn.A ), 5 );
                                    break;
                                case 0xd: /* BUSCD */
                                    saturn.PC += 4;
                                    break;
                                case 0xe: /* PC=(C) */
                                    saturn.PC = read_nibbles( dat_to_addr( saturn.C ), 5 );
                                    break;
                                case 0xf: /* INTOFF */
                                    saturn.PC += 4;
                                    saturn.kbd_ien = false;
                                    break;
                                default:
                                    illegal_instruction = true;
                            }
                            break;
                        case 9: /* C+P+1 */
                            saturn.PC += 3;
                            add_p_plus_one( saturn.C );
                            break;
                        case 0xa: /* RESET */
                            saturn.PC += 3;
                            for ( int i = 0; i < 6; i++ ) {
                                saturn.mem_cntl[ i ].unconfigured = conf_tab[ i ];

                                saturn.mem_cntl[ i ].config[ 0 ] = 0x0;
                                saturn.mem_cntl[ i ].config[ 1 ] = 0x0;
                            }
                            break;
                        case 0xb: /* BUSCC */
                            saturn.PC += 3;
                            break;
                        case 0xc: /* C=P n */
                            op4 = read_nibble( saturn.PC + 3 );
                            saturn.PC += 4;
                            set_register_nibble( saturn.C, op4, saturn.P );
                            break;
                        case 0xd: /* P=C n */
                            op4 = read_nibble( saturn.PC + 3 );
                            saturn.PC += 4;
                            saturn.P = get_register_nibble( saturn.C, op4 );
                            break;
                        case 0xe: /* SREQ? */
                            saturn.PC += 3;
                            saturn.C[ 0 ] = 0;
                            saturn.SR = 0;
                            break;
                        case 0xf: /* CPEX n */
                            op4 = read_nibble( saturn.PC + 3 );
                            saturn.PC += 4;
                            {
                                int tmp = get_register_nibble( saturn.C, op4 );
                                set_register_nibble( saturn.C, op4, saturn.P );
                                saturn.P = tmp;
                            }
                            break;
                        default:
                            illegal_instruction = true;
                    }
                    break;
                case 1:
                    op2 = read_nibble( saturn.PC + 2 );
                    switch ( op2 ) {
                        case 0: /* ASLC */
                            saturn.PC += 3;
                            shift_left_circ_register( saturn.A, W_FIELD );
                            break;
                        case 1: /* BSLC */
                            saturn.PC += 3;
                            shift_left_circ_register( saturn.B, W_FIELD );
                            break;
                        case 2: /* CSLC */
                            saturn.PC += 3;
                            shift_left_circ_register( saturn.C, W_FIELD );
                            break;
                        case 3: /* DSLC */
                            saturn.PC += 3;
                            shift_left_circ_register( saturn.D, W_FIELD );
                            break;
                        case 4: /* ASRC */
                            saturn.PC += 3;
                            shift_right_circ_register( saturn.A, W_FIELD );
                            break;
                        case 5: /* BSRC */
                            saturn.PC += 3;
                            shift_right_circ_register( saturn.B, W_FIELD );
                            break;
                        case 6: /* CSRC */
                            saturn.PC += 3;
                            shift_right_circ_register( saturn.C, W_FIELD );
                            break;
                        case 7: /* DSRC */
                            saturn.PC += 3;
                            shift_right_circ_register( saturn.D, W_FIELD );
                            break;
                        case 8: /* R = R +/- CON */
                            op3 = read_nibble( saturn.PC + 3 );
                            op4 = read_nibble( saturn.PC + 4 );
                            op5 = read_nibble( saturn.PC + 5 );
                            if ( op4 < 8 ) { /* PLUS */
                                switch ( op4 & 3 ) {
                                    case 0: /* A=A+CON */
                                        saturn.PC += 6;
                                        add_register_constant( saturn.A, op3, op5 + 1 );
                                        break;
                                    case 1: /* B=B+CON */
                                        saturn.PC += 6;
                                        add_register_constant( saturn.B, op3, op5 + 1 );
                                        break;
                                    case 2: /* C=C+CON */
                                        saturn.PC += 6;
                                        add_register_constant( saturn.C, op3, op5 + 1 );
                                        break;
                                    case 3: /* D=D+CON */
                                        saturn.PC += 6;
                                        add_register_constant( saturn.D, op3, op5 + 1 );
                                        break;
                                    default:
                                        illegal_instruction = true;
                                }
                            } else { /* MINUS */
                                switch ( op4 & 3 ) {
                                    case 0: /* A=A-CON */
                                        saturn.PC += 6;
                                        sub_register_constant( saturn.A, op3, op5 + 1 );
                                        break;
                                    case 1: /* B=B-CON */
                                        saturn.PC += 6;
                                        sub_register_constant( saturn.B, op3, op5 + 1 );
                                        break;
                                    case 2: /* C=C-CON */
                                        saturn.PC += 6;
                                        sub_register_constant( saturn.C, op3, op5 + 1 );
                                        break;
                                    case 3: /* D=D-CON */
                                        saturn.PC += 6;
                                        sub_register_constant( saturn.D, op3, op5 + 1 );
                                        break;
                                    default:
                                        illegal_instruction = true;
                                }
                            }
                            break;
                        case 9: /* R SRB FIELD */
                            op3 = read_nibble( saturn.PC + 3 );
                            op4 = read_nibble( saturn.PC + 4 );
                            switch ( op4 & 3 ) {
                                case 0:
                                    saturn.PC += 5;
                                    shift_right_bit_register( saturn.A, op3 );
                                    break;
                                case 1:
                                    saturn.PC += 5;
                                    shift_right_bit_register( saturn.B, op3 );
                                    break;
                                case 2:
                                    saturn.PC += 5;
                                    shift_right_bit_register( saturn.C, op3 );
                                    break;
                                case 3:
                                    saturn.PC += 5;
                                    shift_right_bit_register( saturn.D, op3 );
                                    break;
                                default:
                                    illegal_instruction = true;
                            }
                            break;
                        case 0xa: /* R = R FIELD, etc. */
                            op3 = read_nibble( saturn.PC + 3 );
                            op4 = read_nibble( saturn.PC + 4 );
                            op5 = read_nibble( saturn.PC + 5 );
                            switch ( op4 ) {
                                case 0:
                                    switch ( op5 ) {
                                        case 0: /* saturn.R0=A */
                                            saturn.PC += 6;
                                            copy_register( saturn.R0, saturn.A, op3 );
                                            break;
                                        case 1: /* saturn.R1=A */
                                        case 5:
                                            saturn.PC += 6;
                                            copy_register( saturn.R1, saturn.A, op3 );
                                            break;
                                        case 2: /* saturn.R2=A */
                                        case 6:
                                            saturn.PC += 6;
                                            copy_register( saturn.R2, saturn.A, op3 );
                                            break;
                                        case 3: /* saturn.R3=A */
                                        case 7:
                                            saturn.PC += 6;
                                            copy_register( saturn.R3, saturn.A, op3 );
                                            break;
                                        case 4: /* saturn.R4=A */
                                            saturn.PC += 6;
                                            copy_register( saturn.R4, saturn.A, op3 );
                                            break;
                                        case 8: /* saturn.R0=C */
                                            saturn.PC += 6;
                                            copy_register( saturn.R0, saturn.C, op3 );
                                            break;
                                        case 9: /* saturn.R1=C */
                                        case 0xd:
                                            saturn.PC += 6;
                                            copy_register( saturn.R1, saturn.C, op3 );
                                            break;
                                        case 0xa: /* saturn.R2=C */
                                        case 0xe:
                                            saturn.PC += 6;
                                            copy_register( saturn.R2, saturn.C, op3 );
                                            break;
                                        case 0xb: /* saturn.R3=C */
                                        case 0xf:
                                            saturn.PC += 6;
                                            copy_register( saturn.R3, saturn.C, op3 );
                                            break;
                                        case 0xc: /* saturn.R4=C */
                                            saturn.PC += 6;
                                            copy_register( saturn.R4, saturn.C, op3 );
                                            break;
                                        default:
                                            illegal_instruction = true;
                                    }
                                    break;
                                case 1:
                                    switch ( op5 ) {
                                        case 0: /* A=R0 */
                                            saturn.PC += 6;
                                            copy_register( saturn.A, saturn.R0, op3 );
                                            break;
                                        case 1: /* A=R1 */
                                        case 5:
                                            saturn.PC += 6;
                                            copy_register( saturn.A, saturn.R1, op3 );
                                            break;
                                        case 2: /* A=R2 */
                                        case 6:
                                            saturn.PC += 6;
                                            copy_register( saturn.A, saturn.R2, op3 );
                                            break;
                                        case 3: /* A=R3 */
                                        case 7:
                                            saturn.PC += 6;
                                            copy_register( saturn.A, saturn.R3, op3 );
                                            break;
                                        case 4: /* A=R4 */
                                            saturn.PC += 6;
                                            copy_register( saturn.A, saturn.R4, op3 );
                                            break;
                                        case 8: /* C=R0 */
                                            saturn.PC += 6;
                                            copy_register( saturn.C, saturn.R0, op3 );
                                            break;
                                        case 9: /* C=R1 */
                                        case 0xd:
                                            saturn.PC += 6;
                                            copy_register( saturn.C, saturn.R1, op3 );
                                            break;
                                        case 0xa: /* C=R2 */
                                        case 0xe:
                                            saturn.PC += 6;
                                            copy_register( saturn.C, saturn.R2, op3 );
                                            break;
                                        case 0xb: /* C=R3 */
                                        case 0xf:
                                            saturn.PC += 6;
                                            copy_register( saturn.C, saturn.R3, op3 );
                                            break;
                                        case 0xc: /* C=R4 */
                                            saturn.PC += 6;
                                            copy_register( saturn.C, saturn.R4, op3 );
                                            break;
                                        default:
                                            illegal_instruction = true;
                                    }
                                    break;
                                case 2:
                                    switch ( op5 ) {
                                        case 0: /* AR0EX */
                                            saturn.PC += 6;
                                            exchange_register( saturn.A, saturn.R0, op3 );
                                            break;
                                        case 1: /* AR1EX */
                                        case 5:
                                            saturn.PC += 6;
                                            exchange_register( saturn.A, saturn.R1, op3 );
                                            break;
                                        case 2: /* AR2EX */
                                        case 6:
                                            saturn.PC += 6;
                                            exchange_register( saturn.A, saturn.R2, op3 );
                                            break;
                                        case 3: /* AR3EX */
                                        case 7:
                                            saturn.PC += 6;
                                            exchange_register( saturn.A, saturn.R3, op3 );
                                            break;
                                        case 4: /* AR4EX */
                                            saturn.PC += 6;
                                            exchange_register( saturn.A, saturn.R4, op3 );
                                            break;
                                        case 8: /* CR0EX */
                                            saturn.PC += 6;
                                            exchange_register( saturn.C, saturn.R0, op3 );
                                            break;
                                        case 9: /* CR1EX */
                                        case 0xd:
                                            saturn.PC += 6;
                                            exchange_register( saturn.C, saturn.R1, op3 );
                                            break;
                                        case 0xa: /* CR2EX */
                                        case 0xe:
                                            saturn.PC += 6;
                                            exchange_register( saturn.C, saturn.R2, op3 );
                                            break;
                                        case 0xb: /* CR3EX */
                                        case 0xf:
                                            saturn.PC += 6;
                                            exchange_register( saturn.C, saturn.R3, op3 );
                                            break;
                                        case 0xc: /* CR4EX */
                                            saturn.PC += 6;
                                            exchange_register( saturn.C, saturn.R4, op3 );
                                            break;
                                        default:
                                            illegal_instruction = true;
                                    }
                                    break;
                                default:
                                    illegal_instruction = true;
                            }
                            break;
                        case 0xb:
                            op3 = read_nibble( saturn.PC + 3 );
                            switch ( op3 ) {
                                case 2: /* PC=A */
                                    saturn.PC = dat_to_addr( saturn.A );
                                    break;
                                case 3: /* PC=C */
                                    saturn.PC = dat_to_addr( saturn.C );
                                    break;
                                case 4: /* A=PC */
                                    saturn.PC += 4;
                                    addr_to_dat( saturn.PC, saturn.A );
                                    break;
                                case 5: /* C=PC */
                                    saturn.PC += 4;
                                    addr_to_dat( saturn.PC, saturn.C );
                                    break;
                                case 6: /* APCEX */
                                    saturn.PC += 4;
                                    addr_to_dat( saturn.PC, saturn.A );
                                    saturn.PC = dat_to_addr( saturn.A );
                                    break;
                                case 7: /* CPCEX */
                                    saturn.PC += 4;
                                    addr_to_dat( saturn.PC, saturn.C );
                                    saturn.PC = dat_to_addr( saturn.C );
                                    break;
                                default:
                                    illegal_instruction = true;
                            }
                            break;
                        case 0xc: /* ASRB */
                            saturn.PC += 3;
                            shift_right_bit_register( saturn.A, W_FIELD );
                            break;
                        case 0xd: /* BSRB */
                            saturn.PC += 3;
                            shift_right_bit_register( saturn.B, W_FIELD );
                            break;
                        case 0xe: /* CSRB */
                            saturn.PC += 3;
                            shift_right_bit_register( saturn.C, W_FIELD );
                            break;
                        case 0xf: /* DSRB */
                            saturn.PC += 3;
                            shift_right_bit_register( saturn.D, W_FIELD );
                            break;
                        default:
                            illegal_instruction = true;
                    }
                    break;
                case 2:
                    op2 = read_nibble( saturn.PC + 2 );
                    saturn.PC += 3;
                    if ( op2 & 1 )
                        saturn.XM = 0;
                    if ( op2 & 2 )
                        saturn.SB = 0;
                    if ( op2 & 4 )
                        saturn.SR = 0;
                    if ( op2 & 8 )
                        saturn.MP = 0;
                    break;
                case 3:
                    op2 = read_nibble( saturn.PC + 2 );
                    saturn.CARRY = 1;
                    if ( op2 & 1 )
                        if ( saturn.XM != 0 )
                            saturn.CARRY = 0;
                    if ( op2 & 2 )
                        if ( saturn.SB != 0 )
                            saturn.CARRY = 0;
                    if ( op2 & 4 )
                        if ( saturn.SR != 0 )
                            saturn.CARRY = 0;
                    if ( op2 & 8 )
                        if ( saturn.MP != 0 )
                            saturn.CARRY = 0;

                    if ( saturn.CARRY ) {
                        saturn.PC += 3;
                        op3 = read_nibbles( saturn.PC, 2 );
                        if ( op3 ) {
                            if ( op3 & 0x80 )
                                op3 |= jumpmasks[ 2 ];
                            saturn.PC = ( saturn.PC + op3 ) & 0xfffff;
                        } else
                            saturn.PC = pop_return_addr();
                    } else
                        saturn.PC += 5;
                    break;
                case 4:
                case 5:
                    op2 = read_nibble( saturn.PC + 2 );
                    saturn.PC += 3;
                    saturn.PSTAT[ op2 ] = ( op1 == 4 ) ? 0 : 1;
                    break;
                case 6:
                case 7:
                    op2 = read_nibble( saturn.PC + 2 );
                    if ( op1 == 6 )
                        saturn.CARRY = ( get_program_stat( op2 ) == 0 ) ? 1 : 0;
                    else
                        saturn.CARRY = ( get_program_stat( op2 ) != 0 ) ? 1 : 0;
                    if ( saturn.CARRY ) {
                        saturn.PC += 3;
                        op3 = read_nibbles( saturn.PC, 2 );
                        if ( op3 ) {
                            if ( op3 & 0x80 )
                                op3 |= jumpmasks[ 2 ];
                            saturn.PC = ( saturn.PC + op3 ) & 0xfffff;
                        } else
                            saturn.PC = pop_return_addr();
                    } else
                        saturn.PC += 5;
                    break;
                case 8:
                case 9:
                    op2 = read_nibble( saturn.PC + 2 );
                    if ( op1 == 8 )
                        saturn.CARRY = ( saturn.P != op2 ) ? 1 : 0;
                    else
                        saturn.CARRY = ( saturn.P == op2 ) ? 1 : 0;
                    if ( saturn.CARRY ) {
                        saturn.PC += 3;
                        op3 = read_nibbles( saturn.PC, 2 );
                        if ( op3 ) {
                            if ( op3 & 0x80 )
                                op3 |= jumpmasks[ 2 ];
                            saturn.PC = ( saturn.PC + op3 ) & 0xfffff;
                        } else
                            saturn.PC = pop_return_addr();
                    } else
                        saturn.PC += 5;
                    break;
                case 0xa:
                    op2 = read_nibble( saturn.PC + 2 );
                    switch ( op2 ) {
                        case 0: /* ?A=B */
                            saturn.CARRY = is_equal_register( saturn.A, saturn.B, A_FIELD );
                            break;
                        case 1: /* ?B=C */
                            saturn.CARRY = is_equal_register( saturn.B, saturn.C, A_FIELD );
                            break;
                        case 2: /* ?A=C */
                            saturn.CARRY = is_equal_register( saturn.A, saturn.C, A_FIELD );
                            break;
                        case 3: /* ?C=D */
                            saturn.CARRY = is_equal_register( saturn.C, saturn.D, A_FIELD );
                            break;
                        case 4: /* ?A#B */
                            saturn.CARRY = is_not_equal_register( saturn.A, saturn.B, A_FIELD );
                            break;
                        case 5: /* ?B#C */
                            saturn.CARRY = is_not_equal_register( saturn.B, saturn.C, A_FIELD );
                            break;
                        case 6: /* ?A#C */
                            saturn.CARRY = is_not_equal_register( saturn.A, saturn.C, A_FIELD );
                            break;
                        case 7: /* ?C#D */
                            saturn.CARRY = is_not_equal_register( saturn.C, saturn.D, A_FIELD );
                            break;
                        case 8: /* ?A=0 */
                            saturn.CARRY = is_zero_register( saturn.A, A_FIELD );
                            break;
                        case 9: /* ?B=0 */
                            saturn.CARRY = is_zero_register( saturn.B, A_FIELD );
                            break;
                        case 0xa: /* ?C=0 */
                            saturn.CARRY = is_zero_register( saturn.C, A_FIELD );
                            break;
                        case 0xb: /* ?D=0 */
                            saturn.CARRY = is_zero_register( saturn.D, A_FIELD );
                            break;
                        case 0xc: /* ?A#0 */
                            saturn.CARRY = is_not_zero_register( saturn.A, A_FIELD );
                            break;
                        case 0xd: /* ?B#0 */
                            saturn.CARRY = is_not_zero_register( saturn.B, A_FIELD );
                            break;
                        case 0xe: /* ?C#0 */
                            saturn.CARRY = is_not_zero_register( saturn.C, A_FIELD );
                            break;
                        case 0xf: /* ?D#0 */
                            saturn.CARRY = is_not_zero_register( saturn.D, A_FIELD );
                            break;
                        default:
                            illegal_instruction = true;
                    }
                    if ( saturn.CARRY ) {
                        saturn.PC += 3;
                        op3 = read_nibbles( saturn.PC, 2 );
                        if ( op3 ) {
                            if ( op3 & 0x80 )
                                op3 |= jumpmasks[ 2 ];
                            saturn.PC = ( saturn.PC + op3 ) & 0xfffff;
                        } else
                            saturn.PC = pop_return_addr();
                    } else
                        saturn.PC += 5;
                    break;
                case 0xb:
                    op2 = read_nibble( saturn.PC + 2 );
                    switch ( op2 ) {
                        case 0: /* ?A>B */
                            saturn.CARRY = is_greater_register( saturn.A, saturn.B, A_FIELD );
                            break;
                        case 1: /* ?B>C */
                            saturn.CARRY = is_greater_register( saturn.B, saturn.C, A_FIELD );
                            break;
                        case 2: /* ?C>A */
                            saturn.CARRY = is_greater_register( saturn.C, saturn.A, A_FIELD );
                            break;
                        case 3: /* ?D>C */
                            saturn.CARRY = is_greater_register( saturn.D, saturn.C, A_FIELD );
                            break;
                        case 4: /* ?A<B */
                            saturn.CARRY = is_less_register( saturn.A, saturn.B, A_FIELD );
                            break;
                        case 5: /* ?B<C */
                            saturn.CARRY = is_less_register( saturn.B, saturn.C, A_FIELD );
                            break;
                        case 6: /* ?C<A */
                            saturn.CARRY = is_less_register( saturn.C, saturn.A, A_FIELD );
                            break;
                        case 7: /* ?D<C */
                            saturn.CARRY = is_less_register( saturn.D, saturn.C, A_FIELD );
                            break;
                        case 8: /* ?A>=B */
                            saturn.CARRY = is_greater_or_equal_register( saturn.A, saturn.B, A_FIELD );
                            break;
                        case 9: /* ?B>=C */
                            saturn.CARRY = is_greater_or_equal_register( saturn.B, saturn.C, A_FIELD );
                            break;
                        case 0xa: /* ?C>=A */
                            saturn.CARRY = is_greater_or_equal_register( saturn.C, saturn.A, A_FIELD );
                            break;
                        case 0xb: /* ?D>=C */
                            saturn.CARRY = is_greater_or_equal_register( saturn.D, saturn.C, A_FIELD );
                            break;
                        case 0xc: /* ?A<=B */
                            saturn.CARRY = is_less_or_equal_register( saturn.A, saturn.B, A_FIELD );
                            break;
                        case 0xd: /* ?B<=C */
                            saturn.CARRY = is_less_or_equal_register( saturn.B, saturn.C, A_FIELD );
                            break;
                        case 0xe: /* ?C<=A */
                            saturn.CARRY = is_less_or_equal_register( saturn.C, saturn.A, A_FIELD );
                            break;
                        case 0xf: /* ?D<=C */
                            saturn.CARRY = is_less_or_equal_register( saturn.D, saturn.C, A_FIELD );
                            break;
                        default:
                            illegal_instruction = true;
                    }
                    if ( saturn.CARRY ) {
                        saturn.PC += 3;
                        op3 = read_nibbles( saturn.PC, 2 );
                        if ( op3 ) {
                            if ( op3 & 0x80 )
                                op3 |= jumpmasks[ 2 ];
                            saturn.PC = ( saturn.PC + op3 ) & 0xfffff;
                        } else
                            saturn.PC = pop_return_addr();
                    } else
                        saturn.PC += 5;
                    break;
                case 0xc:
                    op2 = read_nibbles( saturn.PC + 2, 4 );
                    if ( op2 & 0x8000 )
                        op2 |= jumpmasks[ 4 ];
                    saturn.PC = ( saturn.PC + op2 + 2 ) & 0xfffff;
                    break;
                case 0xd:
                    op2 = read_nibbles( saturn.PC + 2, 5 );
                    saturn.PC = op2;
                    break;
                case 0xe:
                    op2 = read_nibbles( saturn.PC + 2, 4 );
                    if ( op2 & 0x8000 )
                        op2 |= jumpmasks[ 4 ];
                    push_return_addr( saturn.PC + 6 );
                    saturn.PC = ( saturn.PC + op2 + 6 ) & 0xfffff;
                    break;
                case 0xf:
                    op2 = read_nibbles( saturn.PC + 2, 5 );
                    push_return_addr( saturn.PC + 7 );
                    saturn.PC = op2;
                    break;
                default:
                    illegal_instruction = true;
            }
            break;
        case 9:
            op1 = read_nibble( saturn.PC + 1 );
            op2 = read_nibble( saturn.PC + 2 );
            if ( op1 < 8 ) {
                switch ( op2 ) {
                    case 0: /* ?A=B */
                        saturn.CARRY = is_equal_register( saturn.A, saturn.B, op1 );
                        break;
                    case 1: /* ?B=C */
                        saturn.CARRY = is_equal_register( saturn.B, saturn.C, op1 );
                        break;
                    case 2: /* ?A=C */
                        saturn.CARRY = is_equal_register( saturn.A, saturn.C, op1 );
                        break;
                    case 3: /* ?C=D */
                        saturn.CARRY = is_equal_register( saturn.C, saturn.D, op1 );
                        break;
                    case 4: /* ?A#B */
                        saturn.CARRY = is_not_equal_register( saturn.A, saturn.B, op1 );
                        break;
                    case 5: /* ?B#C */
                        saturn.CARRY = is_not_equal_register( saturn.B, saturn.C, op1 );
                        break;
                    case 6: /* ?A#C */
                        saturn.CARRY = is_not_equal_register( saturn.A, saturn.C, op1 );
                        break;
                    case 7: /* ?C#D */
                        saturn.CARRY = is_not_equal_register( saturn.C, saturn.D, op1 );
                        break;
                    case 8: /* ?A=0 */
                        saturn.CARRY = is_zero_register( saturn.A, op1 );
                        break;
                    case 9: /* ?B=0 */
                        saturn.CARRY = is_zero_register( saturn.B, op1 );
                        break;
                    case 0xa: /* ?C=0 */
                        saturn.CARRY = is_zero_register( saturn.C, op1 );
                        break;
                    case 0xb: /* ?D=0 */
                        saturn.CARRY = is_zero_register( saturn.D, op1 );
                        break;
                    case 0xc: /* ?A#0 */
                        saturn.CARRY = is_not_zero_register( saturn.A, op1 );
                        break;
                    case 0xd: /* ?B#0 */
                        saturn.CARRY = is_not_zero_register( saturn.B, op1 );
                        break;
                    case 0xe: /* ?C#0 */
                        saturn.CARRY = is_not_zero_register( saturn.C, op1 );
                        break;
                    case 0xf: /* ?D#0 */
                        saturn.CARRY = is_not_zero_register( saturn.D, op1 );
                        break;
                    default:
                        illegal_instruction = true;
                }
            } else {
                op1 &= 7;
                switch ( op2 ) {
                    case 0: /* ?A>B */
                        saturn.CARRY = is_greater_register( saturn.A, saturn.B, op1 );
                        break;
                    case 1: /* ?B>C */
                        saturn.CARRY = is_greater_register( saturn.B, saturn.C, op1 );
                        break;
                    case 2: /* ?C>A */
                        saturn.CARRY = is_greater_register( saturn.C, saturn.A, op1 );
                        break;
                    case 3: /* ?D>C */
                        saturn.CARRY = is_greater_register( saturn.D, saturn.C, op1 );
                        break;
                    case 4: /* ?A<B */
                        saturn.CARRY = is_less_register( saturn.A, saturn.B, op1 );
                        break;
                    case 5: /* ?B<C */
                        saturn.CARRY = is_less_register( saturn.B, saturn.C, op1 );
                        break;
                    case 6: /* ?C<A */
                        saturn.CARRY = is_less_register( saturn.C, saturn.A, op1 );
                        break;
                    case 7: /* ?D<C */
                        saturn.CARRY = is_less_register( saturn.D, saturn.C, op1 );
                        break;
                    case 8: /* ?A>=B */
                        saturn.CARRY = is_greater_or_equal_register( saturn.A, saturn.B, op1 );
                        break;
                    case 9: /* ?B>=C */
                        saturn.CARRY = is_greater_or_equal_register( saturn.B, saturn.C, op1 );
                        break;
                    case 0xa: /* ?C>=A */
                        saturn.CARRY = is_greater_or_equal_register( saturn.C, saturn.A, op1 );
                        break;
                    case 0xb: /* ?D>=C */
                        saturn.CARRY = is_greater_or_equal_register( saturn.D, saturn.C, op1 );
                        break;
                    case 0xc: /* ?A<=B */
                        saturn.CARRY = is_less_or_equal_register( saturn.A, saturn.B, op1 );
                        break;
                    case 0xd: /* ?B<=C */
                        saturn.CARRY = is_less_or_equal_register( saturn.B, saturn.C, op1 );
                        break;
                    case 0xe: /* ?C<=A */
                        saturn.CARRY = is_less_or_equal_register( saturn.C, saturn.A, op1 );
                        break;
                    case 0xf: /* ?D<=C */
                        saturn.CARRY = is_less_or_equal_register( saturn.D, saturn.C, op1 );
                        break;
                    default:
                        illegal_instruction = true;
                }
            }
            if ( saturn.CARRY ) {
                saturn.PC += 3;
                op3 = read_nibbles( saturn.PC, 2 );
                if ( op3 ) {
                    if ( op3 & 0x80 )
                        op3 |= jumpmasks[ 2 ];
                    saturn.PC = ( saturn.PC + op3 ) & 0xfffff;
                } else {
                    saturn.PC = pop_return_addr();
                }
            } else {
                saturn.PC += 5;
            }
            break;
        case 0xa:
            op1 = read_nibble( saturn.PC + 1 );
            op2 = read_nibble( saturn.PC + 2 );
            if ( op1 < 8 ) {
                switch ( op2 ) {
                    case 0: /* A=A+B */
                        saturn.PC += 3;
                        add_register( saturn.A, saturn.A, saturn.B, op1 );
                        break;
                    case 1: /* B=B+C */
                        saturn.PC += 3;
                        add_register( saturn.B, saturn.B, saturn.C, op1 );
                        break;
                    case 2: /* C=C+A */
                        saturn.PC += 3;
                        add_register( saturn.C, saturn.C, saturn.A, op1 );
                        break;
                    case 3: /* D=D+C */
                        saturn.PC += 3;
                        add_register( saturn.D, saturn.D, saturn.C, op1 );
                        break;
                    case 4: /* A=A+A */
                        saturn.PC += 3;
                        add_register( saturn.A, saturn.A, saturn.A, op1 );
                        break;
                    case 5: /* B=B+B */
                        saturn.PC += 3;
                        add_register( saturn.B, saturn.B, saturn.B, op1 );
                        break;
                    case 6: /* C=C+C */
                        saturn.PC += 3;
                        add_register( saturn.C, saturn.C, saturn.C, op1 );
                        break;
                    case 7: /* D=D+D */
                        saturn.PC += 3;
                        add_register( saturn.D, saturn.D, saturn.D, op1 );
                        break;
                    case 8: /* B=B+A */
                        saturn.PC += 3;
                        add_register( saturn.B, saturn.B, saturn.A, op1 );
                        break;
                    case 9: /* C=C+B */
                        saturn.PC += 3;
                        add_register( saturn.C, saturn.C, saturn.B, op1 );
                        break;
                    case 0xa: /* A=A+C */
                        saturn.PC += 3;
                        add_register( saturn.A, saturn.A, saturn.C, op1 );
                        break;
                    case 0xb: /* C=C+D */
                        saturn.PC += 3;
                        add_register( saturn.C, saturn.C, saturn.D, op1 );
                        break;
                    case 0xc: /* A=A-1 */
                        saturn.PC += 3;
                        dec_register( saturn.A, op1 );
                        break;
                    case 0xd: /* B=B-1 */
                        saturn.PC += 3;
                        dec_register( saturn.B, op1 );
                        break;
                    case 0xe: /* C=C-1 */
                        saturn.PC += 3;
                        dec_register( saturn.C, op1 );
                        break;
                    case 0xf: /* D=D-1 */
                        saturn.PC += 3;
                        dec_register( saturn.D, op1 );
                        break;
                    default:
                        illegal_instruction = true;
                }
            } else {
                op1 &= 7;
                switch ( op2 ) {
                    case 0: /* A=0 */
                        saturn.PC += 3;
                        zero_register( saturn.A, op1 );
                        break;
                    case 1: /* B=0 */
                        saturn.PC += 3;
                        zero_register( saturn.B, op1 );
                        break;
                    case 2: /* C=0 */
                        saturn.PC += 3;
                        zero_register( saturn.C, op1 );
                        break;
                    case 3: /* D=0 */
                        saturn.PC += 3;
                        zero_register( saturn.D, op1 );
                        break;
                    case 4: /* A=B */
                        saturn.PC += 3;
                        copy_register( saturn.A, saturn.B, op1 );
                        break;
                    case 5: /* B=C */
                        saturn.PC += 3;
                        copy_register( saturn.B, saturn.C, op1 );
                        break;
                    case 6: /* C=A */
                        saturn.PC += 3;
                        copy_register( saturn.C, saturn.A, op1 );
                        break;
                    case 7: /* D=C */
                        saturn.PC += 3;
                        copy_register( saturn.D, saturn.C, op1 );
                        break;
                    case 8: /* B=A */
                        saturn.PC += 3;
                        copy_register( saturn.B, saturn.A, op1 );
                        break;
                    case 9: /* C=B */
                        saturn.PC += 3;
                        copy_register( saturn.C, saturn.B, op1 );
                        break;
                    case 0xa: /* A=C */
                        saturn.PC += 3;
                        copy_register( saturn.A, saturn.C, op1 );
                        break;
                    case 0xb: /* C=D */
                        saturn.PC += 3;
                        copy_register( saturn.C, saturn.D, op1 );
                        break;
                    case 0xc: /* ABEX */
                        saturn.PC += 3;
                        exchange_register( saturn.A, saturn.B, op1 );
                        break;
                    case 0xd: /* BCEX */
                        saturn.PC += 3;
                        exchange_register( saturn.B, saturn.C, op1 );
                        break;
                    case 0xe: /* ACEX */
                        saturn.PC += 3;
                        exchange_register( saturn.A, saturn.C, op1 );
                        break;
                    case 0xf: /* CDEX */
                        saturn.PC += 3;
                        exchange_register( saturn.C, saturn.D, op1 );
                        break;
                    default:
                        illegal_instruction = true;
                }
            }
            break;
        case 0xb:
            op1 = read_nibble( saturn.PC + 1 );
            op2 = read_nibble( saturn.PC + 2 );
            if ( op1 < 8 ) {
                switch ( op2 ) {
                    case 0: /* A=A-B */
                        saturn.PC += 3;
                        sub_register( saturn.A, saturn.A, saturn.B, op1 );
                        break;
                    case 1: /* B=B-C */
                        saturn.PC += 3;
                        sub_register( saturn.B, saturn.B, saturn.C, op1 );
                        break;
                    case 2: /* C=C-A */
                        saturn.PC += 3;
                        sub_register( saturn.C, saturn.C, saturn.A, op1 );
                        break;
                    case 3: /* D=D-C */
                        saturn.PC += 3;
                        sub_register( saturn.D, saturn.D, saturn.C, op1 );
                        break;
                    case 4: /* A=A+1 */
                        saturn.PC += 3;
                        inc_register( saturn.A, op1 );
                        break;
                    case 5: /* B=B+1 */
                        saturn.PC += 3;
                        inc_register( saturn.B, op1 );
                        break;
                    case 6: /* C=C+1 */
                        saturn.PC += 3;
                        inc_register( saturn.C, op1 );
                        break;
                    case 7: /* D=D+1 */
                        saturn.PC += 3;
                        inc_register( saturn.D, op1 );
                        break;
                    case 8: /* B=B-A */
                        saturn.PC += 3;
                        sub_register( saturn.B, saturn.B, saturn.A, op1 );
                        break;
                    case 9: /* C=C-B */
                        saturn.PC += 3;
                        sub_register( saturn.C, saturn.C, saturn.B, op1 );
                        break;
                    case 0xa: /* A=A-C */
                        saturn.PC += 3;
                        sub_register( saturn.A, saturn.A, saturn.C, op1 );
                        break;
                    case 0xb: /* C=C-D */
                        saturn.PC += 3;
                        sub_register( saturn.C, saturn.C, saturn.D, op1 );
                        break;
                    case 0xc: /* A=B-A */
                        saturn.PC += 3;
                        sub_register( saturn.A, saturn.B, saturn.A, op1 );
                        break;
                    case 0xd: /* B=C-B */
                        saturn.PC += 3;
                        sub_register( saturn.B, saturn.C, saturn.B, op1 );
                        break;
                    case 0xe: /* C=A-C */
                        saturn.PC += 3;
                        sub_register( saturn.C, saturn.A, saturn.C, op1 );
                        break;
                    case 0xf: /* D=C-D */
                        saturn.PC += 3;
                        sub_register( saturn.D, saturn.C, saturn.D, op1 );
                        break;
                    default:
                        illegal_instruction = true;
                }
            } else {
                op1 &= 7;
                switch ( op2 ) {
                    case 0: /* ASL */
                        saturn.PC += 3;
                        shift_left_register( saturn.A, op1 );
                        break;
                    case 1: /* BSL */
                        saturn.PC += 3;
                        shift_left_register( saturn.B, op1 );
                        break;
                    case 2: /* CSL */
                        saturn.PC += 3;
                        shift_left_register( saturn.C, op1 );
                        break;
                    case 3: /* DSL */
                        saturn.PC += 3;
                        shift_left_register( saturn.D, op1 );
                        break;
                    case 4: /* ASR */
                        saturn.PC += 3;
                        shift_right_register( saturn.A, op1 );
                        break;
                    case 5: /* BSR */
                        saturn.PC += 3;
                        shift_right_register( saturn.B, op1 );
                        break;
                    case 6: /* CSR */
                        saturn.PC += 3;
                        shift_right_register( saturn.C, op1 );
                        break;
                    case 7: /* DSR */
                        saturn.PC += 3;
                        shift_right_register( saturn.D, op1 );
                        break;
                    case 8: /* A=-A */
                        saturn.PC += 3;
                        complement_2_register( saturn.A, op1 );
                        break;
                    case 9: /* B=-B */
                        saturn.PC += 3;
                        complement_2_register( saturn.B, op1 );
                        break;
                    case 0xa: /* C=-C */
                        saturn.PC += 3;
                        complement_2_register( saturn.C, op1 );
                        break;
                    case 0xb: /* D=-D */
                        saturn.PC += 3;
                        complement_2_register( saturn.D, op1 );
                        break;
                    case 0xc: /* A=-A-1 */
                        saturn.PC += 3;
                        complement_1_register( saturn.A, op1 );
                        break;
                    case 0xd: /* B=-B-1 */
                        saturn.PC += 3;
                        complement_1_register( saturn.B, op1 );
                        break;
                    case 0xe: /* C=-C-1 */
                        saturn.PC += 3;
                        complement_1_register( saturn.C, op1 );
                        break;
                    case 0xf: /* D=-D-1 */
                        saturn.PC += 3;
                        complement_1_register( saturn.D, op1 );
                        break;
                    default:
                        illegal_instruction = true;
                }
            }
            break;
        case 0xc:
            op1 = read_nibble( saturn.PC + 1 );
            switch ( op1 ) {
                case 0: /* A=A+B */
                    saturn.PC += 2;
                    add_register( saturn.A, saturn.A, saturn.B, A_FIELD );
                    break;
                case 1: /* B=B+C */
                    saturn.PC += 2;
                    add_register( saturn.B, saturn.B, saturn.C, A_FIELD );
                    break;
                case 2: /* C=C+A */
                    saturn.PC += 2;
                    add_register( saturn.C, saturn.C, saturn.A, A_FIELD );
                    break;
                case 3: /* D=D+C */
                    saturn.PC += 2;
                    add_register( saturn.D, saturn.D, saturn.C, A_FIELD );
                    break;
                case 4: /* A=A+A */
                    saturn.PC += 2;
                    add_register( saturn.A, saturn.A, saturn.A, A_FIELD );
                    break;
                case 5: /* B=B+B */
                    saturn.PC += 2;
                    add_register( saturn.B, saturn.B, saturn.B, A_FIELD );
                    break;
                case 6: /* C=C+C */
                    saturn.PC += 2;
                    add_register( saturn.C, saturn.C, saturn.C, A_FIELD );
                    break;
                case 7: /* D=D+D */
                    saturn.PC += 2;
                    add_register( saturn.D, saturn.D, saturn.D, A_FIELD );
                    break;
                case 8: /* B=B+A */
                    saturn.PC += 2;
                    add_register( saturn.B, saturn.B, saturn.A, A_FIELD );
                    break;
                case 9: /* C=C+B */
                    saturn.PC += 2;
                    add_register( saturn.C, saturn.C, saturn.B, A_FIELD );
                    break;
                case 0xa: /* A=A+C */
                    saturn.PC += 2;
                    add_register( saturn.A, saturn.A, saturn.C, A_FIELD );
                    break;
                case 0xb: /* C=C+D */
                    saturn.PC += 2;
                    add_register( saturn.C, saturn.C, saturn.D, A_FIELD );
                    break;
                case 0xc: /* A=A-1 */
                    saturn.PC += 2;
                    dec_register( saturn.A, A_FIELD );
                    break;
                case 0xd: /* B=B-1 */
                    saturn.PC += 2;
                    dec_register( saturn.B, A_FIELD );
                    break;
                case 0xe: /* C=C-1 */
                    saturn.PC += 2;
                    dec_register( saturn.C, A_FIELD );
                    break;
                case 0xf: /* D=D-1 */
                    saturn.PC += 2;
                    dec_register( saturn.D, A_FIELD );
                    break;
                default:
                    illegal_instruction = true;
            }
            break;
        case 0xd:
            op1 = read_nibble( saturn.PC + 1 );
            switch ( op1 ) {
                case 0: /* A=0 */
                    saturn.PC += 2;
                    zero_register( saturn.A, A_FIELD );
                    break;
                case 1: /* B=0 */
                    saturn.PC += 2;
                    zero_register( saturn.B, A_FIELD );
                    break;
                case 2: /* C=0 */
                    saturn.PC += 2;
                    zero_register( saturn.C, A_FIELD );
                    break;
                case 3: /* D=0 */
                    saturn.PC += 2;
                    zero_register( saturn.D, A_FIELD );
                    break;
                case 4: /* A=B */
                    saturn.PC += 2;
                    copy_register( saturn.A, saturn.B, A_FIELD );
                    break;
                case 5: /* B=C */
                    saturn.PC += 2;
                    copy_register( saturn.B, saturn.C, A_FIELD );
                    break;
                case 6: /* C=A */
                    saturn.PC += 2;
                    copy_register( saturn.C, saturn.A, A_FIELD );
                    break;
                case 7: /* D=C */
                    saturn.PC += 2;
                    copy_register( saturn.D, saturn.C, A_FIELD );
                    break;
                case 8: /* B=A */
                    saturn.PC += 2;
                    copy_register( saturn.B, saturn.A, A_FIELD );
                    break;
                case 9: /* C=B */
                    saturn.PC += 2;
                    copy_register( saturn.C, saturn.B, A_FIELD );
                    break;
                case 0xa: /* A=C */
                    saturn.PC += 2;
                    copy_register( saturn.A, saturn.C, A_FIELD );
                    break;
                case 0xb: /* C=D */
                    saturn.PC += 2;
                    copy_register( saturn.C, saturn.D, A_FIELD );
                    break;
                case 0xc: /* ABEX */
                    saturn.PC += 2;
                    exchange_register( saturn.A, saturn.B, A_FIELD );
                    break;
                case 0xd: /* BCEX */
                    saturn.PC += 2;
                    exchange_register( saturn.B, saturn.C, A_FIELD );
                    break;
                case 0xe: /* ACEX */
                    saturn.PC += 2;
                    exchange_register( saturn.A, saturn.C, A_FIELD );
                    break;
                case 0xf: /* CDEX */
                    saturn.PC += 2;
                    exchange_register( saturn.C, saturn.D, A_FIELD );
                    break;
                default:
                    illegal_instruction = true;
            }
            break;
        case 0xe:
            op1 = read_nibble( saturn.PC + 1 );
            switch ( op1 ) {
                case 0: /* A=A-B */
                    saturn.PC += 2;
                    sub_register( saturn.A, saturn.A, saturn.B, A_FIELD );
                    break;
                case 1: /* B=B-C */
                    saturn.PC += 2;
                    sub_register( saturn.B, saturn.B, saturn.C, A_FIELD );
                    break;
                case 2: /* C=C-A */
                    saturn.PC += 2;
                    sub_register( saturn.C, saturn.C, saturn.A, A_FIELD );
                    break;
                case 3: /* D=D-C */
                    saturn.PC += 2;
                    sub_register( saturn.D, saturn.D, saturn.C, A_FIELD );
                    break;
                case 4: /* A=A+1 */
                    saturn.PC += 2;
                    inc_register( saturn.A, A_FIELD );
                    break;
                case 5: /* B=B+1 */
                    saturn.PC += 2;
                    inc_register( saturn.B, A_FIELD );
                    break;
                case 6: /* C=C+1 */
                    saturn.PC += 2;
                    inc_register( saturn.C, A_FIELD );
                    break;
                case 7: /* D=D+1 */
                    saturn.PC += 2;
                    inc_register( saturn.D, A_FIELD );
                    break;
                case 8: /* B=B-A */
                    saturn.PC += 2;
                    sub_register( saturn.B, saturn.B, saturn.A, A_FIELD );
                    break;
                case 9: /* C=C-B */
                    saturn.PC += 2;
                    sub_register( saturn.C, saturn.C, saturn.B, A_FIELD );
                    break;
                case 0xa: /* A=A-C */
                    saturn.PC += 2;
                    sub_register( saturn.A, saturn.A, saturn.C, A_FIELD );
                    break;
                case 0xb: /* C=C-D */
                    saturn.PC += 2;
                    sub_register( saturn.C, saturn.C, saturn.D, A_FIELD );
                    break;
                case 0xc: /* A=B-A */
                    saturn.PC += 2;
                    sub_register( saturn.A, saturn.B, saturn.A, A_FIELD );
                    break;
                case 0xd: /* B=C-B */
                    saturn.PC += 2;
                    sub_register( saturn.B, saturn.C, saturn.B, A_FIELD );
                    break;
                case 0xe: /* C=A-C */
                    saturn.PC += 2;
                    sub_register( saturn.C, saturn.A, saturn.C, A_FIELD );
                    break;
                case 0xf: /* D=C-D */
                    saturn.PC += 2;
                    sub_register( saturn.D, saturn.C, saturn.D, A_FIELD );
                    break;
                default:
                    illegal_instruction = true;
            }
            break;
        case 0xf:
            op1 = read_nibble( saturn.PC + 1 );
            switch ( op1 ) {
                case 0: /* ASL */
                    saturn.PC += 2;
                    shift_left_register( saturn.A, A_FIELD );
                    break;
                case 1: /* BSL */
                    saturn.PC += 2;
                    shift_left_register( saturn.B, A_FIELD );
                    break;
                case 2: /* CSL */
                    saturn.PC += 2;
                    shift_left_register( saturn.C, A_FIELD );
                    break;
                case 3: /* DSL */
                    saturn.PC += 2;
                    shift_left_register( saturn.D, A_FIELD );
                    break;
                case 4: /* ASR */
                    saturn.PC += 2;
                    shift_right_register( saturn.A, A_FIELD );
                    break;
                case 5: /* BSR */
                    saturn.PC += 2;
                    shift_right_register( saturn.B, A_FIELD );
                    break;
                case 6: /* CSR */
                    saturn.PC += 2;
                    shift_right_register( saturn.C, A_FIELD );
                    break;
                case 7: /* DSR */
                    saturn.PC += 2;
                    shift_right_register( saturn.D, A_FIELD );
                    break;
                case 8: /* A=-A */
                    saturn.PC += 2;
                    complement_2_register( saturn.A, A_FIELD );
                    break;
                case 9: /* B=-B */
                    saturn.PC += 2;
                    complement_2_register( saturn.B, A_FIELD );
                    break;
                case 0xa: /* C=-C */
                    saturn.PC += 2;
                    complement_2_register( saturn.C, A_FIELD );
                    break;
                case 0xb: /* D=-D */
                    saturn.PC += 2;
                    complement_2_register( saturn.D, A_FIELD );
                    break;
                case 0xc: /* A=-A-1 */
                    saturn.PC += 2;
                    complement_1_register( saturn.A, A_FIELD );
                    break;
                case 0xd: /* B=-B-1 */
                    saturn.PC += 2;
                    complement_1_register( saturn.B, A_FIELD );
                    break;
                case 0xe: /* C=-C-1 */
                    saturn.PC += 2;
                    complement_1_register( saturn.C, A_FIELD );
                    break;
                case 0xf: /* D=-D-1 */
                    saturn.PC += 2;
                    complement_1_register( saturn.D, A_FIELD );
                    break;
                default:
                    illegal_instruction = true;
            }
            break;
        default:
            illegal_instruction = true;
    }
    instructions++;

    if ( illegal_instruction )
        enter_debugger |= ILLEGAL_INSTRUCTION;
}

void schedule( void )
{
    t1_t2_ticks ticks;
    unsigned long steps;
    static unsigned long old_stat_instr;
    static unsigned long old_sched_instr;

    steps = instructions - old_sched_instr;
    old_sched_instr = instructions;

    if ( ( sched_timer2 -= steps ) <= 0 ) {
        if ( !saturn.interruptable )
            sched_timer2 = SCHED_TIMER2;
        else
            sched_timer2 = saturn.t2_tick;

        saturn.t2_instr += steps;
        if ( saturn.t2_ctrl & 0x01 )
            saturn.timer2--;

        if ( saturn.timer2 == 0 && ( saturn.t2_ctrl & 0x02 ) ) {
            saturn.t2_ctrl |= 0x08;
            do_interupt();
        }
    }
    schedule_event = sched_timer2;

    if ( device_check ) {
        device_check = false;
        if ( ( sched_display -= steps ) <= 0 ) {
            if ( device.display_touched )
                device.display_touched -= steps;
            if ( device.display_touched < 0 )
                device.display_touched = 1;
        }

        /* check_device() */
        /* UI */
        // TODO: move this out into ui_*.c
        if ( device.display_touched > 0 && device.display_touched-- == 1 ) {
            device.display_touched = 0;
            ui_update_LCD();
        }
        if ( device.display_touched > 0 )
            device_check = true;
        if ( device.contrast_touched ) {
            device.contrast_touched = false;
            ui_adjust_contrast();
        }
        if ( device.ann_touched ) {
            device.ann_touched = false;
            ui_draw_annunc();
        }

        /* serial */
        if ( device.baud_touched ) {
            device.baud_touched = false;
            serial_baud( saturn.baud );
        }

        if ( device.ioc_touched ) {
            device.ioc_touched = false;
            if ( ( saturn.io_ctrl & 0x02 ) && ( saturn.rcs & 0x01 ) )
                do_interupt();
        }

        if ( device.rbr_touched ) {
            device.rbr_touched = false;
            receive_char();
        }

        if ( device.tbr_touched ) {
            device.tbr_touched = false;
            transmit_char();
        }

        if ( device.t1_touched ) {
            saturn.t1_instr = 0;
            sched_timer1 = saturn.t1_tick;
            restart_timer( T1_TIMER );
            set_t1 = saturn.timer1;
            device.t1_touched = false;
        }

        if ( device.t2_touched ) {
            saturn.t2_instr = 0;
            sched_timer2 = saturn.t2_tick;
            device.t2_touched = false;
        }
        /* end check_device() */

        sched_display = SCHED_NEVER;
        if ( device.display_touched ) {
            if ( device.display_touched < sched_display )
                sched_display = device.display_touched - 1;
            if ( sched_display < schedule_event )
                schedule_event = sched_display;
        }
    }

    if ( ( sched_receive -= steps ) <= 0 ) {
        sched_receive = SCHED_RECEIVE;
        if ( ( saturn.rcs & 0x01 ) == 0 )
            receive_char();
    }
    if ( sched_receive < schedule_event )
        schedule_event = sched_receive;

    if ( ( sched_adjtime -= steps ) <= 0 ) {
        sched_adjtime = SCHED_ADJTIME;

        if ( saturn.PC < SrvcIoStart || saturn.PC > SrvcIoEnd ) {
            ticks = get_t1_t2();
            if ( saturn.t2_ctrl & 0x01 )
                saturn.timer2 = ticks.t2_ticks;

            if ( ( saturn.t2_ctrl & 0x08 ) == 0 && saturn.timer2 <= 0 ) {
                if ( saturn.t2_ctrl & 0x02 ) {
                    saturn.t2_ctrl |= 0x08;
                    do_interupt();
                }
            }

            adj_time_pending = false;

            saturn.timer1 = set_t1 - ticks.t1_ticks;
            if ( ( saturn.t1_ctrl & 0x08 ) == 0 && saturn.timer1 <= 0 ) {
                if ( saturn.t1_ctrl & 0x02 ) {
                    saturn.t1_ctrl |= 0x08;
                    do_interupt();
                }
            }

            saturn.timer1 &= 0x0f;
        } else
            adj_time_pending = true;
    }
    if ( sched_adjtime < schedule_event )
        schedule_event = sched_adjtime;

    if ( ( sched_timer1 -= steps ) <= 0 ) {
        if ( !saturn.interruptable )
            sched_timer1 = SCHED_TIMER1;
        else
            sched_timer1 = saturn.t1_tick;

        saturn.t1_instr += steps;
        saturn.timer1 = ( saturn.timer1 - 1 ) & 0xf;
        if ( saturn.timer1 == 0 && ( saturn.t1_ctrl & 0x02 ) ) {
            saturn.t1_ctrl |= 0x08;
            do_interupt();
        }
    }
    if ( sched_timer1 < schedule_event )
        schedule_event = sched_timer1;

    if ( ( sched_statistics -= steps ) <= 0 ) {
        sched_statistics = SCHED_STATISTICS;
        /* run = get_timer( RUN_TIMER ); */
        delta_t_1 = s_1 - old_s_1;
        delta_t_16 = s_16 - old_s_16;
        old_s_1 = s_1;
        old_s_16 = s_16;
        delta_i = instructions - old_stat_instr;
        old_stat_instr = instructions;
        if ( delta_t_1 > 0 ) {
            t1_i_per_tick = ( ( NB_SAMPLES - 1 ) * t1_i_per_tick + ( delta_i / delta_t_16 ) ) / NB_SAMPLES;
            t2_i_per_tick = t1_i_per_tick / 512;
            saturn.i_per_s = ( ( NB_SAMPLES - 1 ) * saturn.i_per_s + ( delta_i / delta_t_1 ) ) / NB_SAMPLES;
        } else {
            t1_i_per_tick = 8192;
            t2_i_per_tick = 16;
        }
        saturn.t1_tick = t1_i_per_tick;
        saturn.t2_tick = t2_i_per_tick;
    }
    if ( sched_statistics < schedule_event )
        schedule_event = sched_statistics;

    if ( ( sched_instr_rollover -= steps ) <= 0 ) {
        sched_instr_rollover = SCHED_INSTR_ROLLOVER;
        instructions = 1;
        old_sched_instr = 1;
        reset_timer( RUN_TIMER );
        reset_timer( IDLE_TIMER );
        start_timer( RUN_TIMER );
    }
    if ( sched_instr_rollover < schedule_event )
        schedule_event = sched_instr_rollover;

    schedule_event--;

    if ( sigalarm_triggered ) {
        sigalarm_triggered = false;

        ui_refresh_LCD();

        ui_get_event();
    }
}
