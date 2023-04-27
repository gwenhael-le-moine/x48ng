#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "disasm.h"
#include "hp48.h"

#define TAB_SKIP 8

int disassembler_mode = CLASS_MNEMONICS;

const char* mode_name[] = { ( char* )"HP", ( char* )"class" };

static char* hex[] = {
    ( char* )"0123456789ABCDEF",
    ( char* )"0123456789abcdef",
};

static char* opcode_0_tbl[ 32 ] = {
    /*
     * HP Mnemonics
     */
    ( char* )"RTNSXM", ( char* )"RTN", ( char* )"RTNSC", ( char* )"RTNCC",
    ( char* )"SETHEX", ( char* )"SETDEC", ( char* )"RSTK=C", ( char* )"C=RSTK",
    ( char* )"CLRST", ( char* )"C=ST", ( char* )"ST=C", ( char* )"CSTEX",
    ( char* )"P=P+1", ( char* )"P=P-1", ( char* )"(NULL)", ( char* )"RTI",
    /*
     * Class Mnemonics
     */
    ( char* )"rtnsxm", ( char* )"rtn", ( char* )"rtnsc", ( char* )"rtncc",
    ( char* )"sethex", ( char* )"setdec", ( char* )"push", ( char* )"pop",
    ( char* )"clr.3   st", ( char* )"move.3  st, c", ( char* )"move.3  c, st",
    ( char* )"exg.3   c, st", ( char* )"inc.1   p", ( char* )"dec.1   p",
    ( char* )"(null)", ( char* )"rti" };

static char* op_str_0[ 16 ] = {
    /*
     * HP Mnemonics
     */
    ( char* )"A=A%cB", ( char* )"B=B%cC", ( char* )"C=C%cA", ( char* )"D=D%cC",
    ( char* )"B=B%cA", ( char* )"C=C%cB", ( char* )"A=A%cC", ( char* )"C=C%cD",
    /*
     * Class Mnemonics
     */
    ( char* )"b, a", ( char* )"c, b", ( char* )"a, c", ( char* )"c, d",
    ( char* )"a, b", ( char* )"b, c", ( char* )"c, a", ( char* )"d, c" };

static char* op_str_1[ 16 ] = {
    /*
     * HP Mnemonics
     */
    ( char* )"DAT0=A", ( char* )"DAT1=A", ( char* )"A=DAT0", ( char* )"A=DAT1",
    ( char* )"DAT0=C", ( char* )"DAT1=C", ( char* )"C=DAT0", ( char* )"C=DAT1",
    /*
     * Class Mnemonics
     */
    ( char* )"a, (d0)", ( char* )"a, (d1)", ( char* )"(d0), a",
    ( char* )"(d1), a", ( char* )"c, (d0)", ( char* )"c, (d1)",
    ( char* )"(d0), c", ( char* )"(d1), c" };

static char* in_str_80[ 32 ] = {
    /*
     * HP Mnemonics
     */
    ( char* )"OUT=CS", ( char* )"OUT=C", ( char* )"A=IN", ( char* )"C=IN",
    ( char* )"UNCNFG", ( char* )"CONFIG", ( char* )"C=ID", ( char* )"SHUTDN",
    NULL, ( char* )"C+P+1", ( char* )"RESET", ( char* )"BUSCC", NULL, NULL,
    ( char* )"SREQ?", NULL,
    /*
     * Class Mnemonics
     */
    ( char* )"move.s  c, out", ( char* )"move.3  c, out",
    ( char* )"move.4  in, a", ( char* )"move.4  in, c", ( char* )"uncnfg",
    ( char* )"config", ( char* )"c=id", ( char* )"shutdn", NULL,
    ( char* )"add.a   p+1, c", ( char* )"reset", ( char* )"buscc", NULL, NULL,
    ( char* )"sreq?", NULL };

static char* in_str_808[ 32 ] = {
    /*
     * HP Mnemonics
     */
    ( char* )"INTON", NULL, NULL, ( char* )"BUSCB", NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, ( char* )"PC=(A)", ( char* )"BUSCD",
    ( char* )"PC=(C)", ( char* )"INTOFF",
    /*
     * Class Mnemonics
     */
    ( char* )"inton", NULL, NULL, ( char* )"buscb", NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, ( char* )"jmp     (a)", ( char* )"buscd",
    ( char* )"jmp     (c)", ( char* )"intoff" };

static char* op_str_81[ 8 ] = {
    /*
     * HP Mnemonics
     */
    ( char* )"A",
    ( char* )"B",
    ( char* )"C",
    ( char* )"D",
    /*
     * Class Mnemonics
     */
    ( char* )"a",
    ( char* )"b",
    ( char* )"c",
    ( char* )"d",
};

