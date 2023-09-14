#ifndef _X48_GUI_H
#define _X48_GUI_H 1

#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h> /* lineColor(); pixelColor(); rectangleColor();stringColor(); */

#include "hp48.h" /* word_20, word_4 */
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

static letter_t small_font[] = {
    { 0, 0, 0 },
    { nl_gx_width, nl_gx_height, nl_gx_bitmap },          /* \001 == \n gx */
    { comma_gx_width, comma_gx_height, comma_gx_bitmap }, /* \002 == comma gx */
    { arrow_gx_width, arrow_gx_height, arrow_gx_bitmap }, /* \003 == \-> gx */
    { equal_gx_width, equal_gx_height, equal_gx_bitmap }, /* \004 == equal gx */
    { pi_gx_width, pi_gx_height, pi_gx_bitmap },          /* \005 == pi gx */
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 }, /* # 16 */
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { blank_width, blank_height, blank_bitmap }, /* # 32 */
    { 0, 0, 0 },
    { 0, 0, 0 },
    { hash_width, hash_height, hash_bitmap },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { lbrace_width, lbrace_height, lbrace_bitmap },
    { rbrace_width, rbrace_height, rbrace_bitmap },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { comma_width, comma_height, comma_bitmap },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { slash_width, slash_height, slash_bitmap },
    { 0, 0, 0 }, /* # 48 */
    { 0, 0, 0 },
    { two_width, two_height, two_bitmap },
    { three_width, three_height, three_bitmap },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { small_colon_width, small_colon_height, small_colon_bitmap },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { equal_width, equal_height, equal_bitmap },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 }, /* # 64 */
    { A_width, A_height, A_bitmap },
    { B_width, B_height, B_bitmap },
    { C_width, C_height, C_bitmap },
    { D_width, D_height, D_bitmap },
    { E_width, E_height, E_bitmap },
    { F_width, F_height, F_bitmap },
    { G_width, G_height, G_bitmap },
    { H_width, H_height, H_bitmap },
    { I_width, I_height, I_bitmap },
    { J_width, J_height, J_bitmap },
    { K_width, K_height, K_bitmap },
    { L_width, L_height, L_bitmap },
    { M_width, M_height, M_bitmap },
    { N_width, N_height, N_bitmap },
    { O_width, O_height, O_bitmap },
    { P_width, P_height, P_bitmap }, /* # 80 */
    { Q_width, Q_height, Q_bitmap },
    { R_width, R_height, R_bitmap },
    { S_width, S_height, S_bitmap },
    { T_width, T_height, T_bitmap },
    { U_width, U_height, U_bitmap },
    { V_width, V_height, V_bitmap },
    { W_width, W_height, W_bitmap },
    { X_width, X_height, X_bitmap },
    { Y_width, Y_height, Y_bitmap },
    { Z_width, Z_height, Z_bitmap },
    { lbracket_width, lbracket_height, lbracket_bitmap },
    { 0, 0, 0 },
    { rbracket_width, rbracket_height, rbracket_bitmap },
    { 0, 0, 0 },
    { under_width, under_height, under_bitmap },
    { 0, 0, 0 },                                 /* # 96 */
    { arrow_width, arrow_height, arrow_bitmap }, /* a == left arrow   */
    { diff_width, diff_height, diff_bitmap },    /* b == differential */
    { integral_width, integral_height, integral_bitmap },    /* c == integral */
    { sigma_width, sigma_height, sigma_bitmap },             /* d == sigma */
    { sqr_width, sqr_height, sqr_bitmap },                   /* e == sqr */
    { root_width, root_height, root_bitmap },                /* f == root */
    { pow10_width, pow10_height, pow10_bitmap },             /* g == pow10 */
    { exp_width, exp_height, exp_bitmap },                   /* h == exp */
    { prog_width, prog_height, prog_bitmap },                /* i == << >> */
    { string_width, string_height, string_bitmap },          /* j == " " */
    { nl_width, nl_height, nl_bitmap },                      /* k == New Line */
    { pi_width, pi_height, pi_bitmap },                      /* l == pi */
    { angle_width, angle_height, angle_bitmap },             /* m == angle */
    { sqr_gx_width, sqr_gx_height, sqr_gx_bitmap },          /* n == sqr gx */
    { root_gx_width, root_gx_height, root_gx_bitmap },       /* o == root gx */
    { pow10_gx_width, pow10_gx_height, pow10_gx_bitmap },    /* p == pow10 gx */
    { exp_gx_width, exp_gx_height, exp_gx_bitmap },          /* q == exp gx */
    { parens_gx_width, parens_gx_height, parens_gx_bitmap }, /* r == ( ) gx */
    { hash_gx_width, hash_gx_height, hash_gx_bitmap },       /* s == # gx */
    { bracket_gx_width, bracket_gx_height, bracket_gx_bitmap }, /* t == [] gx */
    { under_gx_width, under_gx_height, under_gx_bitmap },       /* u == _ gx */
    { prog_gx_width, prog_gx_height, prog_gx_bitmap },    /* v == << >> gx */
    { quote_gx_width, quote_gx_height, quote_gx_bitmap }, /* w == " " gx */
    { curly_gx_width, curly_gx_height, curly_gx_bitmap }, /* x == {} gx */
    { colon_gx_width, colon_gx_height, colon_gx_bitmap }, /* y == :: gx */
    { angle_gx_width, angle_gx_height, angle_gx_bitmap }, /* z == angle gx */
    { lcurly_width, lcurly_height, lcurly_bitmap },
    { 0, 0, 0 },
    { rcurly_width, rcurly_height, rcurly_bitmap },
    { 0, 0, 0 },
    { 0, 0, 0 } };

/* #endif /\* !_SMALL_H *\/ */

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

extern color_t* colors;
extern disp_t disp; /* PUBLIC API( hp48emu_memory.c ) */
extern ann_struct_t ann_tbl[];

extern unsigned int ARGBColors[ BLACK + 1 ];
extern SDL_Surface* sdlwindow;

/*************************/
/* Functions' prototypes */
/*************************/

/* PUBLIC API( hp48emu_actions.c, hp48_emulate.c ) */
extern int get_ui_event( void );
/* PUBLIC API( hp48_emulate.c ) */
extern void adjust_contrast();

void ShowConnections();

/* #ifndef _DEVICE_H */
/* #define _DEVICE_H 1 */
/* PUBLIC API( main.c ) */
extern void init_display( void );
/* PUBLIC API( hp48emu_actions.c, hp48_emulate.c ) */
extern void update_display( void );
void redraw_display( void );
/* PUBLIC API( hp48emu_memory.c ) */
extern void disp_draw_nibble( word_20 addr, word_4 val );
/* PUBLIC API( hp48emu_memory.c ) */
extern void menu_draw_nibble( word_20 addr, word_4 val );
/* PUBLIC API( hp48_emulate.c ) */
extern void draw_annunc( void );
void redraw_annunc( void );
/* #endif /\* !_DEVICE_H *\/ */

/* PUBLIC API( main.c ) */
extern void SDLCreateHP( void );
/* PUBLIC API( main.c ) */
extern void SDLInit( void );
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

#endif /* !_X48_GUI_H */
