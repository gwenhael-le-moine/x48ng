#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

#include <lauxlib.h>
#include <lua.h>

#include <glib.h>

#include "options.h"

static config_t __config = {
    .progname = ( char* )"x48ng",

    .verbose = false,
    .print_config = false,
    .useTerminal = false,
    .useSerial = false,
    .useDebugger = false,
    .throttle = false,
    .resetOnStartup = false,

    .serialLine = NULL,

    .frontend = FRONTEND_NCURSES,

    .shiftless = false,
    .inhibit_shutdown = false,

    .mono = false,
    .gray = false,

    .big_screen = false,
    .black_lcd = false,

    /* tui */
    .small = false,
    .tiny = false,

    /* sdl */
    .chromeless = false,
    .fullscreen = false,
    .scale = 1.0,

    .style_filename = NULL,
};

char* configDir = ( char* )"x48ng";
char* config_file = ( char* )"config.lua";
char* romFileName = NULL;
char* ramFileName = NULL;
char* stateFileName = NULL;
char* port1FileName = NULL;
char* port2FileName = NULL;

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
            if ( __config.verbose )
                fprintf( stderr, "XDG_CONFIG_HOME is %s\n", xdg_config_home );

            strcpy( dest, xdg_config_home );
            strcat( dest, "/" );
        } else {
            char* home = getenv( "HOME" );

            if ( home ) {
                if ( __config.verbose )
                    fprintf( stderr, "HOME is %s\n", home );

                strcpy( dest, home );
                strcat( dest, "/.config/" );
            } else {
                struct passwd* pwd = getpwuid( getuid() );

                if ( pwd ) {
                    if ( __config.verbose )
                        fprintf( stderr, "pwd->pw_dir is %s\n", pwd->pw_dir );

                    strcpy( dest, pwd->pw_dir );
                    strcat( dest, "/" );
                } else {
                    if ( __config.verbose )
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

static inline bool normalize_config_dir( void )
{
    struct stat st;

    get_absolute_config_dir( configDir, normalized_config_path );
    if ( __config.verbose )
        fprintf( stderr, "normalized_config_path: %s\n", normalized_config_path );

    if ( stat( normalized_config_path, &st ) == -1 )
        if ( errno == ENOENT )
            return false;

    return true;
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

/* static char* make_filename_absolute( char* filename ) */
/* { */
/*     char* full_path = g_build_filename( filename, NULL ); */
/*     if ( !g_file_test( full_path, G_FILE_TEST_EXISTS ) ) */
/*         full_path = g_build_filename( __config.datadir, filename, NULL ); */
/*     if ( !g_file_test( full_path, G_FILE_TEST_EXISTS ) ) */
/*         full_path = g_build_filename( GLOBAL_DATADIR, filename, NULL ); */
/*     if ( !g_file_test( full_path, G_FILE_TEST_EXISTS ) ) */
/*         full_path = g_build_filename( __config.progpath, filename, NULL ); */

/*     return full_path; */
/* } */

config_t* config_init( int argc, char* argv[] )
{
    int option_index;
    int c = '?';

    char* style_filename = NULL;
    char* clopt_style_filename = NULL;
    char* clopt_name = NULL;

    char* clopt_configDir = NULL;
    char* clopt_romFileName = NULL;
    char* clopt_ramFileName = NULL;
    char* clopt_stateFileName = NULL;
    char* clopt_port1FileName = NULL;
    char* clopt_port2FileName = NULL;
    char* clopt_serialLine = NULL;
    int clopt_frontend = -1;
    int clopt_verbose = -1;
    int clopt_useTerminal = -1;
    int clopt_useSerial = -1;
    int clopt_useDebugger = -1;
    int clopt_throttle = -1;
    int clopt_chromeless = -1;
    int clopt_fullscreen = -1;
    double clopt_scale = -1.0;
    int clopt_mono = -1;
    int clopt_gray = -1;
    int clopt_small = -1;
    int clopt_tiny = -1;
    int clopt_shiftless = -1;
    int clopt_inhibit_shutdown = -1;
    int clopt_black_lcd = -1;

    const char* optstring = "c:hvVtsirTn:s:";
    struct option long_options[] = {
        {"help",             no_argument,       NULL,                             'h'         },
        {"version",          no_argument,       NULL,                             'v'         },
        {"verbose",          no_argument,       &clopt_verbose,                   true        },
        {"print-config",     no_argument,       ( int* )&__config.print_config,   true        },

        {"throttle",         no_argument,       &clopt_throttle,                  true        },
        {"reset",            no_argument,       ( int* )&__config.resetOnStartup, true        },

        {"config",           required_argument, NULL,                             'c'         },
        {"config-dir",       required_argument, NULL,                             1000        },
        {"rom",              required_argument, NULL,                             1010        },
        {"ram",              required_argument, NULL,                             1011        },
        {"state",            required_argument, NULL,                             1012        },
        {"port1",            required_argument, NULL,                             1013        },
        {"port2",            required_argument, NULL,                             1014        },

        {"serial-line",      required_argument, NULL,                             1015        },

        {"terminal",         no_argument,       &clopt_useTerminal,               true        },
        {"serial",           no_argument,       &clopt_useSerial,                 true        },

        {"debug",            no_argument,       &clopt_useDebugger,               true        },

        {"sdl2",             no_argument,       &clopt_frontend,                  FRONTEND_SDL}, /* DEPRECATED */
        {"sdl",              no_argument,       &clopt_frontend,                  FRONTEND_SDL},
        {"gtk",              no_argument,       &clopt_frontend,                  FRONTEND_GTK},
        {"no-chrome",        no_argument,       &clopt_chromeless,                true        }, /* DEPRECATED */
        {"chromeless",       no_argument,       &clopt_chromeless,                true        },
        {"fullscreen",       no_argument,       &clopt_fullscreen,                true        },
        {"scale",            required_argument, NULL,                             7110        },
        {"black-lcd",        no_argument,       &clopt_black_lcd,                 true        },

        {"tui",              no_argument,       NULL,                             9100        },
        {"tui-small",        no_argument,       NULL,                             9110        },
        {"tui-tiny",         no_argument,       NULL,                             9120        },
        {"small",            no_argument,       NULL,                             9109        }, /* DEPRECATED */
        {"tiny",             no_argument,       NULL,                             9119        }, /* DEPRECATED */

        {"mono",             no_argument,       &clopt_mono,                      true        },
        {"gray",             no_argument,       &clopt_gray,                      true        },
        {"leave-shift-keys", no_argument,       &clopt_shiftless,                 true        }, /* DEPRECATED */
        {"shiftless",        no_argument,       &clopt_shiftless,                 true        },
        {"inhibit-shutdown", no_argument,       &clopt_inhibit_shutdown,          true        },

        {"style",            required_argument, NULL,                             's'         },

        {"name",             required_argument, NULL,                             'n'         },

        {0,                  0,                 0,                                0           }
    };

    const char* help_text = "usage: %s [options]\n"
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
                            "     --sdl                use SDL2 front-end (default: false)\n"
                            "     --tui                use text front-end (default: false)\n"
                            "     --tui-small          use text small front-end (2×2 pixels per character) (default: "
                            "false)\n"
                            "     --tui-tiny           use text tiny front-end (2×4 pixels per character) (default: "
                            "false)\n"
                            "  -t --terminal           activate pseudo terminal interface (default: "
                            "true)\n"
                            "  -s --serial             activate serial interface (default: false)\n"
                            "     --debug              enable the debugger\n"
                            "  -r --reset              perform a reset on startup\n"
                            "  -T --throttle           try to emulate real speed (default: false)\n"
                            "     --chromeless         only display the LCD (default: "
                            "false)\n"
                            "     --fullscreen         make the UI fullscreen "
                            "(default: false)\n"
                            "     --scale=<number>     make the UI scale <number> times "
                            "(default: 1.0)\n"
                            "     --mono               make the UI monochrome (default: "
                            "false)\n"
                            "     --gray               make the UI grayscale (default: "
                            "false)\n"
                            "     --shiftless          _not_ mapping the shift keys to let them free for numbers (default: "
                            "false)\n"
                            "     --inhibit-shutdown   __tentative fix for stuck-on-OFF bug__ (default: "
                            "false)\n";
    while ( c != EOF ) {
        c = getopt_long( argc, argv, optstring, long_options, &option_index );

        switch ( c ) {
            case 'h':
                fprintf( stderr, help_text, __config.progname, __config.serialLine );
                exit( 0 );
                break;
            case 'v':
                fprintf( stderr, "%s %d.%d.%d\n", __config.progname, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );
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
            case 7110:
                clopt_scale = atof( optarg );
                break;
            case 9100:
                clopt_frontend = FRONTEND_NCURSES;
                clopt_small = false;
                clopt_tiny = false;
                break;
            case 9109:
                fprintf( stdout, "`--small` is deprecated, please use `--tui-small` instead of `--tui --small`" );
                /* break; */ /* intentional fall-through */
            case 9110:
                clopt_frontend = FRONTEND_NCURSES;
                clopt_small = true;
                clopt_tiny = false;
                break;
            case 9119:
                fprintf( stdout, "`--tiny` is deprecated, please use `--tui-tiny` instead of `--tui --tiny`" );
                /* break; */ /* intentional fall-through */
            case 9120:
                clopt_frontend = FRONTEND_NCURSES;
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
                __config.resetOnStartup = true;
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

    if ( !normalize_config_dir() )
        fprintf( stderr, "Configuration directory doesn't exist!\n" );

    /**********************/
    /* 1. read config.lua */
    /**********************/
    normalize_filename( config_file, normalized_config_file );
    if ( !config_read( normalized_config_file ) ) {
        fprintf( stderr, "\nConfiguration file %s doesn't seem to exist or is invalid!\n", normalized_config_file );
        fprintf( stderr, "Continuing using default configuration as printed below.\n\n" );

        fprintf( stderr, "You can solve this by running `mkdir -p %s && %s --print-config >> %s`\n\n", normalized_config_path,
                 __config.progname, normalized_config_file );
        __config.print_config = true;
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
    __config.serialLine = ( char* )luaL_optstring( config_lua_values, -1, "/dev/ttyS0" );

    lua_getglobal( config_lua_values, "pseudo_terminal" );
    __config.useTerminal = lua_toboolean( config_lua_values, -1 );

    lua_getglobal( config_lua_values, "serial" );
    __config.useSerial = lua_toboolean( config_lua_values, -1 );

    lua_getglobal( config_lua_values, "debugger" );
    __config.useDebugger = lua_toboolean( config_lua_values, -1 );

    lua_getglobal( config_lua_values, "throttle" );
    __config.throttle = lua_toboolean( config_lua_values, -1 );

    lua_getglobal( config_lua_values, "frontend" );
#ifdef HAS_SDL
#  define DEFAULT_FRONTEND "sdl2"
#else
#  define DEFAULT_FRONTEND "tui"
#endif
    const char* svalue = luaL_optstring( config_lua_values, -1, DEFAULT_FRONTEND );
    if ( svalue != NULL ) {
        if ( strcmp( svalue, "sdl2" ) == 0 )
            __config.frontend = FRONTEND_SDL;
        if ( strcmp( svalue, "sdl" ) == 0 ) /* retro-compatibility */
            __config.frontend = FRONTEND_SDL;
        if ( strcmp( svalue, "tui" ) == 0 ) {
            __config.frontend = FRONTEND_NCURSES;
            __config.small = false;
            __config.tiny = false;
        }
        if ( strcmp( svalue, "tui-small" ) == 0 ) {
            __config.frontend = FRONTEND_NCURSES;
            __config.small = true;
            __config.tiny = false;
        }
        if ( strcmp( svalue, "tui-tiny" ) == 0 ) {
            __config.frontend = FRONTEND_NCURSES;
            __config.small = false;
            __config.tiny = true;
        }
    }

    /* DEPRECATED */
    lua_getglobal( config_lua_values, "hide_chrome" );
    __config.chromeless = lua_toboolean( config_lua_values, -1 );

    /* DEPRECATED */
    lua_getglobal( config_lua_values, "leave_shift_keys" );
    __config.shiftless = lua_toboolean( config_lua_values, -1 );

    lua_getglobal( config_lua_values, "chromeless" );
    __config.chromeless = lua_toboolean( config_lua_values, -1 );

    lua_getglobal( config_lua_values, "fullscreen" );
    __config.fullscreen = lua_toboolean( config_lua_values, -1 );

    lua_getglobal( config_lua_values, "scale" );
    __config.scale = luaL_optnumber( config_lua_values, -1, 1.0 );

    lua_getglobal( config_lua_values, "mono" );
    __config.mono = lua_toboolean( config_lua_values, -1 );

    lua_getglobal( config_lua_values, "gray" );
    __config.gray = lua_toboolean( config_lua_values, -1 );

    lua_getglobal( config_lua_values, "black_lcd" );
    __config.black_lcd = lua_toboolean( config_lua_values, -1 );

    lua_getglobal( config_lua_values, "shiftless" );
    __config.shiftless = lua_toboolean( config_lua_values, -1 );

    lua_getglobal( config_lua_values, "inhibit_shutdown" );
    __config.inhibit_shutdown = lua_toboolean( config_lua_values, -1 );

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
        __config.serialLine = strdup( clopt_serialLine );

    if ( clopt_verbose != -1 )
        __config.verbose = clopt_verbose;
    if ( clopt_useTerminal != -1 )
        __config.useTerminal = clopt_useTerminal;
    if ( clopt_useSerial != -1 )
        __config.useSerial = clopt_useSerial;
    if ( clopt_throttle != -1 )
        __config.throttle = clopt_throttle;
    if ( clopt_useDebugger != -1 )
        __config.useDebugger = clopt_useDebugger;
    if ( clopt_frontend != -1 )
        __config.frontend = clopt_frontend;
    if ( clopt_chromeless != -1 )
        __config.chromeless = clopt_chromeless;
    if ( clopt_fullscreen != -1 )
        __config.fullscreen = clopt_fullscreen;
    if ( clopt_scale > 0.0 )
        __config.scale = clopt_scale;
    if ( clopt_mono != -1 )
        __config.mono = clopt_mono;
    if ( clopt_gray != -1 )
        __config.gray = clopt_gray;
    if ( clopt_small != -1 )
        __config.small = clopt_small;
    if ( clopt_tiny != -1 )
        __config.tiny = clopt_tiny;
    if ( clopt_black_lcd != -1 )
        __config.black_lcd = clopt_black_lcd == true;
    if ( clopt_shiftless != -1 )
        __config.shiftless = clopt_shiftless;
    if ( clopt_inhibit_shutdown != -1 )
        __config.inhibit_shutdown = clopt_inhibit_shutdown;

    /* After getting configs and params */
    /* normalize config_dir again in case it's been modified */
    if ( !normalize_config_dir() )
        fprintf( stderr, "Configuration directory doesn't exist!\n" );

    normalize_filenames();

    __config.print_config |= __config.verbose;
    if ( __config.print_config ) {
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
        fprintf( stdout, "pseudo_terminal = %s\n", __config.useTerminal ? "true" : "false" );
        fprintf( stdout, "serial = %s\n", __config.useSerial ? "true" : "false" );
        fprintf( stdout, "serial_line = \"%s\"\n", __config.serialLine );
        fprintf( stdout, "\n" );
        fprintf( stdout, "verbose = %s\n", __config.verbose ? "true" : "false" );
        fprintf( stdout, "debugger = %s\n", __config.useDebugger ? "true" : "false" );
        fprintf( stdout, "throttle = %s\n", __config.throttle ? "true" : "false" );
        fprintf( stdout, "\n" );
        fprintf( stdout, "--------------------\n" );
        fprintf( stdout, "-- User Interface --\n" );
        fprintf( stdout, "--------------------\n" );
        fprintf( stdout, "frontend = \"" );
        switch ( __config.frontend ) {
            case FRONTEND_SDL:
                fprintf( stdout, "sdl" );
                break;
            case FRONTEND_GTK:
                fprintf( stdout, "gtk" );
                break;
            case FRONTEND_NCURSES:
                if ( __config.small )
                    fprintf( stdout, "tui-small" );
                else if ( __config.tiny )
                    fprintf( stdout, "tui-tiny" );
                else
                    fprintf( stdout, "tui" );
                break;
        }
        fprintf( stdout, "\" -- possible values: \"sdl\" \"tui\", \"tui-small\", \"tui-tiny\"\n" );
        fprintf( stdout, "chromeless = %s\n", __config.chromeless ? "true" : "false" );
        fprintf( stdout, "fullscreen = %s\n", __config.fullscreen ? "true" : "false" );
        fprintf( stdout, "scale = %f -- applies only to sdl2\n", __config.scale );
        fprintf( stdout, "mono = %s\n", __config.mono ? "true" : "false" );
        fprintf( stdout, "gray = %s\n", __config.gray ? "true" : "false" );
        fprintf( stdout, "black_lcd = %s\n", __config.black_lcd ? "true" : "false" );
        fprintf( stdout, "shiftless = %s\n", __config.shiftless ? "true" : "false" );
        fprintf( stdout, "inhibit_shutdown = %s\n", __config.inhibit_shutdown ? "true" : "false" );
        fprintf( stdout, "--------------------------------------------------------------------------------\n" );

        if ( !__config.verbose )
            exit( 0 );
    }
    if ( __config.verbose ) {
        fprintf( stderr, "normalized_config_path = %s\n", normalized_config_path );
        fprintf( stderr, "normalized_rom_path = %s\n", normalized_rom_path );
        fprintf( stderr, "normalized_ram_path = %s\n", normalized_ram_path );
        fprintf( stderr, "normalized_state_path = %s\n", normalized_state_path );
        fprintf( stderr, "normalized_port1_path = %s\n", normalized_port1_path );
        fprintf( stderr, "normalized_port2_path = %s\n", normalized_port2_path );
    }

    // return ( optind );

    return &__config;
}
