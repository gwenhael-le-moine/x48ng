/*
 *  This file is part of x48, an emulator of the HP-48sx Calculator.
 *  Copyright (C) 1994  Eddie C. Dost  (ecd@dressler.de)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* $Log: hp48_emu.h,v $
 * Revision 1.10  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.9  1994/11/28  02:19:22  ecd
 * removed progname declaration
 *
 * Revision 1.9  1994/11/28  02:19:22  ecd
 * removed progname declaration
 *
 * Revision 1.8  1994/11/04  03:44:47  ecd
 * wrxl
 *
 * Revision 1.7  1994/11/02  14:51:27  ecd
 * minor changes
 *
 * Revision 1.6  1994/10/05  08:33:22  ecd
 * deleted do_interupt definition
 *
 * Revision 1.5  1994/10/01  10:12:24  ecd
 * deleted get_start and get_end functions
 *
 * Revision 1.4  1994/09/30  12:32:49  ecd
 * changed display* routines
 *
 * Revision 1.3  1994/09/13  16:58:42  ecd
 * changed to plain X11
 *
 * Revision 1.2  1994/08/31  18:25:23  ecd
 * some cleanup
 *
 * Revision 1.1  1994/08/26  11:09:18  ecd
 * Initial revision
 *
 *
 *
 * $Id: hp48_emu.h,v 1.10 1995/01/11 18:20:01 ecd Exp ecd $
 */

#ifndef _HP48_EMU_H
#define _HP48_EMU_H 1

#include "global.h"

#include <X11/Xlib.h>

#include "hp48.h"
extern Display* dpy;
extern Window dispW;
extern GC gc;

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
