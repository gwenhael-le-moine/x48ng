#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/stat.h>

#include <getopt.h>

#include "runtime_options.h"

char* progname = "x48ng";

int verbose = 0;
int useTerminal = 1;
int useSerial = 0;
int useDebugger = 1;
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

int frontend_type = FRONTEND_X11;

/* sdl */
int show_ui_chrome = 1;
int show_ui_fullscreen = 0;

/* x11 */
int netbook = 0;
char* name = "x48ng";
char* title = "x48ng";
char* geometry;
/* char* iconGeom; */
/* char* iconName; */
char* x11_visual = "default";
/* default | staticgray | staticcolor | truecolor | grayscale |
 * pseudocolor | directcolor | 0xnn | nn
 */
int mono = 0;
int gray = 0;
int monoIcon = 0;
int iconic = 0;
int xrm = 1;
char* smallFont = "-*-fixed-bold-r-normal-*-14-*-*-*-*-*-iso8859-1";
char* mediumFont = "-*-fixed-bold-r-normal-*-15-*-*-*-*-*-iso8859-1";
char* largeFont = "-*-fixed-medium-r-normal-*-20-*-*-*-*-*-iso8859-1";
char* connFont = "-*-fixed-medium-r-normal-*-12-*-*-*-*-*-iso8859-1";

void get_home_directory( char* path ) {
    char* p;
    struct passwd* pwd;

    if ( homeDirectory[ 0 ] == '/' )
        strcpy( path, homeDirectory );
    else {
        p = getenv( "HOME" );
        if ( p ) {
            strcpy( path, p );
            strcat( path, "/" );
        } else {
            pwd = getpwuid( getuid() );
            if ( pwd ) {
                strcpy( path, pwd->pw_dir );
                strcat( path, "/" );
            } else {
                if ( verbose )
                    fprintf( stderr, "can\'t figure out your home directory, "
                                     "trying /tmp\n" );
                strcpy( path, "/tmp" );
            }
        }
        strcat( path, homeDirectory );
    }
}

