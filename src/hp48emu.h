#ifndef _HP48_EMU_H
#define _HP48_EMU_H 1

#include "hp48.h"

extern void push_return_addr( long addr ); /* hp48emu_actions.c */
extern long pop_return_addr( void );       /* hp48emu_actions.c */

extern void register_to_status( unsigned char* r );   /* hp48emu_actions.c */
extern void status_to_register( unsigned char* r );   /* hp48emu_actions.c */
extern void swap_register_status( unsigned char* r ); /* hp48emu_actions.c */
extern void clear_status( void );                     /* hp48emu_actions.c */

extern long read_nibbles( long addr, int len ); /* hp48emu_memory.c */
extern void write_nibbles( long addr, long val,
                           int len ); /* hp48emu_memory.c */
extern void dev_memory_init( void );  /* hp48emu_memory.c */

extern long read_nibble_from_rom( long addr ); /* hp48emu_memory_*.c */

extern void set_program_stat( int n );   /* hp48emu_actions.c */
extern void clear_program_stat( int n ); /* hp48emu_actions.c */
extern int get_program_stat( int n );    /* hp48emu_actions.c */

extern void set_hardware_stat( int op );    /* hp48emu_actions.c */
extern void clear_hardware_stat( int op );  /* hp48emu_actions.c */
extern int is_zero_hardware_stat( int op ); /* hp48emu_actions.c */

extern void set_register_bit( unsigned char* reg,
                              int n ); /* hp48emu_actions.c */
extern void clear_register_bit( unsigned char* reg,
                                int n ); /* hp48emu_actions.c */
extern int get_register_bit( unsigned char* reg,
                             int n ); /* hp48emu_actions.c */

extern void set_register_nibble( unsigned char* reg, int n,
                                 unsigned char val ); /* hp48emu_actions.c */
extern unsigned char get_register_nibble( unsigned char* reg,
                                          int n ); /* hp48emu_actions.c */

extern void register_to_address( unsigned char* reg, word_20* dat,
                                 int s ); /* hp48emu_actions.c */
extern void address_to_register( word_20 dat, unsigned char* reg,
                                 int s );         /* hp48emu_actions.c */
extern void add_address( word_20* dat, int add ); /* hp48emu_actions.c */

extern char* make_hexstr( long addr, int n ); /* hp48emu_actions.c */
extern void load_constant( unsigned char* reg, int n,
                           long addr ); /* hp48emu_actions.c */
extern void load_address( unsigned char* reg, long addr,
                          int n ); /* hp48emu_actions.c */

extern void store( word_20 dat, unsigned char* reg,
                   int code ); /* hp48emu_actions.c */
extern void store_n( word_20 dat, unsigned char* reg,
                     int n ); /* hp48emu_actions.c */
extern void recall( unsigned char* reg, word_20 dat,
                    int code ); /* hp48emu_actions.c */
extern void recall_n( unsigned char* reg, word_20 dat,
                      int n ); /* hp48emu_actions.c */

extern long dat_to_addr( unsigned char* dat ); /* hp48emu_actions.c */
extern void addr_to_dat( long addr,
                         unsigned char* dat ); /* hp48emu_actions.c */

extern void do_in( void );                     /* hp48emu_actions.c */
extern void do_reset( void );                  /* hp48emu_actions.c */
extern void do_configure( void );              /* hp48emu_actions.c */
extern void do_unconfigure( void );            /* hp48emu_actions.c */
extern void do_inton( void );                  /* hp48emu_actions.c */
extern void do_intoff( void );                 /* hp48emu_actions.c */
extern void do_return_interupt( void );        /* hp48emu_actions.c */
extern void do_reset_interrupt_system( void ); /* hp48emu_actions.c */
extern void do_shutdown( void );               /* hp48emu_actions.c */
extern int get_identification( void );         /* hp48emu_actions.c */

extern void add_p_plus_one( unsigned char* r ); /* hp48emu_register.c */
extern void add_register_constant( unsigned char* res, int code,
                                   int val ); /* hp48emu_register.c */
extern void sub_register_constant( unsigned char* res, int code,
                                   int val ); /* hp48emu_register.c */
extern void add_register( unsigned char* res, unsigned char* r1,
                          unsigned char* r2,
                          int code ); /* hp48emu_register.c */
extern void sub_register( unsigned char* res, unsigned char* r1,
                          unsigned char* r2,
                          int code ); /* hp48emu_register.c */
extern void complement_2_register( unsigned char* r,
                                   int code ); /* hp48emu_register.c */
extern void complement_1_register( unsigned char* r,
                                   int code );          /* hp48emu_register.c */
extern void inc_register( unsigned char* r, int code ); /* hp48emu_register.c */
extern void dec_register( unsigned char* r, int code ); /* hp48emu_register.c */
extern void zero_register( unsigned char* r,
                           int code ); /* hp48emu_register.c */
extern void or_register( unsigned char* res, unsigned char* r1,
                         unsigned char* r2, int code ); /* hp48emu_register.c */
extern void and_register( unsigned char* res, unsigned char* r1,
                          unsigned char* r2,
                          int code ); /* hp48emu_register.c */
extern void copy_register( unsigned char* to, unsigned char* from,
                           int code ); /* hp48emu_register.c */
extern void exchange_register( unsigned char* r1, unsigned char* r2,
                               int code ); /* hp48emu_register.c */

extern void exchange_reg( unsigned char* r, word_20* d,
                          int code ); /* hp48emu_register.c */

extern void shift_left_register( unsigned char* r,
                                 int code ); /* hp48emu_register.c */
extern void shift_left_circ_register( unsigned char* r,
                                      int code ); /* hp48emu_register.c */
extern void shift_right_register( unsigned char* r,
                                  int code ); /* hp48emu_register.c */
extern void shift_right_circ_register( unsigned char* r,
                                       int code ); /* hp48emu_register.c */
extern void shift_right_bit_register( unsigned char* r,
                                      int code ); /* hp48emu_register.c */
extern int is_zero_register( unsigned char* r,
                             int code ); /* hp48emu_register.c */
extern int is_not_zero_register( unsigned char* r,
                                 int code ); /* hp48emu_register.c */
extern int is_equal_register( unsigned char* r1, unsigned char* r2,
                              int code ); /* hp48emu_register.c */
extern int is_not_equal_register( unsigned char* r1, unsigned char* r2,
                                  int code ); /* hp48emu_register.c */
extern int is_less_register( unsigned char* r1, unsigned char* r2,
                             int code ); /* hp48emu_register.c */
extern int is_less_or_equal_register( unsigned char* r1, unsigned char* r2,
                                      int code ); /* hp48emu_register.c */
extern int is_greater_register( unsigned char* r1, unsigned char* r2,
                                int code ); /* hp48emu_register.c */
extern int is_greater_or_equal_register( unsigned char* r1, unsigned char* r2,
                                         int code ); /* hp48emu_register.c */

#endif /* !_HP48_EMU_H */
