#ifndef _TIMERS_H
#define _TIMERS_H 1

#include "types.h"

#define T1_TIMER 0
/* #define T2_TIMER 1 /\* unused? *\/ */

#define RUN_TIMER 2
#define IDLE_TIMER 3

/* LCD refresh rate is 64Hz according to https://www.hpcalc.org/hp48/docs/faq/48faq-6.html */
#define UI_REFRESH_RATE_Hz 64
#define USEC_PER_FRAME ( 1000000 / UI_REFRESH_RATE_Hz )

typedef struct t1_t2_ticks {
    unsigned long t1_ticks;
    unsigned long t2_ticks;
} t1_t2_ticks;

extern void restart_timer( int timer );
extern word_64 get_timer( int timer );

extern void start_timer( int timer ); /* used in debugger.c */
extern void stop_timer( int timer );  /* used in debugger.c */
extern void reset_timer( int timer );
extern t1_t2_ticks get_t1_t2( void ); /* used in debugger.c */
extern void set_accesstime( void );   /* used in debugger.c */

#endif /* !_TIMERS_H */
