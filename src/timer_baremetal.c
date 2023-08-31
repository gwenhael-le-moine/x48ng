#include "timer.h"

void stop_timer( int timer ) {}
void start_timer( int timer ) {}
void restart_timer( int timer ) {}
void reset_timer( int timer ) {}

t1_t2_ticks get_t1_t2( void ) { t1_t2_ticks ticks; return ticks; }

void pause() {};
word_64 get_timer( int timer ) { return 0; }

void set_accesstime( void ) { }

int gettimeofday (struct timeval * __tv, void* __tz) { return 0; }