static char* in_str_81b[ 32 ] = {
    /*
     * HP Mnemonics
     */
    NULL,
    NULL,
    ( char* )"PC=A",
    ( char* )"PC=C",
    ( char* )"A=PC",
    ( char* )"C=PC",
    ( char* )"APCEX",
    ( char* )"CPCEX",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    /*
     * Class Mnemonics
     */
    NULL,
    NULL,
    ( char* )"jmp     a",
    ( char* )"jmp     c",
    ( char* )"move.a  pc, a",
    ( char* )"move.a  pc, c",
    ( char* )"exg.a   a, pc",
    ( char* )"exg.a   c, pc",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

static char* in_str_9[ 16 ] = {
    /*
     * HP Mnemonics
     */
    ( char* )"=", ( char* )"#", ( char* )"=", ( char* )"#", ( char* )">",
    ( char* )"<", ( char* )">=", ( char* )"<=",
    /*
     * Class Mnemonics
     */
    ( char* )"eq", ( char* )"ne", ( char* )"eq", ( char* )"ne", ( char* )"gt",
    ( char* )"lt", ( char* )"ge", ( char* )"le" };

static char* op_str_9[ 16 ] = {
    /*
     * HP Mnemonics
     */
    ( char* )"?A%sB", ( char* )"?B%sC", ( char* )"?C%sA", ( char* )"?D%sC",
    ( char* )"?A%s0", ( char* )"?B%s0", ( char* )"?C%s0", ( char* )"?D%s0",
    /*
     * Class Mnemonics
     */
    ( char* )"a, b", ( char* )"b, c", ( char* )"c, a", ( char* )"d, c",
    ( char* )"a, 0", ( char* )"b, 0", ( char* )"c, 0", ( char* )"d, 0" };

static char* op_str_af[ 32 ] = {
    /*
     * HP Mnemonics
     */
    ( char* )"A=A%sB", ( char* )"B=B%sC", ( char* )"C=C%sA", ( char* )"D=D%sC",
    ( char* )"A=A%sA", ( char* )"B=B%sB", ( char* )"C=C%sC", ( char* )"D=D%sD",
    ( char* )"B=B%sA", ( char* )"C=C%sB", ( char* )"A=A%sC", ( char* )"C=C%sD",
    ( char* )"A=B%sA", ( char* )"B=C%sB", ( char* )"C=A%sC", ( char* )"D=C%sD",
    /*
     * Class Mnemonics
     */
    ( char* )"b, a", ( char* )"c, b", ( char* )"a, c", ( char* )"c, d",
    ( char* )"a, a", ( char* )"b, b", ( char* )"c, c", ( char* )"d, d",
    ( char* )"a, b", ( char* )"b, c", ( char* )"c, a", ( char* )"d, c",
    ( char* )"b, a", ( char* )"c, b", ( char* )"a, c", ( char* )"c, d" };

static char hp_reg_1_af[] = "ABCDABCDBCACABAC";
static char hp_reg_2_af[] = "0000BCACABCDBCCD";

static char* field_tbl[ 32 ] = {
    /*
     * HP Mnemonics
     */
    ( char* )"P",
    ( char* )"WP",
    ( char* )"XS",
    ( char* )"X",
    ( char* )"S",
    ( char* )"M",
    ( char* )"B",
    ( char* )"W",
    ( char* )"P",
    ( char* )"WP",
    ( char* )"XS",
    ( char* )"X",
    ( char* )"S",
    ( char* )"M",
    ( char* )"B",
    ( char* )"A",
    /*
     * Class Mnemonics
     */
    ( char* )".p",
    ( char* )".wp",
    ( char* )".xs",
    ( char* )".x",
    ( char* )".s",
    ( char* )".m",
    ( char* )".b",
    ( char* )".w",
    ( char* )".p",
    ( char* )".wp",
    ( char* )".xs",
    ( char* )".x",
    ( char* )".s",
    ( char* )".m",
    ( char* )".b",
    ( char* )".a",
};

static char* hst_bits[ 8 ] = {
    /*
     * HP Mnemonics
     */
    ( char* )"XM",
    ( char* )"SB",
    ( char* )"SR",
    ( char* )"MP",
    /*
     * Class Mnemonics
     */
    ( char* )"xm",
    ( char* )"sb",
    ( char* )"sr",
    ( char* )"mp",
};

int read_int( word_20* addr, int n ) {
    int i, t;

    for ( i = 0, t = 0; i < n; i++ )
        t |= read_nibble( ( *addr )++ ) << ( i * 4 );
    return t;
}

char* append_str( char* buf, const char* str ) {
    while ( ( *buf = *str++ ) )
        buf++;
    return buf;
}

char* append_tab_16( char* buf ) {
    int n;
    char* p;

    n = 16 - ( strlen( buf ) % 16 );
    p = &buf[ strlen( buf ) ];
    while ( n-- )
        *p++ = ' ';
    *p = '\0';
    return p;
}

char* append_tab( char* buf ) {
    int n;
    char* p;

    n = TAB_SKIP - ( strlen( buf ) % TAB_SKIP );
    p = &buf[ strlen( buf ) ];
    while ( n-- )
        *p++ = ' ';
    *p = '\0';
    return p;
}

char* append_field( char* buf, word_4 fn ) {
    buf = append_str( buf, field_tbl[ fn + 16 * disassembler_mode ] );
    return buf;
}

char* append_imm_nibble( char* buf, word_20* addr, int n ) {
    int i;
    char t[ 16 ];

    if ( disassembler_mode == CLASS_MNEMONICS ) {
        *buf++ = '#';
        if ( n > 1 )
            *buf++ = '$';
    }
    if ( n > 1 ) {
        for ( i = 0; i < n; i++ )
            t[ i ] = hex[ disassembler_mode ][ read_nibble( ( *addr )++ ) ];
        for ( i = n - 1; i >= 0; i-- ) {
            *buf++ = t[ i ];
        }
        *buf = '\0';
    } else {
        sprintf( t, ( char* )"%d", read_nibble( ( *addr )++ ) );
        buf = append_str( buf, t );
    }
    return buf;
}

char* append_addr( char* buf, word_20 addr ) {
    int shift;
    long mask;

    if ( disassembler_mode == CLASS_MNEMONICS ) {
        *buf++ = '$';
    }
    for ( mask = 0xf0000, shift = 16; mask != 0; mask >>= 4, shift -= 4 )
        *buf++ = hex[ disassembler_mode ][ ( addr & mask ) >> shift ];
    *buf = '\0';
    return buf;
}

char* append_r_addr( char* buf, word_20* pc, long disp, int n, int offset ) {
    long sign;

    sign = 1 << ( n * 4 - 1 );
    if ( disp & sign )
        disp |= ~( sign - 1 );
    *pc += disp;

    switch ( disassembler_mode ) {
        case HP_MNEMONICS:
            if ( disp < 0 ) {
                buf = append_str( buf, ( char* )"-" );
                disp = -disp - offset;
            } else {
                buf = append_str( buf, ( char* )"+" );
                disp += offset;
            }
            buf = append_addr( buf, disp );
            break;
        case CLASS_MNEMONICS:
            if ( disp < 0 ) {
                buf = append_str( buf, ( char* )"-" );
                disp = -disp - offset;
            } else {
                buf = append_str( buf, ( char* )"+" );
                disp += offset;
            }
            buf = append_addr( buf, disp );
            break;
        default:
            buf = append_str( buf, ( char* )"Unknown disassembler mode" );
            break;
    }
    return buf;
}

char* append_pc_comment( char* buf, word_20 pc ) {
    char* p = buf;

    while ( strlen( buf ) < 4 * TAB_SKIP )
        p = append_tab( buf );

    switch ( disassembler_mode ) {
        case HP_MNEMONICS:
            p = append_str( p, ( char* )"# Address: (char*)" );
            p = append_addr( p, pc );
            break;
        case CLASS_MNEMONICS:
            p = append_str( p, ( char* )"; address: (char*)" );
            p = append_addr( p, pc );
            break;
        default:
            p = append_str( p, ( char* )"Unknown disassembler mode" );
            break;
    }
    return p;
}

char* append_hst_bits( char* buf, int n ) {
    int i;
    char* p = buf;

    switch ( disassembler_mode ) {
        case HP_MNEMONICS:
            for ( i = 0; i < 4; i++ )
                if ( n & ( 1 << i ) ) {
                    if ( p != buf )
                        p = append_str( p, ( char* )"=" );
                    p = append_str( p, hst_bits[ i + 4 * disassembler_mode ] );
                }
            break;

        case CLASS_MNEMONICS:
            while ( strlen( buf ) < 4 * TAB_SKIP )
                p = append_tab( buf );
            p = &buf[ strlen( buf ) ];
            p = append_str( p, ( char* )"; hst bits: (char*)" );

            for ( buf = p, i = 0; i < 4; i++ )
                if ( n & ( 1 << i ) ) {
                    if ( p != buf )
                        p = append_str( p, ( char* )", (char*)" );
                    p = append_str( p, hst_bits[ i + 4 * disassembler_mode ] );
                }
            break;

        default:
            p = append_str( p, ( char* )"Unknown disassembler mode" );
            break;
    }

    return p;
}

char* disasm_1( word_20* addr, char* out ) {
    word_4 n;
    word_4 fn;
    char* p;
    char buf[ 20 ];
    char c;

    p = out;
    switch ( ( n = read_nibble( ( *addr )++ ) ) ) {
        case 0:
        case 1:
            fn = read_nibble( ( *addr )++ );
            fn = ( fn & 7 );
            if ( fn > 4 )
                fn -= 4;
            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    c = ( char )( ( fn < 8 ) ? 'A' : 'C' );
                    if ( n == 0 )
                        sprintf( buf, ( char* )"R%d=%c", fn, c );
                    else
                        sprintf( buf, ( char* )"%c=R%d", c, fn );
                    p = append_str( out, buf );
                    break;
                case CLASS_MNEMONICS:
                    p = append_str( out, ( char* )"move.w" );
                    p = append_tab( out );
                    c = ( char )( ( fn < 8 ) ? 'a' : 'c' );
                    if ( n == 0 )
                        sprintf( buf, ( char* )"%c, r%d", c, fn );
                    else
                        sprintf( buf, ( char* )"r%d, %c", fn, c );
                    p = append_str( p, buf );
                    break;
                default:
                    p = append_str( out, ( char* )"Unknown disassembler mode" );
                    break;
            }
            break;

        case 2:
            fn = read_nibble( ( *addr )++ );
            fn = ( fn & 7 );
            if ( fn > 4 )
                fn -= 4;
            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    c = ( char )( ( fn < 8 ) ? 'A' : 'C' );
                    sprintf( buf, ( char* )"%cR%dEX", c, fn );
                    p = append_str( out, buf );
                    break;
                case CLASS_MNEMONICS:
                    p = append_str( out, ( char* )"exg.w" );
                    p = append_tab( out );
                    c = ( char )( ( fn < 8 ) ? 'a' : 'c' );
                    sprintf( buf, ( char* )"%c, r%d", c, fn );
                    p = append_str( p, buf );
                    break;
                default:
                    p = append_str( out, ( char* )"Unknown disassembler mode" );
                    break;
            }
            break;

        case 3:
            n = read_nibble( ( *addr )++ );
            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    c = ( n & 4 ) ? 'C' : 'A';
                    if ( n & 2 ) {
                        if ( n < 8 ) {
                            sprintf( buf, ( char* )"%cD%dEX", c, ( n & 1 ) );
                        } else {
                            sprintf( buf, ( char* )"%cD%dXS", c, ( n & 1 ) );
                        }
                    } else {
                        if ( n < 8 ) {
                            sprintf( buf, ( char* )"D%d=%c", ( n & 1 ), c );
                        } else {
                            sprintf( buf, ( char* )"D%d=%cS", ( n & 1 ), c );
                        }
                    }
                    p = append_str( out, buf );
                    break;
                case CLASS_MNEMONICS:
                    p = append_str( out, ( n & 2 ) ? ( char* )"exg."
                                                   : ( char* )"move." );
                    p = append_str( p,
                                    ( n < 8 ) ? ( char* )"a" : ( char* )"4" );
                    p = append_tab( out );
                    c = ( n & 4 ) ? 'c' : 'a';
                    sprintf( buf, ( char* )"%c, d%d", c, ( n & 1 ) );
                    p = append_str( p, buf );
                    break;
                default:
                    p = append_str( out, ( char* )"Unknown disassembler mode" );
                    break;
            }
            break;

        case 4:
        case 5:
            fn = read_nibble( ( *addr )++ );
            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    p = append_str(
                        out, op_str_1[ ( fn & 7 ) + 8 * disassembler_mode ] );
                    p = append_tab( out );
                    if ( n == 4 ) {
                        p = append_str( p, ( fn < 8 ) ? ( char* )"A"
                                                      : ( char* )"B" );
                    } else {
                        n = read_nibble( ( *addr )++ );
                        if ( fn < 8 ) {
                            p = append_field( p, n );
                        } else {
                            sprintf( buf, ( char* )"%d", n + 1 );
                            p = append_str( p, buf );
                        }
                    }
                    break;
                case CLASS_MNEMONICS:
                    p = append_str( out, ( char* )"move" );
                    if ( n == 4 ) {
                        p = append_str( p, ( char* )"." );
                        p = append_str( p, ( fn < 8 ) ? ( char* )"a"
                                                      : ( char* )"b" );
                    } else {
                        n = read_nibble( ( *addr )++ );
                        if ( fn < 8 ) {
                            p = append_field( p, n );
                        } else {
                            sprintf( buf, ( char* )".%d", n + 1 );
                            p = append_str( p, buf );
                        }
                    }
                    p = append_tab( out );
                    p = append_str(
                        p, op_str_1[ ( fn & 7 ) + 8 * disassembler_mode ] );
                    break;
                default:
                    p = append_str( out, ( char* )"Unknown disassembler mode" );
                    break;
            }
            break;

        case 6:
        case 7:
        case 8:
        case 0xc:
            fn = read_nibble( *addr++ );
            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    if ( n == 6 || n == 8 )
                        p = append_str( out, ( char* )"D0=D0" );
                    else
                        p = append_str( out, ( char* )"D1=D1" );
                    if ( n < 8 )
                        p = append_str( p, ( char* )"+" );
                    else
                        p = append_str( p, ( char* )"-" );
                    p = append_tab( out );
                    sprintf( buf, ( char* )"%d", fn + 1 );
                    p = append_str( p, buf );
                    break;
                case CLASS_MNEMONICS:
                    if ( n < 8 )
                        p = append_str( out, ( char* )"add.a" );
                    else
                        p = append_str( out, ( char* )"sub.a" );
                    p = append_tab( out );
                    sprintf( buf, ( char* )"#%d, (char*)", fn + 1 );
                    p = append_str( p, buf );
                    if ( n == 6 || n == 8 )
                        p = append_str( p, ( char* )"d0" );
                    else
                        p = append_str( p, ( char* )"d1" );
                    break;
                default:
                    p = append_str( out, ( char* )"Unknown disassembler mode" );
                    break;
            }
            break;

        case 9:
        case 0xa:
        case 0xb:
        case 0xd:
        case 0xe:
        case 0xf:
            c = ( char )( ( n < 0xd ) ? '0' : '1' );
            switch ( n & 3 ) {
                case 1:
                    n = 2;
                    break;
                case 2:
                    n = 4;
                    break;
                case 3:
                    n = 5;
                    break;
            }
            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    sprintf( buf, ( char* )"D%c=(%d)", c, n );
                    p = append_str( out, buf );
                    p = append_tab( out );
                    p = append_imm_nibble( p, addr, n );
                    break;
                case CLASS_MNEMONICS:
                    if ( n == 5 ) {
                        sprintf( buf, ( char* )"move.a" );
                    } else if ( n == 4 ) {
                        sprintf( buf, ( char* )"move.as" );
                    } else {
                        sprintf( buf, ( char* )"move.b" );
                    }
                    p = append_str( out, buf );
                    p = append_tab( out );
                    p = append_imm_nibble( p, addr, n );
                    sprintf( buf, ( char* )", d%c", c );
                    p = append_str( p, buf );
                    break;
                default:
                    p = append_str( out, ( char* )"Unknown disassembler mode" );
                    break;
            }
            break;

        default:
            break;
    }
    return p;
}

