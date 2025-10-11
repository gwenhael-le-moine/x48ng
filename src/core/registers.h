#ifndef _REGISTERS_H
#  define _REGISTERS_H 1

#  include "types.h"

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
extern void exchange_reg( unsigned char* r, Address* d, int code );
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

#endif /* !_REGISTERS_H */
