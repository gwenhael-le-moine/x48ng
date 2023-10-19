#ifndef _OPTIONS_H
#define _OPTIONS_H 1

#include <stdbool.h>

#define FRONTEND_SDL 0
#define FRONTEND_X11 1
#define FRONTEND_TEXT 2

extern char* progname;

extern bool verbose;
extern bool useTerminal;
extern bool useSerial;
extern bool useDebugger;
extern bool throttle;
extern bool resetOnStartup;
extern int frontend_type;

extern char* serialLine;

extern bool mono;
extern bool gray;

/* tui */
extern bool small;
extern bool tiny;

/* sdl */
extern bool hide_chrome;
extern bool show_ui_fullscreen;

/* x11 */
extern bool netbook;
extern char* name;
extern char* title;
extern char* x11_visual;
extern bool monoIcon;
extern bool iconic;
extern bool xrm;
extern char* smallFont;
extern char* mediumFont;
extern char* largeFont;
extern char* connFont;

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
extern int parse_args( int argc, char* argv[] );

#endif /* !_OPTIONS_H */
