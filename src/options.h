#ifndef _OPTIONS_H
#define _OPTIONS_H 1

extern int verbose;
extern int useTerminal;
extern int useSerial;
extern int throttle;
extern int initialize;
extern int resetOnStartup;

extern char* serialLine;
extern char* romFileName;
extern char* homeDirectory;

extern int parse_args( int argc, char* argv[] );

#endif /* !_OPTIONS_H */
