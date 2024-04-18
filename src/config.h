#ifndef _CONFIG_H
#define _CONFIG_H 1

#include <stdbool.h>

#define FRONTEND_SDL 0
#define FRONTEND_X11 1
#define FRONTEND_TEXT 2

typedef struct {
    char* progname;

    bool verbose;
    bool print_config;
    bool useTerminal;
    bool useSerial;
    bool useDebugger;
    bool throttle;
    bool resetOnStartup;
    int frontend_type;

    char* serialLine;

    bool leave_shift_keys;
    bool inhibit_shutdown;

    bool mono;
    bool gray;

    /* tui */
    bool small;
    bool tiny;

    /* sdl */
    bool hide_chrome;
    bool show_ui_fullscreen;

    /* x11 */
    bool netbook;
    char* name;
    char* title;
    char* x11_visual;
    bool monoIcon;
    bool iconic;
    bool xrm;
    char* smallFont;
    char* mediumFont;
    char* largeFont;
    char* connFont;
} config_t;
extern config_t config;

#define MAX_LENGTH_FILENAME 2048
extern char normalized_config_path[ MAX_LENGTH_FILENAME ];
extern char normalized_config_file[ MAX_LENGTH_FILENAME ];
extern char normalized_rom_path[ MAX_LENGTH_FILENAME ];
extern char normalized_ram_path[ MAX_LENGTH_FILENAME ];
extern char normalized_state_path[ MAX_LENGTH_FILENAME ];
extern char normalized_port1_path[ MAX_LENGTH_FILENAME ];
extern char normalized_port2_path[ MAX_LENGTH_FILENAME ];

/*************/
/* functions */
/*************/
extern int config_init( int argc, char* argv[] );

#endif /* !_CONFIG_H */
