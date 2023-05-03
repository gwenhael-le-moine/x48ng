#ifndef _X48_GUI_H
#define _X48_GUI_H 1

#include "config.h"

#if defined( GUI_IS_X11 )
#include <X11/Xlib.h>
#ifdef HAVE_XSHM
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif
#elif defined( GUI_IS_SDL1 )
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_rotozoom.h>
#endif

// Colors
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

// Buttons
#define BUTTON_A 0
#define BUTTON_B 1
#define BUTTON_C 2
#define BUTTON_D 3
#define BUTTON_E 4
#define BUTTON_F 5

#define BUTTON_MTH 6
#define BUTTON_PRG 7
#define BUTTON_CST 8
#define BUTTON_VAR 9
#define BUTTON_UP 10
#define BUTTON_NXT 11

#define BUTTON_COLON 12
#define BUTTON_STO 13
#define BUTTON_EVAL 14
#define BUTTON_LEFT 15
#define BUTTON_DOWN 16
#define BUTTON_RIGHT 17

#define BUTTON_SIN 18
#define BUTTON_COS 19
#define BUTTON_TAN 20
#define BUTTON_SQRT 21
#define BUTTON_POWER 22
#define BUTTON_INV 23

#define BUTTON_ENTER 24
#define BUTTON_NEG 25
#define BUTTON_EEX 26
#define BUTTON_DEL 27
#define BUTTON_BS 28

#define BUTTON_ALPHA 29
#define BUTTON_7 30
#define BUTTON_8 31
#define BUTTON_9 32
#define BUTTON_DIV 33

#define BUTTON_SHL 34
#define BUTTON_4 35
#define BUTTON_5 36
#define BUTTON_6 37
#define BUTTON_MUL 38

#define BUTTON_SHR 39
#define BUTTON_1 40
#define BUTTON_2 41
#define BUTTON_3 42
#define BUTTON_MINUS 43

#define BUTTON_ON 44
#define BUTTON_0 45
#define BUTTON_PERIOD 46
#define BUTTON_SPC 47
#define BUTTON_PLUS 48

#define LAST_BUTTON 48

#if defined( GUI_IS_SDL1 )
#define UPDATE_MENU 1
#define UPDATE_DISP 2

// Screen size

#define _KEYBOARD_HEIGHT                                                       \
    ( buttons_gx[ LAST_BUTTON ].y + buttons_gx[ LAST_BUTTON ].h )
#define _KEYBOARD_WIDTH                                                        \
    ( buttons_gx[ LAST_BUTTON ].x + buttons_gx[ LAST_BUTTON ].w )

#define _TOP_SKIP 65
#define _SIDE_SKIP 20
#define _BOTTOM_SKIP 25
#define _DISP_KBD_SKIP 65
#define _KBD_UPLINE 25

#define _DISPLAY_WIDTH ( 264 + 8 )
#define _DISPLAY_HEIGHT ( 128 + 16 + 8 )
#define _DISPLAY_OFFSET_X ( SIDE_SKIP + ( 286 - DISPLAY_WIDTH ) / 2 )
#define _DISPLAY_OFFSET_Y TOP_SKIP

#define _DISP_FRAME 8

#define _KEYBOARD_OFFSET_X SIDE_SKIP
#define _KEYBOARD_OFFSET_Y ( TOP_SKIP + DISPLAY_HEIGHT + DISP_KBD_SKIP )
#endif //SDL1

typedef struct color_t {
    const char* name;
    int r, g, b;
#if defined( GUI_IS_X11 )
    int mono_rgb;
    int gray_rgb;
    XColor xcolor;
#endif
} color_t;

#if defined( GUI_IS_X11 )
extern color_t* colors;

#define COLOR( c ) ( colors[ ( c ) ].xcolor.pixel )

#define UPDATE_MENU 1
#define UPDATE_DISP 2
#endif

#if defined( GUI_IS_SDL1 )
typedef struct keypad_t {
    unsigned int width;
    unsigned int height;
} keypad_t;
#endif

typedef struct disp_t {
    unsigned int w, h;
#if defined( GUI_IS_X11 )
    Window win;
    GC gc;
#endif
    short mapped;
    int offset;
    int lines;
#if defined( GUI_IS_X11 )
#ifdef HAVE_XSHM
    int display_update;
    XShmSegmentInfo disp_info;
    XImage* disp_image;
    XShmSegmentInfo menu_info;
    XImage* menu_image;
#endif
#endif
} disp_t;

#if defined( GUI_IS_X11 )
extern disp_t disp;

#ifdef HAVE_XSHM
extern int shm_flag;
#endif

extern Display* dpy;
extern int screen;
#endif