char* disasm_8( word_20* addr, char* out ) {
    word_4 n;
    word_4 fn;
    char* p = out;
    char c;
    char buf[ 20 ];
    word_20 disp, pc;

    fn = read_nibble( ( *addr )++ );
    switch ( fn ) {
        case 0:
            n = read_nibble( ( *addr )++ );
            if ( NULL !=
                 ( p = ( char* )in_str_80[ n + 16 * disassembler_mode ] ) ) {
                p = append_str( out, p );
                return p;
            }
            switch ( n ) {
                case 8:
                    fn = read_nibble( ( *addr )++ );
                    if ( NULL !=
                         ( p = ( char* )
                               in_str_808[ fn + 16 * disassembler_mode ] ) ) {
                        p = append_str( out, p );
                        return p;
                    }
                    switch ( fn ) {
                        case 1:
                            n = read_nibble( ( *addr )++ );
                            if ( n == 0 ) {
                                switch ( disassembler_mode ) {
                                    case HP_MNEMONICS:
                                        p = append_str( out, ( char* )"RSI" );
                                        break;
                                    case CLASS_MNEMONICS:
                                        p = append_str( out, ( char* )"rsi" );
                                        break;
                                    default:
                                        p = append_str(
                                            out, ( char* )"Unknown "
                                                          "disassembler mode" );
                                        break;
                                }
                            }
                            break;
                        case 2:
                            n = read_nibble( ( *addr )++ );
                            switch ( disassembler_mode ) {
                                case HP_MNEMONICS:
                                    if ( n < 5 ) {
                                        sprintf( buf, ( char* )"LA(%d)",
                                                 n + 1 );
                                    } else {
                                        sprintf( buf, ( char* )"LAHEX" );
                                    }
                                    p = append_str( out, buf );
                                    p = append_tab( out );
                                    p = append_imm_nibble( p, addr, n + 1 );
                                    break;
                                case CLASS_MNEMONICS:
                                    sprintf( buf, ( char* )"move.%d", n + 1 );
                                    p = append_str( out, buf );
                                    p = append_tab( out );
                                    p = append_imm_nibble( p, addr, n + 1 );
                                    sprintf( buf, ( char* )", a.p" );
                                    p = append_str( p, buf );
                                    break;
                                default:
                                    p = append_str(
                                        out,
                                        ( char* )"Unknown disassembler mode" );
                                    break;
                            }
                            break;

                        case 4:
                        case 5:
                        case 8:
                        case 9:

                            switch ( disassembler_mode ) {
                                case HP_MNEMONICS:
                                    sprintf( buf, ( char* )"%cBIT=%d",
                                             ( fn & 8 ) ? 'C' : 'A',
                                             ( fn & 1 ) ? 1 : 0 );
                                    p = append_str( out, buf );
                                    p = append_tab( out );
                                    p = append_imm_nibble( p, addr, 1 );
                                    break;
                                case CLASS_MNEMONICS:
                                    p = append_str(
                                        out, ( fn & 1 ) ? ( char* )"bset"
                                                        : ( char* )"bclr" );
                                    p = append_tab( out );
                                    p = append_imm_nibble( p, addr, 1 );
                                    p = append_str( p, ( fn & 8 )
                                                           ? ( char* )", c"
                                                           : ( char* )", a" );
                                    break;
                                default:
                                    p = append_str(
                                        out,
                                        ( char* )"Unknown disassembler mode" );
                                    break;
                            }
                            break;

                        case 6:
                        case 7:
                        case 0xa:
                        case 0xb:

                            n = read_nibble( ( *addr )++ );
                            pc = *addr;
                            disp = read_int( addr, 2 );

                            switch ( disassembler_mode ) {
                                case HP_MNEMONICS:
                                    c = ( char )( ( fn < 0xa ) ? 'A' : 'C' );
                                    sprintf( buf, ( char* )"?%cBIT=%d", c,
                                             ( fn & 1 ) ? 1 : 0 );
                                    p = append_str( out, buf );
                                    p = append_tab( out );
                                    sprintf( buf, ( char* )"%d", n );
                                    p = append_str( p, buf );
                                    if ( disp != 0 ) {
                                        p = append_str(
                                            p, ( char* )", GOYES (char*)" );
                                        p = append_r_addr( p, &pc, disp, 2, 5 );
                                        p = append_pc_comment( out, pc );
                                    } else
                                        p = append_str( p,
                                                        ( char* )", RTNYES" );
                                    break;
                                case CLASS_MNEMONICS:
                                    c = ( char )( ( fn < 0xa ) ? 'a' : 'c' );
                                    p = append_str( out, ( disp == 0 )
                                                             ? ( char* )"rt"
                                                             : ( char* )"b" );
                                    p = append_str( p, ( fn & 1 )
                                                           ? ( char* )"bs"
                                                           : ( char* )"bc" );
                                    p = append_tab( out );
                                    sprintf( buf, ( char* )"#%d, %c", n, c );
                                    p = append_str( p, buf );
                                    if ( disp != 0 ) {
                                        p = append_str( p,
                                                        ( char* )", (char*)" );
                                        p = append_r_addr( p, &pc, disp, 2, 5 );
                                        p = append_pc_comment( out, pc );
                                    }
                                    break;
                                default:
                                    p = append_str(
                                        out,
                                        ( char* )"Unknown disassembler mode" );
                                    break;
                            }
                            break;

                        default:
                            break;
                    }
                    break;

                case 0xc:
                case 0xd:
                case 0xf:
                    fn = read_nibble( ( *addr )++ );
                    switch ( disassembler_mode ) {
                        case HP_MNEMONICS:
                            sprintf( buf,
                                     ( n == 0xf ) ? ( char* )"%c%cEX"
                                                  : ( char* )"%c=%c",
                                     ( n == 0xd ) ? 'P' : 'C',
                                     ( n == 0xd ) ? 'C' : 'P' );
                            p = append_str( out, buf );
                            p = append_tab( out );
                            sprintf( buf, ( char* )"%d", fn );
                            p = append_str( p, buf );
                            break;
                        case CLASS_MNEMONICS:
                            p = append_str( out, ( n == 0xf )
                                                     ? ( char* )"exg.1"
                                                     : ( char* )"move.1" );
                            p = append_tab( out );
                            sprintf( buf,
                                     ( n == 0xd ) ? ( char* )"p, c.%d"
                                                  : ( char* )"c.%d, p",
                                     fn );
                            p = append_str( p, buf );
                            break;
                        default:
                            p = append_str(
                                out, ( char* )"Unknown disassembler mode" );
                            break;
                    }
                    break;

                default:
                    break;
            }
            break;

        case 1:
            switch ( n = read_nibble( ( *addr )++ ) ) {
                case 0:
                case 1:
                case 2:
                case 3:
                    switch ( disassembler_mode ) {
                        case HP_MNEMONICS:
                            sprintf( buf, ( char* )"%sSLC",
                                     op_str_81[ ( n & 3 ) +
                                                4 * disassembler_mode ] );
                            p = append_str( out, buf );
                            break;
                        case CLASS_MNEMONICS:
                            p = append_str( out, ( char* )"rol.w" );
                            p = append_tab( out );
                            p = append_str( p, ( char* )"#4, (char*)" );
                            p = append_str(
                                p, op_str_81[ ( n & 3 ) +
                                              4 * disassembler_mode ] );
                            break;
                        default:
                            p = append_str(
                                out, ( char* )"Unknown disassembler mode" );
                            break;
                    }
                    break;

                case 4:
                case 5:
                case 6:
                case 7:
                    switch ( disassembler_mode ) {
                        case HP_MNEMONICS:
                            sprintf( buf, ( char* )"%sSRC",
                                     op_str_81[ ( n & 3 ) +
                                                4 * disassembler_mode ] );
                            p = append_str( out, buf );
                            break;
                        case CLASS_MNEMONICS:
                            p = append_str( out, ( char* )"ror.w" );
                            p = append_tab( out );
                            p = append_str( p, ( char* )"#4, (char*)" );
                            p = append_str(
                                p, op_str_81[ ( n & 3 ) +
                                              4 * disassembler_mode ] );
                            break;
                        default:
                            p = append_str(
                                out, ( char* )"Unknown disassembler mode" );
                            break;
                    }
                    break;

                case 8:
                    fn = read_nibble( ( *addr )++ );
                    n = read_nibble( ( *addr )++ );
                    switch ( disassembler_mode ) {
                        case HP_MNEMONICS:
                            sprintf(
                                buf, ( char* )"%s=%s%cCON",
                                op_str_81[ ( n & 3 ) + 4 * disassembler_mode ],
                                op_str_81[ ( n & 3 ) + 4 * disassembler_mode ],
                                ( n < 8 ) ? '+' : '-' );
                            p = append_str( out, buf );
                            p = append_tab( out );
                            p = append_field( p, fn );
                            fn = read_nibble( ( *addr )++ );
                            sprintf( buf, ( char* )", %d", fn + 1 );
                            p = append_str( p, buf );
                            break;
                        case CLASS_MNEMONICS:
                            p = append_str( out, ( n < 8 ) ? ( char* )"add"
                                                           : ( char* )"sub" );
                            p = append_field( p, fn );
                            p = append_tab( out );
                            fn = read_nibble( ( *addr )++ );
                            sprintf( buf, ( char* )"#%d, (char*)", fn + 1 );
                            p = append_str( p, buf );
                            p = append_str(
                                p, op_str_81[ ( n & 3 ) +
                                              4 * disassembler_mode ] );
                            break;
                        default:
                            p = append_str(
                                out, ( char* )"Unknown disassembler mode" );
                            break;
                    }
                    break;

                case 9:
                    switch ( disassembler_mode ) {
                        case HP_MNEMONICS:
                            sprintf( buf, ( char* )"%sSRB.F",
                                     op_str_81[ ( n & 3 ) +
                                                4 * disassembler_mode ] );
                            p = append_str( out, buf );
                            p = append_tab( out );
                            p = append_field( p, read_nibble( ( *addr )++ ) );
                            break;
                        case CLASS_MNEMONICS:
                            p = append_str( out, ( char* )"lsr" );
                            p = append_field( p, read_nibble( ( *addr )++ ) );
                            p = append_tab( out );
                            p = append_str( p, ( char* )"#1, (char*)" );
                            p = append_str(
                                p, op_str_81[ ( n & 3 ) +
                                              4 * disassembler_mode ] );
                            break;
                        default:
                            p = append_str(
                                out, ( char* )"Unknown disassembler mode" );
                            break;
                    }
                    break;

                case 0xa:
                    fn = read_nibble( ( *addr )++ );
                    n = read_nibble( ( *addr )++ );
                    if ( n > 2 )
                        break;
                    c = ( char )read_nibble( ( *addr )++ );
                    if ( ( ( int )c & 7 ) > 4 )
                        break;
                    switch ( disassembler_mode ) {
                        case HP_MNEMONICS:
                            if ( n == 2 ) {
                                sprintf( buf, ( char* )"%cR%dEX.F",
                                         ( ( int )c < 8 ) ? 'A' : 'C',
                                         ( int )c & 7 );
                            } else if ( n == 1 ) {
                                sprintf( buf, ( char* )"%c=R%d.F",
                                         ( ( int )c < 8 ) ? 'A' : 'C',
                                         ( int )c & 7 );
                            } else {
                                sprintf( buf, ( char* )"R%d=%c.F", ( int )c & 7,
                                         ( ( int )c < 8 ) ? 'A' : 'C' );
                            }
                            p = append_str( out, buf );
                            p = append_tab( out );
                            p = append_field( p, fn );
                            break;
                        case CLASS_MNEMONICS:
                            p = append_str( out, ( n == 2 ) ? ( char* )"exg"
                                                            : ( char* )"move" );
                            p = append_field( p, fn );
                            p = append_tab( out );
                            if ( n == 1 ) {
                                sprintf( buf, ( char* )"r%d", ( int )c & 7 );
                                p = append_str( p, buf );
                            } else
                                p = append_str( p, ( ( int )c < 8 )
                                                       ? ( char* )"a"
                                                       : ( char* )"c" );
                            p = append_str( p, ( char* )", (char*)" );
                            if ( n == 1 )
                                p = append_str( p, ( ( int )c < 8 )
                                                       ? ( char* )"a"
                                                       : ( char* )"c" );
                            else {
                                sprintf( buf, ( char* )"r%d", ( int )c & 7 );
                                p = append_str( p, buf );
                            }
                            break;
                        default:
                            p = append_str(
                                out, ( char* )"Unknown disassembler mode" );
                            break;
                    }
                    break;

                case 0xb:
                    n = read_nibble( ( *addr )++ );
                    if ( ( n < 2 ) || ( n > 7 ) )
                        break;

                    p = append_str( out,
                                    in_str_81b[ n + 16 * disassembler_mode ] );
                    break;

                case 0xc:
                case 0xd:
                case 0xe:
                case 0xf:
                    switch ( disassembler_mode ) {
                        case HP_MNEMONICS:
                            sprintf( buf, ( char* )"%sSRB",
                                     op_str_81[ ( n & 3 ) +
                                                4 * disassembler_mode ] );
                            p = append_str( out, buf );
                            break;
                        case CLASS_MNEMONICS:
                            p = append_str( out, ( char* )"lsr.w" );
                            p = append_tab( out );
                            p = append_str( p, ( char* )"#1, (char*)" );
                            p = append_str(
                                p, op_str_81[ ( n & 3 ) +
                                              4 * disassembler_mode ] );
                            break;
                        default:
                            p = append_str(
                                out, ( char* )"Unknown disassembler mode" );
                            break;
                    }
                    break;

                default:
                    break;
            }
            break;

        case 2:
            n = read_nibble( ( *addr )++ );
            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    if ( n == 0xf ) {
                        p = append_str( out, ( char* )"CLRHST" );
                    } else {
                        p = append_hst_bits( out, n );
                        p = append_str( p, ( char* )"=0" );
                    }
                    break;
                case CLASS_MNEMONICS:
                    p = append_str( out, ( char* )"clr.1" );
                    p = append_tab( out );
                    sprintf( buf, ( char* )"#%d, hst", n );
                    p = append_str( p, buf );
                    p = append_hst_bits( out, n );
                    break;
                default:
                    p = append_str( out, ( char* )"Unknown disassembler mode" );
                    break;
            }
            break;

        case 3:
            n = read_nibble( ( *addr )++ );
            pc = *addr;
            disp = read_int( addr, 2 );
            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    p = append_str( out, ( char* )"?" );
                    p = append_hst_bits( p, n );
                    p = append_str( p, ( char* )"=0" );
                    p = append_tab( out );
                    if ( disp != 0 ) {
                        p = append_str( p, ( char* )"GOYES (char*)" );
                        p = append_r_addr( p, &pc, disp, 2, 3 );
                        p = append_pc_comment( out, pc );
                    } else
                        p = append_str( p, ( char* )"RTNYES" );
                    break;
                case CLASS_MNEMONICS:
                    p = append_str( out, ( disp == 0 ) ? ( char* )"rt"
                                                       : ( char* )"b" );
                    p = append_str( p, ( char* )"eq.1" );
                    p = append_tab( out );
                    sprintf( buf, ( char* )"#%d, hst", n );
                    p = append_str( p, buf );
                    if ( disp != 0 ) {
                        p = append_str( p, ( char* )", (char*)" );
                        p = append_r_addr( p, &pc, disp, 2, 3 );
                        p = append_pc_comment( out, pc );
                    }
                    p = append_hst_bits( out, n );
                    break;
                default:
                    p = append_str( out, ( char* )"Unknown disassembler mode" );
                    break;
            }
            break;

        case 4:
        case 5:
            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    sprintf( buf, ( char* )"ST=%d", ( fn == 4 ) ? 0 : 1 );
                    p = append_str( out, buf );
                    p = append_tab( out );
                    p = append_imm_nibble( p, addr, 1 );
                    break;
                case CLASS_MNEMONICS:
                    p = append_str( out, ( fn == 4 ) ? ( char* )"bclr"
                                                     : ( char* )"bset" );
                    p = append_tab( out );
                    p = append_imm_nibble( p, addr, 1 );
                    p = append_str( p, ( char* )", st" );
                    break;
                default:
                    p = append_str( out, ( char* )"Unknown disassembler mode" );
                    break;
            }
            break;

        case 6:
        case 7:
            n = read_nibble( ( *addr )++ );
            pc = *addr;
            disp = read_int( addr, 2 );
            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    sprintf( buf, ( char* )"?ST=%d", ( fn == 6 ) ? 0 : 1 );
                    p = append_str( out, buf );
                    p = append_tab( out );
                    sprintf( buf, ( char* )"%d", n );
                    p = append_str( p, buf );
                    if ( disp != 0 ) {
                        p = append_str( p, ( char* )", GOYES (char*)" );
                        p = append_r_addr( p, &pc, disp, 2, 3 );
                        p = append_pc_comment( out, pc );
                    } else
                        p = append_str( p, ( char* )", RTNYES" );
                    break;
                case CLASS_MNEMONICS:
                    p = append_str( out, ( disp == 0 ) ? ( char* )"rt"
                                                       : ( char* )"b" );
                    p = append_str( p, ( fn == 6 ) ? ( char* )"bc"
                                                   : ( char* )"bs" );
                    p = append_tab( out );
                    sprintf( buf, ( char* )"#%d, st", n );
                    p = append_str( p, buf );
                    if ( disp != 0 ) {
                        p = append_str( p, ( char* )", (char*)" );
                        p = append_r_addr( p, &pc, disp, 2, 3 );
                        p = append_pc_comment( out, pc );
                    }
                    break;
                default:
                    p = append_str( out, ( char* )"Unknown disassembler mode" );
                    break;
            }
            break;

        case 8:
        case 9:
            n = read_nibble( ( *addr )++ );
            pc = *addr;
            disp = read_int( addr, 2 );
            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    sprintf( buf, ( char* )"?P%c", ( fn == 8 ) ? '#' : '=' );
                    p = append_str( out, buf );
                    p = append_tab( out );
                    sprintf( buf, ( char* )"%d", n );
                    p = append_str( p, buf );
                    if ( disp != 0 ) {
                        p = append_str( p, ( char* )", GOYES (char*)" );
                        p = append_r_addr( p, &pc, disp, 2, 3 );
                        p = append_pc_comment( out, pc );
                    } else
                        p = append_str( p, ( char* )", RTNYES" );
                    break;
                case CLASS_MNEMONICS:
                    p = append_str( out, ( disp == 0 ) ? ( char* )"rt"
                                                       : ( char* )"b" );
                    p = append_str( p, ( fn == 8 ) ? ( char* )"ne.1"
                                                   : ( char* )"eq.1" );
                    p = append_tab( out );
                    sprintf( buf, ( char* )"#%d, p", n );
                    p = append_str( p, buf );
                    if ( disp != 0 ) {
                        p = append_str( p, ( char* )", (char*)" );
                        p = append_r_addr( p, &pc, disp, 2, 3 );
                        p = append_pc_comment( out, pc );
                    }
                    break;
                default:
                    p = append_str( out, ( char* )"Unknown disassembler mode" );
                    break;
            }
            break;

        case 0xc:
        case 0xe:
            pc = *addr;
            if ( fn == 0xe )
                pc += 4;
            disp = read_int( addr, 4 );
            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    p = append_str( out, ( fn == 0xc ) ? ( char* )"GOLONG"
                                                       : ( char* )"GOSUBL" );
                    p = append_tab( out );
                    p = append_r_addr( p, &pc, disp, 4, ( fn == 0xc ) ? 2 : 6 );
                    p = append_pc_comment( out, pc );
                    break;
                case CLASS_MNEMONICS:
                    p = append_str( out, ( fn == 0xc ) ? ( char* )"bra.4"
                                                       : ( char* )"bsr.4" );
                    p = append_tab( out );
                    p = append_r_addr( p, &pc, disp, 4, ( fn == 0xc ) ? 2 : 6 );
                    p = append_pc_comment( out, pc );
                    break;
                default:
                    p = append_str( out, ( char* )"Unknown disassembler mode" );
                    break;
            }
            break;

        case 0xd:
        case 0xf:
            pc = read_int( addr, 5 );
            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    p = append_str( out, ( fn == 0xc ) ? ( char* )"GOVLNG"
                                                       : ( char* )"GOSBVL" );
                    p = append_tab( out );
                    p = append_addr( p, pc );
                    break;
                case CLASS_MNEMONICS:
                    p = append_str( out, ( fn == 0xc ) ? ( char* )"jmp"
                                                       : ( char* )"jsr" );
                    p = append_tab( out );
                    p = append_addr( p, pc );
                    break;
                default:
                    p = append_str( out, ( char* )"Unknown disassembler mode" );
                    break;
            }
            break;

        default:
            break;
    }
    return p;
}

