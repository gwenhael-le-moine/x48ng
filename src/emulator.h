#ifndef _HP48_H
#define _HP48_H 1

#include <stdint.h> /* int64_t */

#define DEC 10
#define HEX 16

#define NR_MCTL 6
#define NR_RSTK 8
#define NR_PSTAT 16

#define RUN_TIMER 2
#define IDLE_TIMER 3

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

    char contrast_touched;

    char disp_test_touched;

    char crc_touched;

    char power_status_touched;
    char power_ctrl_touched;

    char mode_touched;

    char ann_touched;

    char baud_touched;

    char card_ctrl_touched;
    char card_status_touched;

    char ioc_touched;

    char tcs_touched;
    char rcs_touched;

    char rbr_touched;
    char tbr_touched;

    char sreq_touched;

    char ir_ctrl_touched;

    char base_off_touched;

    char lcr_touched;
    char lbr_touched;

    char scratch_touched;
    char base_nibble_touched;

    char unknown_touched;

    char t1_ctrl_touched;
    char t2_ctrl_touched;

    char unknown2_touched;

    char t1_touched;
    char t2_touched;
} device_t;

typedef struct keystate_t {
    short rows[ 9 ];
} keystate_t;

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

    int annunc;
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

    unsigned char PSTAT[ NR_PSTAT ];
    unsigned char XM, SB, SR, MP;

    word_4 hexmode;

    word_20 rstk[ NR_RSTK ];
    short rstkp;

    keystate_t keybuf;

    unsigned char intenable;
    unsigned char int_pending;
    unsigned char kbd_ien;

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
    mem_cntl_t mem_cntl[ NR_MCTL ];

    unsigned char* rom;
    unsigned char* ram;
    unsigned char* port1;
    unsigned char* port2;
} saturn_t;

#define NIBBLES_PER_ROW 0x22

extern int got_alarm;

extern int set_t1;

extern long sched_adjtime;
extern long schedule_event;

extern char* wire_name;
extern char* ir_name;

extern device_t device;
extern display_t display;

extern saturn_t saturn;

extern int device_check;
extern short port1_is_ram;
extern long port1_mask;
extern short port2_is_ram;
extern long port2_mask;

/**************/
/* emu_init.c */
/**************/
extern int init_emulator( void ); /* used in main.c */
extern int exit_emulator( void ); /* debugger.c; main.c; ui_sdl.c */
extern int read_files( void );    /* debugger.c */
extern int write_files( void );   /* used in debugger.c */

/***************/
/* emu_timer.c */
/***************/
extern void start_timer( int timer );
extern void stop_timer( int timer );

extern t1_t2_ticks get_t1_t2( void );
extern void set_accesstime( void );

/********************/
/* hp48emu_memory.c */
/********************/
extern int ( *read_nibble )( long addr );

/****************/
/* emu_memory.c */
/****************/
extern long read_nibbles( long addr, int len );

/*****************/
/* emu_actions.c */
/*****************/
extern void do_kbd_int( void );

/****************/
/* emu_serial.c */
/****************/
extern int init_serial( void );

/*****************/
/* emu_emulate.c */
/*****************/
extern void emulate( void );
extern int step_instruction( void );
extern void schedule( void );
extern void load_addr( word_20* dat, long addr, int n );

#endif /* !_HP48_H */
