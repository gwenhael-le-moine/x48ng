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

#include <getopt.h>

#include "options.h"
#include "hp48.h"
#include "x48.h"
#include "x48_resources.h"

char* progname;

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


int parse_args( int argc, char* argv[] ) {
    int option_index;
    char c = '?';

    char* optstring = "c:r:S:hvVtsiRTnxgm";
    static struct option long_options[] = {
        { "config-dir", required_argument, NULL, 'c' },
        { "rom", required_argument, NULL, 'r' },
        { "serial-line", required_argument, NULL, 'S' },

        { "help", no_argument, NULL, 'h' },
        { "version", no_argument, NULL, 'v' },
        { "verbose", no_argument, NULL, 'V' },

        { "use-terminal", no_argument, NULL, 't' },
        { "use-serial", no_argument, NULL, 's' },

        { "initialize", no_argument, NULL, 'i' },
        { "reset", no_argument, NULL, 'R' },
        { "throttle", no_argument, NULL, 'T' },
        { "netbook", no_argument, NULL, 'n' },
        { "use-xshm", no_argument, NULL, 'x' },
        { "gray", no_argument, NULL, 'g' },
        { "mono", no_argument, NULL, 'm' },

        { 0, 0, 0, 0 } };

    char* help_text =
        "ngstar [options]\n"
        "\t-h --help :\n\t\t what you are reading\n"
        "\t-v --version :\n\t\t show version\n"
        "\t-c<path> --config-dir=<path> :\n\t\t use <path> as x48ng's home\n"
        "\t-r<filename> --rom=<filename> :\n\t\t use <filename> as ROM\n"
        "\t-S<path> --serial-line=<path> :\n\t\t use <path> as serial device\n"
        "\t-V --verbose :\n\t\t be verbose\n"
        "\t-t --use-terminal\n"
        "\t-s --use-serial\n"
        "\t-i --initialize :\n\t\t initialize the config and content of x48ng's home\n"
        "\t-R --reset\n"
        "\t-T --throttle :\n\t\t try to emulate real speed\n"
        "\t-n --netbook :\n\t\t horizontal GUI\n"
        "\t-g --gray :\n\t\t grayscale GUI\n"
        "\t-m --mono :\n\t\t monochrome GUI\n"
        "\t-x --use-xshm\n"
        ;

    while ( c != EOF ) {
        c = getopt_long( argc, argv, optstring, long_options, &option_index );

        switch ( c ) {
            case 'c':
                homeDirectory = optarg;
                break;
            case 'r':
                romFileName = optarg;
                break;
            case 'S':
                serialLine = optarg;
                break;
            case 'h':
                fprintf( stdout, "%s", help_text );
                exit( 0 );
                break;
            case 'v':
                show_version();
                show_copyright();
                show_warranty();
                break;
            case 'V':
                verbose = 1;
                break;
            case 't':
                useTerminal = 1;
                break;
            case 's':
                useSerial = 1;
                break;
            case 'i':
                initialize = 1;
                break;
            case 'R':
                resetOnStartup = 1;
                break;
            case 'T':
                throttle = 1;
                break;
            case 'n':
                netbook = 1;
                break;
            case 'g':
                gray = 1;
                break;
            case 'm':
                mono = 1;
                break;
            case 'x':
                useXShm = 1;
                break;

            case '?':
            case ':':
                exit( 0 );
                break;
            default:
                break;
        }
    }

    if ( optind < argc ) {
        fprintf(stderr, "Invalid arguments : ");
        while ( optind < argc )
            fprintf(stderr, "%s\n", argv[ optind++ ]);
        fprintf(stderr, "\n");
    }

    return ( optind );
}

int main( int argc, char** argv ) {
    setlocale( LC_ALL, "C" );

    /*
     *  Get the name we are called.
     */
    progname = strrchr( argv[ 0 ], '/' );
    if ( progname == NULL )
        progname = argv[ 0 ];
    else
        progname++;

    /**********/
    /* getopt */
    /**********/
    parse_args( argc, argv );

#if defined( GUI_IS_X11 )
    /*
     * save command line options
     */
    /* save_options( argc, argv ); */

    /*
     *  Open up the display
     */
    if ( InitDisplay( argc, argv ) < 0 )
        {
            fprintf( stderr, "cannot InitDisplay" );
            exit( 1 );
        }

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

    /*
     *  Create the HP-48 window
     */
#if defined( GUI_IS_X11 )
    if ( CreateWindows( argc, argv ) < 0 ) {
        fprintf( stderr, "can\'t create window\n" );
        exit( 1 );
    }

    init_annunc();
#elif defined( GUI_IS_SDL1 )
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