int parse_args( int argc, char* argv[] ) {
    int option_index;
    int c = '?';

    char* optstring = "c:S:u:hvVtsirT";
    static struct option long_options[] = {
        { "config-dir", required_argument, NULL, 1000 },
        { "rom-file", required_argument, NULL, 1010 },
        { "ram-file", required_argument, NULL, 1011 },
        { "state-file", required_argument, NULL, 1012 },
        { "port1-file", required_argument, NULL, 1013 },
        { "port2-file", required_argument, NULL, 1014 },

        { "serial-line", required_argument, NULL, 1015 },

        { "front-end", required_argument, NULL, 'u' },

        { "help", no_argument, NULL, 'h' },
        { "version", no_argument, NULL, 'v' },

        { "verbose", no_argument, &verbose, 1 },
        { "use-terminal", no_argument, &useTerminal, 1 },
        { "use-serial", no_argument, &useSerial, 1 },

        { "initialize", no_argument, &initialize, 1 },
        { "reset", no_argument, &resetOnStartup, 1 },
        { "throttle", no_argument, &throttle, 1 },

        { "no-debug", no_argument, &useDebugger, 0 },

        { "sdl", no_argument, &frontend_type, FRONTEND_SDL },
        { "sdl-no-chrome", no_argument, &show_ui_chrome, 0 },
        { "sdl-fullscreen", no_argument, &show_ui_fullscreen, 1 },

        { "x11", no_argument, &frontend_type, FRONTEND_X11 },
        { "x11-netbook", no_argument, &netbook, 1 },
        { "x11-mono", no_argument, &mono, 1 },
        { "x11-gray", no_argument, &gray, 1 },
        { "x11-visual", required_argument, NULL, 8110 },
        { "x11-small-font", required_argument, NULL, 8111 },
        { "x11-medium-font", required_argument, NULL, 8112 },
        { "x11-large-font", required_argument, NULL, 8113 },
        { "x11-connection-font", required_argument, NULL, 8114 },

        { "tui", no_argument, &frontend_type, FRONTEND_TEXT },

        { 0, 0, 0, 0 } };

    char* help_text =
        "usage: %s [options]\n"
        "options:\n"
        "\t-h --help\t\t\twhat you are reading\n"
        "\t-v --version\t\t\tshow version\n"
        "\t   --config-dir=<path>\t\tuse <path> as x48ng's home (default: "
        "~/.x48ng/)\n"
        "\t   --rom-file=<filename>\tuse <filename> (absolute or relative to "
        "<config-dir>) as ROM (default: rom)\n"
        "\t   --ram-file=<filename>\tuse <filename> (absolute or relative to "
        "<config-dir>) as RAM (default: ram)\n"
        "\t   --state-file=<filename>\tuse <filename> (absolute or relative "
        "to <config-dir>) as STATE (default: hp48)\n"
        "\t   --port1-file=<filename>\tuse <filename> (absolute or relative "
        "to <config-dir>) as PORT1 (default: port1)\n"
        "\t   --port2-file=<filename>\tuse <filename> (absolute or relative "
        "to <config-dir>) as PORT2 (default: port2)\n"
        "\t   --serial-line=<path>\t\tuse <path> as serial device default: "
        "%s)\n"
        "\t-V --verbose\t\t\tbe verbose (default: false)\n"
        "\t-u --front-end\t\t\tspecify a front-end (available: x11, sdl, "
        "text; "
        "default: x11)\n"
        "\t   --x11\t\tuse X11 front-end (default: true)\n"
        "\t   --sdl\t\tuse SDL front-end (default: false)\n"
        "\t   --tui\t\tuse terminal front-end (default: false)\n"
        "\t-t --use-terminal\t\tactivate pseudo terminal interface (default: "
        "true)\n"
        "\t-s --use-serial\t\t\tactivate serial interface (default: false)\n"
        "\t   --no-debug\t\t\tdisable the debugger\n"
        "\t-i --initialize\t\t\tinitialize the content of <config-dir>\n"
        "\t-r --reset\t\t\tperform a reset on startup\n"
        "\t-T --throttle\t\t\ttry to emulate real speed (default: false)\n"
        "\t   --sdl-no-chrome\t\tonly display the LCD (default: "
        "false)\n"
        "\t   --sdl-fullscreen\t\tmake the UI fullscreen "
        "(default: "
        "false)\n"
        "\t   --x11-netbook\t\tmake the UI horizontal (default: "
        "false)\n"
        "\t   --x11-mono\t\t\tmake the UI monochrome (default: "
        "false)\n"
        "\t   --x11-gray\t\t\tmake the UI grayscale (default: "
        "false)\n"
        "\t   --x11-visual=<X visual>\tuse visual <X visual> (default: "
        "default), possible values: "
        "<default | staticgray | staticcolor | truecolor | grayscale | "
        "pseudocolor | directcolor | 0xnn | nn>\n"
        "\t   --x11-small-font=<X font name>\tuse <X font name> as small "
        "font (default: %s)\n"
        "\t   --x11-medium-font=<X font name>\tuse <X font name> as medium "
        "font (default: %s)\n"
        "\t   --x11-large-font=<X font name>\tuse <X font name> as large "
        "font (default: %s)\n"
        "\t   --x11-connection-font=<X font name>\tuse <X font name> as "
        "connection font (default: %s)\n";
    while ( c != EOF ) {
        c = getopt_long( argc, argv, optstring, long_options, &option_index );

        switch ( c ) {
            case 'h':
                fprintf( stdout, help_text, progname, serialLine, smallFont,
                         mediumFont, largeFont, connFont );
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
            case 8110:
                x11_visual = optarg;
                break;
            case 8111:
                smallFont = optarg;
                break;
            case 8112:
                mediumFont = optarg;
                break;
            case 8113:
                largeFont = optarg;
                break;
            case 8114:
                connFont = optarg;
                break;
            case 'u':
                if ( strcmp( optarg, "sdl" ) == 0 )
                    frontend_type = FRONTEND_SDL;
                else if ( strcmp( optarg, "text" ) == 0 )
                    frontend_type = FRONTEND_TEXT;
                else if ( strcmp( optarg, "x11" ) == 0 )
                    frontend_type = FRONTEND_X11;
                else {
                    fprintf( stderr, "Error: unknown frontend '%s'\n", optarg );
                    exit( 1 );
                }
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
        fprintf( stderr, "useDebugger = %i\n", useDebugger );
        fprintf( stderr, "throttle = %i\n", throttle );
        fprintf( stderr, "initialize = %i\n", initialize );
        fprintf( stderr, "resetOnStartup = %i\n", resetOnStartup );
        fprintf( stderr, "frontend_type = " );
        switch ( frontend_type ) {
            case FRONTEND_X11:
                fprintf( stderr, "x11\n" );
                break;
            case FRONTEND_SDL:
                fprintf( stderr, "sdl\n" );
                break;
            case FRONTEND_TEXT:
                fprintf( stderr, "text\n" );
                break;
            default:
                fprintf( stderr, "???\n" );
                break;
        }

        fprintf( stderr, "serialLine = %s\n", serialLine );
        fprintf( stderr, "homeDirectory = %s\n", homeDirectory );
        fprintf( stderr, "romFileName = %s\n", romFileName );
        fprintf( stderr, "ramFileName = %s\n", ramFileName );
        fprintf( stderr, "stateFileName = %s\n", stateFileName );
        fprintf( stderr, "port1FileName = %s\n", port1FileName );
        fprintf( stderr, "port2FileName = %s\n", port2FileName );

        fprintf( stderr, "netbook = %i\n", netbook );
        fprintf( stderr, "mono = %i\n", mono );
        fprintf( stderr, "gray = %i\n", gray );
        fprintf( stderr, "monoIcon = %i\n", monoIcon );
        fprintf( stderr, "iconic = %i\n", iconic );
        fprintf( stderr, "xrm = %i\n", xrm );

        fprintf( stderr, "geometry = %s\n", geometry );
        fprintf( stderr, "x11_visual = %s\n", x11_visual );
        fprintf( stderr, "smallFont = %s\n", smallFont );
        fprintf( stderr, "mediumFont = %s\n", mediumFont );
        fprintf( stderr, "largeFont = %s\n", largeFont );
        fprintf( stderr, "connFont = %s\n", connFont );
    }

    /* check that homeDirectory exists, otherwise initialize */
    char config_dir[ 1024 ];
    char rom_filename[ 1024 ];
    char config_filename[ 1024 ];
    struct stat sb;

    get_home_directory( config_dir );
    if ( romFileName[ 0 ] == '/' )
        strcpy( rom_filename, "" );
    else
        strcpy( rom_filename, config_dir );
    strcat( rom_filename, romFileName );

    if ( stat( config_dir, &sb ) == 0 && S_ISDIR( sb.st_mode ) &&
         stat( rom_filename, &sb ) == 0 ) {
        if ( verbose )
            fprintf( stderr, "%s exists\n", config_dir );

        /* a config_dir exists with a romFileName in it. */
        /* we can initialize if necessary */
        if ( !initialize ) {
            /* Not forced to initialize but does stateFileName exist? */
            if ( stateFileName[ 0 ] == '/' )
                strcpy( config_filename, "" );
            else
                strcpy( config_filename, config_dir );
            strcat( config_filename, stateFileName );
            /* if not then initialize */
            initialize = stat( config_filename, &sb ) == 0 ? 0 : 1;

            /* Not forced to initialize but does ramFileName exist? */
            if ( ramFileName[ 0 ] == '/' )
                strcpy( config_filename, "" );
            else
                strcpy( config_filename, config_dir );
            strcat( config_filename, ramFileName );
            /* if not then initialize */
            initialize = stat( config_filename, &sb ) == 0 ? 0 : 1;
        }
    } else {
        if ( mkdir( config_dir, 0755 ) == 0 )
            fprintf( stderr, "Created %s, please copy a rom in it.\n",
                     config_dir );

        exit( 1 );
    }

    return ( optind );
}