extern int InitDisplay( int argc, char** argv );
#if defined( GUI_IS_X11 )
extern int CreateWindows( int argc, char** argv );
#endif

#if defined( GUI_IS_SDL1 )
typedef struct button_t {
    const char* name;
    short pressed;
    short extra;

    int code;
    int x, y;
    unsigned int w, h;

    int lc;
    const char* label;
    short font_size;
    unsigned int lw, lh;
    unsigned char* lb;

    const char* letter;

    const char* left;
    short is_menu;
    const char* right;
    const char* sub;

    ///////////////////////////////////////////////
    // SDL PORT
    ///////////////////////////////////////////////
    SDL_Surface *surfaceup, *surfacedown;

} button_t;

// This mimicks the structure formerly lcd.c, except with SDL surfaces instead
// of Pixmaps.
typedef struct ann_struct {
    int bit;
    int x;
    int y;
    unsigned int width;
    unsigned int height;
    unsigned char* bits;
    ///////////////////////////////////////////////
    // SDL PORT
    ///////////////////////////////////////////////

    SDL_Surface* surfaceon;
    SDL_Surface* surfaceoff;
} ann_struct_t;

typedef struct SDLWINDOW {
    SDL_Surface *oldsurf, *surf;
    int x, y;
} SDLWINDOW_t;

extern color_t* colors;
extern ann_struct_t ann_tbl[];
extern disp_t disp;
extern button_t* buttons;
extern button_t buttons_sx[];
extern button_t buttons_gx[];

extern int InitDisplay( int argc, char** argv );
extern void SDLCreateHP();
#endif

extern int get_ui_event( void );

extern void adjust_contrast( int contrast );
#if defined( GUI_IS_X11 )
extern void refresh_icon( void );
#endif

extern void ShowConnections( char* w, char* i );

extern void exit_x48( int tell_x11 );

#if defined( GUI_IS_X11 )

#ifdef HAVE_XSHM
extern void refresh_display( void );
#endif

#elif defined( GUI_IS_SDL1 )

extern unsigned int ARGBColors[ BLACK + 1 ];

extern SDL_Surface* sdlwindow;
extern SDL_Surface* sdlsurface;

void SDLInit();
void SDLDrawAnnunc( char* annunc );
#define DISP_ROWS 64
#define NIBS_PER_BUFFER_ROW ( NIBBLES_PER_ROW + 2 )

void SDLCreateAnnunc();
// void SDLDrawLcd(unsigned char lcd_buffer[DISP_ROWS][NIBS_PER_BUFFER_ROW]);
// void SDLDrawLcd();
void SDLDrawNibble( int nx, int ny, int val );
void SDLDrawKeypad( void );
void SDLDrawButtons();
SDL_Surface* SDLCreateSurfFromData( unsigned int w, unsigned int h,
                                    unsigned char* data, unsigned int coloron,
                                    unsigned int coloroff );
SDL_Surface* SDLCreateARGBSurfFromData( unsigned int w, unsigned int h,
                                        unsigned char* data,
                                        unsigned int xpcolor );
// void SDLDrawSmallString(int x, int y, const char *string, unsigned int
// length);
void SDLDrawSmallString( int x, int y, const char* string, unsigned int length,
                         unsigned int coloron, unsigned int coloroff );
void SDLCreateColors();
void SDLDrawKeyLetter();
unsigned SDLBGRA2ARGB( unsigned color );
void SDLDrawBezel( unsigned int width, unsigned int height,
                   unsigned int offset_y, unsigned int offset_x );
void SDLDrawMore( unsigned int w, unsigned int h, unsigned int cut,
                  unsigned int offset_y, unsigned int offset_x,
                  int keypad_width, int keypad_height );
void SDLDrawLogo( unsigned int w, unsigned int h, unsigned int cut,
                  unsigned int offset_y, unsigned int offset_x,
                  int keypad_width, int keypad_height );
void SDLDrawBackground( int width, int height, int w_top, int h_top );
void SDLUIShowKey( int hpkey );
void SDLUIHideKey();
void SDLUIFeedback();
SDLWINDOW_t SDLCreateWindow( int x, int y, int w, int h, unsigned color,
                             int framewidth, int inverted );
void SDLShowWindow( SDLWINDOW_t* win );
void SDLSHideWindow( SDLWINDOW_t* win );
void SDLARGBTo( unsigned color, unsigned* a, unsigned* r, unsigned* g,
                unsigned* b );
unsigned SDLToARGB( unsigned a, unsigned r, unsigned g, unsigned b );
void SDLMessageBox( int w, int h, const char* title, const char* text[],
                    unsigned color, unsigned colortext, int center );
void SDLEventWaitClickOrKey();
void SDLShowInformation();

#endif

#endif /* !_X48_GUI_H */
