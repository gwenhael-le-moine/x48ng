#ifndef _EMULATE_H
#define _EMULATE_H 1

#include "emulator_ui4x_api.h"
#include "memory.h"
#include "types.h"

#define DEC 10
#define HEX 16

#define NB_MCTL 6
#define NB_RSTK 8
#define NB_PSTAT 16

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
    word_20 config[ 2 ];
} mem_cntl_t;

typedef struct saturn_t {
    unsigned long magic;
    char version[ 3 ];

    unsigned char A[ 16 ], B[ 16 ], C[ 16 ], D[ 16 ];

    word_20 d[ 2 ];

#define D0 d[ 0 ]
#define D1 d[ 1 ]

    word_4 P;
    word_20 PC;

    unsigned char R0[ 16 ], R1[ 16 ], R2[ 16 ], R3[ 16 ], R4[ 16 ];
    unsigned char IN[ 4 ];
    unsigned char OUT[ 3 ];

    word_1 CARRY;

    unsigned char PSTAT[ NB_PSTAT ];
    unsigned char XM, SB, SR, MP;

    word_4 hexmode;

    word_20 RSTK[ NB_RSTK ];
    short rstkp;

    short keybuf[ KEYS_BUFFER_SIZE ];

    unsigned char interruptable; /* bool */
    unsigned char int_pending;   /* bool */
    unsigned char kbd_ien;       /* bool */

    word_4 disp_io;

    word_4 contrast_ctrl;
    word_8 disp_test;

    word_16 crc;

    word_4 power_status;
    word_4 power_ctrl;

    word_4 mode;

    word_8 annunc;

    word_4 baud;

    word_4 card_ctrl;
    word_4 card_status;

    word_4 io_ctrl;
    word_4 rcs;
    word_4 tcs;

    word_8 rbr;
    word_8 tbr;

    word_8 sreq;

    word_4 ir_ctrl;

    word_4 base_off;

    word_4 lcr;
    word_4 lbr;

    word_4 scratch;

    word_4 base_nibble;

    word_20 disp_addr;
    word_12 line_offset;
    word_8 line_count;

    word_16 unknown;

    word_4 t1_ctrl;
    word_4 t2_ctrl;

    word_20 menu_addr;

    word_8 unknown2;

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

extern void load_addr( word_20* dat, long addr, int n ); /* used in debugger.c */
extern void step_instruction( void );                    /* used in debugger.c */
extern void schedule( void );                            /* used in debugger.c */

#endif /* !_EMULATE_H */
