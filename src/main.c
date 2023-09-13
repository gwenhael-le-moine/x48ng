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
        "\t-i --initialize :\n\t\t initialize the config and content of "
        "x48ng's home\n"
        "\t-R --reset\n"
        "\t-T --throttle :\n\t\t try to emulate real speed\n";

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
                fprintf( stdout, "\nx48ng %d.%d.%d", VERSION_MAJOR,
                         VERSION_MINOR, PATCHLEVEL );

                fprintf( stdout, "\n\
                               COPYRIGHT\n\
\n\
x48ng is an Emulator for the HP-48 Handheld Calculator.\n\
\n\
This program is free software; you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation; either version 2 of the License, or\n\
(at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program; if not, write to the Free Software\n\
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n\n" );

                fprintf( stdout, "\n\
                              NO WARRANTY\n\
\n\
      BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY\n\
FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN\n\
OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES\n\
PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED\n\
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\n\
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS\n\
TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE\n\
PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,\n\
REPAIR OR CORRECTION.\n\
\n\
      IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING\n\
WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR\n\
REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,\n\
INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING\n\
OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED\n\
TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY\n\
YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER\n\
PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE\n\
POSSIBILITY OF SUCH DAMAGES.\n\n" );
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
        fprintf( stderr, "Invalid arguments : " );
        while ( optind < argc )
            fprintf( stderr, "%s\n", argv[ optind++ ] );
        fprintf( stderr, "\n" );
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

    // SDL Initialization
    SDLInit();

    /*
     * initialize emulator stuff
     */
    init_emulator();

    /*
     *  Create the HP-48 window
     */
    SDLCreateHP();

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
