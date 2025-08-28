#ifndef _DEBUGGER_H
#  define _DEBUGGER_H 1

#  include <stdbool.h>

#  include "debugger.h"
#  include "types.h"

#  define USER_INTERRUPT 1
#  define ILLEGAL_INSTRUCTION 2
#  define TRAP_INSTRUCTION 8

#  define HP_MNEMONICS 0
#  define CLASS_MNEMONICS 1

#  define BREAKPOINT_HIT 4

/*
 * exec_flags values
 */
#  define EXEC_BKPT 1

/*
 * Breakpoint related stuff
 */
#  define BP_EXEC 1
#  define BP_READ 2
#  define BP_WRITE 4
#  define BP_RANGE 8

extern int enter_debugger;
extern bool in_debugger;
extern int exec_flags;

/**************/
/* debugger.c */
/**************/
extern int check_breakpoint( int type, word_20 addr );
extern int debug( void );

#endif /* !_DEBUGGER_H */
