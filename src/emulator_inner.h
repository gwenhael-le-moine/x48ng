#ifndef _EMULATOR_INNER_H
#define _EMULATOR_INNER_H 1

#include "emulator.h"

#define OUT_FIELD 17

#define T1_TIMER 0
#define T2_TIMER 1 /* unused? */

extern int adj_time_pending;

/***************/
/* emu_timer.c */
/***************/
extern void reset_timer( int timer );
extern void restart_timer( int timer );
extern word_64 get_timer( int timer );

/********************/
/* hp48emu_memory.c */
/********************/
extern void ( *write_nibble )( long addr, int val );
extern int ( *read_nibble_crc )( long addr );

/****************/
/* emu_memory.c */
/****************/
extern void dev_memory_init( void ); /*  */

/*****************/
/* emu_actions.c */
/*****************/
void push_return_addr( long addr );
long pop_return_addr( void );
void register_to_status( unsigned char* r );
void status_to_register( unsigned char* r );
void swap_register_status( unsigned char* r );
void clear_status( void );
void set_program_stat( int n );
void clear_program_stat( int n );
int get_program_stat( int n );
void clear_hardware_stat( int op );
int is_zero_hardware_stat( int op );
void set_register_bit( unsigned char* reg, int n );
void clear_register_bit( unsigned char* reg, int n );
int get_register_bit( unsigned char* reg, int n );
void set_register_nibble( unsigned char* reg, int n, unsigned char val );
unsigned char get_register_nibble( unsigned char* reg, int n );
void register_to_address( unsigned char* reg, word_20* dat, int s );
void add_address( word_20* dat, int add );
void load_constant( unsigned char* reg, int n, long addr );
void store( word_20 dat, unsigned char* reg, int code );
void store_n( word_20 dat, unsigned char* reg, int n );
void recall( unsigned char* reg, word_20 dat, int code );
void recall_n( unsigned char* reg, word_20 dat, int n );
long dat_to_addr( unsigned char* dat );
void addr_to_dat( long addr, unsigned char* dat );
void do_interupt( void );
void do_in( void );
void do_reset( void );
void do_configure( void );
void do_unconfigure( void );
void do_inton( void );
void do_intoff( void );
void do_return_interupt( void );
void do_reset_interrupt_system( void );
void do_shutdown( void );
int get_identification( void );

/******************/
/* emu_register.c */
/******************/
void add_p_plus_one( unsigned char* r );
void add_register_constant( unsigned char* res, int code, int val );
void sub_register_constant( unsigned char* res, int code, int val );
void add_register( unsigned char* res, unsigned char* r1, unsigned char* r2,
                   int code );
void sub_register( unsigned char* res, unsigned char* r1, unsigned char* r2,
                   int code );
void complement_2_register( unsigned char* r, int code );
void complement_1_register( unsigned char* r, int code );
void inc_register( unsigned char* r, int code );
void dec_register( unsigned char* r, int code );
void zero_register( unsigned char* r, int code );
void or_register( unsigned char* res, unsigned char* r1, unsigned char* r2,
                  int code );
void and_register( unsigned char* res, unsigned char* r1, unsigned char* r2,
                   int code );
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
int is_greater_or_equal_register( unsigned char* r1, unsigned char* r2,
                                  int code );

/****************/
/* emu_serial.c */
/****************/
extern void serial_baud( int baud );
extern void transmit_char( void );
extern void receive_char( void );

#endif /* _EMULATOR_INNER_H */
