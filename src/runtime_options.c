#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>

#include <getopt.h>

#include <lua.h>
#include <lauxlib.h>

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

char* configDir = ".x48ng";
char* config_file = "config.lua";
char* romFileName = "rom";
char* ramFileName = "ram";
char* stateFileName = "hp48";
char* port1FileName = "port1";
char* port2FileName = "port2";

#ifdef HAS_X11
int frontend_type = FRONTEND_X11;
#elif HAS_SDL
int frontend_type = FRONTEND_SDL;
#else
int frontend_type = FRONTEND_TEXT;
#endif

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

char normalized_config_path[ MAX_LENGTH_FILENAME ];
char normalized_config_file[ MAX_LENGTH_FILENAME ];
char normalized_rom_path[ MAX_LENGTH_FILENAME ];
char normalized_ram_path[ MAX_LENGTH_FILENAME ];
char normalized_state_path[ MAX_LENGTH_FILENAME ];
char normalized_port1_path[ MAX_LENGTH_FILENAME ];
char normalized_port2_path[ MAX_LENGTH_FILENAME ];

/* https://boston.conman.org/2023/09/29.1 */
lua_State* gL;

bool config_read( const char* conf )
{
    int rc;

    assert( conf != NULL );

    /*---------------------------------------------------
    ; Create the Lua state, which includes NO predefined
    ; functions or values.  This is literally an empty
    ; slate.
    ;----------------------------------------------------*/

    gL = luaL_newstate();
    if ( gL == NULL ) {
        fprintf( stderr, "cannot create Lua state" );
        return false;
    }

    /*-----------------------------------------------------
    ; For the truly paranoid about sandboxing, enable the
    ; following code, which removes the string library,
    ; which some people find problematic to leave un-sand-
    ; boxed. But in my opinion, if you are worried about
    ; such attacks in a configuration file, you have bigger
    ; security issues to worry about than this.
    ;------------------------------------------------------*/

#ifdef PARANOID
    lua_pushliteral( gL, "x" );
    lua_pushnil( gL );
    lua_setmetatable( gL, -2 );
    lua_pop( gL, 1 );
#endif

    /*-----------------------------------------------------
    ; Lua 5.2+ can restrict scripts to being text only,
    ; to avoid a potential problem with loading pre-compiled
    ; Lua scripts that may have malformed Lua VM code that
    ; could possibly lead to an exploit, but again, if you
    ; have to worry about that, you have bigger security
    ; issues to worry about.  But in any case, here I'm
    ; restricting the file to "text" only.
    ;------------------------------------------------------*/

    rc = luaL_loadfilex( gL, conf, "t" );
    if ( rc != LUA_OK ) {
        fprintf( stderr, "Lua error: (%d) %s", rc, lua_tostring( gL, -1 ) );
        return false;
    }

    rc = lua_pcall( gL, 0, 0, 0 );
    if ( rc != LUA_OK ) {
        fprintf( stderr, "Lua error: (%d) %s", rc, lua_tostring( gL, -1 ) );
        return false;
    }

    /*--------------------------------------------
    ; the Lua state gL contains our configuration,
    ; we can now query it for values
    ;---------------------------------------------*/
    lua_getglobal( gL, "rom" );
    romFileName = lua_tostring( gL, -1 );
    fprintf( stderr, "config.rom = %s\n", romFileName );

    lua_getglobal( gL, "ram" );
    ramFileName = lua_tostring( gL, -1 );
    fprintf( stderr, "config.ram = %s\n", ramFileName );

    lua_getglobal( gL, "state" );
    stateFileName = lua_tostring( gL, -1 );
    fprintf( stderr, "config.state = %s\n", stateFileName );

    lua_getglobal( gL, "port1" );
    port1FileName = lua_tostring( gL, -1 );
    fprintf( stderr, "config.port1 = %s\n", port1FileName );

    lua_getglobal( gL, "port2" );
    port2FileName = lua_tostring( gL, -1 );
    fprintf( stderr, "config.port2 = %s\n", port2FileName );

    lua_getglobal( gL, "serial_line" );
    serialLine = lua_tostring( gL, -1 );
    fprintf( stderr, "config.serial_line = %s\n", serialLine );

    lua_getglobal( gL, "debugger" );
    useDebugger = lua_toboolean( gL, -1 );
    fprintf( stderr, "config.debugger = %i\n", useDebugger );

    lua_getglobal( gL, "throttle" );
    throttle = lua_toboolean( gL, -1 );
    fprintf( stderr, "config.throttle = %i\n", throttle );

    lua_getglobal( gL, "frontend" );
    const char* config_lua__frontend = lua_tostring( gL, -1 );
    fprintf( stderr, "config.frontend = %s\n", config_lua__frontend );
#ifdef HAS_X11
    if ( strcmp( config_lua__frontend, "x11" ) == 0 )
        frontend_type = FRONTEND_X11;
    else
#endif
#ifdef HAS_SDL
        if ( strcmp( config_lua__frontend, "sdl" ) == 0 )
        frontend_type = FRONTEND_SDL;
    else
#endif
        if ( strcmp( config_lua__frontend, "text" ) == 0 )
        frontend_type = FRONTEND_TEXT;

    return true;
}

