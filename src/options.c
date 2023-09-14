#include <stdio.h>
#include <stdlib.h>

#include <getopt.h>

#include "options.h"

char* progname = "x48ng";

int verbose = 0;
int useTerminal = 1;
int useSerial = 0;
int throttle = 0;
int initialize = 0;
int resetOnStartup = 0;

char* serialLine = "/dev/ttyS0";
char* homeDirectory = ".x48ng";

char* romFileName = "rom";
char* ramFileName = "ram";
char* stateFileName = "hp48";
char* port1FileName = "port1";
char* port2FileName = "port2";

int parse_args( int argc, char* argv[] ) {
    int option_index;
    int c = '?';

    char* optstring = "c:S:hvVtsirT";
    static struct option long_options[] = {
        { "config-dir", required_argument, NULL, 1000 },
        { "rom-file", required_argument, NULL, 1010 },
        { "ram-file", required_argument, NULL, 1011 },
        { "state-file", required_argument, NULL, 1012 },
        { "port1-file", required_argument, NULL, 1013 },
        { "port2-file", required_argument, NULL, 1014 },

        { "serial-line", required_argument, NULL, 1015 },

        { "help", no_argument, NULL, 'h' },
        { "version", no_argument, NULL, 'v' },

        { "verbose", no_argument, &verbose, 1 },
        { "use-terminal", no_argument, &useTerminal, 1 },
        { "use-serial", no_argument, &useSerial, 1 },

        { "initialize", no_argument, &initialize, 1 },
        { "reset", no_argument, &resetOnStartup, 1 },
        { "throttle", no_argument, &throttle, 1 },

        { 0, 0, 0, 0 } };

    char* help_text =
        "usage: %s [options]\n"
        "options:\n"
        "\t-h --help\t\t\t what you are reading\n"
        "\t-v --version\t\t\t show version\n"
        "\t   --config-dir=<path>\t\t use <path> as x48ng's home (default: "
        "~/.x48ng/)\n"
        "\t   --rom-file=<filename>\t use <filename> (absolute or relative to "
        "<config-dir>) as ROM (default: rom)\n"
        "\t   --ram-file=<filename>\t use <filename> (absolute or relative to "
        "<config-dir>) as RAM (default: ram)\n"
        "\t   --state-file=<filename>\t use <filename> (absolute or relative "
        "to <config-dir>) as STATE (default: hp48)\n"
        "\t   --port1-file=<filename>\t use <filename> (absolute or relative "
        "to <config-dir>) as PORT1 (default: port1)\n"
        "\t   --port2-file=<filename>\t use <filename> (absolute or relative "
        "to <config-dir>) as PORT2 (default: port2)\n"
        "\t   --serial-line=<path>\t\t use <path> as serial device default: "
        "%s)\n"
        "\t-V --verbose\t\t\t be verbose (default: false)\n"
        "\t-t --use-terminal\t\t activate pseudo terminal interface (default: "
        "true)\n"
        "\t-s --use-serial\t\t\t activate serial interface (default: false)\n"
        "\t-i --initialize\t\t\t initialize the content of <config-dir>\n"
        "\t-r --reset\t\t\t perform a reset on startup\n"
        "\t-T --throttle\t\t\t try to emulate real speed (default: false)\n";

    while ( c != EOF ) {
        c = getopt_long( argc, argv, optstring, long_options, &option_index );

        switch ( c ) {
            case 'h':
                fprintf( stdout, help_text, progname, serialLine );
                exit( 0 );
                break;
            case 'v':
                fprintf( stdout, "%s %d.%d.%d\n", progname, VERSION_MAJOR,
                         VERSION_MINOR, PATCHLEVEL );
                exit( 0 );
                break;
            case 1000:
                homeDirectory = optarg;
                break;
            case 1010:
                romFileName = optarg;
                break;
            case 1011:
                ramFileName = optarg;
                break;
            case 1012:
                stateFileName = optarg;
                break;
            case 1013:
                port1FileName = optarg;
                break;
            case 1014:
                port2FileName = optarg;
                break;
            case 1015:
                serialLine = optarg;
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
            case 'r':
                resetOnStartup = 1;
                break;
            case 'T':
                throttle = 1;
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

    if ( verbose ) {
        fprintf( stderr, "verbose = %i\n", verbose );
        fprintf( stderr, "useTerminal = %i\n", useTerminal );
        fprintf( stderr, "useSerial = %i\n", useSerial );
        fprintf( stderr, "throttle = %i\n", throttle );
        fprintf( stderr, "initialize = %i\n", initialize );
        fprintf( stderr, "resetOnStartup = %i\n", resetOnStartup );

        fprintf( stderr, "serialLine = %s\n", serialLine );
        fprintf( stderr, "romFileName = %s\n", romFileName );
        fprintf( stderr, "homeDirectory = %s\n", homeDirectory );
    }

    return ( optind );
}
