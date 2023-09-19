#ifndef _OPTIONS_H
#define _OPTIONS_H 1

#define FRONTEND_SDL 0
#define FRONTEND_X11 1

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
extern char* homeDirectory;

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

/* extern int x11_visual; */
extern int mono;
extern int gray;
extern int monoIcon;
extern int iconic;
extern int xrm;

extern char* smallFont;
extern char* mediumFont;
extern char* largeFont;
extern char* connFont;

/*************/
/* functions */
/*************/
extern int parse_args( int argc, char* argv[] );

#endif /* !_OPTIONS_H */
