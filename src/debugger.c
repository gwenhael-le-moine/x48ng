#include "config.h"

#include <stdio.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>

#include "debugger.h"
#include "device.h"
#include "disasm.h"
#include "hp48.h"
#include "resources.h"
#include "romio.h"
#include "rpl.h"
#include "timer.h"
#include "x48_gui.h"

#define MAX_ARGS 16

int enter_debugger = 0;
int in_debugger = 0;
int exec_flags = 0;

static int continue_flag;
static char instr[ 100 ];

/*
 * Pointers in the HP48sx ROM
 */
#define DSKTOP_SX 0x70579
#define DSKBOT_SX 0x7057e
/*
 * Pointers in the HP48gx ROM
 */
#define DSKTOP_GX 0x806f8
#define DSKBOT_GX 0x806fd

/*
 * Breakpoint related stuff
 */
#define BP_EXEC 1
#define BP_READ 2
#define BP_WRITE 4
#define BP_RANGE 8

#define MAX_BREAKPOINTS 32
int num_bkpts;

struct breakpoint {
    word_20 addr;
    word_20 end_addr;
    int flags;
}

bkpt_tbl[ MAX_BREAKPOINTS + 1 ];

/*
 * command functions
 */
static void do_break( int, char** );
static void do_continue( int, char** );
static void do_delete( int, char** );
static void do_exit( int, char** );
static void do_go( int, char** );
static void do_help( int, char** );
static void do_load( int, char** );
static void do_mode( int, char** );
static void do_quit( int, char** );
static void do_regs( int, char** );
static void do_save( int, char** );
static void do_stack( int, char** );
static void do_stat( int, char** );
static void do_step( int, char** );
static void do_ram( int, char** );
static void do_reset( int, char** );
static void do_rstk( int, char** );

struct cmd {
    const char* name;
    void ( *func )( int, char** );
    const char* help;
}

cmd_tbl[] = {
    { "break", do_break,
      "break [address]            Set breakpoint at `address\' or show "
      "breakpoints" },
    { "b", do_break, 0 },

    { "cont", do_continue, "cont                       Continue execution" },
    { "c", do_continue, 0 },

    { "delete", do_delete,
      "delete [all | n]           Delete breakpoint or watchpoint number "
      "`n\',\n                           all breakpoints, or current "
      "breakpoint" },
    { "d", do_delete, 0 },

    { "exit", do_exit,
      "exit                       Exit the emulator without saving" },

    { "go", do_go, "go address                 Set PC to `address\'" },

    { "help", do_help, "help                       Display this information" },
    { "h", do_help, 0 },
    { "?", do_help, 0 },

    { "load", do_load,
      "load                       Load emulator-state from files" },

    { "mode", do_mode,
      "mode [hp | class]          Show or set disassembler mode" },

    { "quit", do_quit,
      "quit                       Exit the emulator after saving its state" },
    { "q", do_quit, 0 },

    { "ram", do_ram, "ram                        Show RAM layout" },

    { "reg", do_regs,
      "reg [register [hexvalue]]  Display or set register value" },
    { "r", do_regs, 0 },

    { "reset", do_reset,
      "reset                      Set the HP48\'s PC to ZERO" },

    { "save", do_save,
      "save                       Save emulator-state to files" },

    { "stack", do_stack, "stack                      Display RPL stack" },

    { "stat", do_stat,
      "stat                       Display statistics for the emulator" },

    { "step", do_step,
      "step [n]                   Step one or n Instruction(s)" },
    { "s", do_step, 0 },

    { "where", do_rstk, "where                      Show ML return stack" },

    { 0, 0, 0 } };

void init_debugger( void ) {
    int i;

    num_bkpts = 0;
    for ( i = 0; i < MAX_BREAKPOINTS; i++ )
        bkpt_tbl[ i ].flags = 0;
    exec_flags = 0;
}

int check_breakpoint( int type, word_20 addr ) {
    struct breakpoint* bp;
    int i, n;

    bp = bkpt_tbl;
    n = num_bkpts;
    i = 0;
    for ( ; n > 0; bp++ ) {
        i++;
        if ( bp->flags == 0 )
            continue;
        n--;
        if ( bp->flags & BP_RANGE && addr >= bp->addr &&
             addr <= bp->end_addr ) {
            goto hit_it;
        }
        if ( bp->flags & type && addr == bp->addr ) {
        hit_it:
            if ( type == BP_READ ) {
                printf( "%.5lX: Read watchpoint %d hit at %.5lX\n", saturn.PC,
                        i, addr );
            } else if ( type == BP_WRITE ) {
                printf( "%.5lX: Write watchpoint %d hit at %.5lX\n", saturn.PC,
                        i, addr );
            } else {
                printf( "Breakpoint %d hit at %.5lX\n", i, addr );
            }
            return 1;
        }
    }
    return 0;
}

char* read_str( char* str, int n, int fp ) {
    int cc;
    int flags;

    while ( 1 ) {
        cc = read( fp, str, n );
        if ( cc > 0 ) {
            str[ cc ] = '\0';
            return str;
        }
        if ( cc == 0 )
            return NULL;

        if ( errno == EINTR )
            continue;

        if ( errno == EAGAIN ) {
            flags = fcntl( fp, F_GETFL, 0 );
            flags &= ~O_NONBLOCK;
            fcntl( fp, F_SETFL, flags );
            continue;
        }
        return NULL;
    }
    /* not reached */
}

static inline void str_to_upper( char* arg ) {
    int i;

    for ( i = 0; i < strlen( arg ); i++ ) {
        if ( 'a' <= arg[ i ] && arg[ i ] <= 'z' ) {
            arg[ i ] = ( char )( ( int )arg[ i ] - ( int )'a' + ( int )'A' );
        }
    }
}

