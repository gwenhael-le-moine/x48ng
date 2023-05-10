#ifndef _RESOURCES_H
#define _RESOURCES_H 1

#if defined( GUI_IS_X11 )
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#endif

extern int verbose;
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

#if defined( GUI_IS_X11 )
extern char* progname;
extern char* res_name;
extern char* res_class;

extern XrmDatabase rdb;

extern void usage( void );
extern void show_version( void );
extern void show_copyright( void );
extern void show_warranty( void );
extern char* get_string_resource_from_db( XrmDatabase db, char* name,
                                          char* class );
extern char* get_string_resource( char* name, char* class );
extern int get_boolean_resource( char* name, char* class );
extern int get_mnemonic_resource( char* name, char* class );
extern Visual* get_visual_resource( Display* dpy, char* name, char* class,
                                    unsigned int* depth );
extern XFontStruct* get_font_resource( Display* dpy, char* res_name,
                                       char* res_class );
#endif

extern void get_resources( void );

#endif /* !_RESOURCES_H */
