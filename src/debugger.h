#ifndef _DEBUGGER_H
#define _DEBUGGER_H 1

#include "config.h"
#include "hp48.h"

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
extern int emulate_debug( void );

extern int step_instruction( void );
extern char* str_nibbles( word_20 addr, int n );

#endif /* !_DEBUGGER_H */