static int decode_dec( int* num, char* arg ) {
    int i;

    if ( arg == ( char* )0 ) {
        printf( "Command requires an argument.\n" );
        return 0;
    }

    *num = 0;
    for ( i = 0; i < strlen( arg ); i++ ) {
        *num *= 10;
        if ( '0' <= arg[ i ] && arg[ i ] <= '9' ) {
            *num += ( ( int )arg[ i ] - ( int )'0' );
        } else {
            *num = 0;
            printf( "Not a number: %s.\n", arg );
            return 0;
        }
    }
    return 1;
}

static int decode_20( word_20* addr, char* arg ) {
    int i;

    if ( arg == ( char* )0 ) {
        printf( "Command requires an argument.\n" );
        return 0;
    }

    *addr = 0;
    for ( i = 0; i < strlen( arg ); i++ ) {
        *addr <<= 4;
        if ( '0' <= arg[ i ] && arg[ i ] <= '9' ) {
            *addr |= ( ( int )arg[ i ] - ( int )'0' );
        } else if ( 'A' <= arg[ i ] && arg[ i ] <= 'F' ) {
            *addr |= ( ( int )arg[ i ] - ( int )'A' + 10 );
        } else {
            *addr = 0;
            printf( "Not a number: %s.\n", arg );
            return 0;
        }
        *addr &= 0xfffff;
    }
    return 1;
}

static int decode_32( word_32* addr, char* arg ) {
    int i;

    if ( arg == ( char* )0 ) {
        printf( "Command requires an argument.\n" );
        return 0;
    }

    *addr = 0;
    for ( i = 0; i < strlen( arg ); i++ ) {
        *addr <<= 4;
        if ( '0' <= arg[ i ] && arg[ i ] <= '9' ) {
            *addr |= ( ( int )arg[ i ] - ( int )'0' );
        } else if ( 'A' <= arg[ i ] && arg[ i ] <= 'F' ) {
            *addr |= ( ( int )arg[ i ] - ( int )'A' + 10 );
        } else {
            *addr = 0;
            printf( "Not a number: %s.\n", arg );
            return 0;
        }
    }
    return 1;
}

static int decode_64( word_64* addr, char* arg ) {
    int i;

    if ( arg == ( char* )0 ) {
        printf( "Command requires an argument.\n" );
        return 0;
    }
#ifdef SIMPLE_64
    *addr = 0;
    for ( i = 0; i < strlen( arg ); i++ ) {
        *addr <<= 4;
        if ( '0' <= arg[ i ] && arg[ i ] <= '9' ) {
            *addr |= ( ( int )arg[ i ] - ( int )'0' );
        } else if ( 'A' <= arg[ i ] && arg[ i ] <= 'F' ) {
            *addr |= ( ( int )arg[ i ] - ( int )'A' + 10 );
        } else {
            *addr = 0;
            printf( "Not a number: %s.\n", arg );
            return 0;
        }
    }
#else
    addr->lo = addr->hi = 0;
    for ( i = 0; i < strlen( arg ); i++ ) {
        addr->hi <<= 4;
        addr->hi |= ( ( addr->lo >> 28 ) & 0x0f );
        addr->lo <<= 4;
        if ( '0' <= arg[ i ] && arg[ i ] <= '9' ) {
            addr->lo |= ( ( int )arg[ i ] - ( int )'0' );
        } else if ( 'A' <= arg[ i ] && arg[ i ] <= 'F' ) {
            addr->lo |= ( ( int )arg[ i ] - ( int )'A' + 10 );
        } else {
            addr->hi = addr->lo = 0;
            printf( "Not a number: %s.\n", arg );
            return 0;
        }
    }
#endif
    return 1;
}

char* str_nibbles( word_20 addr, int n ) {
    static char str[ 1025 ];
    char* cp;
    int i;

    if ( n > 1024 ) {
        str[ 0 ] = '\0';
        return str;
    }

    for ( cp = str, i = 0; i < n; i++ ) {
        sprintf( cp, "%.1X", read_nibble( addr + i ) );
        cp++;
    }
    *cp = '\0';

    return str;
}

static int confirm( const char* prompt ) {
    char ans[ 80 ];

    printf( "%s (y or n) ", prompt );
    fflush( stdout );
    read_str( ans, sizeof( ans ), 0 );
    while ( ans[ 0 ] != 'y' && ans[ 0 ] != 'Y' && ans[ 0 ] != 'n' &&
            ans[ 0 ] != 'N' ) {
        printf( "Please answer y or n.\n" );
        printf( "%s (y or n) ", prompt );
        fflush( stdout );
        read_str( ans, sizeof( ans ), 0 );
    }
    if ( ans[ 0 ] == 'y' || ans[ 0 ] == 'Y' ) {
        return 1;
    } else {
        printf( "Not confirmed.\n" );
        return 0;
    }
}

