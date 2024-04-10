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
int get_start( int code );
int get_end( int code );
void add_p_plus_one( unsigned char* r );
void add_register_constant( unsigned char* res, int code, int val );
void sub_register_constant( unsigned char* res, int code, int val );
void add_register( unsigned char* res, unsigned char* r1, unsigned char* r2, int code );
void sub_register( unsigned char* res, unsigned char* r1, unsigned char* r2, int code );
void complement_2_register( unsigned char* r, int code );
void complement_1_register( unsigned char* r, int code );
void inc_register( unsigned char* r, int code );
void dec_register( unsigned char* r, int code );
void zero_register( unsigned char* r, int code );
void or_register( unsigned char* res, unsigned char* r1, unsigned char* r2, int code );
void and_register( unsigned char* res, unsigned char* r1, unsigned char* r2, int code );
void copy_register( unsigned char* to, unsigned char* from, int code );
void exchange_register( unsigned char* r1, unsigned char* r2, int code );
void exchange_reg( unsigned char* r, word_20* d, int code );
void shift_left_register( unsigned char* r, int code );
void shift_left_circ_register( unsigned char* r, int code );
void shift_right_register( unsigned char* r, int code );
void shift_right_circ_register( unsigned char* r, int code );
void shift_right_bit_register( unsigned char* r, int code );
int is_zero_register( unsigned char* r, int code );
int is_not_zero_register( unsigned char* r, int code );
int is_equal_register( unsigned char* r1, unsigned char* r2, int code );
int is_not_equal_register( unsigned char* r1, unsigned char* r2, int code );
int is_less_register( unsigned char* r1, unsigned char* r2, int code );
int is_less_or_equal_register( unsigned char* r1, unsigned char* r2, int code );
int is_greater_register( unsigned char* r1, unsigned char* r2, int code );
int is_greater_or_equal_register( unsigned char* r1, unsigned char* r2, int code );

/****************/
/* emu_serial.c */
/****************/
extern void serial_baud( int baud );
extern void transmit_char( void );
extern void receive_char( void );

#endif /* _EMULATOR_INNER_H */