void get_absolute_config_dir( char* source, char* dest )
{
    char* home;
    struct passwd* pwd;

    if ( source[ 0 ] != '/' ) {
        home = getenv( "HOME" );
        if ( home ) {
            strcpy( dest, home );
            strcat( dest, "/" );
        } else {
            pwd = getpwuid( getuid() );
            if ( pwd ) {
                strcpy( dest, pwd->pw_dir );
                strcat( dest, "/" );
            } else {
                if ( verbose )
                    fprintf( stderr, "can\'t figure out your home directory, "
                                     "trying /tmp\n" );
                strcpy( dest, "/tmp" );
            }
        }
    }
    strcat( dest, source );
    if ( dest[ strlen( dest ) ] != '/' )
        strcat( dest, "/" );
}

static inline void normalize_filename( const char* orig, char* dest )
{
    if ( orig[ 0 ] == '/' )
        strcpy( dest, "" );
    else
        strcpy( dest, normalized_config_path );
    strcat( dest, orig );
}

int normalized_config_path_exist = 1;
static inline void normalize_config_dir( void )
{
    struct stat st;

    get_absolute_config_dir( configDir, normalized_config_path );
    if ( verbose )
        fprintf( stderr, "normalized_config_path: %s\n", normalized_config_path );

    if ( stat( normalized_config_path, &st ) == -1 )
        if ( errno == ENOENT )
            normalized_config_path_exist = 0;
}

static inline void normalize_filenames( void )
{
    normalize_filename( config_file, normalized_config_file );

    normalize_filename( ramFileName, normalized_ram_path );
    normalize_filename( stateFileName, normalized_state_path );
    normalize_filename( port1FileName, normalized_port1_path );
    normalize_filename( port2FileName, normalized_port2_path );

    if ( romFileName[ 0 ] == '/' )
        strcpy( normalized_rom_path, "" );
    else {
        if ( !normalized_config_path_exist ) {
            fprintf( stderr, "ERROR: Cannot find rom `%s`\n", romFileName );
            fprintf( stderr, "  You need a ROM to use %s\n", progname );

            exit( 1 );
        }
        strcpy( normalized_rom_path, normalized_config_path );
    }
    strcat( normalized_rom_path, romFileName );
}