static void do_break( int argc, char** argv ) {
    int i;
    word_20 addr;

    if ( argc == 1 ) {
        for ( i = 0; i < MAX_BREAKPOINTS; i++ ) {
            if ( bkpt_tbl[ i ].flags == 0 )
                continue;
            if ( bkpt_tbl[ i ].flags == BP_EXEC ) {
                printf( "Breakpoint %d at 0x%.5lX\n", i + 1,
                        bkpt_tbl[ i ].addr );
            } else if ( bkpt_tbl[ i ].flags == BP_RANGE ) {
                printf( "Range watchpoint %d at 0x%.5lX - 0x%.5lX\n", i + 1,
                        bkpt_tbl[ i ].addr, bkpt_tbl[ i ].end_addr );
            } else {
                printf( "Watchpoint %d at 0x%.5lX\n", i + 1,
                        bkpt_tbl[ i ].addr );
            }
        }
    } else {
        str_to_upper( argv[ 1 ] );
        if ( !decode_20( &addr, argv[ 1 ] ) ) {
            return;
        }
        for ( i = 0; i < MAX_BREAKPOINTS; i++ ) {
            if ( bkpt_tbl[ i ].flags == 0 ) {
                bkpt_tbl[ i ].flags = BP_EXEC;
                bkpt_tbl[ i ].addr = addr;
                printf( "Breakpoint %d at 0x%.5lX\n", i + 1,
                        bkpt_tbl[ i ].addr );
                num_bkpts++;
                return;
            }
        }
        printf( "Breakpoint table full\n" );
    }
}

static void do_continue( int argc, char** argv ) { continue_flag = 1; }

static void do_delete( int argc, char** argv ) {
    int num;

    if ( argc == 1 ) {
        for ( num = 0; num < MAX_BREAKPOINTS; num++ ) {
            if ( bkpt_tbl[ num ].addr == saturn.PC ) {
                if ( bkpt_tbl[ num ].flags == BP_EXEC ) {
                    printf( "Breakpoint %d at 0x%.5lX deleted.\n", num + 1,
                            bkpt_tbl[ num ].addr );
                } else if ( bkpt_tbl[ num ].flags == BP_RANGE ) {
                    printf(
                        "Range watchpoint %d at 0x%.5lX - 0x%.5lX deleted.\n",
                        num + 1, bkpt_tbl[ num ].addr,
                        bkpt_tbl[ num ].end_addr );
                } else if ( bkpt_tbl[ num ].flags ) {
                    printf( "Watchpoint %d at 0x%.5lX deleted.\n", num + 1,
                            bkpt_tbl[ num ].addr );
                }
                num_bkpts--;
                bkpt_tbl[ num ].addr = 0;
                bkpt_tbl[ num ].flags = 0;
            }
        }
    } else {
        str_to_upper( argv[ 1 ] );
        if ( !strcmp( "ALL", argv[ 1 ] ) ) {
            for ( num = 0; num < MAX_BREAKPOINTS; num++ ) {
                bkpt_tbl[ num ].addr = 0;
                bkpt_tbl[ num ].flags = 0;
            }
            num_bkpts = 0;
            printf( "All breakpoints deleted.\n" );
        } else {
            if ( decode_dec( &num, argv[ 1 ] ) ) {
                if ( num < 1 || num > MAX_BREAKPOINTS ) {
                    printf( "Breakpoint %d out of range.\n", num );
                    return;
                }
                num -= 1;
                if ( bkpt_tbl[ num ].flags == BP_EXEC ) {
                    printf( "Breakpoint %d at 0x%.5lX deleted.\n", num + 1,
                            bkpt_tbl[ num ].addr );
                } else if ( bkpt_tbl[ num ].flags == BP_RANGE ) {
                    printf(
                        "Range watchpoint %d at 0x%.5lX - 0x%.5lX deleted.\n",
                        num + 1, bkpt_tbl[ num ].addr,
                        bkpt_tbl[ num ].end_addr );
                } else if ( bkpt_tbl[ num ].flags ) {
                    printf( "Watchpoint %d at 0x%.5lX deleted.\n", num + 1,
                            bkpt_tbl[ num ].addr );
                }
                num_bkpts--;
                bkpt_tbl[ num ].addr = 0;
                bkpt_tbl[ num ].flags = 0;
            }
        }
    }
}

static void do_exit( int argc, char** argv ) {
    if ( confirm( "Exit the emulator WITHOUT saving its state?" ) ) {
        printf( "Exit.\n" );
#ifdef GUI_IS_X11
        XCloseDisplay( dpy );
#endif
        exit( 0 );
    }
}

static void do_go( int argc, char** argv ) {
    word_20 addr;

    str_to_upper( argv[ 1 ] );
    if ( decode_20( &addr, argv[ 1 ] ) ) {
        saturn.PC = addr;
        enter_debugger &= ~ILLEGAL_INSTRUCTION;
    }
}

static void do_help( int argc, char** argv ) {
    int i;

    for ( i = 0; cmd_tbl[ i ].name; i++ ) {
        if ( cmd_tbl[ i ].help ) {
            printf( "%s.\n", cmd_tbl[ i ].help );
        }
    }
}

static void do_load( int argc, char** argv ) {
    saturn_t tmp_saturn;
    device_t tmp_device;

    if ( confirm( "Load emulator-state from files?" ) ) {
        memcpy( &tmp_saturn, &saturn, sizeof( saturn ) );
        memcpy( &tmp_device, &device, sizeof( device ) );
        memset( &saturn, 0, sizeof( saturn ) );
        if ( read_files() ) {
            printf( "Loading done.\n" );
            enter_debugger &= ~ILLEGAL_INSTRUCTION;
            if ( tmp_saturn.rom ) {
                free( tmp_saturn.rom );
            }
            if ( tmp_saturn.ram ) {
                free( tmp_saturn.ram );
            }
            if ( tmp_saturn.port1 ) {
                free( tmp_saturn.port1 );
            }
            if ( tmp_saturn.port2 ) {
                free( tmp_saturn.port2 );
            }
            init_display();
            update_display();
#ifdef GUI_IS_X11
#ifdef HAVE_XSHM
            if ( disp.display_update )
                refresh_display();
#endif
#endif
        } else {
            printf( "Loading emulator-state from files failed.\n" );
            if ( saturn.rom ) {
                free( saturn.rom );
            }
            if ( saturn.ram ) {
                free( saturn.ram );
            }
            if ( saturn.port1 ) {
                free( saturn.port1 );
            }
            if ( saturn.port2 ) {
                free( saturn.port2 );
            }
            memcpy( &saturn, &tmp_saturn, sizeof( saturn ) );
            memcpy( &device, &tmp_device, sizeof( device ) );
        }
    }
}

