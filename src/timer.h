#ifndef _TIMER_H
#define _TIMER_H 1

#include "hp48.h" /* word_64 */

#define NR_TIMERS 4

#define T1_TIMER 0
#define T2_TIMER 1 /* unused? */
#define RUN_TIMER 2
#define IDLE_TIMER 3

typedef struct t1_t2_ticks {
    unsigned long t1_ticks;
    unsigned long t2_ticks;
} t1_t2_ticks;

extern void reset_timer( int timer );
extern void start_timer( int timer );
extern void restart_timer( int timer );
extern void stop_timer( int timer );
extern word_64 get_timer( int timer );

extern t1_t2_ticks get_t1_t2( void );
extern void set_accesstime( void );

#endif /* !_TIMER_H */
