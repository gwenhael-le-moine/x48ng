#include <errno.h>
#include <fcntl.h>
#include <langinfo.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "debugger.h"
#include "emulator.h"
#include "config.h"
#include "ui.h" /* setup_frontend(); init_ui(); */

void signal_handler( int sig )
{
    switch ( sig ) {
        case SIGINT: /* Ctrl-C */
            enter_debugger |= USER_INTERRUPT;
            break;
        case SIGALRM:
            sigalarm_triggered = true;
            break;
        case SIGPIPE:
            ui_stop();
            exit_emulator();
            exit( 0 );
        default:
            break;
    }
}

int main( int argc, char** argv )
{
    setlocale( LC_ALL, "C" );

    /*****************************************/
    /* handlers for SIGALRM, SIGPIPE */
    /*****************************************/
    sigset_t set;
    struct sigaction sa;
    sigemptyset( &set );
    sigaddset( &set, SIGALRM );
    sa.sa_handler = signal_handler;
    sa.sa_mask = set;
#ifdef SA_RESTART
    sa.sa_flags = SA_RESTART;
#endif
    sigaction( SIGALRM, &sa, ( struct sigaction* )0 );

    sigemptyset( &set );
    sigaddset( &set, SIGINT );
    sa.sa_handler = signal_handler;
    sa.sa_mask = set;
#ifdef SA_RESTART
    sa.sa_flags = SA_RESTART;
#endif
    sigaction( SIGINT, &sa, ( struct sigaction* )0 );

    sigemptyset( &set );
    sigaddset( &set, SIGPIPE );
    sa.sa_handler = signal_handler;
    sa.sa_mask = set;
#ifdef SA_RESTART
    sa.sa_flags = SA_RESTART;
#endif
    sigaction( SIGPIPE, &sa, ( struct sigaction* )0 );

    /************************************/
    /* set the real time interval timer */
    /************************************/
    /*
      Every <interval>µs setitimer will trigger a SIGALRM
      which will set sigalarm_triggered to true
      In emulate() sigalarm_triggered triggers LCD refresh and UI event handling
     */
    struct itimerval it;
    it.it_interval.tv_sec = 0;
    it.it_interval.tv_usec = USEC_PER_FRAME;
    it.it_value.tv_sec = 0;
    it.it_value.tv_usec = USEC_PER_FRAME;
    setitimer( ITIMER_REAL, &it, ( struct itimerval* )0 );

    /**********************************************************/
    /* Set stdin flags to not include O_NDELAY and O_NONBLOCK */
    /**********************************************************/
    /*
      I don't know what this is for?
     */
    long flags;
    flags = fcntl( STDIN_FILENO, F_GETFL, 0 );
    flags &= ~O_NDELAY;
    flags &= ~O_NONBLOCK;
    fcntl( STDIN_FILENO, F_SETFL, flags );

    /********************/
    /* initialize stuff */
    /********************/
    config_init( argc, argv );

    /* Emulator */
    start_emulator();

    /* (G)UI */
    start_UI( argc, argv );

    /************************/
    /* Start emulation loop */
    /************************/
    struct timeval tv;
    struct timeval tv2;
    struct timezone tz;
    do {
        reset_timer( T1_TIMER );
        reset_timer( RUN_TIMER );
        reset_timer( IDLE_TIMER );

        set_accesstime();

        start_timer( T1_TIMER );
        start_timer( RUN_TIMER );

        sched_timer1 = t1_i_per_tick = saturn.t1_tick;
        sched_timer2 = t2_i_per_tick = saturn.t2_tick;

        set_t1 = saturn.timer1;

        do {
            step_instruction();

            if ( config.useDebugger && ( exec_flags & EXEC_BKPT ) && check_breakpoint( BP_EXEC, saturn.PC ) ) {
                enter_debugger |= BREAKPOINT_HIT;
                break;
            }

            for ( int i = 0; i < KEYS_BUFFER_SIZE; i++ )
                if ( saturn.keybuf[ i ] || config.throttle ) {
                    /* Throttling speed if needed */
                    gettimeofday( &tv, &tz );
                    gettimeofday( &tv2, &tz );

                    while ( ( tv.tv_sec == tv2.tv_sec ) && ( ( tv.tv_usec - tv2.tv_usec ) < 2 ) )
                        gettimeofday( &tv, &tz );

                    break;
                }

            if ( schedule_event < 0 ) // "bug"
                schedule_event = 0;

            if ( schedule_event-- <= 0 )
                schedule();
        } while ( !please_exit && !enter_debugger );

        if ( enter_debugger )
            debug();
    } while ( !please_exit );

    ui_stop();
    exit_emulator();

    return 0;
}
