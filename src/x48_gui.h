#ifndef _X48_GUI_H
#define _X48_GUI_H 1

#include "config.h"

#include <X11/Xlib.h>
#ifdef HAVE_XSHM
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

#define WHITE 0
#define LEFT 1
#define RIGHT 2
#define BUT_TOP 3
#define BUTTON 4
#define BUT_BOT 5
#define LCD 6
#define PIXEL 7
#define PAD_TOP 8
#define PAD 9
#define PAD_BOT 10
#define DISP_PAD_TOP 11
#define DISP_PAD 12
#define DISP_PAD_BOT 13
#define LOGO 14
#define LOGO_BACK 15
#define LABEL 16
#define FRAME 17
#define UNDERLAY 18
#define BLACK 19

typedef struct color_t {
    char* name;
    int r, g, b;
    int mono_rgb;
    int gray_rgb;
    XColor xcolor;
} color_t;

extern color_t* colors;

#define COLOR( c ) ( colors[ ( c ) ].xcolor.pixel )

#define UPDATE_MENU 1
#define UPDATE_DISP 2

typedef struct disp_t {
    unsigned int w, h;
    Window win;
    GC gc;
    short mapped;
    int offset;
    int lines;
#ifdef HAVE_XSHM
    int display_update;
    XShmSegmentInfo disp_info;
    XImage* disp_image;
    XShmSegmentInfo menu_info;
    XImage* menu_image;
#endif
} disp_t;

extern disp_t disp;

#ifdef HAVE_XSHM
extern int shm_flag;
#endif

extern Display* dpy;
extern int screen;

extern int InitDisplay( int argc, char** argv );
extern int CreateWindows( int argc, char** argv );
extern int GetEvent( void );

extern void adjust_contrast( int contrast );
extern void refresh_icon( void );

extern void ShowConnections( char* w, char* i );

extern void exit_x48( int tell_x11 );

#ifdef HAVE_XSHM
extern void refresh_display( void );
#endif

#endif /* !_X48_GUI_H */
