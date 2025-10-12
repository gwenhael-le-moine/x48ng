#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <readline/history.h>
#include <readline/readline.h>

#include "debugger.h"
#include "emulate.h"
#include "init.h"
#include "options.h"
#include "persistence.h"
#include "romio.h"
#include "timers.h"

#include "ui4x/common.h"

#define MAX_ARGS 16

#define TAB_SKIP 8

/*
 * Addresses in SX ROM
 */
#define ROMPTAB_SX 0x707d9
#define ROMPTAB_GX 0x809a3

/*
 * Object Prologs
 */
#define DOBINT 0x02911    /* System Binary	*/
#define DOREAL 0x02933    /* Real			*/
#define DOEREL 0x02955    /* Extended Real	*/
#define DOCMP 0x02977     /* Complex		*/
#define DOECMP 0x0299d    /* Extended Complex	*/
#define DOCHAR 0x029bf    /* Character		*/
#define DOARRY 0x029e8    /* Array		*/
#define DOLNKARRY 0x02a0a /* Linked Array		*/
#define DOCSTR 0x02a2c    /* String		*/
#define DOHSTR 0x02a4e    /* Binary Integer	*/
#define DOLIST 0x02a74    /* List			*/
#define DORRP 0x02a96     /* Directory		*/
#define DOSYMB 0x02ab8    /* Algebraic		*/
#define DOEXT 0x02ada     /* Unit			*/
#define DOTAG 0x02afc     /* Tagged		*/
#define DOGROB 0x02b1e    /* Graphic Object	*/
#define DOLIB 0x02b40     /* Library		*/
#define DOBAK 0x02b62     /* Backup		*/
#define DOEXT0 0x02b88    /* Library Data		*/
#define DOACPTR 0x02baa   /*			*/
#define DOEXT2 0x02bcc    /*			*/
#define DOEXT3 0x02bee    /*			*/
#define DOEXT4 0x02c10    /*			*/
#define DOCOL 0x02d9d     /* Program		*/
#define DOCODE 0x02dcc    /* Code			*/
#define DOIDNT 0x02e48    /* Global Name		*/
#define DOLAM 0x02e6d     /* Local Name		*/
#define DOROMP 0x02e92    /* XLib Name		*/

/*
 * Terminates composite objects
 */
#define SEMI 0x0312b /* Semi			*/

/*
 * Unit Operators
 */
#define UM_MUL 0x10b5e /* Unit Operator *	*/
#define UM_DIV 0x10b68 /* Unit Operator /	*/
#define UM_POW 0x10b72 /* Unit Operator ^	*/
#define UM_PRE 0x10b7c /* Unit Operator prefix */
#define UM_END 0x10b86 /* Unit Operator _	*/

int enter_debugger = 0;
bool in_debugger = false;
int exec_flags = 0;

static bool continue_flag;
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

#define MAX_BREAKPOINTS 32
int num_bkpts;

struct breakpoint {
    Address addr;
    Address end_addr;
    int flags;
} bkpt_tbl[ MAX_BREAKPOINTS + 1 ];

static int disassembler_mode = CLASS_MNEMONICS;

static const char* mode_name[] = { ( char* )"HP", ( char* )"class" };

static char* hex[] = {
    ( char* )"0123456789ABCDEF",
    ( char* )"0123456789abcdef",
};

static char* opcode_0_tbl[ 32 ] = {
    /*
     * HP Mnemonics
     */
    ( char* )"RTNSXM", ( char* )"RTN", ( char* )"RTNSC", ( char* )"RTNCC", ( char* )"SETHEX", ( char* )"SETDEC", ( char* )"RSTK=C",
    ( char* )"C=RSTK", ( char* )"CLRST", ( char* )"C=ST", ( char* )"ST=C", ( char* )"CSTEX", ( char* )"P=P+1", ( char* )"P=P-1",
    ( char* )"(NULL)", ( char* )"RTI",
    /*
     * Class Mnemonics
     */
    ( char* )"rtnsxm", ( char* )"rtn", ( char* )"rtnsc", ( char* )"rtncc", ( char* )"sethex", ( char* )"setdec", ( char* )"push",
    ( char* )"pop", ( char* )"clr.3   st", ( char* )"move.3  st, c", ( char* )"move.3  c, st", ( char* )"exg.3   c, st",
    ( char* )"inc.1   p", ( char* )"dec.1   p", ( char* )"(null)", ( char* )"rti" };

static char* op_str_0[ 16 ] = {
    /*
     * HP Mnemonics
     */
    ( char* )"A=A%cB", ( char* )"B=B%cC", ( char* )"C=C%cA", ( char* )"D=D%cC", ( char* )"B=B%cA", ( char* )"C=C%cB", ( char* )"A=A%cC",
    ( char* )"C=C%cD",
    /*
     * Class Mnemonics
     */
    ( char* )"b, a", ( char* )"c, b", ( char* )"a, c", ( char* )"c, d", ( char* )"a, b", ( char* )"b, c", ( char* )"c, a",
    ( char* )"d, c" };

static char* op_str_1[ 16 ] = {
    /*
     * HP Mnemonics
     */
    ( char* )"DAT0=A", ( char* )"DAT1=A", ( char* )"A=DAT0", ( char* )"A=DAT1", ( char* )"DAT0=C", ( char* )"DAT1=C", ( char* )"C=DAT0",
    ( char* )"C=DAT1",
    /*
     * Class Mnemonics
     */
    ( char* )"a, (d0)", ( char* )"a, (d1)", ( char* )"(d0), a", ( char* )"(d1), a", ( char* )"c, (d0)", ( char* )"c, (d1)",
    ( char* )"(d0), c", ( char* )"(d1), c" };

static char* in_str_80[ 32 ] = {
    /*
     * HP Mnemonics
     */
    ( char* )"OUT=CS", ( char* )"OUT=C", ( char* )"A=IN", ( char* )"C=IN", ( char* )"UNCNFG", ( char* )"CONFIG", ( char* )"C=ID",
    ( char* )"SHUTDN", NULL, ( char* )"C+P+1", ( char* )"RESET", ( char* )"BUSCC", NULL, NULL, ( char* )"SREQ?", NULL,
    /*
     * Class Mnemonics
     */
    ( char* )"move.s  c, out", ( char* )"move.3  c, out", ( char* )"move.4  in, a", ( char* )"move.4  in, c", ( char* )"uncnfg",
    ( char* )"config", ( char* )"c=id", ( char* )"shutdn", NULL, ( char* )"add.a   p+1, c", ( char* )"reset", ( char* )"buscc", NULL, NULL,
    ( char* )"sreq?", NULL };

static char* in_str_808[ 32 ] = {
    /*
     * HP Mnemonics
     */
    ( char* )"INTON", NULL, NULL, ( char* )"BUSCB", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, ( char* )"PC=(A)", ( char* )"BUSCD",
    ( char* )"PC=(C)", ( char* )"INTOFF",
    /*
     * Class Mnemonics
     */
    ( char* )"inton", NULL, NULL, ( char* )"buscb", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, ( char* )"jmp     (a)",
    ( char* )"buscd", ( char* )"jmp     (c)", ( char* )"intoff" };

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
    ( char* )"=", ( char* )"#", ( char* )"=", ( char* )"#", ( char* )">", ( char* )"<", ( char* )">=", ( char* )"<=",
    /*
     * Class Mnemonics
     */
    ( char* )"eq", ( char* )"ne", ( char* )"eq", ( char* )"ne", ( char* )"gt", ( char* )"lt", ( char* )"ge", ( char* )"le" };

static char* op_str_9[ 16 ] = {
    /*
     * HP Mnemonics
     */
    ( char* )"?A%sB", ( char* )"?B%sC", ( char* )"?C%sA", ( char* )"?D%sC", ( char* )"?A%s0", ( char* )"?B%s0", ( char* )"?C%s0",
    ( char* )"?D%s0",
    /*
     * Class Mnemonics
     */
    ( char* )"a, b", ( char* )"b, c", ( char* )"c, a", ( char* )"d, c", ( char* )"a, 0", ( char* )"b, 0", ( char* )"c, 0",
    ( char* )"d, 0" };

static char* op_str_af[ 32 ] = {
    /*
     * HP Mnemonics
     */
    ( char* )"A=A%sB", ( char* )"B=B%sC", ( char* )"C=C%sA", ( char* )"D=D%sC", ( char* )"A=A%sA", ( char* )"B=B%sB", ( char* )"C=C%sC",
    ( char* )"D=D%sD", ( char* )"B=B%sA", ( char* )"C=C%sB", ( char* )"A=A%sC", ( char* )"C=C%sD", ( char* )"A=B%sA", ( char* )"B=C%sB",
    ( char* )"C=A%sC", ( char* )"D=C%sD",
    /*
     * Class Mnemonics
     */
    ( char* )"b, a", ( char* )"c, b", ( char* )"a, c", ( char* )"c, d", ( char* )"a, a", ( char* )"b, b", ( char* )"c, c", ( char* )"d, d",
    ( char* )"a, b", ( char* )"b, c", ( char* )"c, a", ( char* )"d, c", ( char* )"b, a", ( char* )"c, b", ( char* )"a, c",
    ( char* )"c, d" };

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

typedef struct hp_real {
    Address x;
    word_32 ml;
    word_32 mh;
    Nibble m;
    Bit s;
} hp_real;

typedef struct objfunc {
    char* name;
    short length;
    Address prolog;
    char* ( *func )( Address* addr, char* string );
} objfunc_t;

objfunc_t objects[];

typedef struct trans_tbl_t {
    unsigned char hp48_char;
    char* trans;
} trans_tbl_t;

