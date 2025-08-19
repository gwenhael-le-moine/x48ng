#ifndef _EMULATOR_FOR_DEBUGGER_H
#define _EMULATOR_FOR_DEBUGGER_H 1

#include "emulator_core.h"

extern device_t device;

extern int set_t1;

extern long sched_adjtime;
extern long schedule_event;

/**************/
/* emu_init.c */
/**************/
extern void exit_emulator( void ); /* used in debugger.c; main.c; ui_*.c */
extern int read_files( void );     /* used in debugger.c */
extern int write_files( void );    /* used in debugger.c */

/***************/
/* emu_timer.c */
/***************/
extern void start_timer( int timer ); /* used in debugger.c */
extern void stop_timer( int timer );  /* used in debugger.c */
extern t1_t2_ticks get_t1_t2( void ); /* used in debugger.c */
extern void set_accesstime( void );   /* used in debugger.c */

/********************/
/* emu_memory.c */
/********************/
extern int ( *read_nibble )( long addr ); /* used in debugger.c; ui_*.c */

/****************/
/* emu_memory.c */
/****************/
extern long read_nibbles( long addr, int len ); /* used in debugger.c */

/*****************/
/* emu_emulate.c */
/*****************/
extern void load_addr( word_20* dat, long addr, int n ); /* used in debugger.c */
extern void step_instruction( void );                    /* used in debugger.c */
extern void schedule( void );                            /* used in debugger.c */

#endif /* !_EMULATOR_FOR_DEBUGGER_H */
