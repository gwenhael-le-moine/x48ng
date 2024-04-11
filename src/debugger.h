#ifndef _DEBUGGER_H
#define _DEBUGGER_H 1

#include "emulator.h"
#include "emulator_for_debugger.h"

#define USER_INTERRUPT 1
#define ILLEGAL_INSTRUCTION 2
#define TRAP_INSTRUCTION 8

#define HP_MNEMONICS 0
#define CLASS_MNEMONICS 1

extern int enter_debugger;
extern bool in_debugger;
extern int exec_flags;

/**************/
/* debugger.c */
/**************/
extern int debug( void );
extern void emulate_debug( void );

#endif /* !_DEBUGGER_H */