int parse_args( int argc, char* argv[] )
{
    int option_index;
    int c = '?';

    char* optstring = "c:hvVtsirT";
    static struct option long_options[] = {
        {"config",               required_argument, NULL,                'c'          },
        { "config-dir",          required_argument, NULL,                1000         },
        { "rom",                 required_argument, NULL,                1010         },
        { "ram",                 required_argument, NULL,                1011         },
        { "state",               required_argument, NULL,                1012         },
        { "port1",               required_argument, NULL,                1013         },
        { "port2",               required_argument, NULL,                1014         },

        { "serial-line",         required_argument, NULL,                1015         },

        { "help",                no_argument,       NULL,                'h'          },
        { "version",             no_argument,       NULL,                'v'          },

        { "verbose",             no_argument,       &verbose,            1            },
        { "use-terminal",        no_argument,       &useTerminal,        1            },
        { "use-serial",          no_argument,       &useSerial,          1            },

        { "initialize",          no_argument,       &initialize,         1            },
        { "reset",               no_argument,       &resetOnStartup,     1            },
        { "throttle",            no_argument,       &throttle,           1            },

        { "no-debug",            no_argument,       &useDebugger,        0            },

#ifdef HAS_SDL
        { "sdl",                 no_argument,       &frontend_type,      FRONTEND_SDL },
        { "sdl-no-chrome",       no_argument,       &show_ui_chrome,     0            },
        { "sdl-fullscreen",      no_argument,       &show_ui_fullscreen, 1            },
#endif

#ifdef HAS_X11
        { "x11",                 no_argument,       &frontend_type,      FRONTEND_X11 },
        { "x11-netbook",         no_argument,       &netbook,            1            },
        { "x11-visual",          required_argument, NULL,                8110         },
        { "x11-small-font",      required_argument, NULL,                8111         },
        { "x11-medium-font",     required_argument, NULL,                8112         },
        { "x11-large-font",      required_argument, NULL,                8113         },
        { "x11-connection-font", required_argument, NULL,                8114         },
#endif

        { "tui",                 no_argument,       &frontend_type,      FRONTEND_TEXT},

        { "mono",                no_argument,       &mono,               1            },
        { "gray",                no_argument,       &gray,               1            },

        { 0,                     0,                 0,                   0            }
    };

    char* help_text = "usage: %s [options]\n"
                      "options:\n"
                      "\t-h --help\t\t\twhat you are reading\n"
                      "\t-v --version\t\t\tshow version\n"
                      "\t   --config-dir=<path>\t\tuse <path> as x48ng's home (default: "
                      "~/.x48ng/)\n"
                      "\t   --rom=<filename>\tuse <filename> (absolute or relative to "
                      "<config-dir>) as ROM (default: rom)\n"
                      "\t   --ram=<filename>\tuse <filename> (absolute or relative to "
                      "<config-dir>) as RAM (default: ram)\n"
                      "\t   --state=<filename>\tuse <filename> (absolute or relative "
                      "to <config-dir>) as STATE (default: hp48)\n"
                      "\t   --port1=<filename>\tuse <filename> (absolute or relative "
                      "to <config-dir>) as PORT1 (default: port1)\n"
                      "\t   --port2=<filename>\tuse <filename> (absolute or relative "
                      "to <config-dir>) as PORT2 (default: port2)\n"
                      "\t   --serial-line=<path>\t\tuse <path> as serial device default: "
                      "%s)\n"
                      "\t-V --verbose\t\t\tbe verbose (default: false)\n"
#ifdef HAS_X11
                      "\t   --x11\t\tuse X11 front-end (default: true)\n"
#endif
#ifdef HAS_SDL
                      "\t   --sdl\t\tuse SDL front-end (default: false)\n"
#endif
                      "\t   --tui\t\tuse terminal front-end (default: false)\n"
                      "\t-t --use-terminal\t\tactivate pseudo terminal interface (default: "
                      "true)\n"
                      "\t-s --use-serial\t\t\tactivate serial interface (default: false)\n"
                      "\t   --no-debug\t\t\tdisable the debugger\n"
                      "\t-i --initialize\t\t\tinitialize the content of <config-dir>\n"
                      "\t-r --reset\t\t\tperform a reset on startup\n"
                      "\t-T --throttle\t\t\ttry to emulate real speed (default: false)\n"
#ifdef HAS_SDL
                      "\t   --sdl-no-chrome\t\tonly display the LCD (default: "
                      "false)\n"
                      "\t   --sdl-fullscreen\t\tmake the UI fullscreen "
                      "(default: "
                      "false)\n"
#endif
#ifdef HAS_X11
                      "\t   --x11-netbook\t\tmake the UI horizontal (default: "
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
                      "connection font (default: %s)\n"
#endif
                      "\t   --mono\t\t\tmake the UI monochrome (default: "
                      "false)\n"
                      "\t   --gray\t\t\tmake the UI grayscale (default: "
                      "false)\n";
    while ( c != EOF ) {
        c = getopt_long( argc, argv, optstring, long_options, &option_index );

        switch ( c ) {
            case 'h':
                fprintf( stdout, help_text, progname, serialLine, smallFont, mediumFont, largeFont, connFont );
                exit( 0 );
                break;
            case 'v':
                fprintf( stdout, "%s %d.%d.%d\n", progname, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );
                exit( 0 );
                break;
            case 'c':
                config_file = optarg;
                break;
            case 1000:
                configDir = optarg;
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
#ifdef HAS_X11
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
#endif
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

    normalize_config_dir();

    /* read config.lua */
    /* TODO: handle having no config_file */
    /* TODO: command-line options should have priority over config file's values */
    /* TODO: handle config_file being absolute or relative */
    normalize_filename( config_file, normalized_config_file );
    if ( !config_read( normalized_config_file ) )
        exit( 1 );

    normalize_filenames();

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
#ifdef HAS_X11
            case FRONTEND_X11:
                fprintf( stderr, "x11\n" );
                break;
#endif
#ifdef HAS_SDL
            case FRONTEND_SDL:
                fprintf( stderr, "sdl\n" );
                break;
#endif
            case FRONTEND_TEXT:
                fprintf( stderr, "text\n" );
                break;
            default:
                fprintf( stderr, "???\n" );
                break;
        }

        fprintf( stderr, "serialLine = %s\n", serialLine );
        fprintf( stderr, "configDir = %s\n", configDir );
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

        fprintf( stderr, "normalized_config_path = %s\n", normalized_config_path );
        fprintf( stderr, "normalized_rom_path = %s\n", normalized_rom_path );
        fprintf( stderr, "normalized_ram_path = %s\n", normalized_ram_path );
        fprintf( stderr, "normalized_state_path = %s\n", normalized_state_path );
        fprintf( stderr, "normalized_port1_path = %s\n", normalized_port1_path );
        fprintf( stderr, "normalized_port2_path = %s\n", normalized_port2_path );
    }

    return ( optind );
}
