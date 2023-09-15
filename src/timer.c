#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>

#include <sys/time.h>

#include "romio.h"
#include "timer.h"
#include "debugger.h" /* used for in_debugger */

typedef struct x48_timer_t {
    word_1 run;
    word_64 start;
    word_64 stop;
    word_64 value;
} x48_timer_t;

static x48_timer_t timers[ NR_TIMERS ];

static long systime_offset = 0;

/*
 * Ticks for THU 01.01.1970 00:00:00
 */
word_64 unix_0_time = 0x1CF2E8F800000LL;
word_64 ticks_10_min = 0x00b40000L;

/*
 * Will be in saturn_t in the future
 */
word_64 set_0_time = 0x0;

/*
 * Calculated as (unix_0_time + set_0_time)
 */
word_64 time_offset = 0x0;

#define RAM_BASE_SX 0x70000
#define ACCESSTIME_SX ( 0x70052 - RAM_BASE_SX )
#define ACCESSCRC_SX ( 0x7005F - RAM_BASE_SX )
#define TIMEOUT_SX ( 0x70063 - RAM_BASE_SX )
#define TIMEOUTCLK_SX ( 0x70070 - RAM_BASE_SX )

#define RAM_BASE_GX 0x80000
#define ACCESSTIME_GX ( 0x80058 - RAM_BASE_GX )
#define ACCESSCRC_GX ( 0x80065 - RAM_BASE_GX )
#define TIMEOUT_GX ( 0x80069 - RAM_BASE_GX )
#define TIMEOUTCLK_GX ( 0x80076 - RAM_BASE_GX )

#define calc_crc( nib )                                                        \
    ( crc = ( crc >> 4 ) ^ ( ( ( crc ^ ( nib ) ) & 0xf ) * 0x1081 ) )

/*
 * Set ACCESSTIME: (on startup)
 *
 * 1. TICKS = 8192 * gettimeofday()  (UNIX System Time)
 * 2. TICKS += unix_0_time           (TICKS for 1.1.1970, 0:00)
 * 3. TICKS += set_0_time            (Time adjustment from User)
 * 4. TICKS += saturn.timer2         (Timer 2 from last run)
 * 5. Write this into ACCESSTIME
 * 6. Calculate CRC for 13 Nibbles
 * 7. Write this into ACCESSCRC
 * 8. Prevent AutoOff by setting TIMEOUT
 *
 */
void set_accesstime( void ) {
    struct timeval tv;
    struct timezone tz;

    word_64 ticks, timeout, timer2;
    word_20 accesstime_loc, timeout_loc;
    word_20 accesscrc_loc, timeoutclk_loc;
    word_16 crc;
    word_4 val;
    int i;
    time_t gmt;
    struct tm* ltm;

    /*
     * This is done to set the variable 'timezone' on SYSV systems
     */
    ( void )time( &gmt );
    ltm = localtime( &gmt );
    systime_offset = timezone;
    if ( ltm->tm_isdst )
        systime_offset -= 3600;

    gettimeofday( &tv, &tz );
    tv.tv_sec -= systime_offset;

    ticks = tv.tv_sec;
    ticks <<= 13;
    ticks += ( tv.tv_usec << 7 ) / 15625;

    time_offset = unix_0_time + set_0_time;
    ticks += time_offset;

    timer2 = saturn.timer2;
    if ( saturn.timer2 & 0x80000000 ) {
        assert( timer2 < 0 );
    }

    ticks += timer2;

    timeout = ticks;

    crc = 0x0;

    if ( opt_gx ) {
        accesstime_loc = ACCESSTIME_GX;
        accesscrc_loc = ACCESSCRC_GX;
        timeout_loc = TIMEOUT_GX;
        timeoutclk_loc = TIMEOUTCLK_GX;
    } else {
        accesstime_loc = ACCESSTIME_SX;
        accesscrc_loc = ACCESSCRC_SX;
        timeout_loc = TIMEOUT_SX;
        timeoutclk_loc = TIMEOUTCLK_SX;
    }

    for ( i = 0; i < 13; i++ ) {
        val = ticks & 0xf;
        calc_crc( val );
        saturn.ram[ accesstime_loc + i ] = val;
        ticks >>= 4;
    }

    for ( i = 0; i < 4; i++ ) {
        saturn.ram[ accesscrc_loc + i ] = crc & 0xf;
        crc >>= 4;
    }

    timeout += ticks_10_min;

    for ( i = 0; i < 13; i++ ) {
        val = timeout & 0xf;
        calc_crc( val );
        saturn.ram[ timeout_loc + i ] = val;
        timeout >>= 4;
    }

    saturn.ram[ timeoutclk_loc ] = 0xf;
}

void start_timer( int timer ) {
    struct timeval tv;
    struct timezone tz;

    assert( timer <= NR_TIMERS );

    if ( timers[ timer ].run == 1 )
        return;

    gettimeofday( &tv, &tz );

    tv.tv_sec -= systime_offset;

    timers[ timer ].run = 1;
    if ( timer == T1_TIMER ) {
        timers[ timer ].start = ( tv.tv_sec << 9 );
        timers[ timer ].start += ( tv.tv_usec / 15625 ) >> 3;
    } else {
        timers[ timer ].start = tv.tv_sec;
        timers[ timer ].start <<= 13;
        timers[ timer ].start += ( tv.tv_usec << 7 ) / 15625;
    }
}