static void do_mode( int argc, char** argv ) {
    if ( argc < 2 ) {
        printf( "Disassembler uses %s mnemonics.\n",
                mode_name[ disassembler_mode ] );
    } else {
        str_to_upper( argv[ 1 ] );
        if ( !strcmp( "HP", argv[ 1 ] ) ) {
            disassembler_mode = HP_MNEMONICS;
        } else if ( !strcmp( "CLASS", argv[ 1 ] ) ) {
            disassembler_mode = CLASS_MNEMONICS;
        } else {
            printf( "Unknown disassembler mode %s. Try \"help\".\n",
                    argv[ 1 ] );
        }
    }
}

static void do_quit( int argc, char** argv ) {
    if ( confirm( "Quit the emulator and save its state?" ) ) {
        printf( "Exit.\n" );
        exit_emulator();
#ifdef GUI_IS_X11
        XCloseDisplay( dpy );
#endif
        exit( 0 );
    }
}

static void set_reg( word_64 val, int n, unsigned char* r ) {
    int i;

    for ( i = 0; i < n; i++ ) {
#ifdef SIMPLE_64
        r[ i ] =
            ( unsigned char )( ( val & ( 0xf << ( 4 * i ) ) ) >> ( 4 * i ) );
#else
        r[ i ] = ( unsigned char )( ( ( i < 8 ? val.lo : val.hi ) &
                                      ( 0xf << ( 4 * i ) ) ) >>
                                    ( 4 * i ) );
#endif
    }
}

static void dump_reg( const char* reg, int n, unsigned char* r ) {
    int i;

    printf( "%s:\t", reg );
    for ( i = n - 1; i >= 0; i-- ) {
        printf( "%.1X", r[ i ] & 0xf );
    }
    printf( "\n" );
}

static void set_st( word_64 val ) {
    int i;

    for ( i = 0; i < 16; i++ )
#ifdef SIMPLE_64
        saturn.PSTAT[ i ] = ( val & ( 1 << i ) ) ? 1 : 0;
#else
        saturn.PSTAT[ i ] = ( val.lo & ( 1 << i ) ) ? 1 : 0;
#endif
}

static void dump_st( void ) {
    int i;
    int val;

    val = 0;
    for ( i = NR_PSTAT - 1; i >= 0; i-- ) {
        val <<= 1;
        val |= saturn.PSTAT[ i ] ? 1 : 0;
    }
    printf( "    ST:\t%.4X (", val );
    for ( i = NR_PSTAT - 1; i > 0; i-- ) {
        if ( saturn.PSTAT[ i ] ) {
            printf( "%.1X ", i );
        } else {
            printf( "- " );
        }
    }
    if ( saturn.PSTAT[ 0 ] ) {
        printf( "%.1X)\n", 0 );
    } else {
        printf( "-)\n" );
    }
}

static void set_hst( word_64 val ) {
    saturn.XM = 0;
    saturn.SB = 0;
    saturn.SR = 0;
    saturn.MP = 0;
#ifdef SIMPLE_64
    if ( val & 1 )
        saturn.XM = 1;
    if ( val & 2 )
        saturn.SB = 1;
    if ( val & 4 )
        saturn.SR = 1;
    if ( val & 8 )
        saturn.MP = 1;
#else
    if ( val.lo & 1 )
        saturn.XM = 1;
    if ( val.lo & 2 )
        saturn.SB = 1;
    if ( val.lo & 4 )
        saturn.SR = 1;
    if ( val.lo & 8 )
        saturn.MP = 1;
#endif
}

static void dump_hst( void ) {
    short hst = 0;
    if ( saturn.XM != 0 )
        hst |= 1;
    if ( saturn.SB != 0 )
        hst |= 2;
    if ( saturn.SR != 0 )
        hst |= 3;
    if ( saturn.MP != 0 )
        hst |= 4;
    printf( "   HST:\t%.1X    (%s%s%s%s)\n", hst, saturn.MP ? "MP " : "-- ",
            saturn.SR ? "SR " : "-- ", saturn.SB ? "SB " : "-- ",
            saturn.XM ? "XM" : "--" );
}

static const char* mctl_str_gx[] = { "MMIO       ", "SysRAM     ",
                                     "Bank Switch", "Port 1     ",
                                     "Port 2     ", "SysROM     " };

static const char* mctl_str_sx[] = { "MMIO  ", "SysRAM", "Port 1",
                                     "Port 2", "Extra ", "SysROM" };

static void do_ram( int argc, char** argv ) {
    int i;

    for ( i = 0; i < 5; i++ ) {
        printf( "%s ", opt_gx ? mctl_str_gx[ i ] : mctl_str_sx[ i ] );
        if ( saturn.mem_cntl[ i ].unconfigured )
            printf( "unconfigured\n" );
        else if ( i == 0 )
            printf( "configured to 0x%.5lx\n",
                    saturn.mem_cntl[ i ].config[ 0 ] );
        else
            printf( "configured to 0x%.5lX - 0x%.5lX\n",
                    saturn.mem_cntl[ i ].config[ 0 ],
                    ( saturn.mem_cntl[ i ].config[ 0 ] |
                      ~saturn.mem_cntl[ i ].config[ 1 ] ) &
                        0xfffff );
    }
    if ( opt_gx )
        printf( "Port 2      switched to bank %d\n", saturn.bank_switch );
}

