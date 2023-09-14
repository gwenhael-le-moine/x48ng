#ifndef _HP48_EMU_H
#define _HP48_EMU_H 1

#include "hp48.h" /* word_20 */

/********************/
/* hp48emu_memory.c */
/********************/
long read_nibbles( long addr, int len );
void write_nibbles( long addr, long val, int len );
extern void dev_memory_init( void ); /*  */

/*********************/
/* hp48emu_actions.c */
/*********************/
void push_return_addr( long addr );
long pop_return_addr( void );
void register_to_status( unsigned char* r );
void status_to_register( unsigned char* r );
void swap_register_status( unsigned char* r );
void clear_status( void );
void set_program_stat( int n );
void clear_program_stat( int n );
int get_program_stat( int n );
void set_hardware_stat( int op );
void clear_hardware_stat( int op );
int is_zero_hardware_stat( int op );
void set_register_bit( unsigned char* reg, int n );
void clear_register_bit( unsigned char* reg, int n );
int get_register_bit( unsigned char* reg, int n );
void set_register_nibble( unsigned char* reg, int n, unsigned char val );
unsigned char get_register_nibble( unsigned char* reg, int n );
void register_to_address( unsigned char* reg, word_20* dat, int s );
void address_to_register( word_20 dat, unsigned char* reg, int s );
void add_address( word_20* dat, int add );
char* make_hexstr( long addr, int n );
void load_constant( unsigned char* reg, int n, long addr );
void load_address( unsigned char* reg, long addr, int n );
void store( word_20 dat, unsigned char* reg, int code );
void store_n( word_20 dat, unsigned char* reg, int n );
void recall( unsigned char* reg, word_20 dat, int code );
void recall_n( unsigned char* reg, word_20 dat, int n );
long dat_to_addr( unsigned char* dat );
void addr_to_dat( long addr, unsigned char* dat );
extern void do_kbd_int( void );
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

/**********************/
/* hp48emu_register.c */
/**********************/
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

#endif /* !_HP48_EMU_H */
