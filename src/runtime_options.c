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

bool verbose = false;
bool print_config = false;
bool useTerminal = false;
bool useSerial = false;
bool useDebugger = false;
bool throttle = false;
bool resetOnStartup = false;

char* serialLine;

char* configDir = "x48ng";
char* config_file = "config.lua";
char* romFileName = NULL;
char* ramFileName = NULL;
char* stateFileName = NULL;
char* port1FileName = NULL;
char* port2FileName = NULL;

int frontend_type = FRONTEND_TEXT;

bool leave_shift_keys = false;

bool mono = false;
bool gray = false;

/* tui */
bool small = false;
bool tiny = false;

/* sdl */
bool hide_chrome = false;
bool show_ui_fullscreen = false;

/* x11 */
bool netbook = false;
char* name = "x48ng";
char* title = "x48ng";
char* x11_visual = NULL;
/* default | staticgray | staticcolor | truecolor | grayscale |
 * pseudocolor | directcolor | 0xnn | nn
 */
bool monoIcon = false;
bool iconic = false;
bool xrm = true;
char* smallFont = NULL;
char* mediumFont = NULL;
char* largeFont = NULL;
char* connFont = NULL;

char normalized_config_path[ MAX_LENGTH_FILENAME ];
char normalized_config_file[ MAX_LENGTH_FILENAME ];
char normalized_rom_path[ MAX_LENGTH_FILENAME ];
char normalized_ram_path[ MAX_LENGTH_FILENAME ];
char normalized_state_path[ MAX_LENGTH_FILENAME ];
char normalized_port1_path[ MAX_LENGTH_FILENAME ];
char normalized_port2_path[ MAX_LENGTH_FILENAME ];

lua_State* config_lua_values;

#ifndef LUA_OK
#  define LUA_OK 0
#endif

