#ifndef _X48_GUI_H
#define _X48_GUI_H 1

#include <SDL/SDL.h>

#include "hp48.h"               /* word_4; word_20; */
#include "x48_bitmaps.h"

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

#define DISP_ROWS 64
#define NIBS_PER_BUFFER_ROW ( NIBBLES_PER_ROW + 2 )

#define UPDATE_MENU 1
#define UPDATE_DISP 2

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

typedef struct letter_t {
    unsigned int w, h;
    unsigned char* bits;
} letter_t;

typedef struct color_t {
    const char* name;
    int r, g, b;
} color_t;

typedef struct keypad_t {
    unsigned int width;
    unsigned int height;
} keypad_t;

typedef struct disp_t {
    unsigned int w, h;
    short mapped;
    int offset;
    int lines;
} disp_t;

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

    SDL_Surface* surfaceup;
    SDL_Surface* surfacedown;
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

    SDL_Surface* surfaceon;
    SDL_Surface* surfaceoff;
} ann_struct_t;

typedef struct SDLWINDOW {
    SDL_Surface *oldsurf, *surf;
    int x, y;
} SDLWINDOW_t;

/*************************/
/* Functions' prototypes */
/*************************/

void redraw_annunc( void );
void redraw_display( void );

void ShowConnections();
void SDLDrawAnnunc( char* annunc );
void SDLCreateAnnunc( void );
void SDLDrawNibble( int nx, int ny, int val );
void SDLDrawKeypad( void );
void SDLDrawButtons( void );
SDL_Surface* SDLCreateSurfFromData( unsigned int w, unsigned int h,
                                    unsigned char* data, unsigned int coloron,
                                    unsigned int coloroff );
SDL_Surface* SDLCreateARGBSurfFromData( unsigned int w, unsigned int h,
                                        unsigned char* data,
                                        unsigned int xpcolor );
void SDLDrawSmallString( int x, int y, const char* string, unsigned int length,
                         unsigned int coloron, unsigned int coloroff );
void SDLCreateColors( void );
void SDLDrawKeyLetter( void );
unsigned SDLBGRA2ARGB( unsigned color );
void SDLDrawBezel();
void SDLDrawMore( unsigned int cut, unsigned int offset_y, int keypad_width,
                  int keypad_height );
void SDLDrawLogo();
void SDLDrawBackground( int width, int height, int w_top, int h_top );
void SDLUIShowKey( int hpkey );
void SDLUIHideKey( void );
void SDLUIFeedback( void );
SDLWINDOW_t SDLCreateWindow( int x, int y, int w, int h, unsigned color,
                             int framewidth, int inverted );
void SDLShowWindow( SDLWINDOW_t* win );
void SDLSHideWindow( SDLWINDOW_t* win );
void SDLARGBTo( unsigned color, unsigned* a, unsigned* r, unsigned* g,
                unsigned* b );
unsigned SDLToARGB( unsigned a, unsigned r, unsigned g, unsigned b );
void SDLMessageBox( int w, int h, const char* title, const char* text[],
                    unsigned color, unsigned colortext, int center );
void SDLEventWaitClickOrKey( void );
void SDLShowInformation( void );

/**************/
/* public API */
/**************/
 /* used in: hp48emu_memory.c */
extern disp_t disp;
extern void disp_draw_nibble( word_20 addr, word_4 val );
extern void menu_draw_nibble( word_20 addr, word_4 val );

/* used in: hp48emu_actions.c, hp48_emulate.c */
extern int get_ui_event( void );
extern void update_display( void );

 /* used in: hp48_emulate.c */
extern void adjust_contrast();
extern void draw_annunc( void );

/* used in: main.c */
extern void SDLInit( void );
extern void SDLCreateHP( void );
extern void init_display( void );

#endif /* !_X48_GUI_H */
