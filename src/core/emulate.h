#ifndef _EMULATE_H
#  define _EMULATE_H 1

#  include "memory.h"
#  include "types.h"

#  define DEC 10
#  define HEX 16

#  define NB_MCTL 6
#  define NB_RSTK 8
#  define NB_PSTAT 16

typedef struct device_t {
    bool baud_touched;

    bool ioc_touched;

    bool rbr_touched;
    bool tbr_touched;

    bool t1_touched;
    bool t2_touched;
} device_t;

typedef struct mem_cntl_t {
    unsigned short unconfigured;
    Address config[ 2 ];
} mem_cntl_t;

enum RegisterNames { A, B, C, D };
enum StatusRegisterNames { XM, SB, SR, MP };

typedef struct saturn_t {
    unsigned long magic;
    char version[ 3 ];

    Nibble reg[ 4 ][ 16 ];
    Nibble reg_r[ 5 ][ 16 ];
    Address d[ 2 ];

    Nibble p;
    Address pc;

    Nibble in[ 4 ];
    Nibble out[ 3 ];
    Nibble st[ 4 ];

    Address rstk[ NB_RSTK ];
    short rstk_ptr;

    Bit hexmode;
    Bit carry;
    Bit kbd_ien;       /* bool */
    Bit interruptable; /* bool */
    Bit int_pending;   /* bool */

    unsigned char pstat[ NB_PSTAT ];

    short keybuf[ KEYS_BUFFER_SIZE ];

    Nibble disp_io;

    Nibble contrast_ctrl;
    Byte disp_test;

    word_16 crc;

    Nibble power_status;
    Nibble power_ctrl;

    Nibble mode;

    Byte annunc;

    Nibble baud;

    Nibble card_ctrl;
    Nibble card_status;

    Nibble io_ctrl;
    Nibble rcs;
    Nibble tcs;

    Byte rbr;
    Byte tbr;

    Byte sreq;

    Nibble ir_ctrl;

    Nibble base_off;

    Nibble lcr;
    Nibble lbr;

    Nibble scratch;

    Nibble base_nibble;

    Address disp_addr;
    word_12 line_offset;
    Byte line_count;

    word_16 unknown;

    Nibble t1_ctrl;
    Nibble t2_ctrl;

    Address menu_addr;

    Byte unknown2;

    char timer1; /* may NOT be unsigned !!! */
    word_32 timer2;

    long t1_instr;
    long t2_instr;

    short t1_tick;
    short t2_tick;
    long i_per_s;

    word_16 bank_switch;
    mem_cntl_t mem_cntl[ NB_MCTL ];

    unsigned char* rom;
    unsigned char* ram;
    unsigned char* port1;
    unsigned char* port2;
} saturn_t;

extern device_t device;
extern int set_t1;
extern long schedule_event;
extern long sched_adjtime;
extern bool adj_time_pending;
extern bool device_check;

extern saturn_t saturn;
extern bool sigalarm_triggered;
extern long sched_timer1;
extern long sched_timer2;
extern unsigned long t1_i_per_tick;
extern unsigned long t2_i_per_tick;

extern void do_interupt( void );
extern void do_kbd_int( void );

extern void load_addr( Address* dat, Address addr, int n ); /* used in debugger.c */
extern void step_instruction( void );                    /* used in debugger.c */
extern void schedule( void );                            /* used in debugger.c */

#endif /* !_EMULATE_H */
