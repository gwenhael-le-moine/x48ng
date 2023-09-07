#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <langinfo.h>
#include <locale.h>

#include "options.h"
#include "hp48.h"
#include "x48.h"
#include "x48_resources.h"

#if defined( GUI_IS_X11 )
char* progname;
#endif

int saved_argc;
char** saved_argv;

void signal_handler( int sig ) {
    switch ( sig ) {
        case SIGALRM:
            got_alarm = 1;
            break;
        case SIGPIPE:
            exit_x48( 0 );
            exit( 0 );
        default:
            break;
    }
}

int main( int argc, char** argv ) {
    setlocale( LC_ALL, "C" );

#if defined( GUI_IS_X11 )
    /*
     *  Get the name we are called.
     */
    progname = strrchr( argv[ 0 ], '/' );
    if ( progname == NULL )
        progname = argv[ 0 ];
    else
        progname++;

    /*
     * save command line options
     */
    save_options( argc, argv );

    /*
     *  Open up the display
     */
    if ( InitDisplay( argc, argv ) < 0 )
        exit( 1 );

#elif defined( GUI_IS_SDL1 )
    // SDL Initialization
    SDLInit();

    // Global parameter initialization
    get_resources();
#endif

    /*
     * initialize emulator stuff
     */
    init_emulator();

#if defined( GUI_IS_X11 )
    /*
     *  Create the HP-48 window
     */
    if ( CreateWindows( saved_argc, saved_argv ) < 0 ) {
        fprintf( stderr, "can\'t create window\n" );
        exit( 1 );
    }

    init_annunc();
#elif defined( GUI_IS_SDL1 )
    // Create the HP-48 window
    SDLCreateHP();
#endif

    serial_init();

    init_display();

    /*****************************************/
    /* handlers for SIGALRM, SIGINT, SIGPIPE */
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
    struct itimerval it;
    int interval = 20000;
    it.it_interval.tv_sec = 0;
    it.it_interval.tv_usec = interval;
    it.it_value.tv_sec = 0;
    it.it_value.tv_usec = interval;
    setitimer( ITIMER_REAL, &it, ( struct itimerval* )0 );

    /**********************************************************/
    /* Set stdin flags to not include O_NDELAY and O_NONBLOCK */
    /**********************************************************/
    long flags;
    flags = fcntl( STDIN_FILENO, F_GETFL, 0 );
    flags &= ~O_NDELAY;
    flags &= ~O_NONBLOCK;
    fcntl( STDIN_FILENO, F_SETFL, flags );

    /************************/
    /* Start emulation loop */
    /************************/
    do {
        emulate();
    } while ( 1 );

    return 0;
}
