#ifndef _HP48_EMU_H
#define _HP48_EMU_H 1

#include "config.h"
#include "hp48.h"

#ifdef GUI_IS_X11
#include <X11/Xlib.h>

extern Display* dpy;
extern Window dispW;
extern GC gc;
#endif

extern void push_return_addr( long addr );
extern long pop_return_addr( void );

extern void init_annunc( void );

extern void init_saturn( void );

extern void check_timer( void );

extern void register_to_status( unsigned char* r );
extern void status_to_register( unsigned char* r );
extern void swap_register_status( unsigned char* r );
extern void clear_status( void );

extern long read_nibbles( long addr, int len );
extern void write_nibbles( long addr, long val, int len );
extern void dev_memory_init( void );

extern void set_program_stat( int n );
extern void clear_program_stat( int n );
extern int get_program_stat( int n );

extern void set_hardware_stat( int op );
extern void clear_hardware_stat( int op );
extern int is_zero_hardware_stat( int op );

extern void set_register_bit( unsigned char* reg, int n );
extern void clear_register_bit( unsigned char* reg, int n );
extern int get_register_bit( unsigned char* reg, int n );

extern void set_register_nibble( unsigned char* reg, int n, unsigned char val );
extern unsigned char get_register_nibble( unsigned char* reg, int n );

extern void register_to_address( unsigned char* reg, word_20* dat, int s );
extern void address_to_register( word_20 dat, unsigned char* reg, int s );
extern void add_address( word_20* dat, int add );

extern char* make_hexstr( long addr, int n );
extern void load_constant( unsigned char* reg, int n, long addr );
extern void load_address( unsigned char* reg, long addr, int n );

extern void store( word_20 dat, unsigned char* reg, int code );
extern void store_n( word_20 dat, unsigned char* reg, int n );
extern void recall( unsigned char* reg, word_20 dat, int code );
extern void recall_n( unsigned char* reg, word_20 dat, int n );

extern long dat_to_addr( unsigned char* dat );
extern void addr_to_dat( long addr, unsigned char* dat );

extern void do_in( void );
extern void do_reset( void );
extern void do_configure( void );
extern void do_unconfigure( void );
extern void do_inton( void );
extern void do_intoff( void );
extern void do_return_interupt( void );
extern void do_reset_interrupt_system( void );
extern void do_shutdown( void );
extern int get_identification( void );

extern void add_p_plus_one( unsigned char* r );
extern void add_register_constant( unsigned char* res, int code, int val );
extern void sub_register_constant( unsigned char* res, int code, int val );
extern void add_register( unsigned char* res, unsigned char* r1,
                          unsigned char* r2, int code );
extern void sub_register( unsigned char* res, unsigned char* r1,
                          unsigned char* r2, int code );
extern void complement_2_register( unsigned char* r, int code );
extern void complement_1_register( unsigned char* r, int code );
extern void inc_register( unsigned char* r, int code );
extern void dec_register( unsigned char* r, int code );
extern void zero_register( unsigned char* r, int code );
extern void or_register( unsigned char* res, unsigned char* r1,
                         unsigned char* r2, int code );
extern void and_register( unsigned char* res, unsigned char* r1,
                          unsigned char* r2, int code );
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
extern int is_not_equal_register( unsigned char* r1, unsigned char* r2,
                                  int code );
extern int is_less_register( unsigned char* r1, unsigned char* r2, int code );
extern int is_less_or_equal_register( unsigned char* r1, unsigned char* r2,
                                      int code );
extern int is_greater_register( unsigned char* r1, unsigned char* r2,
                                int code );
extern int is_greater_or_equal_register( unsigned char* r1, unsigned char* r2,
                                         int code );

#endif /* !_HP48_EMU_H */