static void do_regs( int argc, char** argv ) {
    int i;
    word_64 val;

    if ( argc < 2 ) {
        /*
         * dump all registers
         */
        printf( "CPU is in %s mode. Registers:\n",
                saturn.hexmode == HEX ? "HEX" : "DEC" );
        dump_reg( "     A", 16, saturn.A );
        dump_reg( "     B", 16, saturn.B );
        dump_reg( "     C", 16, saturn.C );
        dump_reg( "     D", 16, saturn.D );
        printf( "    D0:\t%.5lX ->", saturn.D0 );
        for ( i = 0; i < 20; i += 5 ) {
            printf( " %s", str_nibbles( saturn.D0 + i, 5 ) );
        }
        printf( "\n" );
        printf( "    D1:\t%.5lX ->", saturn.D1 );
        for ( i = 0; i < 20; i += 5 ) {
            printf( " %s", str_nibbles( saturn.D1 + i, 5 ) );
        }
        printf( "\n" );
        printf( "     P:\t%.1X\n", saturn.P );
        disassemble( saturn.PC, instr );
        printf( "    PC:\t%.5lX -> %s\n", saturn.PC, instr );
        dump_reg( "    R0", 16, saturn.R0 );
        dump_reg( "    R1", 16, saturn.R1 );
        dump_reg( "    R2", 16, saturn.R2 );
        dump_reg( "    R3", 16, saturn.R3 );
        dump_reg( "    R4", 16, saturn.R4 );
        dump_reg( "    IN", 4, saturn.IN );
        dump_reg( "   OUT", 3, saturn.OUT );
        printf( " CARRY:\t%.1d\n", saturn.CARRY );
        dump_st();
        dump_hst();
    } else if ( argc == 2 ) {
        /*
         * dump specified register
         */
        str_to_upper( argv[ 1 ] );
        if ( !strcmp( "A", argv[ 1 ] ) ) {
            dump_reg( "     A", 16, saturn.A );
        } else if ( !strcmp( "B", argv[ 1 ] ) ) {
            dump_reg( "     B", 16, saturn.B );
        } else if ( !strcmp( "C", argv[ 1 ] ) ) {
            dump_reg( "     C", 16, saturn.C );
        } else if ( !strcmp( "D", argv[ 1 ] ) ) {
            dump_reg( "     D", 16, saturn.D );
        } else if ( !strcmp( "D0", argv[ 1 ] ) ) {
            printf( "    D0:\t%.5lX ->", saturn.D0 );
            for ( i = 0; i < 20; i += 5 ) {
                printf( " %s", str_nibbles( saturn.D0 + i, 5 ) );
            }
            printf( "\n" );
        } else if ( !strcmp( "D1", argv[ 1 ] ) ) {
            printf( "    D1:\t%.5lX ->", saturn.D1 );
            for ( i = 0; i < 20; i += 5 ) {
                printf( " %s", str_nibbles( saturn.D1 + i, 5 ) );
            }
            printf( "\n" );
        } else if ( !strcmp( "P", argv[ 1 ] ) ) {
            printf( "     P:\t%.1X\n", saturn.P );
        } else if ( !strcmp( "PC", argv[ 1 ] ) ) {
            disassemble( saturn.PC, instr );
            printf( "    PC:\t%.5lX -> %s\n", saturn.PC, instr );
        } else if ( !strcmp( "R0", argv[ 1 ] ) ) {
            dump_reg( "    R0", 16, saturn.R0 );
        } else if ( !strcmp( "R1", argv[ 1 ] ) ) {
            dump_reg( "    R1", 16, saturn.R1 );
        } else if ( !strcmp( "R2", argv[ 1 ] ) ) {
            dump_reg( "    R2", 16, saturn.R2 );
        } else if ( !strcmp( "R3", argv[ 1 ] ) ) {
            dump_reg( "    R3", 16, saturn.R3 );
        } else if ( !strcmp( "R4", argv[ 1 ] ) ) {
            dump_reg( "    R4", 16, saturn.R4 );
        } else if ( !strcmp( "IN", argv[ 1 ] ) ) {
            dump_reg( "    IN", 4, saturn.IN );
        } else if ( !strcmp( "OUT", argv[ 1 ] ) ) {
            dump_reg( "   OUT", 3, saturn.OUT );
        } else if ( !strcmp( "CARRY", argv[ 1 ] ) ) {
            printf( " CARRY:\t%.1d\n", saturn.CARRY );
        } else if ( !strcmp( "CY", argv[ 1 ] ) ) {
            printf( " CARRY:\t%.1d\n", saturn.CARRY );
        } else if ( !strcmp( "ST", argv[ 1 ] ) ) {
            dump_st();
        } else if ( !strcmp( "HST", argv[ 1 ] ) ) {
            dump_hst();
        } else {
            printf( "No Register %s in CPU.\n", argv[ 1 ] );
        }
    } else {
        /*
         * set specified register
         */
        str_to_upper( argv[ 1 ] );
        str_to_upper( argv[ 2 ] );
        if ( decode_64( &val, argv[ 2 ] ) ) {
            if ( !strcmp( "A", argv[ 1 ] ) ) {
                set_reg( val, 16, saturn.A );
                dump_reg( "     A", 16, saturn.A );
            } else if ( !strcmp( "B", argv[ 1 ] ) ) {
                set_reg( val, 16, saturn.B );
                dump_reg( "     B", 16, saturn.B );
            } else if ( !strcmp( "C", argv[ 1 ] ) ) {
                set_reg( val, 16, saturn.C );
                dump_reg( "     C", 16, saturn.C );
            } else if ( !strcmp( "D", argv[ 1 ] ) ) {
                set_reg( val, 16, saturn.D );
                dump_reg( "     D", 16, saturn.D );
            } else if ( !strcmp( "D0", argv[ 1 ] ) ) {
#ifdef SIMPLE_64
                saturn.D0 = ( word_20 )( val & 0xfffff );
#else
                saturn.D0 = ( word_20 )( val.lo & 0xfffff );
#endif
                printf( "    D0:\t%.5lX ->", saturn.D0 );
                for ( i = 0; i < 20; i += 5 ) {
                    printf( " %s", str_nibbles( saturn.D0 + i, 5 ) );
                }
                printf( "\n" );
            } else if ( !strcmp( "D1", argv[ 1 ] ) ) {
#ifdef SIMPLE_64
                saturn.D1 = ( word_20 )( val & 0xfffff );
#else
                saturn.D1 = ( word_20 )( val.lo & 0xfffff );
#endif
                printf( "    D1:\t%.5lX ->", saturn.D1 );
                for ( i = 0; i < 20; i += 5 ) {
                    printf( " %s", str_nibbles( saturn.D1 + i, 5 ) );
                }
                printf( "\n" );
            } else if ( !strcmp( "P", argv[ 1 ] ) ) {
#ifdef SIMPLE_64
                saturn.P = ( word_4 )( val & 0xf );
#else
                saturn.P = ( word_4 )( val.lo & 0xf );
#endif
                printf( "     P:\t%.1X\n", saturn.P );
            } else if ( !strcmp( "PC", argv[ 1 ] ) ) {
#ifdef SIMPLE_64
                saturn.PC = ( word_20 )( val & 0xfffff );
#else
                saturn.PC = ( word_20 )( val.lo & 0xfffff );
#endif
                disassemble( saturn.PC, instr );
                printf( "    PC:\t%.5lX -> %s\n", saturn.PC, instr );
            } else if ( !strcmp( "R0", argv[ 1 ] ) ) {
                set_reg( val, 16, saturn.R0 );
                dump_reg( "    R0", 16, saturn.R0 );
            } else if ( !strcmp( "R1", argv[ 1 ] ) ) {
                set_reg( val, 16, saturn.R1 );
                dump_reg( "    R1", 16, saturn.R1 );
            } else if ( !strcmp( "R2", argv[ 1 ] ) ) {
                set_reg( val, 16, saturn.R2 );
                dump_reg( "    R2", 16, saturn.R2 );
            } else if ( !strcmp( "R3", argv[ 1 ] ) ) {
                set_reg( val, 16, saturn.R3 );
                dump_reg( "    R3", 16, saturn.R3 );
            } else if ( !strcmp( "R4", argv[ 1 ] ) ) {
                set_reg( val, 16, saturn.R4 );
                dump_reg( "    R4", 16, saturn.R4 );
            } else if ( !strcmp( "IN", argv[ 1 ] ) ) {
                set_reg( val, 4, saturn.IN );
                dump_reg( "    IN", 4, saturn.IN );
            } else if ( !strcmp( "OUT", argv[ 1 ] ) ) {
                set_reg( val, 3, saturn.OUT );
                dump_reg( "   OUT", 3, saturn.OUT );
            } else if ( !strcmp( "CARRY", argv[ 1 ] ) ) {
#ifdef SIMPLE_64
                saturn.CARRY = ( word_1 )( val & 0x1 );
#else
                saturn.CARRY = ( word_1 )( val.lo & 0x1 );
#endif
                printf( " CARRY:\t%.1d\n", saturn.CARRY );
            } else if ( !strcmp( "CY", argv[ 1 ] ) ) {
#ifdef SIMPLE_64
                saturn.CARRY = ( word_1 )( val & 0x1 );
#else
                saturn.CARRY = ( word_1 )( val.lo & 0x1 );
#endif
                printf( " CARRY:\t%.1d\n", saturn.CARRY );
            } else if ( !strcmp( "ST", argv[ 1 ] ) ) {
                set_st( val );
                dump_st();
            } else if ( !strcmp( "HST", argv[ 1 ] ) ) {
                set_hst( val );
                dump_hst();
            } else {
                printf( "No Register %s in CPU.\n", argv[ 1 ] );
            }
        }
    }
}

