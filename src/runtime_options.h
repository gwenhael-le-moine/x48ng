#ifndef _OPTIONS_H
#define _OPTIONS_H 1

extern char* progname;

extern int verbose;
extern int show_ui_chrome;
extern int useTerminal;
extern int useSerial;
extern int useDebugger;
extern int throttle;
extern int initialize;
extern int resetOnStartup;

extern char* serialLine;
extern char* homeDirectory;

extern char* romFileName;
extern char* ramFileName;
extern char* stateFileName;
extern char* port1FileName;
extern char* port2FileName;

extern int parse_args( int argc, char* argv[] );

#endif /* !_OPTIONS_H */