static trans_tbl_t hp48_trans_tbl[ 256 ] = {
    {0,    ( char* )"\\0"  },
    {1,    ( char* )"\\001"},
    {2,    ( char* )"\\002"},
    {3,    ( char* )"\\003"},
    {4,    ( char* )"\\004"},
    {5,    ( char* )"\\005"},
    {6,    ( char* )"\\006"},
    {7,    ( char* )"\\007"},
    {8,    ( char* )"\\b"  },
    {9,    ( char* )"\\t"  },
    {10,   ( char* )"\\n"  },
    {11,   ( char* )"\\011"},
    {12,   ( char* )"\\f"  },
    {13,   ( char* )"\\r"  },
    {14,   ( char* )"\\014"},
    {15,   ( char* )"\\015"},
    {16,   ( char* )"\\016"},
    {17,   ( char* )"\\017"},
    {18,   ( char* )"\\018"},
    {19,   ( char* )"\\019"},
    {20,   ( char* )"\\020"},
    {21,   ( char* )"\\021"},
    {22,   ( char* )"\\022"},
    {23,   ( char* )"\\023"},
    {24,   ( char* )"\\024"},
    {25,   ( char* )"\\025"},
    {26,   ( char* )"\\026"},
    {27,   ( char* )"\\027"},
    {28,   ( char* )"\\028"},
    {29,   ( char* )"\\029"},
    {30,   ( char* )"\\030"},
    {31,   ( char* )"\\031"},
    {' ',  0               },
    {'!',  0               },
    {'"',  0               },
    {'#',  0               },
    {'$',  0               },
    {'%',  0               },
    {'&',  0               },
    {'\'', 0               },
    {'(',  0               },
    {')',  0               },
    {'*',  0               },
    {'+',  0               },
    {',',  0               },
    {'-',  0               },
    {'.',  0               },
    {'/',  0               },
    {'0',  0               },
    {'1',  0               },
    {'2',  0               },
    {'3',  0               },
    {'4',  0               },
    {'5',  0               },
    {'6',  0               },
    {'7',  0               },
    {'8',  0               },
    {'9',  0               },
    {':',  0               },
    {';',  0               },
    {'<',  0               },
    {'=',  0               },
    {'>',  0               },
    {'?',  0               },
    {'@',  0               },
    {'A',  0               },
    {'B',  0               },
    {'C',  0               },
    {'D',  0               },
    {'E',  0               },
    {'F',  0               },
    {'G',  0               },
    {'H',  0               },
    {'I',  0               },
    {'J',  0               },
    {'K',  0               },
    {'L',  0               },
    {'M',  0               },
    {'N',  0               },
    {'O',  0               },
    {'P',  0               },
    {'Q',  0               },
    {'R',  0               },
    {'S',  0               },
    {'T',  0               },
    {'U',  0               },
    {'V',  0               },
    {'W',  0               },
    {'X',  0               },
    {'Y',  0               },
    {'Z',  0               },
    {'[',  0               },
    {'\\', 0               },
    {']',  0               },
    {'^',  0               },
    {'_',  0               },
    {'`',  0               },
    {'a',  0               },
    {'b',  0               },
    {'c',  0               },
    {'d',  0               },
    {'e',  0               },
    {'f',  0               },
    {'g',  0               },
    {'h',  0               },
    {'i',  0               },
    {'j',  0               },
    {'k',  0               },
    {'l',  0               },
    {'m',  0               },
    {'n',  0               },
    {'o',  0               },
    {'p',  0               },
    {'q',  0               },
    {'r',  0               },
    {'s',  0               },
    {'t',  0               },
    {'u',  0               },
    {'v',  0               },
    {'w',  0               },
    {'x',  0               },
    {'y',  0               },
    {'z',  0               },
    {'{',  0               },
    {'|',  0               },
    {'}',  0               },
    {'~',  0               },
    {127,  ( char* )"\\127"},
    {128,  ( char* )"\\<)" },
    {129,  ( char* )"\\x-" },
    {130,  ( char* )"\\.V" },
    {131,  ( char* )"\\v/" },
    {132,  ( char* )"\\.S" },
    {133,  ( char* )"\\GS" },
    {134,  ( char* )"\\|>" },
    {135,  ( char* )"\\pi" },
    {136,  ( char* )"\\.d" },
    {137,  ( char* )"\\<=" },
    {138,  ( char* )"\\>=" },
    {139,  ( char* )"\\=/" },
    {140,  ( char* )"\\Ga" },
    {141,  ( char* )"\\->" },
    {142,  ( char* )"\\<-" },
    {143,  ( char* )"\\|v" },
    {144,  ( char* )"\\|^" },
    {145,  ( char* )"\\Gg" },
    {146,  ( char* )"\\Gd" },
    {147,  ( char* )"\\Ge" },
    {148,  ( char* )"\\Gn" },
    {149,  ( char* )"\\Gh" },
    {150,  ( char* )"\\Gl" },
    {151,  ( char* )"\\Gr" },
    {152,  ( char* )"\\Gs" },
    {153,  ( char* )"\\Gt" },
    {154,  ( char* )"\\Gw" },
    {155,  ( char* )"\\GD" },
    {156,  ( char* )"\\PI" },
    {157,  ( char* )"\\GW" },
    {158,  ( char* )"\\[]" },
    {159,  ( char* )"\\oo" },
    {160,  ( char* )"\\160"},
    {161,  ( char* )"\\161"},
    {162,  ( char* )"\\162"},
    {163,  ( char* )"\\163"},
    {164,  ( char* )"\\164"},
    {165,  ( char* )"\\165"},
    {166,  ( char* )"\\166"},
    {167,  ( char* )"\\167"},
    {168,  ( char* )"\\168"},
    {169,  ( char* )"\\169"},
    {170,  ( char* )"\\170"},
    {171,  ( char* )"\\<<" },
    {172,  ( char* )"\\172"},
    {173,  ( char* )"\\173"},
    {174,  ( char* )"\\174"},
    {175,  ( char* )"\\175"},
    {176,  ( char* )"\\^o" },
    {177,  ( char* )"\\177"},
    {178,  ( char* )"\\178"},
    {179,  ( char* )"\\179"},
    {180,  ( char* )"\\180"},
    {181,  ( char* )"\\Gm" },
    {182,  ( char* )"\\182"},
    {183,  ( char* )"\\183"},
    {184,  ( char* )"\\184"},
    {185,  ( char* )"\\185"},
    {186,  ( char* )"\\186"},
    {187,  ( char* )"\\>>" },
    {188,  ( char* )"\\188"},
    {189,  ( char* )"\\189"},
    {190,  ( char* )"\\190"},
    {191,  ( char* )"\\191"},
    {192,  ( char* )"\\192"},
    {193,  ( char* )"\\193"},
    {194,  ( char* )"\\194"},
    {195,  ( char* )"\\195"},
    {196,  ( char* )"\\196"},
    {197,  ( char* )"\\197"},
    {198,  ( char* )"\\198"},
    {199,  ( char* )"\\199"},
    {200,  ( char* )"\\200"},
    {201,  ( char* )"\\201"},
    {202,  ( char* )"\\202"},
    {203,  ( char* )"\\203"},
    {204,  ( char* )"\\204"},
    {205,  ( char* )"\\205"},
    {206,  ( char* )"\\206"},
    {207,  ( char* )"\\207"},
    {208,  ( char* )"\\208"},
    {209,  ( char* )"\\209"},
    {210,  ( char* )"\\210"},
    {211,  ( char* )"\\211"},
    {212,  ( char* )"\\212"},
    {213,  ( char* )"\\213"},
    {214,  ( char* )"\\214"},
    {215,  ( char* )"\\.x" },
    {216,  ( char* )"\\O/" },
    {217,  ( char* )"\\217"},
    {218,  ( char* )"\\218"},
    {219,  ( char* )"\\219"},
    {220,  ( char* )"\\220"},
    {221,  ( char* )"\\221"},
    {222,  ( char* )"\\222"},
    {223,  ( char* )"\\223"},
    {224,  ( char* )"\\224"},
    {225,  ( char* )"\\225"},
    {226,  ( char* )"\\226"},
    {227,  ( char* )"\\227"},
    {228,  ( char* )"\\228"},
    {229,  ( char* )"\\229"},
    {230,  ( char* )"\\230"},
    {231,  ( char* )"\\231"},
    {232,  ( char* )"\\232"},
    {233,  ( char* )"\\233"},
    {234,  ( char* )"\\234"},
    {235,  ( char* )"\\235"},
    {236,  ( char* )"\\236"},
    {237,  ( char* )"\\237"},
    {238,  ( char* )"\\238"},
    {239,  ( char* )"\\239"},
    {240,  ( char* )"\\240"},
    {241,  ( char* )"\\241"},
    {242,  ( char* )"\\242"},
    {243,  ( char* )"\\243"},
    {244,  ( char* )"\\244"},
    {245,  ( char* )"\\245"},
    {246,  ( char* )"\\246"},
    {247,  ( char* )"\\:-" },
    {248,  ( char* )"\\248"},
    {249,  ( char* )"\\249"},
    {250,  ( char* )"\\250"},
    {251,  ( char* )"\\251"},
    {252,  ( char* )"\\252"},
    {253,  ( char* )"\\253"},
    {254,  ( char* )"\\254"},
    {255,  ( char* )"\\255"}
};

static char* append_str( char* buf, const char* str )
{
    while ( ( *buf = *str++ ) )
        buf++;
    return buf;
}

static char* append_tab( char* buf )
{
    int n;
    char* p;

    n = TAB_SKIP - ( strlen( buf ) % TAB_SKIP );
    p = &buf[ strlen( buf ) ];
    while ( n-- )
        *p++ = ' ';
    *p = '\0';
    return p;
}

static int read_int( Address* addr, int n )
{
    int i, t;

    for ( i = 0, t = 0; i < n; i++ )
        t |= bus_fetch_nibble( ( *addr )++ ) << ( i * 4 );
    return t;
}

static char* append_tab_16( char* buf )
{
    int n;
    char* p;

    n = 16 - ( strlen( buf ) % 16 );
    p = &buf[ strlen( buf ) ];
    while ( n-- )
        *p++ = ' ';
    *p = '\0';
    return p;
}

static char* append_field( char* buf, Nibble fn )
{
    buf = append_str( buf, field_tbl[ fn + 16 * disassembler_mode ] );
    return buf;
}

static char* append_imm_nibble( char* buf, Address* addr, int n )
{
    int i;
    char t[ 16 ];

    if ( disassembler_mode == CLASS_MNEMONICS ) {
        *buf++ = '#';
        if ( n > 1 )
            *buf++ = '$';
    }
    if ( n > 1 ) {
        for ( i = 0; i < n; i++ )
            t[ i ] = hex[ disassembler_mode ][ bus_fetch_nibble( ( *addr )++ ) ];
        for ( i = n - 1; i >= 0; i-- ) {
            *buf++ = t[ i ];
        }
        *buf = '\0';
    } else {
        sprintf( t, ( char* )"%d", bus_fetch_nibble( ( *addr )++ ) );
        buf = append_str( buf, t );
    }
    return buf;
}

