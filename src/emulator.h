#ifndef _HP48_H
#define _HP48_H 1

#include <stdint.h> /* int64_t */
#include <stdbool.h>

#define DEC 10
#define HEX 16

#define NR_MCTL 6
#define NR_RSTK 8
#define NR_PSTAT 16

#define RUN_TIMER 2
#define IDLE_TIMER 3

// Keys
#define NB_KEYS 49

#define HPKEY_A 0
#define HPKEY_B 1
#define HPKEY_C 2
#define HPKEY_D 3
#define HPKEY_E 4
#define HPKEY_F 5

#define HPKEY_MTH 6
#define HPKEY_PRG 7
#define HPKEY_CST 8
#define HPKEY_VAR 9
#define HPKEY_UP 10
#define HPKEY_NXT 11

#define HPKEY_COLON 12
#define HPKEY_STO 13
#define HPKEY_EVAL 14
#define HPKEY_LEFT 15
#define HPKEY_DOWN 16
#define HPKEY_RIGHT 17

#define HPKEY_SIN 18
#define HPKEY_COS 19
#define HPKEY_TAN 20
#define HPKEY_SQRT 21
#define HPKEY_POWER 22
#define HPKEY_INV 23

#define HPKEY_ENTER 24
#define HPKEY_NEG 25
#define HPKEY_EEX 26
#define HPKEY_DEL 27
#define HPKEY_BS 28

#define HPKEY_ALPHA 29
#define HPKEY_7 30
#define HPKEY_8 31
#define HPKEY_9 32
#define HPKEY_DIV 33

#define HPKEY_SHL 34
#define HPKEY_4 35
#define HPKEY_5 36
#define HPKEY_6 37
#define HPKEY_MUL 38

#define HPKEY_SHR 39
#define HPKEY_1 40
#define HPKEY_2 41
#define HPKEY_3 42
#define HPKEY_MINUS 43

#define HPKEY_ON 44
#define HPKEY_0 45
#define HPKEY_PERIOD 46
#define HPKEY_SPC 47
#define HPKEY_PLUS 48

#define FIRST_HPKEY HPKEY_A
#define LAST_HPKEY HPKEY_PLUS

// Annunciators
#define NB_ANNUNCIATORS 6

#define ANN_LEFT 0x81
#define ANN_RIGHT 0x82
#define ANN_ALPHA 0x84
#define ANN_BATTERY 0x88
#define ANN_BUSY 0x90
#define ANN_IO 0xa0

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

typedef struct hpkey_t {
    int code;
    short pressed;
} hpkey_t;

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

    unsigned char PSTAT[ NR_PSTAT ];
    unsigned char XM, SB, SR, MP;

    word_4 hexmode;

    word_20 rstk[ NR_RSTK ];
    short rstkp;

    keystate_t keybuf;

    unsigned char interruptable;
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
#define NIBBLES_NB_BITS 4

extern bool got_alarm;

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

extern hpkey_t keyboard[ NB_KEYS ];

extern int annunciators_bits[ NB_ANNUNCIATORS ];

/**************/
/* emu_init.c */
/**************/
extern void init_display( void );  /* used in ui_*.c */
extern int init_emulator( void );  /* used in main.c */
extern void exit_emulator( void ); /* debugger.c; main.c; ui_*.c */
extern int read_files( void );     /* debugger.c */
extern int write_files( void );    /* used in debugger.c */

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
extern void press_key( int hpkey );
extern void release_key( int hpkey );
extern void release_all_keys( void );

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