static void do_save( int argc, char** argv ) {
    if ( write_files() ) {
        printf( "Saving done.\n" );
    } else {
        printf( "Saving emulator-state failed.\n" );
    }
}

struct se {
    int se_n;
    word_20 se_p;
    struct se* se_next;
};

char* get_stack( void ) {
    word_20 dsktop, dskbot;
    word_20 sp = 0, end = 0, ent = 0;
    word_20 ram_base, ram_mask;
    char dat[ 65536 ];
    char typ[ 256 ];
    int i = 0, n;

    ram_base = saturn.mem_cntl[ 1 ].config[ 0 ];
    ram_mask = saturn.mem_cntl[ 1 ].config[ 1 ];
    if ( opt_gx ) {
        saturn.mem_cntl[ 1 ].config[ 0 ] = 0x80000;
        saturn.mem_cntl[ 1 ].config[ 1 ] = 0xc0000;
        dsktop = DSKTOP_GX;
        dskbot = DSKBOT_GX;
    } else {
        saturn.mem_cntl[ 1 ].config[ 0 ] = 0x70000;
        saturn.mem_cntl[ 1 ].config[ 1 ] = 0xf0000;
        dsktop = DSKTOP_SX;
        dskbot = DSKBOT_SX;
    }

    load_addr( &sp, dsktop, 5 );
    load_addr( &end, dskbot, 5 );

    n = ( end - sp ) / 5 - 1; /* end never matches sp */

    printf( "n = %d\n", n );

    // TODO Get the top of the stack in the buffer.
    // TODO create a window and put the complete stack into it!
    if ( n ) {
        load_addr( &ent, sp, 5 );
        decode_rpl_obj_2( ent, typ, dat );
        printf( "%d %ld -> [%s] %s\n", i, ent, typ, dat );
    }

    for ( i = 0; i < n; i++ ) {
        load_addr( &ent, sp + ( 5 * i ), 5 );
        decode_rpl_obj_2( ent, typ, dat );
        printf( "%d %ld -> [%s] %s\n", i, ent, typ, dat );
    }

    saturn.mem_cntl[ 1 ].config[ 0 ] = ram_base;
    saturn.mem_cntl[ 1 ].config[ 1 ] = ram_mask;

    return 0;
}