void restart_timer( int timer ) {
    struct timeval tv;
    struct timezone tz;

    if ( timer > NR_TIMERS )
        return;

    timers[ timer ].start = 0;
    timers[ timer ].stop = 0;
    timers[ timer ].value = 0;

    gettimeofday( &tv, &tz );

    tv.tv_sec -= systime_offset;

    timers[ timer ].run = 1;
    if ( timer == T1_TIMER ) {
        timers[ timer ].start = ( tv.tv_sec << 9 );
        timers[ timer ].start += ( tv.tv_usec / 15625 ) >> 3;
    } else {
        timers[ timer ].start = tv.tv_sec;
        timers[ timer ].start <<= 13;
        timers[ timer ].start += ( tv.tv_usec << 7 ) / 15625;
    }
}

void stop_timer( int timer ) {
    struct timeval tv;
    struct timezone tz;

    if ( timer > NR_TIMERS )
        return;

    if ( timers[ timer ].run == 0 )
        return;

    gettimeofday( &tv, &tz );

    tv.tv_sec -= systime_offset;

    timers[ timer ].run = 0;
    if ( timer == T1_TIMER ) {
        timers[ timer ].stop = ( tv.tv_sec << 9 );
        timers[ timer ].stop += ( tv.tv_usec / 15625 ) >> 3;
    } else {
        timers[ timer ].stop = tv.tv_sec;
        timers[ timer ].stop <<= 13;
        timers[ timer ].stop += ( tv.tv_usec << 7 ) / 15625;
    }

    timers[ timer ].value += timers[ timer ].stop - timers[ timer ].start;
    //  add_sub_64(&timers[timer].stop, &timers[timer].start,
    //  &timers[timer].value);
}

void reset_timer( int timer ) {
    if ( timer > NR_TIMERS )
        return;
    timers[ timer ].run = 0;
    timers[ timer ].start = 0;
    timers[ timer ].stop = 0;
    timers[ timer ].value = 0;
}

static word_64 zero = 0;

word_64 get_timer( int timer ) {
    struct timeval tv;
    struct timezone tz;

    word_64 stop;

    if ( timer > NR_TIMERS )
        return zero;

    if ( timers[ timer ].run ) {

        gettimeofday( &tv, &tz );

        tv.tv_sec -= systime_offset;

        if ( timer == T1_TIMER ) {
            stop = ( tv.tv_sec << 9 );
            stop += ( tv.tv_usec / 15625 ) >> 3;
        } else {
            stop = tv.tv_sec;
            stop <<= 13;
            stop += ( tv.tv_usec << 7 ) / 15625;
        }
        timers[ timer ].value += stop - timers[ timer ].start;
    }

    return timers[ timer ].value;
}

/*
 * Calculate TIMER 2 Ticks:
 *
 * 1. TICKS = 8192 * gettimeofday()  (UNIX System Time)
 * 2. TICKS += unix_0_time           (TICKS for 1.1.1970, 0:00)
 * 3. TICKS += set_0_time            (Time adjustment from User)
 * 4. Get value of ACCESSTIME
 * 5. Return (ACCESSTIME - TICKS)
 *
 */

t1_t2_ticks get_t1_t2( void ) {
    struct timeval tv;
    struct timezone tz;

    word_64 stop;
    t1_t2_ticks ticks;
    word_64 access_time;
    word_64 adj_time;
    word_64 diff_time;
    word_64 delta;
    word_20 accesstime_loc;
    int i;

    gettimeofday( &tv, &tz );

    tv.tv_sec -= systime_offset;

    if ( timers[ T1_TIMER ].run ) {
        stop = ( tv.tv_sec << 9 );
        stop += ( tv.tv_usec / 15625 ) >> 3;
        if ( timers[ T1_TIMER ].start <= stop ) {
            timers[ T1_TIMER ].value += stop - timers[ T1_TIMER ].start;
        } else {
            fprintf( stderr, "clock running backwards\n" );
        }
    }
    ticks.t1_ticks = timers[ T1_TIMER ].value;

    stop = tv.tv_sec;
    stop <<= 13;
    stop += ( tv.tv_usec << 7 ) / 15625;

    stop += time_offset;

    accesstime_loc = opt_gx ? ACCESSTIME_GX : ACCESSTIME_SX;

    access_time = 0x0;

    for ( i = 13 - 1; i >= 0; i-- ) {
        access_time <<= 4;
        access_time |= ( ( int )saturn.ram[ accesstime_loc + i ] & 0xf );
    }

    access_time -= stop;

    if ( adj_time_pending || in_debugger ) {
        /*
         * We have been inside an interrupt for very long, maybe
         * or we are sleeping in the debugger.
         * Don't adjust the time, can't come from user, anyhow.
         */

        if ( ( saturn.timer2 >= 0 && access_time < 0 ) ||
             ( ( unsigned long )saturn.timer2 > access_time ) ) {
            /*
             * check OK, return calculated time
             */
            ticks.t2_ticks = access_time;
        } else {
            /*
             * Don't increment timer2, return old value and
             * slow down timer2.
             */
            ticks.t2_ticks = saturn.timer2;
            saturn.t2_tick++;
        }

        return ticks;
    }

    diff_time = saturn.timer2;

    adj_time = access_time - diff_time;
    delta = labs( adj_time );

    if ( delta > 0x3c000 ) /* Half a minute */
    {
        set_0_time += adj_time;
        time_offset += adj_time;
        access_time -= adj_time;
    }

    if ( ( saturn.timer2 >= 0 && ( access_time < 0 ) ) ||
         ( ( unsigned long )saturn.timer2 > access_time ) ) {
        /*
         * check OK, return calculated time
         */
        ticks.t2_ticks = access_time;
    } else {
        /*
         * Don't increment timer2, return old value and
         * slow down timer2.
         */
        ticks.t2_ticks = saturn.timer2;
        saturn.t2_tick++;
    }

    return ticks;
}
