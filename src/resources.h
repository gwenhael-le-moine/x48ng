#ifndef _RESOURCES_H
#define _RESOURCES_H 1

#if defined( GUI_IS_X11 )
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#endif

extern int verbose;
extern int quiet;
extern int useTerminal;
extern int useSerial;
extern int useXShm;
extern int useDebugger;
extern int netbook;
extern int throttle;
extern int initialize;
extern int resetOnStartup;
#if defined( GUI_IS_X11 )
extern char* serialLine;
extern char* romFileName;
extern char* homeDirectory;
#elif defined( GUI_IS_SDL1 )
extern char serialLine[];
extern char romFileName[];
extern char homeDirectory[];
#endif

extern char* progname;
extern char* res_name;
extern char* res_class;

#if defined( GUI_IS_X11 )
extern XrmDatabase rdb;

extern void usage();
extern void show_version();
extern void show_copyright();
extern void show_warranty();
extern void get_resources();
extern char* get_string_resource_from_db( XrmDatabase db, char* name,
                                          char* class );
extern char* get_string_resource( char* name, char* class );
extern int get_boolean_resource( char* name, char* class );
extern int get_mnemonic_resource( char* name, char* class );
extern Visual* get_visual_resource( Display* dpy, char* name, char* class,
                                    unsigned int* depth );
extern XFontStruct* get_font_resource( Display* dpy, char* res_name,
                                       char* res_class );

#ifndef isupper
#define isupper( c ) ( ( c ) >= 'A' && ( c ) <= 'Z' )
#endif
#ifndef islower
#define islower( c ) ( ( c ) >= 'a' && ( c ) <= 'z' )
#endif
#ifndef _tolower
#define _tolower( c ) ( ( c ) - 'A' + 'a' )
#endif
#ifndef _toupper
#define _toupper( c ) ( ( c ) - 'a' + 'A' )
#endif
#endif

#if defined( GUI_IS_SDL1 )
void get_resources();
#endif

#endif /* !_RESOURCES_H */