static void do_stack( int argc, char** argv ) {
    word_20 dsktop, dskbot;
    word_20 sp = 0, end = 0, ent = 0;
    word_20 ram_base, ram_mask;
    char buf[ 65536 ];
    struct se *stack, *se;
    int n;

    ram_base = saturn.mem_cntl[ 1 ].config[ 0 ];
    ram_mask = saturn.mem_cntl[ 1 ].config[ 1 ];
    if ( opt_gx ) {
        saturn.mem_cntl[ 1 ].config[ 0 ] = 0x80000;
        saturn.mem_cntl[ 1 ].config[ 1 ] = 0xc0000;
        dsktop = DSKTOP_GX;
        dskbot = DSKBOT_GX;
    } else {
        saturn.mem_cntl[ 1 ].config[ 0 ] = 0x70000;
        saturn.mem_cntl[ 1 ].config[ 1 ] = 0xf0000;
        dsktop = DSKTOP_SX;
        dskbot = DSKBOT_SX;
    }

    load_addr( &sp, dsktop, 5 );
    load_addr( &end, dskbot, 5 );

    stack = ( struct se* )0;
    n = 0;
    do {
        load_addr( &ent, sp, 5 );
        if ( ent == 0 )
            break;
        n++;
        sp += 5;
        se = ( struct se* )malloc( sizeof( struct se ) );
        if ( se == 0 ) {
            fprintf( stderr, "Out off memory.\n" );
            break;
        }
        se->se_n = n;
        se->se_p = ent;
        se->se_next = stack;
        stack = se;
    } while ( sp <= end );

    if ( n == 0 )
        printf( "Empty stack.\n" );

    se = stack;
    while ( se ) {
        decode_rpl_obj( se->se_p, buf );
        if ( se->se_n != 1 )
            if ( strlen( buf ) > 63 ) {
                sprintf( &buf[ 60 ], "..." );
                buf[ 63 ] = '\0';
            }
        printf( "%5d: %.5lX -> %s\n", se->se_n, se->se_p, buf );
        se = se->se_next;
    }

    se = stack;
    while ( se ) {
        stack = se;
        se = se->se_next;
        free( stack );
    }

    saturn.mem_cntl[ 1 ].config[ 0 ] = ram_base;
    saturn.mem_cntl[ 1 ].config[ 1 ] = ram_mask;
}

static void do_stat( int argc, char** argv ) {
    printf( "Instructions/s: %ld\n", saturn.i_per_s );
    printf( "Timer 1 I/TICK: %d\n", saturn.t1_tick );
    printf( "Timer 2 I/TICK: %d\n", saturn.t2_tick );
}

static void do_step( int argc, char** argv ) {
    word_20 next_instr;
    word_32 n;
    int leave;

    if ( enter_debugger & ILLEGAL_INSTRUCTION ) {
        printf( "Can\'t step into an illegal instruction." );
        return;
    }

    n = 1;
    if ( argc > 1 )
        if ( !decode_32( &n, argv[ 1 ] ) )
            return;

    if ( n <= 0 )
        return;

    in_debugger = 1;
    step_instruction();

    if ( exec_flags & EXEC_BKPT ) {
        if ( check_breakpoint( BP_EXEC, saturn.PC ) ) {
            enter_debugger |= BREAKPOINT_HIT;
            return;
        }
    }

    next_instr = saturn.PC;

    sched_adjtime = 0;
    schedule();

    enter_debugger = 0;
    while ( 1 ) {
        if ( enter_debugger )
            break;

        leave = 0;

        if ( saturn.PC == next_instr ) {
            n--;
            leave = 1;
            if ( n == 0 )
                break;
        }

        step_instruction();

        if ( exec_flags & EXEC_BKPT ) {
            if ( check_breakpoint( BP_EXEC, saturn.PC ) ) {
                enter_debugger |= BREAKPOINT_HIT;
                break;
            }
        }

        if ( leave )
            next_instr = saturn.PC;

        schedule();
    }
}

static void do_reset( int argc, char** argv ) {
    if ( confirm( "Do a RESET (PC = 00000)?" ) ) {
        saturn.PC = 0;
        enter_debugger &= ~ILLEGAL_INSTRUCTION;
    }
}

static void do_rstk( int argc, char** argv ) {
    int i, j;

    disassemble( saturn.PC, instr );
    printf( "PC: %.5lX: %s\n", saturn.PC, instr );
    if ( saturn.rstkp < 0 ) {
        printf( "Empty return stack.\n" );
    } else {
        j = 0;
        for ( i = saturn.rstkp; i >= 0; i-- ) {
            disassemble( saturn.rstk[ i ], instr );
            printf( "%2d: %.5lX: %s\n", j, saturn.rstk[ i ], instr );
            j++;
        }
    }
}