word_20 disassemble( word_20 addr, char* out ) {
    word_4 n;
    word_4 fn;
    char* p = out;
    char c;
    char buf[ 20 ];
    word_20 disp, pc;

    switch ( n = read_nibble( addr++ ) ) {
        case 0:
            if ( ( n = read_nibble( addr++ ) ) != 0xe ) {
                p = append_str( out,
                                opcode_0_tbl[ n + 16 * disassembler_mode ] );
                break;
            }
            fn = read_nibble( addr++ );
            n = read_nibble( addr++ );
            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    sprintf( buf, op_str_0[ ( n & 7 ) + 8 * HP_MNEMONICS ],
                             ( n < 8 ) ? '&' : '!' );
                    p = append_str( out, buf );
                    p = append_tab( out );
                    p = append_field( p, fn );
                    break;
                case CLASS_MNEMONICS:
                    p = append_str( out, ( n < 8 ) ? ( char* )"and"
                                                   : ( char* )"or" );
                    p = append_field( p, fn );
                    p = append_tab( out );
                    p = append_str(
                        p, op_str_0[ ( n & 7 ) + 8 * CLASS_MNEMONICS ] );
                    break;
                default:
                    p = append_str( out, ( char* )"Unknown disassembler mode" );
                    break;
            }
            break;

        case 1:
            p = disasm_1( &addr, out );
            break;

        case 2:
            n = read_nibble( addr++ );
            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    sprintf( buf, ( char* )"P=%d", n );
                    p = append_str( out, buf );
                    break;
                case CLASS_MNEMONICS:
                    sprintf( buf, ( char* )"move.1  #%d, p", n );
                    p = append_str( out, buf );
                    break;
                default:
                    p = append_str( out, ( char* )"Unknown disassembler mode" );
                    break;
            }
            break;

        case 3:
            fn = read_nibble( addr++ );
            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    if ( fn < 5 ) {
                        sprintf( buf, ( char* )"LC(%d)", fn + 1 );
                    } else {
                        sprintf( buf, ( char* )"LCHEX" );
                    }
                    p = append_str( out, buf );
                    p = append_tab( out );
                    p = append_imm_nibble( p, &addr, fn + 1 );
                    break;
                case CLASS_MNEMONICS:
                    sprintf( buf, ( char* )"move.%d", fn + 1 );
                    p = append_str( out, buf );
                    p = append_tab( out );
                    p = append_imm_nibble( p, &addr, fn + 1 );
                    sprintf( buf, ( char* )", c.p" );
                    p = append_str( p, buf );
                    break;
                default:
                    p = append_str( out, ( char* )"Unknown disassembler mode" );
                    break;
            }
            break;

        case 4:
        case 5:
            pc = addr;
            disp = read_int( &addr, 2 );
            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    if ( disp == 2 ) {
                        p = append_str( out, ( char* )"NOP3" );
                        break;
                    }
                    sprintf( buf,
                             ( disp == 0 ) ? ( char* )"RTN%sC"
                                           : ( char* )"GO%sC",
                             ( n == 4 ) ? ( char* )"" : ( char* )"N" );
                    p = append_str( out, buf );
                    if ( disp != 0 ) {
                        p = append_tab( out );
                        p = append_r_addr( p, &pc, disp, 2, 1 );
                        p = append_pc_comment( out, pc );
                    }
                    break;

                case CLASS_MNEMONICS:
                    if ( disp == 2 ) {
                        p = append_str( out, ( char* )"nop3" );
                        break;
                    }
                    p = append_str( out, ( disp == 0 ) ? ( char* )"rtc"
                                                       : ( char* )"bc" );
                    p = append_str( p,
                                    ( n == 4 ) ? ( char* )"s" : ( char* )"c" );
                    if ( disp != 0 ) {
                        p = append_tab( out );
                        p = append_r_addr( p, &pc, disp, 2, 1 );
                        p = append_pc_comment( out, pc );
                    }
                    break;

                default:
                    p = append_str( out, ( char* )"Unknown disassembler mode" );
                    break;
            }
            break;

        case 6:
            pc = addr;
            disp = read_int( &addr, 3 );
            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    if ( disp == 3 ) {
                        p = append_str( out, ( char* )"NOP4" );
                        break;
                    }
                    if ( disp == 4 ) {
                        p = append_str( out, ( char* )"NOP5" );
                        break;
                    }
                    p = append_str( out, ( char* )"GOTO" );
                    p = append_tab( out );
                    p = append_r_addr( p, &pc, disp, 3, 1 );
                    p = append_pc_comment( out, pc );
                    break;

                case CLASS_MNEMONICS:
                    if ( disp == 3 ) {
                        p = append_str( out, ( char* )"nop4" );
                        break;
                    }
                    if ( disp == 4 ) {
                        p = append_str( out, ( char* )"nop5" );
                        break;
                    }
                    p = append_str( out, ( char* )"bra.3" );
                    p = append_tab( out );
                    p = append_r_addr( p, &pc, disp, 3, 1 );
                    p = append_pc_comment( out, pc );
                    break;

                default:
                    p = append_str( out, ( char* )"Unknown disassembler mode" );
                    break;
            }
            break;

        case 7:
            pc = addr + 3;
            disp = read_int( &addr, 3 );
            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    p = append_str( out, ( char* )"GOSUB" );
                    p = append_tab( out );
                    p = append_r_addr( p, &pc, disp, 3, 4 );
                    p = append_pc_comment( out, pc );
                    break;

                case CLASS_MNEMONICS:
                    p = append_str( out, ( char* )"bsr.3" );
                    p = append_tab( out );
                    p = append_r_addr( p, &pc, disp, 3, 4 );
                    p = append_pc_comment( out, pc );
                    break;

                default:
                    p = append_str( out, ( char* )"Unknown disassembler mode" );
                    break;
            }
            break;

        case 8:
            fn = read_nibble( addr ); /* PEEK */
            if ( fn != 0xa && fn != 0xb ) {
                p = disasm_8( &addr, out );
                break;
            }
            /* Fall through */

        case 9:
            fn = read_nibble( addr++ );
            if ( n == 8 ) {
                c = ( char )( ( fn == 0xa ) ? 0 : 1 );
                fn = 0xf;
            } else {
                c = ( char )( ( fn < 8 ) ? 0 : 1 );
                fn &= 7;
            }

            n = read_nibble( addr++ );
            pc = addr;
            disp = read_int( &addr, 2 );

            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    if ( ( c == 0 ) && ( n >= 8 ) )
                        sprintf( buf,
                                 op_str_9[ ( n & 3 ) + 8 * HP_MNEMONICS + 4 ],
                                 in_str_9[ ( ( n >> 2 ) & 3 ) + 4 * c +
                                           8 * HP_MNEMONICS ] );
                    else
                        sprintf( buf, op_str_9[ ( n & 3 ) + 8 * HP_MNEMONICS ],
                                 in_str_9[ ( ( n >> 2 ) & 3 ) + 4 * c +
                                           8 * HP_MNEMONICS ] );
                    p = append_str( out, buf );
                    p = append_tab( out );
                    p = append_field( p, fn );
                    p = append_str( p, ( char* )", (char*)" );
                    p = append_str( p, ( disp == 0 )
                                           ? ( char* )"RTNYES"
                                           : ( char* )"GOYES (char*)" );
                    if ( disp != 0 ) {
                        p = append_r_addr( p, &pc, disp, 2, 3 );
                        p = append_pc_comment( out, pc );
                    }
                    break;

                case CLASS_MNEMONICS:
                    p = append_str( out, ( disp == 0 ) ? ( char* )"rt"
                                                       : ( char* )"b" );
                    p = append_str( p, in_str_9[ ( ( n >> 2 ) & 3 ) + 4 * c +
                                                 8 * CLASS_MNEMONICS ] );
                    p = append_field( p, fn );
                    p = append_tab( out );
                    if ( ( c == 0 ) && ( n >= 8 ) )
                        p = append_str(
                            p,
                            op_str_9[ ( n & 3 ) + 8 * CLASS_MNEMONICS + 4 ] );
                    else
                        p = append_str(
                            p, op_str_9[ ( n & 3 ) + 8 * CLASS_MNEMONICS ] );
                    if ( disp != 0 ) {
                        p = append_str( p, ( char* )", (char*)" );
                        p = append_r_addr( p, &pc, disp, 2, 3 );
                        p = append_pc_comment( out, pc );
                    }
                    break;

                default:
                    p = append_str( out, ( char* )"Unknown disassembler mode" );
                    break;
            }
            break;

        default:
            switch ( n ) {
                case 0xa:
                    fn = read_nibble( addr++ );
                    c = ( char )( ( fn < 8 ) ? 0 : 1 );
                    fn &= 7;
                    disp = 0xa;
                    break;
                case 0xb:
                    fn = read_nibble( addr++ );
                    c = ( char )( ( fn < 8 ) ? 0 : 1 );
                    fn &= 7;
                    disp = 0xb;
                    break;
                case 0xc:
                case 0xd:
                    fn = 0xf;
                    c = ( char )( n & 1 );
                    disp = 0xa;
                    break;
                case 0xe:
                case 0xf:
                    fn = 0xf;
                    c = ( char )( n & 1 );
                    disp = 0xb;
                    break;
                default:
                    fn = 0;
                    disp = 0;
                    c = 0;
                    break;
            }

            n = read_nibble( addr++ );
            pc = 0;

            switch ( disp ) {
                case 0xa:
                    switch ( disassembler_mode ) {
                        case HP_MNEMONICS:
                            if ( c == 0 ) {
                                if ( n < 0xc ) {
                                    p = ( char* )"+";
                                } else {
                                    p = ( char* )"%c=%c-1";
                                    pc = 2;
                                }
                            } else {
                                if ( n < 4 ) {
                                    p = ( char* )"%c=0";
                                    pc = 1;
                                } else if ( n >= 0xc ) {
                                    p = ( char* )"%c%cEX";
                                    pc = 3;
                                } else {
                                    p = ( char* )"%c=%c";
                                    pc = 3;
                                }
                            }
                            break;

                        case CLASS_MNEMONICS:
                            if ( c == 0 ) {
                                if ( n < 0xc ) {
                                    p = ( char* )"add";
                                } else {
                                    p = ( char* )"dec";
                                    pc = 1;
                                }
                            } else {
                                if ( n < 4 ) {
                                    p = ( char* )"clr";
                                    pc = 1;
                                } else if ( n >= 0xc ) {
                                    p = ( char* )"exg";
                                } else {
                                    p = ( char* )"move";
                                    if ( n < 8 )
                                        n -= 4;
                                }
                            }
                            break;

                        default:
                            p = append_str(
                                out, ( char* )"Unknown disassembler mode" );
                            return addr;
                    }
                    break;

                case 0xb:
                    switch ( disassembler_mode ) {
                        case HP_MNEMONICS:
                            if ( c == 0 ) {
                                if ( n >= 0xc ) {
                                    p = ( char* )"-";
                                } else if ( ( n >= 4 ) && ( n <= 7 ) ) {
                                    p = ( char* )"%c=%c+1";
                                    pc = 2;
                                    n -= 4;
                                } else {
                                    p = ( char* )"-";
                                }
                            } else {
                                if ( n < 4 ) {
                                    p = ( char* )"%cSL";
                                    pc = 1;
                                } else if ( n < 8 ) {
                                    p = ( char* )"%cSR";
                                    pc = 1;
                                } else if ( n < 0xc ) {
                                    p = ( char* )"%c=%c-1";
                                    pc = 2;
                                } else {
                                    p = ( char* )"%c=-%c-1";
                                    pc = 2;
                                }
                            }
                            break;

                        case CLASS_MNEMONICS:
                            if ( c == 0 ) {
                                if ( n >= 0xc ) {
                                    p = ( char* )"subr";
                                } else if ( ( n >= 4 ) && ( n <= 7 ) ) {
                                    p = ( char* )"inc";
                                    pc = 1;
                                    n -= 4;
                                } else {
                                    p = ( char* )"sub";
                                }
                            } else {
                                pc = 1;
                                if ( n < 4 ) {
                                    p = ( char* )"lsl";
                                } else if ( n < 8 ) {
                                    p = ( char* )"lsr";
                                } else if ( n < 0xc ) {
                                    p = ( char* )"neg";
                                } else {
                                    p = ( char* )"not";
                                }
                            }
                            break;

                        default:
                            p = append_str(
                                out, ( char* )"Unknown disassembler mode" );
                            return addr;
                    }
                    break;
            }

            switch ( disassembler_mode ) {
                case HP_MNEMONICS:

                    if ( pc == 0 ) {
                        sprintf( buf, op_str_af[ n + 16 * HP_MNEMONICS ], p );
                    } else if ( pc == 1 ) {
                        sprintf( buf, p, ( n & 3 ) + 'A' );
                    } else if ( pc == 2 ) {
                        sprintf( buf, p, ( n & 3 ) + 'A', ( n & 3 ) + 'A' );
                    } else {
                        sprintf( buf, p, hp_reg_1_af[ n ], hp_reg_2_af[ n ] );
                    }
                    p = append_str( out, buf );
                    p = append_tab( out );
                    p = append_field( p, fn );
                    break;

                case CLASS_MNEMONICS:

                    p = append_str( out, p );
                    p = append_field( p, fn );
                    p = append_tab( out );
                    if ( pc == 1 ) {
                        sprintf( buf, ( char* )"%c", ( n & 3 ) + 'a' );
                        p = append_str( p, buf );
                    } else {
                        p = append_str( p,
                                        op_str_af[ n + 16 * CLASS_MNEMONICS ] );
                    }
                    break;

                default:
                    p = append_str( p, ( char* )"Unknown disassembler mode" );
                    break;
            }

            break;
    }
    *p = '\0';
    return addr;
}
