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
    address_t config[ 2 ];
} mem_cntl_t;

typedef struct saturn_t {
    unsigned long magic;
    char version[ 3 ];

    unsigned char A[ 16 ], B[ 16 ], C[ 16 ], D[ 16 ];

    address_t d[ 2 ];

#  define D0 d[ 0 ]
#  define D1 d[ 1 ]

    nibble_t P;
    address_t PC;

    unsigned char R0[ 16 ], R1[ 16 ], R2[ 16 ], R3[ 16 ], R4[ 16 ];
    unsigned char IN[ 4 ];
    unsigned char OUT[ 3 ];

    bit_t CARRY;

    unsigned char PSTAT[ NB_PSTAT ];
    unsigned char XM, SB, SR, MP;

    nibble_t hexmode;

    address_t RSTK[ NB_RSTK ];
    short rstkp;

    short keybuf[ KEYS_BUFFER_SIZE ];

    unsigned char interruptable; /* bool */
    unsigned char int_pending;   /* bool */
    unsigned char kbd_ien;       /* bool */

    nibble_t disp_io;

    nibble_t contrast_ctrl;
    byte_t disp_test;

    word_16 crc;

    nibble_t power_status;
    nibble_t power_ctrl;

    nibble_t mode;

    byte_t annunc;

    nibble_t baud;

    nibble_t card_ctrl;
    nibble_t card_status;

    nibble_t io_ctrl;
    nibble_t rcs;
    nibble_t tcs;

    byte_t rbr;
    byte_t tbr;

    byte_t sreq;

    nibble_t ir_ctrl;

    nibble_t base_off;

    nibble_t lcr;
    nibble_t lbr;

    nibble_t scratch;

    nibble_t base_nibble;

    address_t disp_addr;
    word_12 line_offset;
    byte_t line_count;

    word_16 unknown;

    nibble_t t1_ctrl;
    nibble_t t2_ctrl;

    address_t menu_addr;

    byte_t unknown2;

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

extern void load_addr( address_t* dat, long addr, int n ); /* used in debugger.c */
extern void step_instruction( void );                    /* used in debugger.c */
extern void schedule( void );                            /* used in debugger.c */

#endif /* !_EMULATE_H */
