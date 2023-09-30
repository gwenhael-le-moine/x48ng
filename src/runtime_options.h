#ifndef _OPTIONS_H
#define _OPTIONS_H 1

#ifdef HAS_SDL
#  define FRONTEND_SDL 0
#endif

#ifdef HAS_X11
#  define FRONTEND_X11 1
#endif

#define FRONTEND_TEXT 2

extern char* progname;

extern int verbose;
extern int useTerminal;
extern int useSerial;
extern int useDebugger;
extern int throttle;
extern int initialize;
extern int resetOnStartup;
extern int frontend_type;

extern char* serialLine;

extern char* configDir;
extern char* config_file;
extern char* romFileName;
extern char* ramFileName;
extern char* stateFileName;
extern char* port1FileName;
extern char* port2FileName;

/* sdl */
extern int show_ui_chrome;
extern int show_ui_fullscreen;

/* x11 */
extern int netbook;
extern char* name;
extern char* title;
extern char* geometry;
/* extern char* iconGeom; */
/* extern char* iconName; */
extern char* x11_visual;
extern int mono;
extern int gray;
extern int monoIcon;
extern int iconic;
extern int xrm;
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
extern void get_absolute_config_dir( char* source, char* path );
extern int parse_args( int argc, char* argv[] );

#endif /* !_OPTIONS_H */
