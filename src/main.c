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
#include "runtime_options.h"
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
    parse_args_and_read_config( argc, argv );

    /* Emulator */
    start_emulator();

    /* (G)UI */
    start_UI( argc, argv );

    /************************/
    /* Start emulation loop */
    /************************/
    do {
        if ( !exec_flags )
            emulate();
        else
            emulate_debug();

        debug();
    } while ( true );

    return 0;
}
