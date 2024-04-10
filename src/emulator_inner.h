#ifndef _EMULATOR_INNER_H
#define _EMULATOR_INNER_H 1

#include "emulator.h"

#define OUT_FIELD 17

#define T1_TIMER 0
/* #define T2_TIMER 1 /\* unused? *\/ */

extern bool adj_time_pending;

extern int start_fields[ 19 ];
extern int end_fields[ 19 ];

/***************/
/* emu_timer.c */
/***************/
extern void reset_timer( int timer );
extern void restart_timer( int timer );
extern word_64 get_timer( int timer );

/*****************/
/* emu_emulate.c */
/*****************/
extern void do_interupt( void );
extern void do_kbd_int( void );

/********************/
/* emu_memory.c */
/********************/
extern void ( *write_nibble )( long addr, int val );
extern int ( *read_nibble_crc )( long addr );

/****************/
/* emu_memory.c */
/****************/
extern void dev_memory_init( void ); /*  */

/******************/
/* emu_register.c */
/******************/
extern int get_start( int code );
extern int get_end( int code );
extern void add_p_plus_one( unsigned char* r );
extern void add_register_constant( unsigned char* res, int code, int val );
extern void sub_register_constant( unsigned char* res, int code, int val );
extern void add_register( unsigned char* res, unsigned char* r1, unsigned char* r2, int code );
extern void sub_register( unsigned char* res, unsigned char* r1, unsigned char* r2, int code );
extern void complement_2_register( unsigned char* r, int code );
extern void complement_1_register( unsigned char* r, int code );
extern void inc_register( unsigned char* r, int code );
extern void dec_register( unsigned char* r, int code );
extern void zero_register( unsigned char* r, int code );
extern void or_register( unsigned char* res, unsigned char* r1, unsigned char* r2, int code );
extern void and_register( unsigned char* res, unsigned char* r1, unsigned char* r2, int code );
extern void copy_register( unsigned char* to, unsigned char* from, int code );
extern void exchange_register( unsigned char* r1, unsigned char* r2, int code );
extern void exchange_reg( unsigned char* r, word_20* d, int code );
extern void shift_left_register( unsigned char* r, int code );
extern void shift_left_circ_register( unsigned char* r, int code );
extern void shift_right_register( unsigned char* r, int code );
extern void shift_right_circ_register( unsigned char* r, int code );
extern void shift_right_bit_register( unsigned char* r, int code );
extern int is_zero_register( unsigned char* r, int code );
extern int is_not_zero_register( unsigned char* r, int code );
extern int is_equal_register( unsigned char* r1, unsigned char* r2, int code );
extern int is_not_equal_register( unsigned char* r1, unsigned char* r2, int code );
extern int is_less_register( unsigned char* r1, unsigned char* r2, int code );
extern int is_less_or_equal_register( unsigned char* r1, unsigned char* r2, int code );
extern int is_greater_register( unsigned char* r1, unsigned char* r2, int code );
extern int is_greater_or_equal_register( unsigned char* r1, unsigned char* r2, int code );

/****************/
/* emu_serial.c */
/****************/
extern void serial_baud( int baud );
extern void transmit_char( void );
extern void receive_char( void );
extern int init_serial( void );

#endif /* _EMULATOR_INNER_H */
