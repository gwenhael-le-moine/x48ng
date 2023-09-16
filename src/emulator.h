#ifndef _HP48_H
#define _HP48_H 1

#include <stdint.h> /* int64_t */

#define NR_MCTL 6

#define P_FIELD 0  /* unused? */
#define WP_FIELD 1 /* unused? */
#define XS_FIELD 2 /* unused? */
#define X_FIELD 3  /* unused? */
#define S_FIELD 4  /* unused? */
#define M_FIELD 5  /* unused? */
#define B_FIELD 6  /* unused? */
#define W_FIELD 7
#define A_FIELD 15
#define IN_FIELD 16
#define OUT_FIELD 17
#define OUTS_FIELD 18

#define DEC 10
#define HEX 16

#define NR_RSTK 8
#define NR_PSTAT 16

#define NR_TIMERS 4

#define T1_TIMER 0
#define T2_TIMER 1 /* unused? */
#define RUN_TIMER 2
#define IDLE_TIMER 3

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

typedef unsigned char word_1;
typedef unsigned char word_4;
typedef unsigned char word_8;
typedef unsigned short word_12;
typedef unsigned short word_16;
typedef long word_20;
typedef long word_32;

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
extern long sched_timer1;
extern long sched_timer2;

extern int adj_time_pending;
extern long sched_adjtime;
extern long schedule_event;

extern char* wire_name;
extern char* ir_name;

extern device_t device;
extern display_t display;

extern saturn_t saturn;

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
extern void reset_timer( int timer );
extern void start_timer( int timer );
extern void restart_timer( int timer );
extern void stop_timer( int timer );
extern word_64 get_timer( int timer );

extern t1_t2_ticks get_t1_t2( void );
extern void set_accesstime( void );

/********************/
/* hp48emu_memory.c */
/********************/
extern void ( *write_nibble )( long addr, int val );
extern int ( *read_nibble )( long addr );
extern int ( *read_nibble_crc )( long addr );

/****************/
/* emu_memory.c */
/****************/
extern long read_nibbles( long addr, int len );
extern void dev_memory_init( void ); /*  */

/*****************/
/* emu_actions.c */
/*****************/
void push_return_addr( long addr );
long pop_return_addr( void );
void register_to_status( unsigned char* r );
void status_to_register( unsigned char* r );
void swap_register_status( unsigned char* r );
void clear_status( void );
void set_program_stat( int n );
void clear_program_stat( int n );
int get_program_stat( int n );
void clear_hardware_stat( int op );
int is_zero_hardware_stat( int op );
void set_register_bit( unsigned char* reg, int n );
void clear_register_bit( unsigned char* reg, int n );
int get_register_bit( unsigned char* reg, int n );
void set_register_nibble( unsigned char* reg, int n, unsigned char val );
unsigned char get_register_nibble( unsigned char* reg, int n );
void register_to_address( unsigned char* reg, word_20* dat, int s );
void add_address( word_20* dat, int add );
void load_constant( unsigned char* reg, int n, long addr );
void store( word_20 dat, unsigned char* reg, int code );
void store_n( word_20 dat, unsigned char* reg, int n );
void recall( unsigned char* reg, word_20 dat, int code );
void recall_n( unsigned char* reg, word_20 dat, int n );
long dat_to_addr( unsigned char* dat );
void addr_to_dat( long addr, unsigned char* dat );
extern void do_kbd_int( void );
void do_interupt( void );
void do_in( void );
void do_reset( void );
void do_configure( void );
void do_unconfigure( void );
void do_inton( void );
void do_intoff( void );
void do_return_interupt( void );
void do_reset_interrupt_system( void );
void do_shutdown( void );
int get_identification( void );

/******************/
/* emu_register.c */
/******************/
void add_p_plus_one( unsigned char* r );
void add_register_constant( unsigned char* res, int code, int val );
void sub_register_constant( unsigned char* res, int code, int val );
void add_register( unsigned char* res, unsigned char* r1, unsigned char* r2,
                   int code );
void sub_register( unsigned char* res, unsigned char* r1, unsigned char* r2,
                   int code );
void complement_2_register( unsigned char* r, int code );
void complement_1_register( unsigned char* r, int code );
void inc_register( unsigned char* r, int code );
void dec_register( unsigned char* r, int code );
void zero_register( unsigned char* r, int code );
void or_register( unsigned char* res, unsigned char* r1, unsigned char* r2,
                  int code );
void and_register( unsigned char* res, unsigned char* r1, unsigned char* r2,
                   int code );
void copy_register( unsigned char* to, unsigned char* from, int code );
void exchange_register( unsigned char* r1, unsigned char* r2, int code );
void exchange_reg( unsigned char* r, word_20* d, int code );
void shift_left_register( unsigned char* r, int code );
void shift_left_circ_register( unsigned char* r, int code );
void shift_right_register( unsigned char* r, int code );
void shift_right_circ_register( unsigned char* r, int code );
void shift_right_bit_register( unsigned char* r, int code );
int is_zero_register( unsigned char* r, int code );
int is_not_zero_register( unsigned char* r, int code );
int is_equal_register( unsigned char* r1, unsigned char* r2, int code );
int is_not_equal_register( unsigned char* r1, unsigned char* r2, int code );
int is_less_register( unsigned char* r1, unsigned char* r2, int code );
int is_less_or_equal_register( unsigned char* r1, unsigned char* r2, int code );
int is_greater_register( unsigned char* r1, unsigned char* r2, int code );
int is_greater_or_equal_register( unsigned char* r1, unsigned char* r2,
                                  int code );

/****************/
/* emu_serial.c */
/****************/
extern int init_serial( void );
extern void serial_baud( int baud );
extern void transmit_char( void );
extern void receive_char( void );

/*****************/
/* emu_emulate.c */
/*****************/
extern void emulate( void );
int step_instruction( void );
void schedule( void );
void load_addr( word_20* dat, long addr, int n );

#endif /* !_HP48_H */
