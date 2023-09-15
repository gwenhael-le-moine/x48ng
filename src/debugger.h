#ifndef _DEBUGGER_H
#define _DEBUGGER_H 1

#include "emulator.h"

#define USER_INTERRUPT 1
#define ILLEGAL_INSTRUCTION 2
#define BREAKPOINT_HIT 4
#define TRAP_INSTRUCTION 8

/*
 * exec_flags values
 */
#define EXEC_BKPT 1

extern int enter_debugger;
extern int in_debugger;
extern int exec_flags;

extern void init_debugger( void );
extern int debug( void );
extern void emulate_debug( void );

extern int step_instruction( void );
extern char* str_nibbles( word_20 addr, int n );

/*************************/
/* debugger_disasm.h */
/*************************/

/* #ifndef _DISASM_H */
/* #define _DISASM_H 1 */

#define HP_MNEMONICS 0
#define CLASS_MNEMONICS 1

extern int disassembler_mode;
extern const char* mode_name[];

extern char* append_str( char* buf, const char* string );
extern char* append_tab( char* buf );
extern char* append_tab_16( char* buf );

extern word_20 disassemble( word_20 addr, char* out );

/* #endif /\* !_DISASM_H *\/ */

/*************************/
/* debugger_rpl.h */
/*************************/

/* #ifndef _RPL_H */
/* #define _RPL_H 1 */

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

typedef struct hp_real {
    word_20 x;
    word_32 ml;
    word_32 mh;
    word_4 m;
    word_1 s;
} hp_real;

extern char* decode_rpl_obj( word_20 addr, char* buf );
extern void decode_rpl_obj_2( word_20 addr, char* typ, char* dat );

extern char* skip_ob( word_20* addr, char* string );
extern char* dec_rpl_obj( word_20* addr, char* string );
extern char* dec_bin_int( word_20* addr, char* string );
extern char* dec_real( word_20* addr, char* string );
extern char* dec_long_real( word_20* addr, char* string );
extern char* dec_complex( word_20* addr, char* string );
extern char* dec_long_complex( word_20* addr, char* string );
extern char* dec_char( word_20* addr, char* string );
extern char* dec_array( word_20* addr, char* string );
extern char* dec_lnk_array( word_20* addr, char* string );
extern char* dec_string( word_20* addr, char* string );
extern char* dec_hex_string( word_20* addr, char* string );
extern char* dec_list( word_20* addr, char* string );
extern char* dec_symb( word_20* addr, char* string );
extern char* dec_unit( word_20* addr, char* string );
extern char* dec_library( word_20* addr, char* string );
extern char* dec_library_data( word_20* addr, char* string );
extern char* dec_acptr( word_20* addr, char* string );
extern char* dec_prog( word_20* addr, char* string );
extern char* dec_code( word_20* addr, char* string );
extern char* dec_global_ident( word_20* addr, char* string );
extern char* dec_local_ident( word_20* addr, char* string );
extern char* dec_xlib_name( word_20* addr, char* string );
extern char* dec_unit_op( word_20* addr, char* string );

/* #endif /\* !_RPL_H *\/ */

#endif /* !_DEBUGGER_H */