static inline bool config_read( const char* filename )
{
    int rc;

    assert( filename != NULL );

    /*---------------------------------------------------
    ; Create the Lua state, which includes NO predefined
    ; functions or values.  This is literally an empty
    ; slate.
    ;----------------------------------------------------*/
    config_lua_values = luaL_newstate();
    if ( config_lua_values == NULL ) {
        fprintf( stderr, "cannot create Lua state\n" );
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
    lua_pushliteral( config_lua_values, "x" );
    lua_pushnil( config_lua_values );
    lua_setmetatable( config_lua_values, -2 );
    lua_pop( config_lua_values, 1 );
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
    rc = luaL_loadfile( config_lua_values, filename );
    if ( rc != LUA_OK ) {
        fprintf( stderr, "Lua error: (%d) %s\n", rc, lua_tostring( config_lua_values, -1 ) );
        return false;
    }

    rc = lua_pcall( config_lua_values, 0, 0, 0 );
    if ( rc != LUA_OK ) {
        fprintf( stderr, "Lua error: (%d) %s\n", rc, lua_tostring( config_lua_values, -1 ) );
        return false;
    }

    return true;
}

static inline void get_absolute_config_dir( char* source, char* dest )
{
    if ( source[ 0 ] != '/' ) {
        char* xdg_config_home = getenv( "XDG_CONFIG_HOME" );

        if ( xdg_config_home ) {
            if ( verbose )
                fprintf( stderr, "XDG_CONFIG_HOME is %s\n", xdg_config_home );

            strcpy( dest, xdg_config_home );
            strcat( dest, "/" );
        } else {
            char* home = getenv( "HOME" );

            if ( home ) {
                if ( verbose )
                    fprintf( stderr, "HOME is %s\n", home );

                strcpy( dest, home );
                strcat( dest, "/.config/" );
            } else {
                struct passwd* pwd = getpwuid( getuid() );

                if ( pwd ) {
                    if ( verbose )
                        fprintf( stderr, "pwd->pw_dir is %s\n", pwd->pw_dir );

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

static int normalized_config_path_exist = 1;
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

    normalize_filename( romFileName, normalized_rom_path );
    normalize_filename( ramFileName, normalized_ram_path );
    normalize_filename( stateFileName, normalized_state_path );
    normalize_filename( port1FileName, normalized_port1_path );
    normalize_filename( port2FileName, normalized_port2_path );
}

int parse_args( int argc, char* argv[] )
{
    int option_index;
    int c = '?';

    char* clopt_configDir = NULL;
    char* clopt_romFileName = NULL;
    char* clopt_ramFileName = NULL;
    char* clopt_stateFileName = NULL;
    char* clopt_port1FileName = NULL;
    char* clopt_port2FileName = NULL;
    char* clopt_serialLine = NULL;
    char* clopt_x11_visual = NULL;
    char* clopt_smallFont = NULL;
    char* clopt_mediumFont = NULL;
    char* clopt_largeFont = NULL;
    char* clopt_connFont = NULL;
    int clopt_frontend_type = -1;
    int clopt_verbose = -1;
    int clopt_useTerminal = -1;
    int clopt_useSerial = -1;
    int clopt_useDebugger = -1;
    int clopt_throttle = -1;
    int clopt_hide_chrome = -1;
    int clopt_show_ui_fullscreen = -1;
    int clopt_netbook = -1;
    int clopt_mono = -1;
    int clopt_gray = -1;
    int clopt_small = -1;
    int clopt_tiny = -1;
    int clopt_leave_shift_keys = -1;

    char* optstring = "c:hvVtsirT";
    struct option long_options[] = {
        {"config",            required_argument, NULL,                      'c'         },
        { "config-dir",       required_argument, NULL,                      1000        },
        { "rom",              required_argument, NULL,                      1010        },
        { "ram",              required_argument, NULL,                      1011        },
        { "state",            required_argument, NULL,                      1012        },
        { "port1",            required_argument, NULL,                      1013        },
        { "port2",            required_argument, NULL,                      1014        },

        { "serial-line",      required_argument, NULL,                      1015        },

        { "help",             no_argument,       NULL,                      'h'         },
        { "version",          no_argument,       NULL,                      'v'         },

        { "print-config",     no_argument,       ( int* )&print_config,     true        },
        { "verbose",          no_argument,       &clopt_verbose,            true        },
        { "terminal",         no_argument,       &clopt_useTerminal,        true        },
        { "serial",           no_argument,       &clopt_useSerial,          true        },

        { "reset",            no_argument,       ( int* )&resetOnStartup,   true        },
        { "throttle",         no_argument,       &clopt_throttle,           true        },

        { "debug",            no_argument,       &clopt_useDebugger,        true        },

        { "sdl",              no_argument,       &clopt_frontend_type,      FRONTEND_SDL},
        { "no-chrome",        no_argument,       &clopt_hide_chrome,        true        },
        { "fullscreen",       no_argument,       &clopt_show_ui_fullscreen, true        },

        { "x11",              no_argument,       &clopt_frontend_type,      FRONTEND_X11},
        { "netbook",          no_argument,       &clopt_netbook,            true        },
        { "visual",           required_argument, NULL,                      8110        },
        { "small-font",       required_argument, NULL,                      8111        },
        { "medium-font",      required_argument, NULL,                      8112        },
        { "large-font",       required_argument, NULL,                      8113        },
        { "connection-font",  required_argument, NULL,                      8114        },

        { "tui",              no_argument,       NULL,                      9100        },
        { "small",            no_argument,       NULL,                      9109        }, /* DEPRECATED */
        { "tui-small",        no_argument,       NULL,                      9110        },
        { "tiny",             no_argument,       NULL,                      9119        }, /* DEPRECATED */
        { "tui-tiny",         no_argument,       NULL,                      9120        },

        { "mono",             no_argument,       &clopt_mono,               true        },
        { "gray",             no_argument,       &clopt_gray,               true        },
        { "leave-shift-keys", no_argument,       &clopt_leave_shift_keys,   true        },

        { 0,                  0,                 0,                         0           }
    };

    char* help_text = "usage: %s [options]\n"
                      "options:\n"
                      "  -h --help               what you are reading\n"
                      "  -v --version            show version\n"
                      "     --print-config       print configuration as config file\n"
                      "  -c --config=<path>      use <path> as x48ng's config file (default: "
                      "$XDG_CONFIG_HOME/x48ng/config.lua)\n"
                      "     --config-dir=<path>  use <path> as x48ng's home (default: "
                      "$XDG_CONFIG_HOME/x48ng/)\n"
                      "     --rom=<filename>     use <filename> (absolute or relative to "
                      "<config-dir>) as ROM (default: rom)\n"
                      "     --ram=<filename>     use <filename> (absolute or relative to "
                      "<config-dir>) as RAM (default: ram)\n"
                      "     --state=<filename>   use <filename> (absolute or relative "
                      "to <config-dir>) as STATE (default: hp48)\n"
                      "     --port1=<filename>   use <filename> (absolute or relative "
                      "to <config-dir>) as PORT1 (default: port1)\n"
                      "     --port2=<filename>   use <filename> (absolute or relative "
                      "to <config-dir>) as PORT2 (default: port2)\n"
                      "     --serial-line=<path> use <path> as serial device default: "
                      "%s)\n"
                      "  -V --verbose            be verbose (default: false)\n"
                      "     --x11                use X11 front-end (default: true)\n"
                      "     --sdl                use SDL front-end (default: false)\n"
                      "     --tui                use text front-end (default: false)\n"
                      "     --tui-small          use text small front-end (2×2 pixels per character) (default: "
                      "false)\n"
                      "     --tui-tiny           use text tiny front-end (2×4 pixels per character) (default: "
                      "false)\n"
                      "  -t --use-terminal       activate pseudo terminal interface (default: "
                      "true)\n"
                      "  -s --use-serial         activate serial interface (default: false)\n"
                      "     --debug              enable the debugger\n"
                      "  -r --reset              perform a reset on startup\n"
                      "  -T --throttle           try to emulate real speed (default: false)\n"
                      "     --no-chrome          only display the LCD (default: "
                      "false)\n"
                      "     --fullscreen         make the UI fullscreen "
                      "(default: false)\n"
                      "     --netbook            make the UI horizontal (default: "
                      "false)\n"
                      "     --visual=<X visual>  use x11 visual <X visual> (default: "
                      "default), possible values: "
                      "<default | staticgray | staticcolor | truecolor | grayscale | "
                      "pseudocolor | directcolor | 0xnn | nn>\n"
                      "     --small-font=<font>  use <X font name> as small "
                      "font (default: %s)\n"
                      "     --medium-font=<font> use <X font name> as medium "
                      "font (default: %s)\n"
                      "     --large-font=<font>  use <X font name> as large "
                      "font (default: %s)\n"
                      "     --connection-font=<font> use <X font name> as "
                      "connection font (default: %s)\n"
                      "     --mono               make the UI monochrome (default: "
                      "false)\n"
                      "     --gray               make the UI grayscale (default: "
                      "false)\n"
                      "     --leave-shift-keys _not_ mapping the shift keys to let them free for numbers (default: "
                      "false)\n";
    while ( c != EOF ) {
        c = getopt_long( argc, argv, optstring, long_options, &option_index );

        switch ( c ) {
            case 'h':
                fprintf( stderr, help_text, progname, serialLine, smallFont, mediumFont, largeFont, connFont );
                exit( 0 );
                break;
            case 'v':
                fprintf( stderr, "%s %d.%d.%d\n", progname, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );
                exit( 0 );
                break;
            case 'c':
                config_file = optarg;
                break;
            case 1000:
                clopt_configDir = optarg;
                break;
            case 1010:
                clopt_romFileName = optarg;
                break;
            case 1011:
                clopt_ramFileName = optarg;
                break;
            case 1012:
                clopt_stateFileName = optarg;
                break;
            case 1013:
                clopt_port1FileName = optarg;
                break;
            case 1014:
                clopt_port2FileName = optarg;
                break;
            case 1015:
                clopt_serialLine = optarg;
                break;
            case 8110:
                clopt_x11_visual = optarg;
                break;
            case 8111:
                clopt_smallFont = optarg;
                break;
            case 8112:
                clopt_mediumFont = optarg;
                break;
            case 8113:
                clopt_largeFont = optarg;
                break;
            case 8114:
                clopt_connFont = optarg;
                break;
            case 9100:
                clopt_frontend_type = FRONTEND_TEXT;
                clopt_small = false;
                clopt_tiny = false;
                break;
            case 9109:
                fprintf( stdout, "`--small` is deprecated, please use `--tui-small` instead of `--tui --small`" );
            case 9110:
                clopt_frontend_type = FRONTEND_TEXT;
                clopt_small = true;
                clopt_tiny = false;
                break;
            case 9119:
                fprintf( stdout, "`--tiny` is deprecated, please use `--tui-tiny` instead of `--tui --tiny`" );
            case 9120:
                clopt_frontend_type = FRONTEND_TEXT;
                clopt_small = false;
                clopt_tiny = true;
                break;
            case 'V':
                clopt_verbose = true;
                break;
            case 't':
                clopt_useTerminal = true;
                break;
            case 's':
                clopt_useSerial = true;
                break;
            case 'r':
                resetOnStartup = true;
                break;
            case 'T':
                clopt_throttle = true;
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

    /**********************/
    /* 1. read config.lua */
    /**********************/
    normalize_filename( config_file, normalized_config_file );
    if ( !config_read( normalized_config_file ) ) {
        fprintf( stderr, "\nConfiguration file %s doesn't seem to exist or is invalid!\n", normalized_config_file );
        fprintf( stderr, "Continuing using default configuration as printed below.\n\n" );

        fprintf( stderr, "You can solve this by running `mkdir -p %s && %s --print-config >> %s`\n\n", normalized_config_path, progname,
                 normalized_config_file );
        print_config = true;
    }

    lua_getglobal( config_lua_values, "config_dir" );
    configDir = ( char* )luaL_optstring( config_lua_values, -1, "x48ng" );

    lua_getglobal( config_lua_values, "rom" );
    romFileName = ( char* )luaL_optstring( config_lua_values, -1, "rom" );

    lua_getglobal( config_lua_values, "ram" );
    ramFileName = ( char* )luaL_optstring( config_lua_values, -1, "ram" );

    lua_getglobal( config_lua_values, "state" );
    stateFileName = ( char* )luaL_optstring( config_lua_values, -1, "state" );

    lua_getglobal( config_lua_values, "port1" );
    port1FileName = ( char* )luaL_optstring( config_lua_values, -1, "port1" );

    lua_getglobal( config_lua_values, "port2" );
    port2FileName = ( char* )luaL_optstring( config_lua_values, -1, "port2" );

    lua_getglobal( config_lua_values, "serial_line" );
    serialLine = ( char* )luaL_optstring( config_lua_values, -1, "/dev/ttyS0" );

    lua_getglobal( config_lua_values, "pseudo_terminal" );
    useTerminal = lua_toboolean( config_lua_values, -1 );

    lua_getglobal( config_lua_values, "serial" );
    useSerial = lua_toboolean( config_lua_values, -1 );

    lua_getglobal( config_lua_values, "debugger" );
    useDebugger = lua_toboolean( config_lua_values, -1 );

    lua_getglobal( config_lua_values, "throttle" );
    throttle = lua_toboolean( config_lua_values, -1 );

    lua_getglobal( config_lua_values, "frontend" );
#ifdef HAS_X11
#  define DEFAULT_FRONTEND "x11"
#elif HAS_SDL
#  define DEFAULT_FRONTEND "sdl"
#else
#  define DEFAULT_FRONTEND "tui"
#endif
    const char* svalue = luaL_optstring( config_lua_values, -1, DEFAULT_FRONTEND );
    if ( svalue != NULL ) {
        if ( strcmp( svalue, "x11" ) == 0 )
            frontend_type = FRONTEND_X11;
        if ( strcmp( svalue, "sdl" ) == 0 )
            frontend_type = FRONTEND_SDL;
        if ( strcmp( svalue, "tui" ) == 0 ) {
            frontend_type = FRONTEND_TEXT;
            small = false;
            tiny = false;
        }
        if ( strcmp( svalue, "tui-small" ) == 0 ) {
            frontend_type = FRONTEND_TEXT;
            small = true;
            tiny = false;
        }
        if ( strcmp( svalue, "tui-tiny" ) == 0 ) {
            frontend_type = FRONTEND_TEXT;
            small = false;
            tiny = true;
        }
    }

    lua_getglobal( config_lua_values, "hide_chrome" );
    hide_chrome = lua_toboolean( config_lua_values, -1 );

    lua_getglobal( config_lua_values, "fullscreen" );
    show_ui_fullscreen = lua_toboolean( config_lua_values, -1 );

    lua_getglobal( config_lua_values, "netbook" );
    netbook = lua_toboolean( config_lua_values, -1 );

    lua_getglobal( config_lua_values, "mono" );
    mono = lua_toboolean( config_lua_values, -1 );

    lua_getglobal( config_lua_values, "gray" );
    gray = lua_toboolean( config_lua_values, -1 );

    /* DEPRECATED */
    lua_getglobal( config_lua_values, "small" );
    small = lua_toboolean( config_lua_values, -1 );

    /* DEPRECATED */
    lua_getglobal( config_lua_values, "tiny" );
    tiny = lua_toboolean( config_lua_values, -1 );

    lua_getglobal( config_lua_values, "leave_shift_keys" );
    leave_shift_keys = lua_toboolean( config_lua_values, -1 );

    lua_getglobal( config_lua_values, "x11_visual" );
    x11_visual = ( char* )luaL_optstring( config_lua_values, -1, "default" );

    lua_getglobal( config_lua_values, "font_small" );
    smallFont = ( char* )luaL_optstring( config_lua_values, -1, "-*-fixed-bold-r-normal-*-14-*-*-*-*-*-iso8859-1" );

    lua_getglobal( config_lua_values, "font_medium" );
    mediumFont = ( char* )luaL_optstring( config_lua_values, -1, "-*-fixed-bold-r-normal-*-15-*-*-*-*-*-iso8859-1" );

    lua_getglobal( config_lua_values, "font_large" );
    largeFont = ( char* )luaL_optstring( config_lua_values, -1, "-*-fixed-medium-r-normal-*-20-*-*-*-*-*-iso8859-1" );

    lua_getglobal( config_lua_values, "font_devices" );
    connFont = ( char* )luaL_optstring( config_lua_values, -1, "-*-fixed-medium-r-normal-*-12-*-*-*-*-*-iso8859-1" );

    /****************************************************/
    /* 2. treat command-line params which have priority */
    /****************************************************/
    if ( clopt_configDir != NULL )
        configDir = strdup( clopt_configDir );
    if ( clopt_romFileName != NULL )
        romFileName = strdup( clopt_romFileName );
    if ( clopt_ramFileName != NULL )
        ramFileName = strdup( clopt_ramFileName );
    if ( clopt_stateFileName != NULL )
        stateFileName = strdup( clopt_stateFileName );
    if ( clopt_port1FileName != NULL )
        port1FileName = strdup( clopt_port1FileName );
    if ( clopt_port2FileName != NULL )
        port2FileName = strdup( clopt_port2FileName );
    if ( clopt_serialLine != NULL )
        serialLine = strdup( clopt_serialLine );
    if ( clopt_x11_visual != NULL )
        x11_visual = strdup( clopt_x11_visual );
    if ( clopt_smallFont != NULL )
        smallFont = strdup( clopt_smallFont );
    if ( clopt_mediumFont != NULL )
        mediumFont = strdup( clopt_mediumFont );
    if ( clopt_largeFont != NULL )
        largeFont = strdup( clopt_largeFont );
    if ( clopt_connFont != NULL )
        connFont = strdup( clopt_connFont );

    if ( clopt_verbose != -1 )
        verbose = clopt_verbose;
    if ( clopt_useTerminal != -1 )
        useTerminal = clopt_useTerminal;
    if ( clopt_useSerial != -1 )
        useSerial = clopt_useSerial;
    if ( clopt_throttle != -1 )
        throttle = clopt_throttle;
    if ( clopt_useDebugger != -1 )
        useDebugger = clopt_useDebugger;
    if ( clopt_frontend_type != -1 )
        frontend_type = clopt_frontend_type;
    if ( clopt_hide_chrome != -1 )
        hide_chrome = clopt_hide_chrome;
    if ( clopt_show_ui_fullscreen != -1 )
        show_ui_fullscreen = clopt_show_ui_fullscreen;
    if ( clopt_netbook != -1 )
        netbook = clopt_netbook;
    if ( clopt_mono != -1 )
        mono = clopt_mono;
    if ( clopt_gray != -1 )
        gray = clopt_gray;
    if ( clopt_small != -1 )
        small = clopt_small;
    if ( clopt_tiny != -1 )
        tiny = clopt_tiny;
    if ( clopt_leave_shift_keys != -1 )
        leave_shift_keys = clopt_leave_shift_keys;

    /* After getting configs and params */
    /* normalize config_dir again in case it's been modified */
    normalize_config_dir();
    normalize_filenames();

    print_config |= verbose;
    if ( print_config ) {
        fprintf( stdout, "--------------------------------------------------------------------------------\n" );
        fprintf( stdout, "-- Configuration file for x48ng\n" );
        fprintf( stdout, "-- This is a comment\n" );
        fprintf( stdout, "-- `config_dir` is relative to $XDG_CONFIG_HOME/, or $HOME/.config/ or absolute\n" );
        fprintf( stdout, "config_dir = \"%s\"\n", configDir );
        fprintf( stdout, "\n" );
        fprintf( stdout, "-- Pathes are either relative to `config_dir` or absolute\n" );
        fprintf( stdout, "rom = \"%s\"\n", romFileName );
        fprintf( stdout, "ram = \"%s\"\n", ramFileName );
        fprintf( stdout, "state = \"%s\"\n", stateFileName );
        fprintf( stdout, "port1 = \"%s\"\n", port1FileName );
        fprintf( stdout, "port2 = \"%s\"\n", port2FileName );
        fprintf( stdout, "\n" );
        fprintf( stdout, "pseudo_terminal = %s\n", useTerminal ? "true" : "false" );
        fprintf( stdout, "serial = %s\n", useSerial ? "true" : "false" );
        fprintf( stdout, "serial_line = \"%s\"\n", serialLine );
        fprintf( stdout, "\n" );
        fprintf( stdout, "verbose = %s\n", verbose ? "true" : "false" );
        fprintf( stdout, "debugger = %s\n", useDebugger ? "true" : "false" );
        fprintf( stdout, "throttle = %s\n", throttle ? "true" : "false" );
        fprintf( stdout, "\n" );
        fprintf( stdout, "--------------------\n" );
        fprintf( stdout, "-- User Interface --\n" );
        fprintf( stdout, "--------------------\n" );
        fprintf( stdout, "frontend = \"" );
        switch ( frontend_type ) {
            case FRONTEND_X11:
                fprintf( stdout, "x11" );
                break;
            case FRONTEND_SDL:
                fprintf( stdout, "sdl" );
                break;
            case FRONTEND_TEXT:
                if ( small )
                    fprintf( stdout, "tui-small" );
                else if ( tiny )
                    fprintf( stdout, "tui-tiny" );
                else
                    fprintf( stdout, "tui" );
                break;
        }
        fprintf( stdout, "\" -- possible values: \"x11\", \"sdl\", \"tui\", \"tui-small\", \"tui-tiny\"\n" );
        fprintf( stdout, "hide_chrome = %s\n", hide_chrome ? "true" : "false" );
        fprintf( stdout, "fullscreen = %s\n", show_ui_fullscreen ? "true" : "false" );
        fprintf( stdout, "mono = %s\n", mono ? "true" : "false" );
        fprintf( stdout, "gray = %s\n", gray ? "true" : "false" );
        fprintf( stdout, "leave_shift_keys = %s\n", leave_shift_keys ? "true" : "false" );
        fprintf( stdout, "\n" );
        fprintf( stdout, "x11_visual = \"%s\"\n", x11_visual );
        fprintf( stdout, "netbook = %s\n", netbook ? "true" : "false" );
        fprintf( stdout, "font_small = \"%s\"\n", smallFont );
        fprintf( stdout, "font_medium = \"%s\"\n", mediumFont );
        fprintf( stdout, "font_large = \"%s\"\n", largeFont );
        fprintf( stdout, "font_devices = \"%s\"\n", connFont );
        fprintf( stdout, "--------------------------------------------------------------------------------\n" );

        if ( !verbose )
            exit( 0 );
    }
    if ( verbose ) {
        fprintf( stderr, "normalized_config_path = %s\n", normalized_config_path );
        fprintf( stderr, "normalized_rom_path = %s\n", normalized_rom_path );
        fprintf( stderr, "normalized_ram_path = %s\n", normalized_ram_path );
        fprintf( stderr, "normalized_state_path = %s\n", normalized_state_path );
        fprintf( stderr, "normalized_port1_path = %s\n", normalized_port1_path );
        fprintf( stderr, "normalized_port2_path = %s\n", normalized_port2_path );
    }

    return ( optind );
}