int debug( void ) {
    t1_t2_ticks ticks;
    struct cmd* cmdp;
    char* cp;
    int argc;
    char* argv[ MAX_ARGS ];
    char* rl = NULL;
    static char* cl = ( char* )0;
    static char* old_line = ( char* )0;
    int i;

    /*
     * do we want to debug ???
     */
    if ( !useDebugger ) {
        if ( enter_debugger & ILLEGAL_INSTRUCTION ) {
            if ( !quiet )
                fprintf( stderr, "%s: reset (illegal instruction at 0x%.5lX)\n",
                         progname, saturn.PC );
            saturn.PC = 0;
        }
        if ( enter_debugger & USER_INTERRUPT )
            if ( verbose )
                printf( "%s: user interrupt (SIGINT) ignored\n", progname );
        exit_x48( 1 );
        if ( enter_debugger & BREAKPOINT_HIT )
            if ( verbose )
                printf( "%s: breakpoint hit at 0x%.5lX ignored\n", progname,
                        saturn.PC );
        if ( enter_debugger & TRAP_INSTRUCTION )
            if ( verbose )
                printf( "%s: trap instruction at 0x%.5lX ignored\n", progname,
                        saturn.PC );
        enter_debugger = 0;
        return 0;
    }

    /*
     * update the lcd if necessary
     */
    if ( device.display_touched ) {
        device.display_touched = 0;
        update_display();
#ifdef GUI_IS_X11
#ifdef HAVE_XSHM
        if ( disp.display_update )
            refresh_display();
#endif
#endif
    }

    /*
     * debugging is counted as idle time
     */
    stop_timer( RUN_TIMER );
    start_timer( IDLE_TIMER );

    continue_flag = 0;

    if ( enter_debugger & ILLEGAL_INSTRUCTION ) {
        printf( "ILLEGAL INSTRUCTION at %.5lX : %s\n", saturn.PC,
                str_nibbles( saturn.PC, 16 ) );
    }

    if ( enter_debugger & TRAP_INSTRUCTION ) {
        printf( "TRAP at %.5lX : %s\n", saturn.PC - 5,
                str_nibbles( saturn.PC - 5, 16 ) );
        enter_debugger &= ~TRAP_INSTRUCTION;
    }

    do {

        /*
         * print current instruction
         */
        disassemble( saturn.PC, instr );
        printf( "%.5lX: %s\n", saturn.PC, instr );

        /*
         * read a command
         */
        rl = readline( "x48-debug> " );

        if ( rl == ( char* )0 ) {
            continue_flag = 1;
            continue;
        }
        if ( *rl == '\0' ) {
            free( rl );
            rl = ( char* )0;
            if ( cl ) {
                free( cl );
                cl = ( char* )0;
            }
            cl = strdup( old_line == NULL ? "(null)" : old_line );
        } else {
            if ( cl ) {
                free( cl );
                cl = ( char* )0;
            }
            if ( old_line ) {
                free( old_line );
                old_line = ( char* )0;
            }
            cl = strdup( rl );
            old_line = strdup( rl );

            add_history( rl );

            free( rl );
            rl = ( char* )0;
        }

        /*
         * decode the commandline
         */
        cp = strtok( cl, " \t" );
        for ( cmdp = cmd_tbl; cmdp->name; cmdp++ ) {
            if ( strcmp( cp, cmdp->name ) == 0 ) {
                break;
            }
        }

        argc = 0;
        argv[ argc++ ] = cp;
        while ( ( cp = strtok( ( char* )0, " \t" ) ) != ( char* )0 ) {
            argv[ argc++ ] = cp;
            if ( argc == MAX_ARGS )
                break;
        }
        for ( i = argc; i < MAX_ARGS; i++ )
            argv[ i ] = ( char* )NULL;

        /*
         * execute the command, if valid
         */
        if ( cmdp->func ) {
            ( *cmdp->func )( argc, argv );
        } else {
            printf( "Undefined command \"%s\". Try \"help\".\n", argv[ 0 ] );
        }
        in_debugger = 0;

    } while ( !continue_flag );

    /*
     * adjust the hp48's timers
     */
    in_debugger = 1;
    ticks = get_t1_t2();
    in_debugger = 0;

    if ( saturn.t2_ctrl & 0x01 ) {
        saturn.timer2 = ticks.t2_ticks;
    }

    saturn.timer1 = ( set_t1 - ticks.t1_ticks ) & 0xf;

    sched_adjtime = 0;

    /*
     * restart timers
     */
    stop_timer( IDLE_TIMER );
    start_timer( RUN_TIMER );

    set_accesstime();

    if ( enter_debugger & ILLEGAL_INSTRUCTION ) {
        printf( "Reset (ILLEGAL INSTRUCTION)\n" );
        saturn.PC = 0;
    } else {
        printf( "Continue.\n" );
    }

    enter_debugger = 0;

    /*
     * Set exec_flags according to breakpoints, etc.
     */
    exec_flags = 0;
    if ( num_bkpts )
        exec_flags |= EXEC_BKPT;

    return 0;
}

int emulate_debug( void ) {
    do {

        step_instruction();

        if ( exec_flags & EXEC_BKPT ) {
            if ( check_breakpoint( BP_EXEC, saturn.PC ) ) {
                enter_debugger |= BREAKPOINT_HIT;
                break;
            }
        }

        if ( schedule_event-- == 0 ) {
            schedule();
        }

    } while ( !enter_debugger );

    return 0;
}
