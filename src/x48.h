#ifndef _X48_GUI_H
#define _X48_GUI_H 1

#include <SDL/SDL.h>

#include "hp48.h" /* word_4; word_20; */

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
typedef struct disp_t {
    unsigned int w, h;
    short mapped;
    int offset;
    int lines;
} disp_t;
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
