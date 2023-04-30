#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "debugger.h"
#include "hp48.h"
#include "x48_gui.h"
#ifdef GUI_IS_SDL1
#include "resources.h"
#endif

#include <langinfo.h>
#include <locale.h>

char* progname;
char* res_name;
char* res_class;

int saved_argc;
char** saved_argv;

saturn_t saturn;

void signal_handler( int sig ) {
    switch ( sig ) {
        case SIGINT:
            enter_debugger |= USER_INTERRUPT;
            break;
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

#ifdef GUI_IS_X11
void save_options( int argc, char** argv ) {
    int l;

    saved_argc = argc;
    saved_argv = ( char** )malloc( ( argc + 2 ) * sizeof( char* ) );
    if ( saved_argv == ( char** )0 ) {
        fprintf( stderr, "%s: malloc failed in save_options(), exit\n",
                 progname );
        exit( 1 );
    }
    saved_argv[ argc ] = ( char* )0;
    while ( argc-- ) {
        l = strlen( argv[ argc ] ) + 1;
        saved_argv[ argc ] = ( char* )malloc( l );
        if ( saved_argv[ argc ] == ( char* )0 ) {
            fprintf( stderr, "%s: malloc failed in save_options(), exit\n",
                     progname );
            exit( 1 );
        }
        memcpy( saved_argv[ argc ], argv[ argc ], l );
    }
}
#endif
#ifdef GUI_IS_SDL1
// Some error or information messages
const char* errinit_title = "Emulator initialization failed";
const char* errinit_text[] = { "",
                               "In order to work the emulator needs",
                               "the following files:",
                               "  rom:   an HP48 rom dump",
                               "  ram:   ram file",
                               "  hp48:  HP state file",
                               "",
                               "These files must be in ~/.hp48",
                               "",
                               "Install these files and try again.",
                               0 };
#endif

int main( int argc, char** argv ) {
    sigset_t set;
    struct sigaction sa;
    long flags;
    struct itimerval it;
#ifdef GUI_IS_SDL1
    int rv, i;
    /* unsigned t1, t2; */

    printf( "x48-sdl\n" );

    // SDL Initialization
    SDLInit();

    // Global parameter initialization
    get_resources();
#endif

    setlocale( LC_ALL, "C" );

    /*
     *  Get the name we are called.
     */
    progname = strrchr( argv[ 0 ], '/' );
    if ( progname == NULL )
        progname = argv[ 0 ];
    else
        progname++;

#ifdef GUI_IS_X11
    /*
     * save command line options
     */
    save_options( argc, argv );

    /*
     *  Open up the display
     */
    if ( InitDisplay( argc, argv ) < 0 ) {
        exit( 1 );
    }

    /*
     * initialize emulator stuff
     */
    init_emulator();

    /*
     *  Create the HP-48 window
     */
    if ( CreateWindows( saved_argc, saved_argv ) < 0 ) {
        fprintf( stderr, "%s: can\'t create window\n", progname );
        exit( 1 );
    }
#endif
#ifdef GUI_IS_SDL1
    // initialize emulator stuff
    rv = init_emulator();
    if ( rv != 0 ) {
        printf( "%s\n", errinit_title );
        for ( i = 0; errinit_text[ i ]; i++ )
            printf( "%s\n", errinit_text[ i ] );
        SDLMessageBox( 300, 200, errinit_title, errinit_text, 0xf0e0c0c0,
                       0xff000000, 0 );

        return 0;
    }

    // Create the HP-48 window
    SDLCreateHP();

    // Some more initialization
    printf( "init active stuff\n" );
#endif

    /*
     * can't be done before windows exist
     */
    init_active_stuff();

    /*
     *  install a handler for SIGALRM
     */
    printf( "SIGALRM\n" );
    sigemptyset( &set );
    sigaddset( &set, SIGALRM );
    sa.sa_handler = signal_handler;
    sa.sa_mask = set;
#ifdef SA_RESTART
    sa.sa_flags = SA_RESTART;
#endif
    sigaction( SIGALRM, &sa, ( struct sigaction* )0 );

    /*
     *  install a handler for SIGINT
     */
    printf( "SIGINT\n" );
    sigemptyset( &set );
    sigaddset( &set, SIGINT );
    sa.sa_handler = signal_handler;
    sa.sa_mask = set;
#ifdef SA_RESTART
    sa.sa_flags = SA_RESTART;
#endif
    sigaction( SIGINT, &sa, ( struct sigaction* )0 );

    /*
     *  install a handler for SIGPIPE
     */
    printf( "SIGPIPE\n" );
    sigemptyset( &set );
    sigaddset( &set, SIGPIPE );
    sa.sa_handler = signal_handler;
    sa.sa_mask = set;
#ifdef SA_RESTART
    sa.sa_flags = SA_RESTART;
#endif
    sigaction( SIGPIPE, &sa, ( struct sigaction* )0 );

    /*
     * set the real time interval timer
     */
    printf( "int timer\n" );
    int interval = 20000;
    it.it_interval.tv_sec = 0;
    it.it_interval.tv_usec = interval;
    it.it_value.tv_sec = 0;
    it.it_value.tv_usec = interval;
    setitimer( ITIMER_REAL, &it, ( struct itimerval* )0 );

    /*
     * Set stdin flags to not include O_NDELAY and O_NONBLOCK
     */
    printf( "stdin flags\n" );
    flags = fcntl( STDIN_FILENO, F_GETFL, 0 );
    flags &= ~O_NDELAY;
    flags &= ~O_NONBLOCK;
    fcntl( STDIN_FILENO, F_SETFL, flags );

    printf( "start emulate\n" );
    do {
        if ( !exec_flags )
            emulate();
        else
            emulate_debug();

        debug();
    } while ( 1 );

    return 0;
}
