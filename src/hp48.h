#ifndef _HP48_H
#define _HP48_H 1

#include "config.h"

#include <sys/time.h>

#include "mmu.h"
#include <stdint.h>

#define RAM_SIZE_SX 0x10000
#define RAM_SIZE_GX 0x40000

#define P_FIELD 0
#define WP_FIELD 1
#define XS_FIELD 2
#define X_FIELD 3
#define S_FIELD 4
#define M_FIELD 5
#define B_FIELD 6
#define W_FIELD 7
#define A_FIELD 15
#define IN_FIELD 16
#define OUT_FIELD 17
#define OUTS_FIELD 18

#define DEC 10
#define HEX 16

#define NR_RSTK 8
#define NR_PSTAT 16

typedef unsigned char word_1;
typedef unsigned char word_4;
typedef unsigned char word_8;
typedef unsigned short word_12;
typedef unsigned short word_16;
typedef long word_20;
typedef long word_32;

#define SIMPLE_64
typedef int64_t word_64;

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
    char version[ 4 ];

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

#if defined( GUI_IS_SDL1 )
#define DISP_ROWS 64
#define NIBS_PER_BUFFER_ROW ( NIBBLES_PER_ROW + 2 )

extern unsigned char disp_buf[ DISP_ROWS ][ NIBS_PER_BUFFER_ROW ];
extern unsigned char lcd_buffer[ DISP_ROWS ][ NIBS_PER_BUFFER_ROW ];
#endif

extern int got_alarm;

extern int set_t1;
extern long sched_timer1;
extern long sched_timer2;

extern int adj_time_pending;
extern long sched_adjtime;
extern long schedule_event;

extern display_t display;
extern void init_display();

extern saturn_t saturn;

extern int exit_emulator();
extern int init_emulator();
extern void init_active_stuff();

extern int serial_init();
extern void serial_baud( int baud );
extern void transmit_char();
extern void receive_char();

extern void do_kbd_int();
extern void do_interupt();

extern void ( *write_nibble )( long addr, int val );
extern int ( *read_nibble )( long addr );
extern int ( *read_nibble_crc )( long addr );

extern int emulate();
extern int step_instruction();
extern void schedule();

extern int read_rom( const char* fname );
extern int read_files();
extern int write_files();

extern void load_addr( word_20* dat, long addr, int n );
#endif /* !_HP48_H */