static char* append_addr( char* buf, Address addr )
{
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

static char* append_r_addr( char* buf, Address* pc, long disp, int n, int offset )
{
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

static char* append_pc_comment( char* buf, Address pc )
{
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

static char* append_hst_bits( char* buf, int n )
{
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

static char* disasm_1( Address* addr, char* out )
{
    Nibble n;
    Nibble fn;
    char* p;
    char buf[ 20 ];
    char c;

    p = out;
    switch ( ( n = bus_fetch_nibble( ( *addr )++ ) ) ) {
        case 0:
        case 1:
            fn = bus_fetch_nibble( ( *addr )++ );
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
            fn = bus_fetch_nibble( ( *addr )++ );
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
            n = bus_fetch_nibble( ( *addr )++ );
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
                    p = append_str( out, ( n & 2 ) ? ( char* )"exg." : ( char* )"move." );
                    p = append_str( p, ( n < 8 ) ? ( char* )"a" : ( char* )"4" );
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
            fn = bus_fetch_nibble( ( *addr )++ );
            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    p = append_str( out, op_str_1[ ( fn & 7 ) + 8 * disassembler_mode ] );
                    p = append_tab( out );
                    if ( n == 4 ) {
                        p = append_str( p, ( fn < 8 ) ? ( char* )"A" : ( char* )"B" );
                    } else {
                        n = bus_fetch_nibble( ( *addr )++ );
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
                        p = append_str( p, ( fn < 8 ) ? ( char* )"a" : ( char* )"b" );
                    } else {
                        n = bus_fetch_nibble( ( *addr )++ );
                        if ( fn < 8 ) {
                            p = append_field( p, n );
                        } else {
                            sprintf( buf, ( char* )".%d", n + 1 );
                            p = append_str( p, buf );
                        }
                    }
                    p = append_tab( out );
                    p = append_str( p, op_str_1[ ( fn & 7 ) + 8 * disassembler_mode ] );
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
            fn = bus_fetch_nibble( *addr++ );
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

static char* disasm_8( Address* addr, char* out )
{
    Nibble n;
    Nibble fn;
    char* p = out;
    char c;
    char buf[ 20 ];
    Address disp, pc;

    fn = bus_fetch_nibble( ( *addr )++ );
    switch ( fn ) {
        case 0:
            n = bus_fetch_nibble( ( *addr )++ );
            if ( NULL != ( p = ( char* )in_str_80[ n + 16 * disassembler_mode ] ) ) {
                p = append_str( out, p );
                return p;
            }
            switch ( n ) {
                case 8:
                    fn = bus_fetch_nibble( ( *addr )++ );
                    if ( NULL != ( p = ( char* )in_str_808[ fn + 16 * disassembler_mode ] ) ) {
                        p = append_str( out, p );
                        return p;
                    }
                    switch ( fn ) {
                        case 1:
                            n = bus_fetch_nibble( ( *addr )++ );
                            if ( n == 0 ) {
                                switch ( disassembler_mode ) {
                                    case HP_MNEMONICS:
                                        p = append_str( out, ( char* )"RSI" );
                                        break;
                                    case CLASS_MNEMONICS:
                                        p = append_str( out, ( char* )"rsi" );
                                        break;
                                    default:
                                        p = append_str( out, ( char* )"Unknown "
                                                                      "disassembler mode" );
                                        break;
                                }
                            }
                            break;
                        case 2:
                            n = bus_fetch_nibble( ( *addr )++ );
                            switch ( disassembler_mode ) {
                                case HP_MNEMONICS:
                                    if ( n < 5 ) {
                                        sprintf( buf, ( char* )"LA(%d)", n + 1 );
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
                                    p = append_str( out, ( char* )"Unknown disassembler mode" );
                                    break;
                            }
                            break;

                        case 4:
                        case 5:
                        case 8:
                        case 9:

                            switch ( disassembler_mode ) {
                                case HP_MNEMONICS:
                                    sprintf( buf, ( char* )"%cBIT=%d", ( fn & 8 ) ? 'C' : 'A', ( fn & 1 ) ? 1 : 0 );
                                    p = append_str( out, buf );
                                    p = append_tab( out );
                                    p = append_imm_nibble( p, addr, 1 );
                                    break;
                                case CLASS_MNEMONICS:
                                    p = append_str( out, ( fn & 1 ) ? ( char* )"bset" : ( char* )"bclr" );
                                    p = append_tab( out );
                                    p = append_imm_nibble( p, addr, 1 );
                                    p = append_str( p, ( fn & 8 ) ? ( char* )", c" : ( char* )", a" );
                                    break;
                                default:
                                    p = append_str( out, ( char* )"Unknown disassembler mode" );
                                    break;
                            }
                            break;

                        case 6:
                        case 7:
                        case 0xa:
                        case 0xb:

                            n = bus_fetch_nibble( ( *addr )++ );
                            pc = *addr;
                            disp = read_int( addr, 2 );

                            switch ( disassembler_mode ) {
                                case HP_MNEMONICS:
                                    c = ( char )( ( fn < 0xa ) ? 'A' : 'C' );
                                    sprintf( buf, ( char* )"?%cBIT=%d", c, ( fn & 1 ) ? 1 : 0 );
                                    p = append_str( out, buf );
                                    p = append_tab( out );
                                    sprintf( buf, ( char* )"%d", n );
                                    p = append_str( p, buf );
                                    if ( disp != 0 ) {
                                        p = append_str( p, ( char* )", GOYES (char*)" );
                                        p = append_r_addr( p, &pc, disp, 2, 5 );
                                        p = append_pc_comment( out, pc );
                                    } else
                                        p = append_str( p, ( char* )", RTNYES" );
                                    break;
                                case CLASS_MNEMONICS:
                                    c = ( char )( ( fn < 0xa ) ? 'a' : 'c' );
                                    p = append_str( out, ( disp == 0 ) ? ( char* )"rt" : ( char* )"b" );
                                    p = append_str( p, ( fn & 1 ) ? ( char* )"bs" : ( char* )"bc" );
                                    p = append_tab( out );
                                    sprintf( buf, ( char* )"#%d, %c", n, c );
                                    p = append_str( p, buf );
                                    if ( disp != 0 ) {
                                        p = append_str( p, ( char* )", (char*)" );
                                        p = append_r_addr( p, &pc, disp, 2, 5 );
                                        p = append_pc_comment( out, pc );
                                    }
                                    break;
                                default:
                                    p = append_str( out, ( char* )"Unknown disassembler mode" );
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
                    fn = bus_fetch_nibble( ( *addr )++ );
                    switch ( disassembler_mode ) {
                        case HP_MNEMONICS:
                            sprintf( buf, ( n == 0xf ) ? ( char* )"%c%cEX" : ( char* )"%c=%c", ( n == 0xd ) ? 'P' : 'C',
                                     ( n == 0xd ) ? 'C' : 'P' );
                            p = append_str( out, buf );
                            p = append_tab( out );
                            sprintf( buf, ( char* )"%d", fn );
                            p = append_str( p, buf );
                            break;
                        case CLASS_MNEMONICS:
                            p = append_str( out, ( n == 0xf ) ? ( char* )"exg.1" : ( char* )"move.1" );
                            p = append_tab( out );
                            sprintf( buf, ( n == 0xd ) ? ( char* )"p, c.%d" : ( char* )"c.%d, p", fn );
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
            break;

        case 1:
            switch ( n = bus_fetch_nibble( ( *addr )++ ) ) {
                case 0:
                case 1:
                case 2:
                case 3:
                    switch ( disassembler_mode ) {
                        case HP_MNEMONICS:
                            sprintf( buf, ( char* )"%sSLC", op_str_81[ ( n & 3 ) + 4 * disassembler_mode ] );
                            p = append_str( out, buf );
                            break;
                        case CLASS_MNEMONICS:
                            p = append_str( out, ( char* )"rol.w" );
                            p = append_tab( out );
                            p = append_str( p, ( char* )"#4, (char*)" );
                            p = append_str( p, op_str_81[ ( n & 3 ) + 4 * disassembler_mode ] );
                            break;
                        default:
                            p = append_str( out, ( char* )"Unknown disassembler mode" );
                            break;
                    }
                    break;

                case 4:
                case 5:
                case 6:
                case 7:
                    switch ( disassembler_mode ) {
                        case HP_MNEMONICS:
                            sprintf( buf, ( char* )"%sSRC", op_str_81[ ( n & 3 ) + 4 * disassembler_mode ] );
                            p = append_str( out, buf );
                            break;
                        case CLASS_MNEMONICS:
                            p = append_str( out, ( char* )"ror.w" );
                            p = append_tab( out );
                            p = append_str( p, ( char* )"#4, (char*)" );
                            p = append_str( p, op_str_81[ ( n & 3 ) + 4 * disassembler_mode ] );
                            break;
                        default:
                            p = append_str( out, ( char* )"Unknown disassembler mode" );
                            break;
                    }
                    break;

                case 8:
                    fn = bus_fetch_nibble( ( *addr )++ );
                    n = bus_fetch_nibble( ( *addr )++ );
                    switch ( disassembler_mode ) {
                        case HP_MNEMONICS:
                            sprintf( buf, ( char* )"%s=%s%cCON", op_str_81[ ( n & 3 ) + 4 * disassembler_mode ],
                                     op_str_81[ ( n & 3 ) + 4 * disassembler_mode ], ( n < 8 ) ? '+' : '-' );
                            p = append_str( out, buf );
                            p = append_tab( out );
                            p = append_field( p, fn );
                            fn = bus_fetch_nibble( ( *addr )++ );
                            sprintf( buf, ( char* )", %d", fn + 1 );
                            p = append_str( p, buf );
                            break;
                        case CLASS_MNEMONICS:
                            p = append_str( out, ( n < 8 ) ? ( char* )"add" : ( char* )"sub" );
                            p = append_field( p, fn );
                            p = append_tab( out );
                            fn = bus_fetch_nibble( ( *addr )++ );
                            sprintf( buf, ( char* )"#%d, (char*)", fn + 1 );
                            p = append_str( p, buf );
                            p = append_str( p, op_str_81[ ( n & 3 ) + 4 * disassembler_mode ] );
                            break;
                        default:
                            p = append_str( out, ( char* )"Unknown disassembler mode" );
                            break;
                    }
                    break;

                case 9:
                    switch ( disassembler_mode ) {
                        case HP_MNEMONICS:
                            sprintf( buf, ( char* )"%sSRB.F", op_str_81[ ( n & 3 ) + 4 * disassembler_mode ] );
                            p = append_str( out, buf );
                            p = append_tab( out );
                            p = append_field( p, bus_fetch_nibble( ( *addr )++ ) );
                            break;
                        case CLASS_MNEMONICS:
                            p = append_str( out, ( char* )"lsr" );
                            p = append_field( p, bus_fetch_nibble( ( *addr )++ ) );
                            p = append_tab( out );
                            p = append_str( p, ( char* )"#1, (char*)" );
                            p = append_str( p, op_str_81[ ( n & 3 ) + 4 * disassembler_mode ] );
                            break;
                        default:
                            p = append_str( out, ( char* )"Unknown disassembler mode" );
                            break;
                    }
                    break;

                case 0xa:
                    fn = bus_fetch_nibble( ( *addr )++ );
                    n = bus_fetch_nibble( ( *addr )++ );
                    if ( n > 2 )
                        break;
                    c = ( char )bus_fetch_nibble( ( *addr )++ );
                    if ( ( ( int )c & 7 ) > 4 )
                        break;
                    switch ( disassembler_mode ) {
                        case HP_MNEMONICS:
                            if ( n == 2 ) {
                                sprintf( buf, ( char* )"%cR%dEX.F", ( ( int )c < 8 ) ? 'A' : 'C', ( int )c & 7 );
                            } else if ( n == 1 ) {
                                sprintf( buf, ( char* )"%c=R%d.F", ( ( int )c < 8 ) ? 'A' : 'C', ( int )c & 7 );
                            } else {
                                sprintf( buf, ( char* )"R%d=%c.F", ( int )c & 7, ( ( int )c < 8 ) ? 'A' : 'C' );
                            }
                            p = append_str( out, buf );
                            p = append_tab( out );
                            p = append_field( p, fn );
                            break;
                        case CLASS_MNEMONICS:
                            p = append_str( out, ( n == 2 ) ? ( char* )"exg" : ( char* )"move" );
                            p = append_field( p, fn );
                            p = append_tab( out );
                            if ( n == 1 ) {
                                sprintf( buf, ( char* )"r%d", ( int )c & 7 );
                                p = append_str( p, buf );
                            } else
                                p = append_str( p, ( ( int )c < 8 ) ? ( char* )"a" : ( char* )"c" );
                            p = append_str( p, ( char* )", (char*)" );
                            if ( n == 1 )
                                p = append_str( p, ( ( int )c < 8 ) ? ( char* )"a" : ( char* )"c" );
                            else {
                                sprintf( buf, ( char* )"r%d", ( int )c & 7 );
                                p = append_str( p, buf );
                            }
                            break;
                        default:
                            p = append_str( out, ( char* )"Unknown disassembler mode" );
                            break;
                    }
                    break;

                case 0xb:
                    n = bus_fetch_nibble( ( *addr )++ );
                    if ( ( n < 2 ) || ( n > 7 ) )
                        break;

                    p = append_str( out, in_str_81b[ n + 16 * disassembler_mode ] );
                    break;

                case 0xc:
                case 0xd:
                case 0xe:
                case 0xf:
                    switch ( disassembler_mode ) {
                        case HP_MNEMONICS:
                            sprintf( buf, ( char* )"%sSRB", op_str_81[ ( n & 3 ) + 4 * disassembler_mode ] );
                            p = append_str( out, buf );
                            break;
                        case CLASS_MNEMONICS:
                            p = append_str( out, ( char* )"lsr.w" );
                            p = append_tab( out );
                            p = append_str( p, ( char* )"#1, (char*)" );
                            p = append_str( p, op_str_81[ ( n & 3 ) + 4 * disassembler_mode ] );
                            break;
                        default:
                            p = append_str( out, ( char* )"Unknown disassembler mode" );
                            break;
                    }
                    break;

                default:
                    break;
            }
            break;

        case 2:
            n = bus_fetch_nibble( ( *addr )++ );
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
            n = bus_fetch_nibble( ( *addr )++ );
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
                    p = append_str( out, ( disp == 0 ) ? ( char* )"rt" : ( char* )"b" );
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
                    p = append_str( out, ( fn == 4 ) ? ( char* )"bclr" : ( char* )"bset" );
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
            n = bus_fetch_nibble( ( *addr )++ );
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
                    p = append_str( out, ( disp == 0 ) ? ( char* )"rt" : ( char* )"b" );
                    p = append_str( p, ( fn == 6 ) ? ( char* )"bc" : ( char* )"bs" );
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
            n = bus_fetch_nibble( ( *addr )++ );
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
                    p = append_str( out, ( disp == 0 ) ? ( char* )"rt" : ( char* )"b" );
                    p = append_str( p, ( fn == 8 ) ? ( char* )"ne.1" : ( char* )"eq.1" );
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
                    p = append_str( out, ( fn == 0xc ) ? ( char* )"GOLONG" : ( char* )"GOSUBL" );
                    p = append_tab( out );
                    p = append_r_addr( p, &pc, disp, 4, ( fn == 0xc ) ? 2 : 6 );
                    p = append_pc_comment( out, pc );
                    break;
                case CLASS_MNEMONICS:
                    p = append_str( out, ( fn == 0xc ) ? ( char* )"bra.4" : ( char* )"bsr.4" );
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
                    p = append_str( out, ( fn == 0xc ) ? ( char* )"GOVLNG" : ( char* )"GOSBVL" );
                    p = append_tab( out );
                    p = append_addr( p, pc );
                    break;
                case CLASS_MNEMONICS:
                    p = append_str( out, ( fn == 0xc ) ? ( char* )"jmp" : ( char* )"jsr" );
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

static Address disassemble( Address addr, char* out )
{
    Nibble n;
    Nibble fn;
    char* p = out;
    char c;
    char buf[ 20 ];
    Address disp, pc;

    switch ( n = bus_fetch_nibble( addr++ ) ) {
        case 0:
            if ( ( n = bus_fetch_nibble( addr++ ) ) != 0xe ) {
                p = append_str( out, opcode_0_tbl[ n + 16 * disassembler_mode ] );
                break;
            }
            fn = bus_fetch_nibble( addr++ );
            n = bus_fetch_nibble( addr++ );
            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    sprintf( buf, op_str_0[ ( n & 7 ) + 8 * HP_MNEMONICS ], ( n < 8 ) ? '&' : '!' );
                    p = append_str( out, buf );
                    p = append_tab( out );
                    p = append_field( p, fn );
                    break;
                case CLASS_MNEMONICS:
                    p = append_str( out, ( n < 8 ) ? ( char* )"and" : ( char* )"or" );
                    p = append_field( p, fn );
                    p = append_tab( out );
                    p = append_str( p, op_str_0[ ( n & 7 ) + 8 * CLASS_MNEMONICS ] );
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
            n = bus_fetch_nibble( addr++ );
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
            fn = bus_fetch_nibble( addr++ );
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
                    sprintf( buf, ( disp == 0 ) ? ( char* )"RTN%sC" : ( char* )"GO%sC", ( n == 4 ) ? ( char* )"" : ( char* )"N" );
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
                    p = append_str( out, ( disp == 0 ) ? ( char* )"rtc" : ( char* )"bc" );
                    p = append_str( p, ( n == 4 ) ? ( char* )"s" : ( char* )"c" );
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
            fn = bus_fetch_nibble( addr ); /* PEEK */
            if ( fn != 0xa && fn != 0xb ) {
                p = disasm_8( &addr, out );
                break;
            }
            /* Fall through */

        case 9:
            fn = bus_fetch_nibble( addr++ );
            if ( n == 8 ) {
                c = ( char )( ( fn == 0xa ) ? 0 : 1 );
                fn = 0xf;
            } else {
                c = ( char )( ( fn < 8 ) ? 0 : 1 );
                fn &= 7;
            }

            n = bus_fetch_nibble( addr++ );
            pc = addr;
            disp = read_int( &addr, 2 );

            switch ( disassembler_mode ) {
                case HP_MNEMONICS:
                    if ( ( c == 0 ) && ( n >= 8 ) )
                        sprintf( buf, op_str_9[ ( n & 3 ) + 8 * HP_MNEMONICS + 4 ],
                                 in_str_9[ ( ( n >> 2 ) & 3 ) + 4 * c + 8 * HP_MNEMONICS ] );
                    else
                        sprintf( buf, op_str_9[ ( n & 3 ) + 8 * HP_MNEMONICS ], in_str_9[ ( ( n >> 2 ) & 3 ) + 4 * c + 8 * HP_MNEMONICS ] );
                    p = append_str( out, buf );
                    p = append_tab( out );
                    p = append_field( p, fn );
                    p = append_str( p, ( char* )", (char*)" );
                    p = append_str( p, ( disp == 0 ) ? ( char* )"RTNYES" : ( char* )"GOYES (char*)" );
                    if ( disp != 0 ) {
                        p = append_r_addr( p, &pc, disp, 2, 3 );
                        p = append_pc_comment( out, pc );
                    }
                    break;

                case CLASS_MNEMONICS:
                    p = append_str( out, ( disp == 0 ) ? ( char* )"rt" : ( char* )"b" );
                    p = append_str( p, in_str_9[ ( ( n >> 2 ) & 3 ) + 4 * c + 8 * CLASS_MNEMONICS ] );
                    p = append_field( p, fn );
                    p = append_tab( out );
                    if ( ( c == 0 ) && ( n >= 8 ) )
                        p = append_str( p, op_str_9[ ( n & 3 ) + 8 * CLASS_MNEMONICS + 4 ] );
                    else
                        p = append_str( p, op_str_9[ ( n & 3 ) + 8 * CLASS_MNEMONICS ] );
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
                    fn = bus_fetch_nibble( addr++ );
                    c = ( char )( ( fn < 8 ) ? 0 : 1 );
                    fn &= 7;
                    disp = 0xa;
                    break;
                case 0xb:
                    fn = bus_fetch_nibble( addr++ );
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

            n = bus_fetch_nibble( addr++ );
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
                            p = append_str( out, ( char* )"Unknown disassembler mode" );
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
                            p = append_str( out, ( char* )"Unknown disassembler mode" );
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
                        p = append_str( p, op_str_af[ n + 16 * CLASS_MNEMONICS ] );
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

static char* skip_ob( Address* addr, char* string )
{
    Address size, type;
    char* p = string;
    struct objfunc* op;

    type = read_nibbles( *addr - 5, 5 );
    for ( op = objects; op->prolog != 0; op++ ) {
        if ( op->prolog == type )
            break;
    }

    if ( op->prolog ) {
        sprintf( p, "%s", op->name );
        p += strlen( p );
    }

    size = read_nibbles( *addr, 5 );
    *addr += size;

    *p = '\0';
    return p;
}

static long hxs2real( long hxs )
{
    int n = 0, c = 1;

    while ( hxs ) {
        n += ( hxs & 0xf ) * c;
        c *= 10;
        hxs >>= 4;
    }
    return n;
}

static char* dec_bin_int( Address* addr, char* string )
{
    char* p = string;
    Address n = 0;

    n = read_nibbles( *addr, 5 );
    *addr += 5;
    sprintf( p, "<%lXh>", ( long )n );
    p += strlen( p );
    return p;
}

static char* real_number( Address* addr, char* string, int ml, int xl )
{
    hp_real r;
    long re, xs;
    int i;
    char fmt[ 26 ];
    char m[ 16 ];
    char* p = string;

    /*
     * Read the number
     */
    r.x = read_nibbles( *addr, xl );
    *addr += xl;
    r.ml = read_nibbles( *addr, ml - 8 );
    *addr += ml - 8;
    r.mh = read_nibbles( *addr, 8 );
    *addr += 8;
    r.m = read_nibbles( *addr, 1 );
    ( *addr )++;
    r.s = read_nibbles( *addr, 1 );
    ( *addr )++;

    /*
     * Figure out the exponent
     */
    xs = 5;
    while ( --xl )
        xs *= 10;
    re = hxs2real( r.x );
    if ( re >= xs )
        re = re - 2 * xs;

    if ( ( re >= 0 ) && ( re < ml + 1 ) ) {
        if ( r.s >= 5 )
            *p++ = '-';

        sprintf( fmt, "%%.1X%%.8lX%%.%dlX", ml - 8 );
        sprintf( m, fmt, r.m, r.mh, r.ml );

        for ( i = 0; i <= re; i++ )
            *p++ = m[ i ];
        *p++ = '.';
        for ( ; i < ml + 1; i++ )
            *p++ = m[ i ];
        p--;
        while ( *p == '0' )
            p--;
        if ( *p == '.' )
            p--;
        *++p = '\0';

        return p;
    }

    if ( ( re < 0 ) && ( re >= -ml - 1 ) ) {
        sprintf( fmt, "%%.1X%%.8lX%%.%dlX", ml - 8 );
        sprintf( m, fmt, r.m, r.mh, r.ml );

        for ( i = ml; m[ i ] == '0'; i-- )
            ;

        if ( -re <= ml - i + 1 ) {
            if ( r.s >= 5 )
                *p++ = '-';

            *p++ = '.';

            for ( i = 1; i < -re; i++ )
                *p++ = '0';

            for ( i = 0; i < ml + 1; i++ )
                *p++ = m[ i ];
            p--;
            while ( *p == '0' )
                p--;
            *++p = '\0';

            return p;
        }
    }

    sprintf( fmt, "%%s%%X.%%.8lX%%.%dlX", ml - 8 );
    sprintf( p, fmt, ( r.s >= 5 ) ? "-" : "", r.m, r.mh, r.ml );

    p += strlen( p ) - 1;

    while ( *p == '0' )
        p--;
    *++p = '\0';

    if ( re ) {
        sprintf( p, "E%ld", re );
        p += strlen( p );
        *p = '\0';
    }

    return p;
}

static char* dec_real( Address* addr, char* string ) { return real_number( addr, string, 11, 3 ); }

static char* dec_long_real( Address* addr, char* string ) { return real_number( addr, string, 14, 5 ); }

static char* dec_complex( Address* addr, char* string )
{
    char* p = string;

    *p++ = '(';
    p = real_number( addr, p, 11, 3 );
    *p++ = ',';
    p = real_number( addr, p, 11, 3 );
    *p++ = ')';
    *p = '\0';
    return p;
}

static char* dec_long_complex( Address* addr, char* string )
{
    char* p = string;

    *p++ = '(';
    p = real_number( addr, p, 14, 5 );
    *p++ = ',';
    p = real_number( addr, p, 14, 5 );
    *p++ = ')';
    *p = '\0';
    return p;
}

static char* dec_string( Address* addr, char* string )
{
    Address len;
    unsigned char c;
    char* p = string;
    int i, n;

    len = read_nibbles( *addr, 5 );
    *addr += 5;
    len -= 5;
    len /= 2;

    n = len;
    if ( len > 1000 )
        n = 1000;

    *p++ = '\"';
    for ( i = 0; i < n; i++ ) {
        c = read_nibbles( *addr, 2 );
        *addr += 2;
        if ( hp48_trans_tbl[ c ].trans ) {
            sprintf( p, "%s", hp48_trans_tbl[ c ].trans );
            p += strlen( p );
        } else
            *p++ = c;
    }

    if ( n != len ) {
        *p++ = '.';
        *p++ = '.';
        *p++ = '.';
    }

    *p++ = '\"';
    *p = '\0';
    return p;
}

static char* dec_hex_string( Address* addr, char* string )
{
    int len, lead, i, n;
    char* p = string;

    len = read_nibbles( *addr, 5 );
    *addr += 5;
    len -= 5;

    if ( len <= 16 ) {
        *p++ = '#';
        *p++ = ' ';
        lead = 1;
        for ( i = len - 1; i >= 0; i-- ) {
            *p = hex[ disassembler_mode ][ bus_fetch_nibble( *addr + i ) ];
            if ( lead ) {
                if ( ( i != 0 ) && ( *p == '0' ) )
                    p--;
                else
                    lead = 0;
            }
            p++;
        }

        *p++ = 'h';
    } else {
        *p++ = 'C';
        *p++ = '#';
        *p++ = ' ';

        sprintf( p, "%d", len );
        p += strlen( p );

        *p++ = ' ';

        n = len;
        if ( len > 1000 )
            n = 1000;

        for ( i = 0; i < n; i++ )
            *p++ = hex[ disassembler_mode ][ bus_fetch_nibble( *addr + i ) ];

        if ( n != len ) {
            *p++ = '.';
            *p++ = '.';
            *p++ = '.';
        }
    }

    *addr += len;

    *p = '\0';
    return p;
}

static char* xlib_name( int lib, int command, char* string )
{
    int n, len;
    int i, lib_n = 0;
    unsigned char c;
    Address romptab, acptr;
    Address offset, hash_end;
    Address lib_addr, name_addr;
    Address type, ram_base, ram_mask;
    short present = 0;
    char* p = string;

    /*
     * Configure RAM to address 0x70000
     */
    ram_base = saturn.mem_cntl[ 1 ].config[ 0 ];
    ram_mask = saturn.mem_cntl[ 1 ].config[ 1 ];
    if ( opt_gx ) {
        saturn.mem_cntl[ 1 ].config[ 0 ] = 0x80000;
        saturn.mem_cntl[ 1 ].config[ 1 ] = 0xc0000;
        romptab = ROMPTAB_GX;
    } else {
        saturn.mem_cntl[ 1 ].config[ 0 ] = 0x70000;
        saturn.mem_cntl[ 1 ].config[ 1 ] = 0xf0000;
        romptab = ROMPTAB_SX;
    }

    /*
     * look up number of installed libs in romptab
     */
    n = read_nibbles( romptab, 3 );
    romptab += 3;

    if ( n > 0 ) {
        /*
         * look up lib number in romptab
         */
        while ( n-- ) {
            lib_n = read_nibbles( romptab, 3 );
            romptab += 3;
            if ( lib_n == lib )
                break;
            romptab += 5;
            if ( opt_gx )
                romptab += 8;
        }
        if ( lib_n == lib ) {
            /*
             * look at hash table pointer
             */
            lib_addr = read_nibbles( romptab, 5 );
            if ( opt_gx ) {
                romptab += 5;
                acptr = read_nibbles( romptab, 5 );
                if ( acptr != 0x00000 ) {
                    saturn.mem_cntl[ 1 ].config[ 0 ] = ram_base;
                    saturn.mem_cntl[ 1 ].config[ 1 ] = ram_mask;
                    sprintf( p, "XLIB %d %d", lib, command );
                    p += strlen( p );
                    return p;
                }
            }
            lib_addr += 3;
            offset = read_nibbles( lib_addr, 5 );
            if ( offset > 0 ) {
                /*
                 * look at the hash table
                 */
                lib_addr += offset;

                /*
                 * check if library is in ROM
                 */
                if ( !opt_gx )
                    if ( lib_addr < 0x70000 )
                        saturn.mem_cntl[ 1 ].config[ 0 ] = 0xf0000;

                /*
                 * check pointer type
                 */
                type = read_nibbles( lib_addr, 5 );
                if ( type == DOBINT ) {
                    /*
                     * follow pointer to real address
                     */
                    lib_addr += 5;
                    lib_addr = read_nibbles( lib_addr, 5 );
                } else if ( type == DOACPTR ) {
                    /*
                     * follow pointer to real address
                     */
                    lib_addr += 5;
                    acptr = lib_addr + 5;
                    lib_addr = read_nibbles( lib_addr, 5 );
                    acptr = read_nibbles( acptr, 5 );
                    if ( acptr != 0x00000 ) {
                        saturn.mem_cntl[ 1 ].config[ 0 ] = ram_base;
                        saturn.mem_cntl[ 1 ].config[ 1 ] = ram_mask;
                        sprintf( p, "XLIB %d %d", lib, command );
                        p += strlen( p );
                        return p;
                    }
                }

                /*
                 * get length of hash table
                 */
                lib_addr += 5;
                hash_end = read_nibbles( lib_addr, 5 );
                hash_end += lib_addr;

                /*
                 * go into real name table
                 */
                lib_addr += 85;
                offset = read_nibbles( lib_addr, 5 );
                lib_addr += offset;

                /*
                 * look at library name number 'command'
                 */
                offset = 5 * command;
                lib_addr += offset;
                if ( lib_addr < hash_end ) {
                    offset = read_nibbles( lib_addr, 5 );
                    if ( offset > 0 ) {
                        name_addr = lib_addr - offset;
                        len = read_nibbles( name_addr, 2 );
                        name_addr += 2;
                        present = 1;
                        for ( i = 0; i < len; i++ ) {
                            c = read_nibbles( name_addr, 2 );
                            name_addr += 2;
                            if ( hp48_trans_tbl[ c ].trans ) {
                                sprintf( p, "%s", hp48_trans_tbl[ c ].trans );
                                p += strlen( p );
                            } else
                                *p++ = c;
                        }
                        *p = '\0';
                    }
                }
            }
        }
    }

    /*
     * Reconfigure RAM
     */
    saturn.mem_cntl[ 1 ].config[ 0 ] = ram_base;
    saturn.mem_cntl[ 1 ].config[ 1 ] = ram_mask;

    if ( !present ) {
        sprintf( p, "XLIB %d %d", lib, command );
        p += strlen( p );
    }
    return p;
}

static short check_xlib( Address addr, char* string )
{
    int n, lib, command;
    Address romptab;
    Address offset, link_end;
    Address acptr;
    Address lib_addr;
    Address type, ram_base, ram_mask;
    char* p = string;

    /*
     * Configure RAM to address 0x70000
     */
    ram_base = saturn.mem_cntl[ 1 ].config[ 0 ];
    ram_mask = saturn.mem_cntl[ 1 ].config[ 1 ];
    if ( opt_gx ) {
        saturn.mem_cntl[ 1 ].config[ 0 ] = 0x80000;
        saturn.mem_cntl[ 1 ].config[ 1 ] = 0xc0000;
        romptab = ROMPTAB_GX;
    } else {
        saturn.mem_cntl[ 1 ].config[ 0 ] = 0x70000;
        saturn.mem_cntl[ 1 ].config[ 1 ] = 0xf0000;
        romptab = ROMPTAB_SX;
    }

    /*
     * look up number of installed libs in romptab
     */
    n = read_nibbles( romptab, 3 );
    romptab += 3;

    /*
    fprintf(stderr, "Number of Libraries = %d\n", n);
    fflush(stderr);
    */

    if ( n > 0 ) {
        /*
         * look up lib number in romptab
         */
        while ( n-- ) {
            lib = read_nibbles( romptab, 3 );
            romptab += 3;
            /*
            fprintf(stderr, "Library num = %d\n", lib);
            fflush(stderr);
            */
            /*
             * look at link table pointer
             */
            lib_addr = read_nibbles( romptab, 5 );
            /*
            fprintf(stderr, "Library addr = %.5lx\n", lib_addr);
            fflush(stderr);
            */
            romptab += 5;

            if ( opt_gx ) {
                acptr = read_nibbles( romptab, 5 );
                romptab += 8;
                if ( acptr != 0x00000 )
                    continue;
            }

            lib_addr += 13;
            offset = read_nibbles( lib_addr, 5 );
            if ( offset > 0 ) {
                /*
                 * look at the link table
                 */
                lib_addr += offset;
                /*
                fprintf(stderr, "Link table addr = %.5lx\n", lib_addr);
                fflush(stderr);
                */
                /*
                 * check if library is in ROM
                 */
                if ( !opt_gx )
                    if ( lib_addr < 0x70000 )
                        saturn.mem_cntl[ 1 ].config[ 0 ] = 0xf0000;

                /*
                 * check pointer type
                 */
                type = read_nibbles( lib_addr, 5 );
                if ( type == DOBINT ) {
                    /*
                     * follow pointer to real address
                     */
                    lib_addr += 5;
                    lib_addr = read_nibbles( lib_addr, 5 );
                }
                /*
                fprintf(stderr, "Link table addr (2) = %.5lx\n", lib_addr);
                fflush(stderr);
                */
                /*
                 * get length of link table
                 */
                lib_addr += 5;
                link_end = read_nibbles( lib_addr, 5 );
                link_end += lib_addr;
                /*
                fprintf(stderr, "Link table end = %.5lx\n", link_end);
                fflush(stderr);
                */
                /*
                 * look at library commands
                 */
                lib_addr += 5;
                command = 0;
                while ( lib_addr < link_end ) {
                    offset = read_nibbles( lib_addr, 5 );
                    if ( offset > 0 ) {
                        if ( addr == ( ( lib_addr + offset ) & 0xfffff ) ) {
                            p = xlib_name( lib, command, p );
                            saturn.mem_cntl[ 1 ].config[ 0 ] = ram_base;
                            saturn.mem_cntl[ 1 ].config[ 1 ] = ram_mask;
                            return 1;
                        }
                    }
                    lib_addr += 5;
                    command++;
                }
                if ( opt_gx )
                    saturn.mem_cntl[ 1 ].config[ 0 ] = 0x80000;
                else
                    saturn.mem_cntl[ 1 ].config[ 0 ] = 0x70000;
            }
        }
    }

    /*
     * Reconfigure RAM
     */
    saturn.mem_cntl[ 1 ].config[ 0 ] = ram_base;
    saturn.mem_cntl[ 1 ].config[ 1 ] = ram_mask;

    return 0;
}

static char* dec_rpl_obj( Address* addr, char* string )
{
    Address prolog = 0;
    Address prolog_2;
    char* p = string;
    char tmp_str[ 80 ];
    struct objfunc* op;

    prolog = read_nibbles( *addr, 5 );

    for ( op = objects; op->prolog != 0; op++ ) {
        if ( op->prolog == prolog )
            break;
    }

    if ( op->prolog == 0 ) {
        if ( check_xlib( prolog, tmp_str ) ) {
            p = append_str( p, tmp_str );
        } else {
            prolog_2 = read_nibbles( prolog, 5 );
            for ( op = objects; op->prolog != 0; op++ ) {
                if ( op->prolog == prolog_2 )
                    break;
            }
            if ( op->prolog )
                p = dec_rpl_obj( &prolog, p );
            else
                p = append_str( p, "External" );
        }
        *addr += 5;
        return p;
    }

    *addr += 5;
    p = ( *op->func )( addr, p );

    return p;
}

static char* dec_list( Address* addr, char* string )
{
    Address semi;
    char* p = string;

    *p++ = '{';
    *p++ = ' ';
    semi = read_nibbles( *addr, 5 );
    while ( semi != SEMI ) {
        p = dec_rpl_obj( addr, p );
        semi = read_nibbles( *addr, 5 );
        if ( semi != SEMI ) {
            *p++ = ' ';
            *p = '\0';
        }
    }
    *p++ = ' ';
    *p++ = '}';
    *p = '\0';

    *addr += 5;
    return p;
}

static char* dec_symb( Address* addr, char* string )
{
    Address semi;
    char* p = string;

    semi = read_nibbles( *addr, 5 );
    *p++ = '\'';
    while ( semi != SEMI ) {
        p = dec_rpl_obj( addr, p );
        semi = read_nibbles( *addr, 5 );
        if ( semi != SEMI ) {
            *p++ = ' ';
            *p = '\0';
        }
    }
    *addr += 5;

    *p++ = '\'';
    *p = '\0';
    return p;
}

static char* dec_unit( Address* addr, char* string )
{
    Address semi;
    char* p = string;

    semi = read_nibbles( *addr, 5 );
    while ( semi != SEMI ) {
        p = dec_rpl_obj( addr, p );
        semi = read_nibbles( *addr, 5 );
        if ( semi != SEMI ) {
            *p++ = ' ';
            *p = '\0';
        }
    }
    *addr += 5;
    return p;
}

static char* dec_unit_op( Address* addr, char* string )
{
    Address op;
    char* p = string;

    op = read_nibbles( *addr - 5, 5 );
    switch ( op ) {
        case UM_MUL:
            *p++ = '*';
            break;
        case UM_DIV:
            *p++ = '/';
            break;
        case UM_POW:
            *p++ = '^';
            break;
        case UM_END:
            *p++ = '_';
            break;
        case UM_PRE:
            p--;
            break;
        default:
            break;
    }
    *p = '\0';
    return p;
}

static char* dec_library( Address* addr, char* string )
{
    Address libsize, libidsize;
    /*
      Address        hashoff, mesgoff, linkoff, cfgoff;
      Address        mesgloc, cfgloc;
    */
    int i, libnum;
    unsigned char c;
    char* p = string;

    libsize = read_nibbles( *addr, 5 );
    libidsize = read_nibbles( *addr + 5, 2 );
    libnum = read_nibbles( *addr + 2 * libidsize + 9, 3 );

    sprintf( p, "Library %d:  ", libnum );
    p += strlen( p );

    for ( i = 0; i < libidsize; i++ ) {
        c = read_nibbles( *addr + 2 * i + 7, 2 );
        if ( hp48_trans_tbl[ c ].trans ) {
            sprintf( p, "%s", hp48_trans_tbl[ c ].trans );
            p += strlen( p );
        } else
            *p++ = c;
    }

    *addr += libsize;

    *p = '\0';
    return p;
}

static char* dec_library_data( Address* addr, char* string )
{
    Address size;
    char* p = string;

    size = read_nibbles( *addr, 5 );

    sprintf( p, "Library Data" );
    p += strlen( p );

    *addr += size;

    *p = '\0';
    return p;
}

static char* dec_acptr( Address* addr, char* string )
{
    Address size;
    char* p = string;
    int i;

    if ( opt_gx ) {
        size = 10;
        sprintf( p, "ACPTR " );
        p += strlen( p );
        for ( i = 0; i < 5; i++ )
            *p++ = hex[ disassembler_mode ][ bus_fetch_nibble( *addr + i ) ];
        *p++ = ' ';
        for ( i = 5; i < 10; i++ )
            *p++ = hex[ disassembler_mode ][ bus_fetch_nibble( *addr + i ) ];
    } else {
        size = read_nibbles( *addr, 5 );
        sprintf( p, "Ext 1" );
        p += strlen( p );
    }

    *addr += size;

    *p = '\0';
    return p;
}

static char* dec_prog( Address* addr, char* string )
{
    Address semi;
    char* p = string;

    semi = read_nibbles( *addr, 5 );
    while ( semi != SEMI ) {
        p = dec_rpl_obj( addr, p );
        semi = read_nibbles( *addr, 5 );
        if ( semi != SEMI ) {
            *p++ = ' ';
            *p = '\0';
        }
    }
    *addr += 5;
    return p;
}

static char* dec_code( Address* addr, char* string )
{
    char* p = string;
    Address n, len;

    len = read_nibbles( *addr, 5 );
    sprintf( p, "Code" );
    p += strlen( p );

    n = 0;
    while ( n < len ) {
        /*
         *addr = disassemble(*addr, p);
         */
        n += len;
    }

    *addr += len;
    return p;
}

static char* dec_local_ident( Address* addr, char* string )
{
    int len, i, n;
    char* p = string;
    unsigned char c;

    len = read_nibbles( *addr, 2 );
    *addr += 2;

    n = len;
    if ( len > 1000 )
        n = 1000;

    for ( i = 0; i < n; i++ ) {
        c = read_nibbles( *addr, 2 );
        *addr += 2;
        if ( hp48_trans_tbl[ c ].trans ) {
            sprintf( p, "%s", hp48_trans_tbl[ c ].trans );
            p += strlen( p );
        } else
            *p++ = c;
    }

    if ( n != len ) {
        *p++ = '.';
        *p++ = '.';
        *p++ = '.';
    }

    *p = '\0';
    return p;
}

static char* dec_global_ident( Address* addr, char* string )
{
    int len, i, n;
    char* p = string;
    unsigned char c;

    len = read_nibbles( *addr, 2 );
    *addr += 2;

    n = len;
    if ( len > 1000 )
        n = 1000;

    for ( i = 0; i < n; i++ ) {
        c = read_nibbles( *addr, 2 );
        *addr += 2;
        if ( hp48_trans_tbl[ c ].trans ) {
            sprintf( p, "%s", hp48_trans_tbl[ c ].trans );
            p += strlen( p );
        } else
            *p++ = c;
    }

    if ( n != len ) {
        *p++ = '.';
        *p++ = '.';
        *p++ = '.';
    }

    *p = '\0';
    return p;
}

static char* dec_xlib_name( Address* addr, char* string )
{
    int lib, command;

    lib = read_nibbles( *addr, 3 );
    *addr += 3;
    command = read_nibbles( *addr, 3 );
    *addr += 3;

    return xlib_name( lib, command, string );
}

static char* any_array( Address* addr, char* string, short lnk_flag )
{
    Address len, type, dim;
    Address *dim_lens, *dims;
    Address array_addr, elem_addr;
    long elems;
    int d, i;
    char* p = string;
    struct objfunc* op;

    array_addr = *addr;
    len = read_nibbles( *addr, 5 );
    *addr += 5;
    type = read_nibbles( *addr, 5 );
    *addr += 5;
    dim = read_nibbles( *addr, 5 );
    *addr += 5;

    for ( op = objects; op->prolog != 0; op++ ) {
        if ( op->prolog == type )
            break;
    }

    dim_lens = ( Address* )malloc( dim * sizeof( Address ) );
    dims = ( Address* )malloc( dim * sizeof( Address ) );
    elems = 1;
    for ( i = 0; i < dim; i++ ) {
        dim_lens[ i ] = read_nibbles( *addr, 5 );
        dims[ i ] = dim_lens[ i ];
        elems *= dim_lens[ i ];
        *addr += 5;
    }

    if ( op->prolog == 0 ) {
        sprintf( p, "of Type %.5lX, Dim %ld, Size ", type, ( long )dim );
        p += strlen( p );
        for ( i = 0; i < dim; i++ ) {
            sprintf( p, "%ld", ( long )dim_lens[ i ] );
            p += strlen( p );
            if ( i < dim - 1 ) {
                sprintf( p, " x " );
                p += strlen( p );
            }
        }
        *p = '\0';
        *addr = array_addr + len;
        free( dim_lens );
        free( dims );
        return p;
    }

    d = -1;
    while ( elems-- ) {
        if ( d < dim - 1 ) {
            for ( ; d < dim - 1; d++ ) {
                *p++ = '[';
            }
            d = dim - 1;
        }
        if ( lnk_flag ) {
            elem_addr = read_nibbles( *addr, 5 );
            elem_addr += *addr;
            *addr += 5;
            p = ( *op->func )( &elem_addr, p );
        } else
            p = ( *op->func )( addr, p );
        *p = '\0';
        dims[ d ]--;
        if ( dims[ d ] )
            *p++ = ' ';
        while ( dims[ d ] == 0 ) {
            dims[ d ] = dim_lens[ d ];
            d--;
            dims[ d ]--;
            *p++ = ']';
        }
    }

    free( dim_lens );
    free( dims );
    *addr = array_addr + len;

    *p = '\0';
    return p;
}

static char* dec_array( Address* addr, char* string ) { return any_array( addr, string, 0 ); }

static char* dec_lnk_array( Address* addr, char* string ) { return any_array( addr, string, 1 ); }

static char* dec_char( Address* addr, char* string )
{
    char* p = string;
    unsigned char c;

    c = read_nibbles( *addr, 2 );
    *addr += 2;

    *p++ = '\'';
    if ( hp48_trans_tbl[ c ].trans ) {
        sprintf( p, "%s", hp48_trans_tbl[ c ].trans );
        p += strlen( p );
    } else
        *p++ = c;
    *p++ = '\'';

    *p = 0;
    return p;
}

objfunc_t objects[] = {
    {( char* )"System Binary", 0, DOBINT,    dec_bin_int     },
    {( char* )"Real",          0, DOREAL,    dec_real        },
    {( char* )"Long Real",     0, DOEREL,    dec_long_real   },
    {( char* )"Complex",       0, DOCMP,     dec_complex     },
    {( char* )"Long Complex",  0, DOECMP,    dec_long_complex},
    {( char* )"Character",     0, DOCHAR,    dec_char        },
    {( char* )"Array",         0, DOARRY,    dec_array       },
    {( char* )"Linked Array",  0, DOLNKARRY, dec_lnk_array   },
    {( char* )"String",        2, DOCSTR,    dec_string      },
    {( char* )"Hex String",    1, DOHSTR,    dec_hex_string  },
    {( char* )"List",          0, DOLIST,    dec_list        },
    {( char* )"Directory",     0, DORRP,     skip_ob         },
    {( char* )"Symbolic",      0, DOSYMB,    dec_symb        },
    {( char* )"Unit",          0, DOEXT,     dec_unit        },
    {( char* )"Tagged",        0, DOTAG,     skip_ob         },
    {( char* )"Graphic",       0, DOGROB,    skip_ob         },
    {( char* )"Library",       0, DOLIB,     dec_library     },
    {( char* )"Backup",        0, DOBAK,     skip_ob         },
    {( char* )"Library Data",  0, DOEXT0,    dec_library_data},
    {( char* )"ACPTR",         0, DOACPTR,   dec_acptr       },
    {( char* )"External 2",    0, DOEXT2,    skip_ob         },
    {( char* )"External 3",    0, DOEXT3,    skip_ob         },
    {( char* )"External 4",    0, DOEXT4,    skip_ob         },
    {( char* )"Program",       0, DOCOL,     dec_prog        },
    {( char* )"Code",          1, DOCODE,    dec_code        },
    {( char* )"Global Ident",  0, DOIDNT,    dec_global_ident},
    {( char* )"Local Ident",   0, DOLAM,     dec_local_ident },
    {( char* )"XLib Name",     0, DOROMP,    dec_xlib_name   },
    {( char* )"*",             0, UM_MUL,    dec_unit_op     },
    {( char* )"/",             0, UM_DIV,    dec_unit_op     },
    {( char* )"^",             0, UM_POW,    dec_unit_op     },
    {( char* )" (char*)",      0, UM_PRE,    dec_unit_op     },
    {( char* )"_",             0, UM_END,    dec_unit_op     },
    {0,                        0, 0,         0               }
};

static char* decode_rpl_obj( Address addr, char* buf )
{
    Address prolog = 0;
    int len;
    char* p = buf;
    char tmp_str[ 80 ];
    struct objfunc* op;

    prolog = read_nibbles( addr, 5 );

    for ( op = objects; op->prolog != 0; op++ ) {
        if ( op->prolog == prolog )
            break;
    }

    if ( op->prolog == 0 ) {
        if ( addr == SEMI ) {
            p = append_str( buf, "Primitive Code" );
            p = append_tab_16( buf );
            p = append_str( p, "SEMI" );
        } else if ( addr + 5 == prolog ) {
            p = append_str( buf, "Primitive Code" );
            p = append_tab_16( buf );
            sprintf( p, "at %.5lX", prolog );
            p += strlen( p );
            *p = '\0';
        } else {
            p = append_str( buf, "PTR" );
            p = append_tab_16( buf );
            sprintf( p, "%.5lX", prolog );
            p += strlen( p );
            *p = '\0';
        }
        return p;
    }

    if ( op->prolog == DOCOL ) {
        if ( check_xlib( addr, tmp_str ) ) {
            p = append_str( buf, "XLib Call" );
            p = append_tab_16( buf );
            p = append_str( p, tmp_str );
            return p;
        }
    }

    p = append_str( buf, op->name );

    if ( op->length ) {
        len = ( read_nibbles( addr + 5, 5 ) - 5 ) / op->length;
        sprintf( p, " %d", len );
        p += strlen( p );
    }

    p = append_tab_16( buf );
    addr += 5;
    p = ( *op->func )( &addr, p );

    return p;
}

/*
 * command functions
 */
static void cmd_break( int, char** );
static void cmd_continue( int, char** );
static void cmd_delete( int, char** );
static void cmd_exit( int, char** );
static void cmd_go( int, char** );
static void cmd_help( int, char** );
static void cmd_load( int, char** );
static void cmd_mode( int, char** );
static void cmd_quit( int, char** );
static void cmd_regs( int, char** );
static void cmd_save( int, char** );
static void cmd_stack( int, char** );
static void cmd_stat( int, char** );
static void cmd_step( int, char** );
static void cmd_ram( int, char** );
static void cmd_reset( int, char** );
static void cmd_rstk( int, char** );

struct cmd {
    const char* name;
    void ( *func )( int, char** );
    const char* help;
}

cmd_tbl[] = {
    {"break",  cmd_break,
     "break [address]            Set breakpoint at `address\' or show "
      "breakpoints"                                                                               },
    {"b",      cmd_break,    0                                                                    },

    {"cont",   cmd_continue, "cont                       Continue execution"                      },
    {"c",      cmd_continue, 0                                                                    },

    {"delete", cmd_delete,
     "delete [all | n]           Delete breakpoint or watchpoint number "
      "`n\',\n                           all breakpoints, or current "
      "breakpoint"                                                                                },
    {"d",      cmd_delete,   0                                                                    },

    {"exit",   cmd_exit,     "exit                       Exit the emulator without saving"        },

    {"go",     cmd_go,       "go address                 Set PC to `address\'"                    },

    {"help",   cmd_help,     "help                       Display this information"                },
    {"h",      cmd_help,     0                                                                    },
    {"?",      cmd_help,     0                                                                    },

    {"load",   cmd_load,     "load                       Load emulator-state from files"          },

    {"mode",   cmd_mode,     "mode [hp | class]          Show or set disassembler mode"           },

    {"quit",   cmd_quit,     "quit                       Exit the emulator after saving its state"},
    {"q",      cmd_quit,     0                                                                    },

    {"ram",    cmd_ram,      "ram                        Show RAM layout"                         },

    {"reg",    cmd_regs,     "reg [register [hexvalue]]  Display or set register value"           },
    {"r",      cmd_regs,     0                                                                    },

    {"reset",  cmd_reset,    "reset                      Set the HP48\'s PC to ZERO"              },

    {"save",   cmd_save,     "save                       Save emulator-state to files"            },

    {"stack",  cmd_stack,    "stack                      Display RPL stack"                       },

    {"stat",   cmd_stat,     "stat                       Display statistics for the emulator"     },

    {"step",   cmd_step,     "step [n]                   Step one or n Instruction(s)"            },
    {"s",      cmd_step,     0                                                                    },

    {"where",  cmd_rstk,     "where                      Show ML return stack"                    },

    {0,        0,            0                                                                    }
};

int check_breakpoint( int type, Address addr )
{
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
        if ( bp->flags & BP_RANGE && addr >= bp->addr && addr <= bp->end_addr )
            goto hit_it;

        if ( bp->flags & type && addr == bp->addr ) {
hit_it:
            if ( type == BP_READ ) {
                printf( "%.5lX: Read watchpoint %d hit at %.5lX\n", saturn.pc, i, addr );
            } else if ( type == BP_WRITE ) {
                printf( "%.5lX: Write watchpoint %d hit at %.5lX\n", saturn.pc, i, addr );
            } else
                printf( "Breakpoint %d hit at %.5lX\n", i, addr );

            return 1;
        }
    }
    return 0;
}

static char* read_str( char* str, int n, int fp )
{
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

static inline void str_to_upper( char* arg )
{
    for ( unsigned long i = 0; i < strlen( arg ); i++ )
        if ( 'a' <= arg[ i ] && arg[ i ] <= 'z' )
            arg[ i ] = ( char )( ( int )arg[ i ] - ( int )'a' + ( int )'A' );
}

static int decode_dec( int* num, char* arg )
{
    if ( arg == ( char* )0 ) {
        printf( "Command requires an argument.\n" );
        return 0;
    }

    *num = 0;
    for ( unsigned long i = 0; i < strlen( arg ); i++ ) {
        *num *= 10;
        if ( '0' <= arg[ i ] && arg[ i ] <= '9' )
            *num += ( ( int )arg[ i ] - ( int )'0' );
        else {
            *num = 0;
            printf( "Not a number: %s.\n", arg );
            return 0;
        }
    }

    return 1;
}

static int decode_20( Address* addr, char* arg )
{
    if ( arg == ( char* )0 ) {
        printf( "Command requires an argument.\n" );
        return 0;
    }

    *addr = 0;
    for ( size_t i = 0; i < strlen( arg ); i++ ) {
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

static int decode_32( word_32* addr, char* arg )
{
    if ( arg == ( char* )0 ) {
        printf( "Command requires an argument.\n" );
        return 0;
    }

    *addr = 0;
    for ( size_t i = 0; i < strlen( arg ); i++ ) {
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

static int decode_64( word_64* addr, char* arg )
{
    if ( arg == ( char* )0 ) {
        printf( "Command requires an argument.\n" );
        return 0;
    }

    *addr = 0;
    for ( unsigned long i = 0; i < strlen( arg ); i++ ) {
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

static char* str_nibbles( Address addr, int n )
{
    static char str[ 1025 ];
    char* cp;
    int i;

    if ( n > 1024 ) {
        str[ 0 ] = '\0';
        return str;
    }

    for ( cp = str, i = 0; i < n; i++ ) {
        sprintf( cp, "%.1X", bus_fetch_nibble( addr + i ) );
        cp++;
    }
    *cp = '\0';

    return str;
}

static int confirm( const char* prompt )
{
    char ans[ 80 ];

    printf( "%s (y or n) ", prompt );
    fflush( stdout );
    read_str( ans, sizeof( ans ), 0 );
    while ( ans[ 0 ] != 'y' && ans[ 0 ] != 'Y' && ans[ 0 ] != 'n' && ans[ 0 ] != 'N' ) {
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

static void cmd_break( int argc, char** argv )
{
    int i;
    Address addr;

    if ( argc == 1 ) {
        for ( i = 0; i < MAX_BREAKPOINTS; i++ ) {
            if ( bkpt_tbl[ i ].flags == 0 )
                continue;
            if ( bkpt_tbl[ i ].flags == BP_EXEC ) {
                printf( "Breakpoint %d at 0x%.5lX\n", i + 1, bkpt_tbl[ i ].addr );
            } else if ( bkpt_tbl[ i ].flags == BP_RANGE ) {
                printf( "Range watchpoint %d at 0x%.5lX - 0x%.5lX\n", i + 1, bkpt_tbl[ i ].addr, bkpt_tbl[ i ].end_addr );
            } else {
                printf( "Watchpoint %d at 0x%.5lX\n", i + 1, bkpt_tbl[ i ].addr );
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
                printf( "Breakpoint %d at 0x%.5lX\n", i + 1, bkpt_tbl[ i ].addr );
                num_bkpts++;
                return;
            }
        }
        printf( "Breakpoint table full\n" );
    }
}

static void cmd_continue( int argc, char** argv ) { continue_flag = true; }

static void cmd_delete( int argc, char** argv )
{
    int num;

    if ( argc == 1 ) {
        for ( num = 0; num < MAX_BREAKPOINTS; num++ ) {
            if ( bkpt_tbl[ num ].addr == saturn.pc ) {
                if ( bkpt_tbl[ num ].flags == BP_EXEC ) {
                    printf( "Breakpoint %d at 0x%.5lX deleted.\n", num + 1, bkpt_tbl[ num ].addr );
                } else if ( bkpt_tbl[ num ].flags == BP_RANGE ) {
                    printf( "Range watchpoint %d at 0x%.5lX - 0x%.5lX deleted.\n", num + 1, bkpt_tbl[ num ].addr,
                            bkpt_tbl[ num ].end_addr );
                } else if ( bkpt_tbl[ num ].flags ) {
                    printf( "Watchpoint %d at 0x%.5lX deleted.\n", num + 1, bkpt_tbl[ num ].addr );
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
                    printf( "Breakpoint %d at 0x%.5lX deleted.\n", num + 1, bkpt_tbl[ num ].addr );
                } else if ( bkpt_tbl[ num ].flags == BP_RANGE ) {
                    printf( "Range watchpoint %d at 0x%.5lX - 0x%.5lX deleted.\n", num + 1, bkpt_tbl[ num ].addr,
                            bkpt_tbl[ num ].end_addr );
                } else if ( bkpt_tbl[ num ].flags ) {
                    printf( "Watchpoint %d at 0x%.5lX deleted.\n", num + 1, bkpt_tbl[ num ].addr );
                }
                num_bkpts--;
                bkpt_tbl[ num ].addr = 0;
                bkpt_tbl[ num ].flags = 0;
            }
        }
    }
}

static void cmd_go( int argc, char** argv )
{
    Address addr;

    str_to_upper( argv[ 1 ] );
    if ( decode_20( &addr, argv[ 1 ] ) ) {
        saturn.pc = addr;
        enter_debugger &= ~ILLEGAL_INSTRUCTION;
    }
}

static void cmd_help( int argc, char** argv )
{
    int i;

    for ( i = 0; cmd_tbl[ i ].name; i++ ) {
        if ( cmd_tbl[ i ].help ) {
            printf( "%s.\n", cmd_tbl[ i ].help );
        }
    }
}

static void cmd_load( int argc, char** argv )
{
    saturn_t tmp_saturn;
    device_t tmp_device;

    if ( !confirm( "Load emulator-state from files?" ) )
        return;

    memcpy( &tmp_saturn, &saturn, sizeof( saturn ) );
    memcpy( &tmp_device, &device, sizeof( device ) );
    memset( &saturn, 0, sizeof( saturn ) );

    if ( read_files() ) {
        printf( "Loading done.\n" );

        enter_debugger &= ~ILLEGAL_INSTRUCTION;
        if ( tmp_saturn.rom )
            free( tmp_saturn.rom );

        if ( tmp_saturn.ram )
            free( tmp_saturn.ram );

        if ( tmp_saturn.port1 )
            free( tmp_saturn.port1 );

        if ( tmp_saturn.port2 )
            free( tmp_saturn.port2 );

        /* After reloading state we need to refresh the UI's LCD */
        ui_update_display();
    } else {
        printf( "Loading emulator-state from files failed.\n" );
        if ( saturn.rom )
            free( saturn.rom );

        if ( saturn.ram )
            free( saturn.ram );

        if ( saturn.port1 )
            free( saturn.port1 );

        if ( saturn.port2 )
            free( saturn.port2 );

        memcpy( &saturn, &tmp_saturn, sizeof( saturn ) );
        memcpy( &device, &tmp_device, sizeof( device ) );
    }
}

static void cmd_mode( int argc, char** argv )
{
    if ( argc < 2 ) {
        printf( "Disassembler uses %s mnemonics.\n", mode_name[ disassembler_mode ] );
    } else {
        str_to_upper( argv[ 1 ] );
        if ( !strcmp( "HP", argv[ 1 ] ) ) {
            disassembler_mode = HP_MNEMONICS;
        } else if ( !strcmp( "CLASS", argv[ 1 ] ) ) {
            disassembler_mode = CLASS_MNEMONICS;
        } else {
            printf( "Unknown disassembler mode %s. Try \"help\".\n", argv[ 1 ] );
        }
    }
}

static void cmd_exit( int argc, char** argv )
{
    if ( confirm( "Exit the emulator WITHOUT saving its state?" ) ) {
        printf( "Exit.\n" );

        save_before_exit = false;
        // please_exit = true;
        close_and_exit();
    }
}

static void cmd_quit( int argc, char** argv )
{
    if ( confirm( "Quit the emulator and save its state?" ) ) {
        printf( "Exit.\n" );

        save_before_exit = true;
        // please_exit = true;
        close_and_exit();
    }
}

static void set_reg( word_64 val, int n, unsigned char* r )
{
    int i;

    for ( i = 0; i < n; i++ ) {
        r[ i ] = ( unsigned char )( ( val & ( 0xf << ( 4 * i ) ) ) >> ( 4 * i ) );
    }
}

static void dump_reg( const char* reg, int n, unsigned char* r )
{
    int i;

    printf( "%s:\t", reg );
    for ( i = n - 1; i >= 0; i-- ) {
        printf( "%.1X", r[ i ] & 0xf );
    }
    printf( "\n" );
}

static void set_st( word_64 val )
{
    int i;

    for ( i = 0; i < 16; i++ )
        saturn.pstat[ i ] = ( val & ( 1 << i ) ) ? 1 : 0;
}

static void dump_st( void )
{
    int i;
    int val;

    val = 0;
    for ( i = NB_PSTAT - 1; i >= 0; i-- ) {
        val <<= 1;
        val |= saturn.pstat[ i ] ? 1 : 0;
    }
    printf( "    ST:\t%.4X (", val );
    for ( i = NB_PSTAT - 1; i > 0; i-- ) {
        if ( saturn.pstat[ i ] ) {
            printf( "%.1X ", i );
        } else {
            printf( "- " );
        }
    }
    if ( saturn.pstat[ 0 ] ) {
        printf( "%.1X)\n", 0 );
    } else {
        printf( "-)\n" );
    }
}

static void set_hst( word_64 val )
{
    saturn.st[ XM ] = 0;
    saturn.st[ SB ] = 0;
    saturn.st[ SR ] = 0;
    saturn.st[ MP ] = 0;

    if ( val & 1 )
        saturn.st[ XM ] = 1;
    if ( val & 2 )
        saturn.st[ SB ] = 1;
    if ( val & 4 )
        saturn.st[ SR ] = 1;
    if ( val & 8 )
        saturn.st[ MP ] = 1;
}

static void dump_hst( void )
{
    short hst = 0;
    if ( saturn.st[ XM ] != 0 )
        hst |= 1;
    if ( saturn.st[ SB ] != 0 )
        hst |= 2;
    if ( saturn.st[ SR ] != 0 )
        hst |= 3;
    if ( saturn.st[ MP ] != 0 )
        hst |= 4;
    printf( "   HST:\t%.1X    (%s%s%s%s)\n", hst, saturn.st[ MP ] ? "MP " : "-- ", saturn.st[ SR ] ? "SR " : "-- ", saturn.st[ SB ] ? "SB " : "-- ",
            saturn.st[ XM ] ? "XM" : "--" );
}

static const char* mctl_str_gx[] = { "MMIO       ", "SysRAM     ", "Bank Switch", "Port 1     ", "Port 2     ", "SysROM     " };

static const char* mctl_str_sx[] = { "MMIO  ", "SysRAM", "Port 1", "Port 2", "Extra ", "SysROM" };

static void cmd_ram( int argc, char** argv )
{
    int i;

    for ( i = 0; i < 5; i++ ) {
        printf( "%s ", opt_gx ? mctl_str_gx[ i ] : mctl_str_sx[ i ] );
        if ( saturn.mem_cntl[ i ].unconfigured )
            printf( "unconfigured\n" );
        else if ( i == 0 )
            printf( "configured to 0x%.5lx\n", saturn.mem_cntl[ i ].config[ 0 ] );
        else
            printf( "configured to 0x%.5lX - 0x%.5lX\n", saturn.mem_cntl[ i ].config[ 0 ],
                    ( saturn.mem_cntl[ i ].config[ 0 ] | ~saturn.mem_cntl[ i ].config[ 1 ] ) & 0xfffff );
    }
    if ( opt_gx )
        printf( "Port 2      switched to bank %d\n", saturn.bank_switch );
}

static void cmd_regs( int argc, char** argv )
{
    int i;
    word_64 val;

    if ( argc < 2 ) {
        /*
         * dump all registers
         */
        printf( "CPU is in %s mode. Registers:\n", saturn.hexmode == HEX ? "HEX" : "DEC" );
        dump_reg( "     A", 16, saturn.reg[ A ] );
        dump_reg( "     B", 16, saturn.reg[ B ] );
        dump_reg( "     C", 16, saturn.reg[ C ] );
        dump_reg( "     D", 16, saturn.reg[ D ] );
        printf( "    D0:\t%.5lX ->", saturn.d[0] );
        for ( i = 0; i < 20; i += 5 ) {
            printf( " %s", str_nibbles( saturn.d[0] + i, 5 ) );
        }
        printf( "\n" );
        printf( "    D1:\t%.5lX ->", saturn.d[1] );
        for ( i = 0; i < 20; i += 5 ) {
            printf( " %s", str_nibbles( saturn.d[1] + i, 5 ) );
        }
        printf( "\n" );
        printf( "     P:\t%.1X\n", saturn.p );
        disassemble( saturn.pc, instr );
        printf( "    PC:\t%.5lX -> %s\n", saturn.pc, instr );
        dump_reg( "    R0", 16, saturn.reg_r[ 0 ] );
        dump_reg( "    R1", 16, saturn.reg_r[ 1 ] );
        dump_reg( "    R2", 16, saturn.reg_r[ 2 ] );
        dump_reg( "    R3", 16, saturn.reg_r[ 3 ] );
        dump_reg( "    R4", 16, saturn.reg_r[ 4 ] );
        dump_reg( "    IN", 4, saturn.in );
        dump_reg( "   OUT", 3, saturn.out );
        printf( " CARRY:\t%.1d\n", saturn.carry );
        dump_st();
        dump_hst();
    } else if ( argc == 2 ) {
        /*
         * dump specified register
         */
        str_to_upper( argv[ 1 ] );
        if ( !strcmp( "A", argv[ 1 ] ) ) {
            dump_reg( "     A", 16, saturn.reg[ A ] );
        } else if ( !strcmp( "B", argv[ 1 ] ) ) {
            dump_reg( "     B", 16, saturn.reg[ B ] );
        } else if ( !strcmp( "C", argv[ 1 ] ) ) {
            dump_reg( "     C", 16, saturn.reg[ C ] );
        } else if ( !strcmp( "D", argv[ 1 ] ) ) {
            dump_reg( "     D", 16, saturn.reg[ D ] );
        } else if ( !strcmp( "D0", argv[ 1 ] ) ) {
            printf( "    D0:\t%.5lX ->", saturn.d[0] );
            for ( i = 0; i < 20; i += 5 ) {
                printf( " %s", str_nibbles( saturn.d[0] + i, 5 ) );
            }
            printf( "\n" );
        } else if ( !strcmp( "D1", argv[ 1 ] ) ) {
            printf( "    D1:\t%.5lX ->", saturn.d[1] );
            for ( i = 0; i < 20; i += 5 ) {
                printf( " %s", str_nibbles( saturn.d[1] + i, 5 ) );
            }
            printf( "\n" );
        } else if ( !strcmp( "P", argv[ 1 ] ) ) {
            printf( "     P:\t%.1X\n", saturn.p );
        } else if ( !strcmp( "PC", argv[ 1 ] ) ) {
            disassemble( saturn.pc, instr );
            printf( "    PC:\t%.5lX -> %s\n", saturn.pc, instr );
        } else if ( !strcmp( "R0", argv[ 1 ] ) ) {
            dump_reg( "    R0", 16, saturn.reg_r[ 0 ] );
        } else if ( !strcmp( "R1", argv[ 1 ] ) ) {
            dump_reg( "    R1", 16, saturn.reg_r[ 1 ] );
        } else if ( !strcmp( "R2", argv[ 1 ] ) ) {
            dump_reg( "    R2", 16, saturn.reg_r[ 2 ] );
        } else if ( !strcmp( "R3", argv[ 1 ] ) ) {
            dump_reg( "    R3", 16, saturn.reg_r[ 3 ] );
        } else if ( !strcmp( "R4", argv[ 1 ] ) ) {
            dump_reg( "    R4", 16, saturn.reg_r[ 4 ] );
        } else if ( !strcmp( "IN", argv[ 1 ] ) ) {
            dump_reg( "    IN", 4, saturn.in );
        } else if ( !strcmp( "OUT", argv[ 1 ] ) ) {
            dump_reg( "   OUT", 3, saturn.out );
        } else if ( !strcmp( "CARRY", argv[ 1 ] ) ) {
            printf( " CARRY:\t%.1d\n", saturn.carry );
        } else if ( !strcmp( "CY", argv[ 1 ] ) ) {
            printf( " CARRY:\t%.1d\n", saturn.carry );
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
                set_reg( val, 16, saturn.reg[ A ] );
                dump_reg( "     A", 16, saturn.reg[ A ] );
            } else if ( !strcmp( "B", argv[ 1 ] ) ) {
                set_reg( val, 16, saturn.reg[ B ] );
                dump_reg( "     B", 16, saturn.reg[ B ] );
            } else if ( !strcmp( "C", argv[ 1 ] ) ) {
                set_reg( val, 16, saturn.reg[ C ] );
                dump_reg( "     C", 16, saturn.reg[ C ] );
            } else if ( !strcmp( "D", argv[ 1 ] ) ) {
                set_reg( val, 16, saturn.reg[ D ] );
                dump_reg( "     D", 16, saturn.reg[ D ] );
            } else if ( !strcmp( "D0", argv[ 1 ] ) ) {
                saturn.d[0] = ( Address )( val & 0xfffff );
                printf( "    D0:\t%.5lX ->", saturn.d[0] );
                for ( i = 0; i < 20; i += 5 ) {
                    printf( " %s", str_nibbles( saturn.d[0] + i, 5 ) );
                }
                printf( "\n" );
            } else if ( !strcmp( "D1", argv[ 1 ] ) ) {
                saturn.d[1] = ( Address )( val & 0xfffff );
                printf( "    D1:\t%.5lX ->", saturn.d[1] );
                for ( i = 0; i < 20; i += 5 ) {
                    printf( " %s", str_nibbles( saturn.d[1] + i, 5 ) );
                }
                printf( "\n" );
            } else if ( !strcmp( "P", argv[ 1 ] ) ) {
                saturn.p = ( Nibble )( val & 0xf );
                printf( "     P:\t%.1X\n", saturn.p );
            } else if ( !strcmp( "PC", argv[ 1 ] ) ) {
                saturn.pc = ( Address )( val & 0xfffff );
                disassemble( saturn.pc, instr );
                printf( "    PC:\t%.5lX -> %s\n", saturn.pc, instr );
            } else if ( !strcmp( "R0", argv[ 1 ] ) ) {
                set_reg( val, 16, saturn.reg_r[ 0 ] );
                dump_reg( "    R0", 16, saturn.reg_r[ 0 ] );
            } else if ( !strcmp( "R1", argv[ 1 ] ) ) {
                set_reg( val, 16, saturn.reg_r[ 1 ] );
                dump_reg( "    R1", 16, saturn.reg_r[ 1 ] );
            } else if ( !strcmp( "R2", argv[ 1 ] ) ) {
                set_reg( val, 16, saturn.reg_r[ 2 ] );
                dump_reg( "    R2", 16, saturn.reg_r[ 2 ] );
            } else if ( !strcmp( "R3", argv[ 1 ] ) ) {
                set_reg( val, 16, saturn.reg_r[ 3 ] );
                dump_reg( "    R3", 16, saturn.reg_r[ 3 ] );
            } else if ( !strcmp( "R4", argv[ 1 ] ) ) {
                set_reg( val, 16, saturn.reg_r[ 4 ] );
                dump_reg( "    R4", 16, saturn.reg_r[ 4 ] );
            } else if ( !strcmp( "IN", argv[ 1 ] ) ) {
                set_reg( val, 4, saturn.in );
                dump_reg( "    IN", 4, saturn.in );
            } else if ( !strcmp( "OUT", argv[ 1 ] ) ) {
                set_reg( val, 3, saturn.out );
                dump_reg( "   OUT", 3, saturn.out );
            } else if ( !strcmp( "CARRY", argv[ 1 ] ) ) {
                saturn.carry = ( Bit )( val & 0x1 );
                printf( " CARRY:\t%.1d\n", saturn.carry );
            } else if ( !strcmp( "CY", argv[ 1 ] ) ) {
                saturn.carry = ( Bit )( val & 0x1 );
                printf( " CARRY:\t%.1d\n", saturn.carry );
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

static void cmd_save( int argc, char** argv )
{
    if ( write_files() ) {
        printf( "Saving done.\n" );
    } else {
        printf( "Saving emulator-state failed.\n" );
    }
}

struct se {
    int se_n;
    Address se_p;
    struct se* se_next;
};

static void cmd_stack( int argc, char** argv )
{
    Address dsktop, dskbot;
    Address sp = 0, end = 0, ent = 0;
    Address ram_base, ram_mask;
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
            fprintf( stderr, "Out of memory.\n" );
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

static void cmd_stat( int argc, char** argv )
{
    printf( "Instructions/s: %ld\n", saturn.i_per_s );
    printf( "Timer 1 I/TICK: %d\n", saturn.t1_tick );
    printf( "Timer 2 I/TICK: %d\n", saturn.t2_tick );
}

static void cmd_step( int argc, char** argv )
{
    Address next_instr;
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

    in_debugger = true;
    step_instruction();

    if ( exec_flags & EXEC_BKPT ) {
        if ( check_breakpoint( BP_EXEC, saturn.pc ) ) {
            enter_debugger |= BREAKPOINT_HIT;
            return;
        }
    }

    next_instr = saturn.pc;

    sched_adjtime = 0;
    schedule();

    enter_debugger = 0;
    while ( 1 ) {
        if ( enter_debugger )
            break;

        leave = 0;

        if ( saturn.pc == next_instr ) {
            n--;
            leave = 1;
            if ( n == 0 )
                break;
        }

        step_instruction();

        if ( exec_flags & EXEC_BKPT ) {
            if ( check_breakpoint( BP_EXEC, saturn.pc ) ) {
                enter_debugger |= BREAKPOINT_HIT;
                break;
            }
        }

        if ( leave )
            next_instr = saturn.pc;

        schedule();
    }
}

static void cmd_reset( int argc, char** argv )
{
    if ( confirm( "Do a RESET (PC = 00000)?" ) ) {
        saturn.pc = 0;
        enter_debugger &= ~ILLEGAL_INSTRUCTION;
    }
}

static void cmd_rstk( int argc, char** argv )
{
    int i, j;

    disassemble( saturn.pc, instr );
    printf( "PC: %.5lX: %s\n", saturn.pc, instr );
    if ( saturn.rstk_ptr < 0 ) {
        printf( "Empty return stack.\n" );
    } else {
        j = 0;
        for ( i = saturn.rstk_ptr; i >= 0; i-- ) {
            disassemble( saturn.rstk[ i ], instr );
            printf( "%2d: %.5lX: %s\n", j, saturn.rstk[ i ], instr );
            j++;
        }
    }
}

int debug( void )
{
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
    if ( !config.useDebugger ) {
        if ( enter_debugger & ILLEGAL_INSTRUCTION ) {
            if ( config.verbose )
                fprintf( stderr, "reset (illegal instruction at 0x%.5lX)\n", saturn.pc );
            saturn.pc = 0;
        }
        if ( enter_debugger & USER_INTERRUPT )
            if ( config.verbose )
                printf( "usnterrupt (SIGINT) ignored\n" );

        // please_exit = true;

        if ( enter_debugger & BREAKPOINT_HIT )
            if ( config.verbose )
                printf( "breakpoint hit at 0x%.5lX ignored\n", saturn.pc );
        if ( enter_debugger & TRAP_INSTRUCTION )
            if ( config.verbose )
                printf( "trap instruction at 0x%.5lX ignored\n", saturn.pc );

        enter_debugger = 0;
        return 0;
    }

    /*
     * debugging is counted as idle time
     */
    stop_timer( RUN_TIMER );
    start_timer( IDLE_TIMER );

    continue_flag = false;

    if ( enter_debugger & ILLEGAL_INSTRUCTION ) {
        printf( "ILLEGAL INSTRUCTION at %.5lX : %s\n", saturn.pc, str_nibbles( saturn.pc, 16 ) );
    }

    if ( enter_debugger & TRAP_INSTRUCTION ) {
        printf( "TRAP at %.5lX : %s\n", saturn.pc - 5, str_nibbles( saturn.pc - 5, 16 ) );
        enter_debugger &= ~TRAP_INSTRUCTION;
    }

    do {

        /*
         * print current instruction
         */
        disassemble( saturn.pc, instr );
        printf( "%.5lX: %s\n", saturn.pc, instr );

        /*
         * read a command
         */
        rl = readline( "x48ng-debug> " );

        if ( rl == ( char* )0 ) {
            continue_flag = true;
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
        in_debugger = false;

    } while ( !continue_flag /* && !please_exit */ );

    /*
     * adjust the hp48's timers
     */
    in_debugger = true;
    ticks = get_t1_t2();
    in_debugger = false;

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
        saturn.pc = 0;
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
