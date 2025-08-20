#ifndef _EMULATOR_CORE_H
#define _EMULATOR_CORE_H 1

#include <stdint.h> /* int64_t */
#include <stdbool.h>

#include "emulator.h"

#define T1_TIMER 0
/* #define T2_TIMER 1 /\* unused? *\/ */

#define DEC 10
#define HEX 16

#define NB_MCTL 6
#define NB_RSTK 8
#define NB_PSTAT 16

#define RUN_TIMER 2
#define IDLE_TIMER 3

/* LCD refresh rate is 64Hz according to https://www.hpcalc.org/hp48/docs/faq/48faq-6.html */
#define UI_REFRESH_RATE_Hz 64
#define USEC_PER_FRAME ( 1000000 / UI_REFRESH_RATE_Hz )

#define NIBBLES_PER_ROW 34
#define NIBBLES_NB_BITS 4

#define DISP_ROWS 64
#define NIBS_PER_BUFFER_ROW ( NIBBLES_PER_ROW + 2 )

#define KEYS_BUFFER_SIZE 9

typedef unsigned char word_1;
typedef unsigned char word_4;
typedef unsigned char word_8;
typedef unsigned short word_12;
typedef unsigned short word_16;
typedef long word_20;
typedef long word_32;
typedef int64_t word_64;

typedef struct t1_t2_ticks {
    unsigned long t1_ticks;
    unsigned long t2_ticks;
} t1_t2_ticks;

typedef struct device_t {
    int display_touched;

    bool contrast_touched;

    bool ann_touched;

    bool baud_touched;

    bool ioc_touched;

    bool rbr_touched;
    bool tbr_touched;

    bool t1_touched;
    bool t2_touched;
} device_t;

typedef struct display_t {
    int on;

    long disp_start;
    long disp_end;

    int offset;
    int lines;
    int nibs_per_line;

    int contrast;

    long menu_start;
    long menu_end;
} display_t;

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

/*****************/
/* emu_emulate.c */
/*****************/
extern saturn_t saturn;
extern bool sigalarm_triggered;
extern long sched_timer1;
extern long sched_timer2;
extern unsigned long t1_i_per_tick;
extern unsigned long t2_i_per_tick;

/***************/
/* emu_timer.c */
/***************/
extern void reset_timer( int timer );

/**************/
/* emu_init.c */
/**************/
// extern hpkey_t keyboard[ NB_KEYS ];
extern int annunciators_bits[ NB_ANNUNCIATORS ];
// extern bool please_exit;
extern bool save_before_exit;

extern void start_emulator( void ); /* used in main.c */
extern void stop_emulator( void );  /* used in debugger.c; ui_*.c */

/********************/
/* emu_memory.c */
/********************/
extern unsigned char lcd_nibbles_buffer[ DISP_ROWS ][ NIBS_PER_BUFFER_ROW ];
extern display_t display;

extern int ( *read_nibble )( long addr ); /* used in debugger.c; ui_*.c */

/******************/
/* emu_keyboard.c */
/******************/
extern void press_key( int hpkey );   /* used in ui_*.c */
extern void release_key( int hpkey ); /* used in ui_*.c */
extern void release_all_keys( void ); /* used in ui_*.c */

#endif /* !_EMULATOR_CORE_H */
