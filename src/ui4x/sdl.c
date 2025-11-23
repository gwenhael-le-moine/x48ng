#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAS_SDL2
#  include <SDL2/SDL.h>

#  define SDL_DestroySurface SDL_FreeSurface
#  define SDL_RenderPoint SDL_RenderDrawPoint
#  define SDL_RenderLine SDL_RenderDrawLine
#  define SDL_FRect SDL_Rect
#  define SDL_RenderTexture SDL_RenderCopy

#  define SDLK_A SDLK_a
#  define SDLK_B SDLK_b
#  define SDLK_C SDLK_c
#  define SDLK_D SDLK_d
#  define SDLK_E SDLK_e
#  define SDLK_F SDLK_f
#  define SDLK_G SDLK_g
#  define SDLK_H SDLK_h
#  define SDLK_I SDLK_i
#  define SDLK_J SDLK_j
#  define SDLK_K SDLK_k
#  define SDLK_L SDLK_l
#  define SDLK_M SDLK_m
#  define SDLK_N SDLK_n
#  define SDLK_O SDLK_o
#  define SDLK_P SDLK_p
#  define SDLK_Q SDLK_q
#  define SDLK_R SDLK_r
#  define SDLK_S SDLK_s
#  define SDLK_T SDLK_t
#  define SDLK_U SDLK_u
#  define SDLK_V SDLK_v
#  define SDLK_W SDLK_w
#  define SDLK_X SDLK_x
#  define SDLK_Y SDLK_y
#  define SDLK_Z SDLK_z

#  define SDL_EVENT_QUIT SDL_QUIT
#  define SDL_EVENT_MOUSE_BUTTON_DOWN SDL_MOUSEBUTTONDOWN
#  define SDL_EVENT_MOUSE_BUTTON_UP SDL_MOUSEBUTTONUP
#  define SDL_EVENT_KEY_DOWN SDL_KEYDOWN
#  define SDL_EVENT_KEY_UP SDL_KEYUP

#  define SDL_WINDOW_HIGH_PIXEL_DENSITY SDL_WINDOW_ALLOW_HIGHDPI
#  define SDL_WINDOW_FULLSCREEN SDL_WINDOW_FULLSCREEN_DESKTOP

#  define SDL_SetTextureScaleMode( t, m )
#  define SDL_Init !SDL_Init

#  define CREATE_SURFACE( w, h, a, b, c, d, e ) SDL_CreateRGBSurface( SDL_SWSURFACE, w, h, a, b, c, d, e )
#  define EVENT_KEY( e ) e.key.keysym.sym
#  define CREATE_WINDOW( n, w, h, flags ) SDL_CreateWindow( n, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, flags )
#  define CREATE_RENDERER( w ) SDL_CreateRenderer( w, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE )
#  define SET_RENDER_LOGICAL_PRESENTATION( r, w, h ) SDL_RenderSetLogicalSize( r, w, h )
#else
#  include <SDL3/SDL.h>

#  define CREATE_SURFACE( w, h, a, b, c, d, e ) SDL_CreateSurface( w, h, SDL_GetPixelFormatForMasks( a, b, c, d, e ) )
#  define EVENT_KEY( e ) e.key.key
#  define CREATE_WINDOW( n, w, h, flags ) SDL_CreateWindow( n, w, h, flags )
#  define CREATE_RENDERER( w ) SDL_CreateRenderer( w, NULL )
#  define SET_RENDER_LOGICAL_PRESENTATION( r, w, h ) SDL_SetRenderLogicalPresentation( r, w, h, SDL_LOGICAL_PRESENTATION_LETTERBOX )
#endif

#include "api.h"
#include "inner.h"

#define COLORS                                                                                                                             \
    ( ui4x_config.model == MODEL_48GX                                                                                                      \
          ? colors_48gx                                                                                                                    \
          : ( ui4x_config.model == MODEL_48SX                                                                                              \
                  ? colors_48sx                                                                                                            \
                  : ( ui4x_config.model == MODEL_49G ? colors_49g : ( ui4x_config.model == MODEL_40G ? colors_40g : colors_50g ) ) ) )

#define COLOR_LCD_BG ( ui4x_config.black_lcd ? UI4X_COLOR_BLACK_LCD_BG : UI4X_COLOR_LCD_BG )
#define COLOR_PIXEL_ON ( ui4x_config.black_lcd ? UI4X_COLOR_BLACK_PIXEL_ON : UI4X_COLOR_PIXEL_ON )

#define PADDING_TOP ( ( ui4x_config.model == MODEL_49G || ui4x_config.model == MODEL_50G ) ? 48 : 65 )
#define PADDING_SIDE 20
#define PADDING_BOTTOM 25
#define PADDING_BETWEEN_DISPLAY_AND_KEYBOARD ( ( ui4x_config.model == MODEL_49G || ui4x_config.model == MODEL_50G ) ? 32 : 65 )
#define OFFSET_Y_48GSX_INDENT_BELOW_MENU_KEYS 25

#define WIDTH_DISPLAY ( ( ( LCD_WIDTH + 1 ) * 2 ) + 8 )
#define HEIGHT_DISPLAY ( ( LCD_HEIGHT * 2 ) + 16 + 8 )
#define OFFSET_X_DISPLAY ( ui4x_config.chromeless ? 0 : ( PADDING_SIDE + ( 286 - WIDTH_DISPLAY ) / 2 ) )
#define OFFSET_Y_DISPLAY ( ui4x_config.chromeless ? 0 : PADDING_TOP )

#define BEZEL_WIDTH_DISPLAY 8

#define OFFSET_X_KEYBOARD PADDING_SIDE
#define OFFSET_Y_KEYBOARD ( PADDING_TOP + HEIGHT_DISPLAY + PADDING_BETWEEN_DISPLAY_AND_KEYBOARD )

#define N_LEVELS_OF_GRAY ( ( ui4x_config.model == MODEL_50G ) ? 16 : 4 )

/***********/
/* typedef */
/***********/
typedef struct on_off_sdl_textures_struct_t {
    SDL_Texture* up;
    SDL_Texture* down;
} on_off_sdl_textures_struct_t;

typedef struct annunciators_ui_t {
    int x;
    int y;
    unsigned int width;
    unsigned int height;
    unsigned char* bits;
} annunciators_ui_t;

/*************/
/* variables */
/*************/
static annunciators_ui_t annunciators_ui[ NB_ANNUNCIATORS ] = {
    {.x = 16,  .y = 4, .width = ann_left_width,    .height = ann_left_height,    .bits = ann_left_bitmap   },
    {.x = 61,  .y = 4, .width = ann_right_width,   .height = ann_right_height,   .bits = ann_right_bitmap  },
    {.x = 106, .y = 4, .width = ann_alpha_width,   .height = ann_alpha_height,   .bits = ann_alpha_bitmap  },
    {.x = 151, .y = 4, .width = ann_battery_width, .height = ann_battery_height, .bits = ann_battery_bitmap},
    {.x = 196, .y = 4, .width = ann_busy_width,    .height = ann_busy_height,    .bits = ann_busy_bitmap   },
    {.x = 241, .y = 4, .width = ann_io_width,      .height = ann_io_height,      .bits = ann_io_bitmap     },
};

static int display_buffer_grayscale[ LCD_WIDTH * 80 ];
static int last_annunciators = -1;
static int last_contrast = -1;

static color_t colors[ NB_COLORS ];
static color_t pixel_colors[ 16 ]; /* up to 16 shades */

static on_off_sdl_textures_struct_t buttons_textures[ NB_HP4950_KEYS ];
static on_off_sdl_textures_struct_t annunciators_textures[ NB_ANNUNCIATORS ];

static SDL_Window* window;
static SDL_Renderer* renderer;
static SDL_Texture* main_texture;
static SDL_Texture* display_texture;

/****************************/
/* functions implementation */
/****************************/

int SmallTextWidth( const char* string, unsigned int length )
{
    int w = 0;
    for ( unsigned int i = 0; i < length; i++ ) {
        if ( small_font[ ( int )string[ i ] ].h != 0 )
            w += small_font[ ( int )string[ i ] ].w + 1;
        else
            w += 5;
    }

    return w;
}

int BigTextWidth( const char* string, unsigned int length )
{
    int w = 0;
    for ( unsigned int i = 0; i < length; i++ ) {
        if ( big_font[ ( int )string[ i ] ].h != 0 )
            w += big_font[ ( int )string[ i ] ].w;
        else
            w += 7;
    }

    return w;
}

static inline unsigned color2bgra( int color )
{
    return 0xff000000 | ( colors[ color ].r << 16 ) | ( colors[ color ].g << 8 ) | colors[ color ].b;
}

/*
        Create a SDL_Texture from binary bitmap data
*/
static SDL_Texture* bitmap_to_texture( unsigned int w, unsigned int h, unsigned char* data, int color_fg, int color_bg )
{
    SDL_Surface* surf = CREATE_SURFACE( w, h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 );

    SDL_LockSurface( surf );

    unsigned char* pixels = ( unsigned char* )surf->pixels;
    unsigned int pitch = surf->pitch;
    unsigned byteperline = w / 8;
    if ( byteperline * 8 != w )
        byteperline++;

    for ( unsigned int y = 0; y < h; y++ ) {
        unsigned int* lineptr = ( unsigned int* )( pixels + y * pitch );
        for ( unsigned int x = 0; x < w; x++ ) {
            // Address the correct byte
            char c = data[ y * byteperline + ( x >> 3 ) ];
            // Look for the bit in that byte
            char b = c & ( 1 << ( x & 7 ) );

            lineptr[ x ] = color2bgra( b ? color_fg : color_bg );
        }
    }

    SDL_UnlockSurface( surf );

    SDL_Texture* tex = SDL_CreateTextureFromSurface( renderer, surf );
    SDL_DestroySurface( surf );

    return tex;
}

static void __draw_pixel_rgba( int x, int y, int r, int g, int b, int a )
{
    SDL_SetRenderDrawColor( renderer, r, g, b, a );
    SDL_RenderPoint( renderer, x, y );
}

static void __draw_pixel( int x, int y, int color )
{
    __draw_pixel_rgba( x, y, colors[ color ].r, colors[ color ].g, colors[ color ].b, colors[ color ].a );
}

static void __draw_line( int x1, int y1, int x2, int y2, int color )
{
    SDL_SetRenderDrawColor( renderer, colors[ color ].r, colors[ color ].g, colors[ color ].b, colors[ color ].a );
    SDL_RenderLine( renderer, x1, y1, x2, y2 );
}

static void __draw_rect( int x, int y, int w, int h, int color )
{
    SDL_FRect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;

    SDL_SetRenderDrawColor( renderer, colors[ color ].r, colors[ color ].g, colors[ color ].b, colors[ color ].a );
    SDL_RenderFillRect( renderer, &rect );
}

static void __draw_texture( int x, int y, unsigned int w, unsigned int h, SDL_Texture* texture )
{
    SDL_FRect drect;
    drect.x = x;
    drect.y = y;
    drect.w = w;
    drect.h = h;

    SDL_RenderTexture( renderer, texture, NULL, &drect );
}

static void __draw_bitmap( int x, int y, unsigned int w, unsigned int h, unsigned char* data, int color_fg, int color_bg )
{
    __draw_texture( x, y, w, h, bitmap_to_texture( w, h, data, color_fg, color_bg ) );
}

static void write_with_small_font( int x, int y, const char* string, int color_fg, int color_bg )
{
    int c;
    for ( unsigned int i = 0; i < strlen( string ); i++ ) {
        c = ( int )string[ i ];
        if ( small_font[ c ].h != 0 )
            __draw_bitmap( x - 1, ( int )( y - small_font[ c ].h ), small_font[ c ].w, small_font[ c ].h, small_font[ c ].bits, color_fg,
                           color_bg );

        x += SmallTextWidth( &string[ i ], 1 );
    }
}

static void write_with_big_font( int x, int y, const char* string, int color_fg, int color_bg )
{
    int c;
    for ( unsigned int i = 0; i < strlen( string ); i++ ) {
        c = ( int )string[ i ];
        if ( big_font[ c ].h != 0 )
            __draw_bitmap( x, y + ( big_font[ c ].h > 10 ? 0 : 2 ), big_font[ c ].w, big_font[ c ].h, big_font[ c ].bits, color_fg,
                           color_bg );

        x += BigTextWidth( &string[ i ], 1 ) - 1;
    }
}

// This should be called once to setup the surfaces. Calling it multiple
// times is fine, it won't do anything on subsequent calls.
static void create_annunciators_textures( void )
{
    for ( int i = 0; i < NB_ANNUNCIATORS; i++ ) {
        annunciators_textures[ i ].up = bitmap_to_texture( annunciators_ui[ i ].width, annunciators_ui[ i ].height,
                                                           annunciators_ui[ i ].bits, UI4X_COLOR_ANNUNCIATOR, COLOR_LCD_BG );
        annunciators_textures[ i ].down = bitmap_to_texture( annunciators_ui[ i ].width, annunciators_ui[ i ].height,
                                                             annunciators_ui[ i ].bits, COLOR_LCD_BG, COLOR_LCD_BG );
    }
}

// Find which key is pressed, if any.
// Returns -1 is no key is pressed
static int mouse_click_to_hpkey( int x, int y )
{
    x /= ui4x_config.zoom;
    y /= ui4x_config.zoom;

    /* return immediatly if the click isn't even in the keyboard area */
    if ( y < OFFSET_Y_KEYBOARD )
        return -1;

    x -= OFFSET_X_KEYBOARD;
    y -= OFFSET_Y_KEYBOARD;

    for ( int i = 0; i < NB_KEYS; i++ )
        if ( ( BUTTONS[ i ].x < x && x < ( BUTTONS[ i ].x + BUTTONS[ i ].w ) ) &&
             ( BUTTONS[ i ].y < y && y < ( BUTTONS[ i ].y + BUTTONS[ i ].h ) ) )
            return i;

    return -1;
}

// Map the keyboard keys to the HP keys
// Returns -1 if there is no mapping
static int sdlkey_to_hpkey( SDL_Keycode k )
{
    switch ( k ) {
        case SDLK_0:
        case SDLK_KP_0:
            return UI4X_KEY_0;
        case SDLK_1:
        case SDLK_KP_1:
            return UI4X_KEY_1;
        case SDLK_2:
        case SDLK_KP_2:
            return UI4X_KEY_2;
        case SDLK_3:
        case SDLK_KP_3:
            return UI4X_KEY_3;
        case SDLK_4:
        case SDLK_KP_4:
            return UI4X_KEY_4;
        case SDLK_5:
        case SDLK_KP_5:
            return UI4X_KEY_5;
        case SDLK_6:
        case SDLK_KP_6:
            return UI4X_KEY_6;
        case SDLK_7:
        case SDLK_KP_7:
            return UI4X_KEY_7;
        case SDLK_8:
        case SDLK_KP_8:
            return UI4X_KEY_8;
        case SDLK_9:
        case SDLK_KP_9:
            return UI4X_KEY_9;
        case SDLK_A:
            return UI4X_KEY_A;
        case SDLK_B:
            return UI4X_KEY_B;
        case SDLK_C:
            return UI4X_KEY_C;
        case SDLK_D:
            return UI4X_KEY_D;
        case SDLK_E:
            return UI4X_KEY_E;
        case SDLK_F:
            return UI4X_KEY_F;
        case SDLK_G:
            return UI4X_KEY_G;
        case SDLK_H:
            return UI4X_KEY_H;
        case SDLK_I:
            return UI4X_KEY_I;
        case SDLK_J:
            return UI4X_KEY_J;
        case SDLK_K:
            return UI4X_KEY_K;
        case SDLK_UP:
            return UI4X_KEY_UP;
        case SDLK_L:
            return UI4X_KEY_L;
        case SDLK_M:
            return UI4X_KEY_M;
        case SDLK_N:
            return UI4X_KEY_N;
        case SDLK_O:
            return UI4X_KEY_O;
        case SDLK_P:
            return UI4X_KEY_P;
        case SDLK_LEFT:
            return UI4X_KEY_LEFT;
        case SDLK_Q:
            return UI4X_KEY_Q;
        case SDLK_DOWN:
            return UI4X_KEY_DOWN;
        case SDLK_R:
            return UI4X_KEY_R;
        case SDLK_RIGHT:
            return UI4X_KEY_RIGHT;
        case SDLK_S:
            return UI4X_KEY_S;
        case SDLK_T:
            return UI4X_KEY_T;
        case SDLK_U:
            return UI4X_KEY_U;
        case SDLK_V:
            return UI4X_KEY_V;
        case SDLK_W:
            return UI4X_KEY_W;
        case SDLK_X:
            return UI4X_KEY_X;
        case SDLK_Y:
            return UI4X_KEY_Y;
        case SDLK_Z:
            return UI4X_KEY_Z;
        case SDLK_SPACE:
            return UI4X_KEY_SPACE;
        case SDLK_F1:
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            return UI4X_KEY_ENTER;
        case SDLK_BACKSPACE:
            return UI4X_KEY_BACKSPACE;
        case SDLK_DELETE:
            return UI4X_KEY_DELETE;
        case SDLK_PERIOD:
        case SDLK_KP_PERIOD:
            return UI4X_KEY_PERIOD;
        case SDLK_PLUS:
        case SDLK_KP_PLUS:
        case SDLK_EQUALS:
            return UI4X_KEY_PLUS;
        case SDLK_MINUS:
        case SDLK_KP_MINUS:
            return UI4X_KEY_MINUS;
        case SDLK_ASTERISK:
        case SDLK_KP_MULTIPLY:
            return UI4X_KEY_MULTIPLY;
        case SDLK_SLASH:
        case SDLK_KP_DIVIDE:
            return UI4X_KEY_Z;
        case SDLK_F5:
        case SDLK_ESCAPE:
            return UI4X_KEY_ON;
        case SDLK_LSHIFT:
            if ( !ui4x_config.shiftless )
                return UI4X_KEY_LSHIFT;
            break;
        case SDLK_RSHIFT:
            if ( !ui4x_config.shiftless )
                return UI4X_KEY_RSHIFT;
            break;
        case SDLK_F2:
        case SDLK_RCTRL:
            return UI4X_KEY_LSHIFT;
        case SDLK_F3:
        case SDLK_LCTRL:
            return UI4X_KEY_RSHIFT;
        case SDLK_F4:
        case SDLK_LALT:
        case SDLK_RALT:
            return UI4X_KEY_ALPHA;

        case SDLK_F7:
        case SDLK_F10:
            ui4x_emulator_api.do_stop();
            return -1;
        case SDLK_F12:
            if ( ui4x_emulator_api.do_reset != NULL )
                ui4x_emulator_api.do_reset();
            return -1;

        default:
            return -1;
    }

    return -1;
}

static void _draw_bezel( unsigned int cut, unsigned int offset_y, int keypad_width, int keypad_height )
{
    // bottom lines
    __draw_line( 1, keypad_height - 1, keypad_width - 1, keypad_height - 1, UI4X_COLOR_FACEPLATE_EDGE_TOP );
    __draw_line( 2, keypad_height - 2, keypad_width - 2, keypad_height - 2, UI4X_COLOR_FACEPLATE_EDGE_TOP );

    // right lines
    __draw_line( keypad_width - 1, keypad_height - 1, keypad_width - 1, cut, UI4X_COLOR_FACEPLATE_EDGE_TOP );
    __draw_line( keypad_width - 2, keypad_height - 2, keypad_width - 2, cut, UI4X_COLOR_FACEPLATE_EDGE_TOP );

    // right lines
    __draw_line( keypad_width - 1, cut - 1, keypad_width - 1, 1, UI4X_COLOR_UPPER_FACEPLATE_EDGE_TOP );
    __draw_line( keypad_width - 2, cut - 1, keypad_width - 2, 2, UI4X_COLOR_UPPER_FACEPLATE_EDGE_TOP );

    // top lines
    __draw_line( 0, 0, keypad_width - 2, 0, UI4X_COLOR_UPPER_FACEPLATE_EDGE_BOTTOM );
    __draw_line( 1, 1, keypad_width - 3, 1, UI4X_COLOR_UPPER_FACEPLATE_EDGE_BOTTOM );

    // left lines
    __draw_line( 0, cut - 1, 0, 0, UI4X_COLOR_UPPER_FACEPLATE_EDGE_BOTTOM );
    __draw_line( 1, cut - 1, 1, 1, UI4X_COLOR_UPPER_FACEPLATE_EDGE_BOTTOM );

    // left lines
    __draw_line( 0, keypad_height - 2, 0, cut, UI4X_COLOR_FACEPLATE_EDGE_BOTTOM );
    __draw_line( 1, keypad_height - 3, 1, cut, UI4X_COLOR_FACEPLATE_EDGE_BOTTOM );

    if ( ui4x_config.model == MODEL_48GX || ui4x_config.model == MODEL_48SX ) {
        // lower the menu BUTTONS

        // bottom lines
        __draw_line( 3, keypad_height - 3, keypad_width - 3, keypad_height - 3, UI4X_COLOR_FACEPLATE_EDGE_TOP );
        __draw_line( 4, keypad_height - 4, keypad_width - 4, keypad_height - 4, UI4X_COLOR_FACEPLATE_EDGE_TOP );

        // right lines
        __draw_line( keypad_width - 3, keypad_height - 3, keypad_width - 3, cut, UI4X_COLOR_FACEPLATE_EDGE_TOP );
        __draw_line( keypad_width - 4, keypad_height - 4, keypad_width - 4, cut, UI4X_COLOR_FACEPLATE_EDGE_TOP );

        // right lines
        __draw_line( keypad_width - 3, cut - 1, keypad_width - 3, offset_y - ( OFFSET_Y_48GSX_INDENT_BELOW_MENU_KEYS - 1 ),
                     UI4X_COLOR_UPPER_FACEPLATE_EDGE_TOP );
        __draw_line( keypad_width - 4, cut - 1, keypad_width - 4, offset_y - ( OFFSET_Y_48GSX_INDENT_BELOW_MENU_KEYS - 2 ),
                     UI4X_COLOR_UPPER_FACEPLATE_EDGE_TOP );

        // top lines
        __draw_line( 2, offset_y - OFFSET_Y_48GSX_INDENT_BELOW_MENU_KEYS, keypad_width - 4,
                     offset_y - OFFSET_Y_48GSX_INDENT_BELOW_MENU_KEYS, UI4X_COLOR_UPPER_FACEPLATE_EDGE_BOTTOM );
        __draw_line( 3, offset_y - ( OFFSET_Y_48GSX_INDENT_BELOW_MENU_KEYS - 1 ), keypad_width - 5,
                     offset_y - ( OFFSET_Y_48GSX_INDENT_BELOW_MENU_KEYS - 1 ), UI4X_COLOR_UPPER_FACEPLATE_EDGE_BOTTOM );

        // left lines
        __draw_line( 2, cut - 1, 2, offset_y - ( OFFSET_Y_48GSX_INDENT_BELOW_MENU_KEYS - 1 ), UI4X_COLOR_UPPER_FACEPLATE_EDGE_BOTTOM );
        __draw_line( 3, cut - 1, 3, offset_y - ( OFFSET_Y_48GSX_INDENT_BELOW_MENU_KEYS - 2 ), UI4X_COLOR_UPPER_FACEPLATE_EDGE_BOTTOM );

        // left lines
        __draw_line( 2, keypad_height - 4, 2, cut, UI4X_COLOR_FACEPLATE_EDGE_BOTTOM );
        __draw_line( 3, keypad_height - 5, 3, cut, UI4X_COLOR_FACEPLATE_EDGE_BOTTOM );

        // lower the keyboard

        // bottom lines
        __draw_line( 5, keypad_height - 5, keypad_width - 3, keypad_height - 5, UI4X_COLOR_FACEPLATE_EDGE_TOP );
        __draw_line( 6, keypad_height - 6, keypad_width - 4, keypad_height - 6, UI4X_COLOR_FACEPLATE_EDGE_TOP );

        // right lines
        __draw_line( keypad_width - 5, keypad_height - 5, keypad_width - 5, cut + 1, UI4X_COLOR_FACEPLATE_EDGE_TOP );
        __draw_line( keypad_width - 6, keypad_height - 6, keypad_width - 6, cut + 2, UI4X_COLOR_FACEPLATE_EDGE_TOP );

        // top lines
        __draw_line( 4, cut, keypad_width - 6, cut, UI4X_COLOR_UPPER_FACEPLATE_EDGE_BOTTOM );
        __draw_line( 5, cut + 1, keypad_width - 7, cut + 1, UI4X_COLOR_UPPER_FACEPLATE_EDGE_BOTTOM );

        // left lines
        __draw_line( 4, keypad_height - 6, 4, cut + 1, UI4X_COLOR_FACEPLATE_EDGE_BOTTOM );
        __draw_line( 5, keypad_height - 7, 5, cut + 2, UI4X_COLOR_FACEPLATE_EDGE_BOTTOM );

        // round off the bottom edge
        __draw_line( keypad_width - 7, keypad_height - 7, keypad_width - 7, keypad_height - 14, UI4X_COLOR_FACEPLATE_EDGE_TOP );
        __draw_line( keypad_width - 8, keypad_height - 8, keypad_width - 8, keypad_height - 11, UI4X_COLOR_FACEPLATE_EDGE_TOP );
        __draw_line( keypad_width - 7, keypad_height - 7, keypad_width - 14, keypad_height - 7, UI4X_COLOR_FACEPLATE_EDGE_TOP );
        __draw_line( keypad_width - 7, keypad_height - 8, keypad_width - 11, keypad_height - 8, UI4X_COLOR_FACEPLATE_EDGE_TOP );
        __draw_pixel( keypad_width - 9, keypad_height - 9, UI4X_COLOR_FACEPLATE_EDGE_TOP );

        __draw_line( 7, keypad_height - 7, 13, keypad_height - 7, UI4X_COLOR_FACEPLATE_EDGE_TOP );
        __draw_line( 8, keypad_height - 8, 10, keypad_height - 8, UI4X_COLOR_FACEPLATE_EDGE_TOP );

        __draw_line( 6, keypad_height - 8, 6, keypad_height - 14, UI4X_COLOR_FACEPLATE_EDGE_BOTTOM );
        __draw_line( 7, keypad_height - 9, 7, keypad_height - 11, UI4X_COLOR_FACEPLATE_EDGE_BOTTOM );
    }
}

static void _draw_header( void )
{
    int x = OFFSET_X_DISPLAY;
    int y;

    // insert the HP Logo
    if ( ui4x_config.model == MODEL_48GX )
        x -= 6;

    __draw_bitmap( x, 10, hp_width, hp_height, hp_bitmap, UI4X_COLOR_HP_LOGO, UI4X_COLOR_HP_LOGO_BG );

    switch ( ui4x_config.model ) {
        case MODEL_49G:
            write_with_big_font( WIDTH_DISPLAY - PADDING_SIDE, 10, "HP 49G", UI4X_COLOR_HP_LOGO, UI4X_COLOR_UPPER_FACEPLATE );
            break;

        case MODEL_50G:
            write_with_big_font( WIDTH_DISPLAY - PADDING_SIDE, 10, "HP 50G", UI4X_COLOR_HP_LOGO, UI4X_COLOR_UPPER_FACEPLATE );
            break;

        case MODEL_48GX:
            x = OFFSET_X_DISPLAY + WIDTH_DISPLAY - gx_128K_ram_width + gx_128K_ram_x_hot + 2;
            y = 10 + gx_128K_ram_y_hot;
            __draw_bitmap( x, y, gx_128K_ram_width, gx_128K_ram_height, gx_128K_ram_bitmap, UI4X_COLOR_48GX_128K_RAM,
                           UI4X_COLOR_UPPER_FACEPLATE );

            x = OFFSET_X_DISPLAY + hp_width;
            y = hp_height + 8 - hp48gx_height;
            __draw_bitmap( x, y, hp48gx_width, hp48gx_height, hp48gx_bitmap, UI4X_COLOR_HP_LOGO, UI4X_COLOR_UPPER_FACEPLATE );

            x = OFFSET_X_DISPLAY + WIDTH_DISPLAY - gx_128K_ram_width + gx_green_x_hot + 2;
            y = 10 + gx_green_y_hot;
            __draw_bitmap( x, y, gx_green_width, gx_green_height, gx_green_bitmap, UI4X_COLOR_RIGHTSHIFT, UI4X_COLOR_UPPER_FACEPLATE );

            x = OFFSET_X_DISPLAY + WIDTH_DISPLAY - gx_128K_ram_width + gx_silver_x_hot + 2;
            y = 10 + gx_silver_y_hot;
            __draw_bitmap( x, y, gx_silver_width, gx_silver_height, gx_silver_bitmap, UI4X_COLOR_HP_LOGO,
                           UI4X_COLOR_LABEL ); // Background transparent: draw only silver line
            break;

        case MODEL_48SX:
            __draw_line( OFFSET_X_DISPLAY, 9, OFFSET_X_DISPLAY + hp_width - 1, 9, UI4X_COLOR_FRAME );
            __draw_line( OFFSET_X_DISPLAY - 1, 10, OFFSET_X_DISPLAY - 1, 10 + hp_height - 1, UI4X_COLOR_FRAME );
            __draw_line( OFFSET_X_DISPLAY, 10 + hp_height, OFFSET_X_DISPLAY + hp_width - 1, 10 + hp_height, UI4X_COLOR_FRAME );
            __draw_line( OFFSET_X_DISPLAY + hp_width, 10, OFFSET_X_DISPLAY + hp_width, 10 + hp_height - 1, UI4X_COLOR_FRAME );

            x = OFFSET_X_DISPLAY;
            y = PADDING_TOP - BEZEL_WIDTH_DISPLAY - hp48sx_height - 3;
            __draw_bitmap( x, y, hp48sx_width, hp48sx_height, hp48sx_bitmap, UI4X_COLOR_HP_LOGO, UI4X_COLOR_UPPER_FACEPLATE );

            x = OFFSET_X_DISPLAY + WIDTH_DISPLAY - 1 - science_width;
            y = PADDING_TOP - BEZEL_WIDTH_DISPLAY - science_height - 4;
            __draw_bitmap( x, y, science_width, science_height, science_bitmap, UI4X_COLOR_HP_LOGO, UI4X_COLOR_UPPER_FACEPLATE );
            break;

        case MODEL_40G:
            write_with_big_font( WIDTH_DISPLAY - PADDING_SIDE, 10, "HP 40G", UI4X_COLOR_HP_LOGO, UI4X_COLOR_UPPER_FACEPLATE );
            break;
    }
}

static SDL_Texture* create_button_texture( int hpkey, bool is_up )
{
    bool is_down = !is_up;
    int x, y;
    int on_key_offset_y = ( hpkey == UI4X_KEY_ON ) ? 1 : 0;
    SDL_Texture* texture =
        SDL_CreateTexture( renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, BUTTONS[ hpkey ].w, BUTTONS[ hpkey ].h );
    SDL_SetRenderTarget( renderer, texture );

    // Fill the button and outline
    // fix outer-corners color
    int outer_color = UI4X_COLOR_FACEPLATE;
    if ( BUTTONS[ hpkey ].highlight )
        outer_color = UI4X_COLOR_KEYPAD_HIGHLIGHT;
    if ( ( ui4x_config.model == MODEL_48GX || ui4x_config.model == MODEL_48SX ) && hpkey < HP48_KEY_G )
        outer_color = UI4X_COLOR_UPPER_FACEPLATE;
    int inner_color = UI4X_COLOR_BUTTON;
    int edge_top_color = UI4X_COLOR_BUTTON_EDGE_TOP;
    int edge_bottom_color = UI4X_COLOR_BUTTON_EDGE_BOTTOM;
    if ( ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ) &&
         ( hpkey == HP4950_KEY_ALPHA || hpkey == HP4950_KEY_LEFTSHIFT || hpkey == HP4950_KEY_RIGHTSHIFT ) ) {
        // inner_color = BUTTONS[ hpkey ].label_color;
        edge_top_color = BUTTONS[ hpkey ].label_color;
        edge_bottom_color = BUTTONS[ hpkey ].label_color;
    }

    __draw_rect( 0, 0, BUTTONS[ hpkey ].w, BUTTONS[ hpkey ].h, outer_color );
    __draw_rect( 1, 1, BUTTONS[ hpkey ].w - 2, BUTTONS[ hpkey ].h - 2, inner_color );

    // draw label in button
    int label_color = BUTTONS[ hpkey ].label_color;
    /* if ( ui4x_config.model == MODEL_49G && ( hpkey == HP49_KEY_ALPHA || hpkey == HP49_KEY_SHL || hpkey == HP49_KEY_SHR ) ) */
    /*     label_color = UI4X_COLOR_LABEL; */
    if ( BUTTONS[ hpkey ].label_graphic != NULL ) {
        /* Button has a texture */
        x = ( 1 + BUTTONS[ hpkey ].w - BUTTONS[ hpkey ].label_graphic_w ) / 2;
        y = ( 1 + BUTTONS[ hpkey ].h - BUTTONS[ hpkey ].label_graphic_h ) / 2;
        if ( is_up )
            y += 1;

        __draw_bitmap( x, y, BUTTONS[ hpkey ].label_graphic_w, BUTTONS[ hpkey ].label_graphic_h, BUTTONS[ hpkey ].label_graphic,
                       label_color, inner_color );
    } else if ( BUTTONS[ hpkey ].label_sdl != NULL ) {
        /* Button has a text label */
        x = strlen( BUTTONS[ hpkey ].label_sdl ) - 1;
        x += ( ( BUTTONS[ hpkey ].w - BigTextWidth( BUTTONS[ hpkey ].label_sdl, strlen( BUTTONS[ hpkey ].label_sdl ) ) ) / 2 );
        y = ( BUTTONS[ hpkey ].h + 1 ) / 2 - 6;
        if ( is_down )
            y -= 1;

        write_with_big_font( x, y, BUTTONS[ hpkey ].label_sdl, label_color, inner_color );
    }

    // draw edge of button
    // top
    __draw_line( 1, 1, BUTTONS[ hpkey ].w - 2, 1, edge_top_color );
    __draw_line( 2, 2, BUTTONS[ hpkey ].w - 3, 2, edge_top_color );
    if ( is_up ) {
        __draw_line( 3, 3, BUTTONS[ hpkey ].w - 4, 3, edge_top_color );
        __draw_line( 4, 4, BUTTONS[ hpkey ].w - 5, 4, edge_top_color );
    }
    // top-left
    __draw_pixel( 4, 3 + ( is_up ? 2 : 0 ), edge_top_color );
    // left
    __draw_line( 1, 1, 1, BUTTONS[ hpkey ].h - 2, edge_top_color );
    __draw_line( 2, 2, 2, BUTTONS[ hpkey ].h - 3, edge_top_color );
    __draw_line( 3, 3, 3, BUTTONS[ hpkey ].h - 4, edge_top_color );
    // right
    __draw_line( BUTTONS[ hpkey ].w - 2, BUTTONS[ hpkey ].h - 2, BUTTONS[ hpkey ].w - 2, 3, edge_bottom_color );
    __draw_line( BUTTONS[ hpkey ].w - 3, BUTTONS[ hpkey ].h - 3, BUTTONS[ hpkey ].w - 3, 4, edge_bottom_color );
    __draw_line( BUTTONS[ hpkey ].w - 4, BUTTONS[ hpkey ].h - 4, BUTTONS[ hpkey ].w - 4, 5, edge_bottom_color );
    __draw_pixel( BUTTONS[ hpkey ].w - 5, BUTTONS[ hpkey ].h - 4, edge_bottom_color );
    // bottom
    __draw_line( 3, BUTTONS[ hpkey ].h - 2, BUTTONS[ hpkey ].w - 2, BUTTONS[ hpkey ].h - 2, edge_bottom_color );
    __draw_line( 4, BUTTONS[ hpkey ].h - 3, BUTTONS[ hpkey ].w - 3, BUTTONS[ hpkey ].h - 3, edge_bottom_color );

    // draw black frame around button
    // top
    __draw_line( 2, 0, BUTTONS[ hpkey ].w - 3, 0, UI4X_COLOR_FRAME );
    // left
    __draw_line( 0, 2, 0, BUTTONS[ hpkey ].h - 3, UI4X_COLOR_FRAME );
    // right
    __draw_line( BUTTONS[ hpkey ].w - 1, BUTTONS[ hpkey ].h - 3, BUTTONS[ hpkey ].w - 1, 2, UI4X_COLOR_FRAME );
    // bottom
    __draw_line( 2, BUTTONS[ hpkey ].h - 1, BUTTONS[ hpkey ].w - 3, BUTTONS[ hpkey ].h - 1, UI4X_COLOR_FRAME );
    // top-left
    __draw_pixel( 1, 1, UI4X_COLOR_FRAME );
    // top-right
    __draw_pixel( BUTTONS[ hpkey ].w - 2, 1, UI4X_COLOR_FRAME );
    // bottom-left
    __draw_pixel( 1, BUTTONS[ hpkey ].h - 2, UI4X_COLOR_FRAME );
    // bottom-right
    __draw_pixel( BUTTONS[ hpkey ].w - 2, BUTTONS[ hpkey ].h - 2, UI4X_COLOR_FRAME );
    if ( hpkey == UI4X_KEY_ON ) {
        // top
        __draw_line( 2, 1, BUTTONS[ hpkey ].w - 3, 1, UI4X_COLOR_FRAME );
        // top-left
        __draw_pixel( 1, 1 + on_key_offset_y, UI4X_COLOR_FRAME );
        // top-right
        __draw_pixel( BUTTONS[ hpkey ].w - 2, 1 + on_key_offset_y, UI4X_COLOR_FRAME );
    }

    if ( is_down ) {
        // top
        __draw_line( 2, 1 + on_key_offset_y, BUTTONS[ hpkey ].w - 3, 1 + on_key_offset_y, UI4X_COLOR_FRAME );
        // left
        __draw_line( 1, 2, 1, BUTTONS[ hpkey ].h, UI4X_COLOR_FRAME );
        // right
        __draw_line( BUTTONS[ hpkey ].w - 2, 2, BUTTONS[ hpkey ].w - 2, BUTTONS[ hpkey ].h, UI4X_COLOR_FRAME );
        // top-left
        __draw_pixel( 2, 2 + on_key_offset_y, UI4X_COLOR_FRAME );
        // top-right
        __draw_pixel( BUTTONS[ hpkey ].w - 3, 2 + on_key_offset_y, UI4X_COLOR_FRAME );
    }

    return texture;
}

static void create_buttons_textures( void )
{
    for ( int i = 0; i < NB_KEYS; i++ ) {
        buttons_textures[ i ].up = create_button_texture( i, true );
        buttons_textures[ i ].down = create_button_texture( i, false );
    }

    // Give back to renderer as it was
    SDL_SetRenderTarget( renderer, main_texture );
}

static void _draw_key( int hpkey )
{
    __draw_texture( OFFSET_X_KEYBOARD + BUTTONS[ hpkey ].x, OFFSET_Y_KEYBOARD + BUTTONS[ hpkey ].y, BUTTONS[ hpkey ].w, BUTTONS[ hpkey ].h,
                    ui4x_emulator_api.is_key_pressed( hpkey ) ? buttons_textures[ hpkey ].down : buttons_textures[ hpkey ].up );
}

#define SMALL_ASCENT 8
#define SMALL_DESCENT 2

static void _draw_keypad( void )
{
    int x, xr, y;
    int left_label_width, right_label_width;
    int space_char_width = SmallTextWidth( " ", 1 );
    int total_top_labels_width;

    // for .highlight highlighted area, 48 only
    int pw = ui4x_config.model == MODEL_48GX ? 58 : 44;
    int ph = ui4x_config.model == MODEL_48GX ? 48 : 9;

    for ( int i = 0; i < NB_KEYS; i++ ) {
        // Background
        if ( BUTTONS[ i ].highlight ) {
            x = OFFSET_X_KEYBOARD + BUTTONS[ i ].x;
            y = OFFSET_Y_KEYBOARD + BUTTONS[ i ].y - SMALL_ASCENT - SMALL_DESCENT;

            if ( ui4x_config.model == MODEL_48GX ) {
                x -= 6;
                y -= 6;
            } else
                x += ( BUTTONS[ i ].w - pw ) / 2;

            __draw_rect( x, y, pw, ph, UI4X_COLOR_KEYPAD_HIGHLIGHT );
        }

        // Letter (small character bottom right of key)
        if ( BUTTONS[ i ].letter != ( char* )0 ) {
            x = OFFSET_X_KEYBOARD + BUTTONS[ i ].x + BUTTONS[ i ].w + 3;
            y = OFFSET_Y_KEYBOARD + BUTTONS[ i ].y + BUTTONS[ i ].h + 1;

            if ( ui4x_config.model == MODEL_48SX )
                y -= 2;

            write_with_small_font( x, y, BUTTONS[ i ].letter, UI4X_COLOR_ALPHA,
                                   ( ( ui4x_config.model == MODEL_48GX || ui4x_config.model == MODEL_48SX ) && i < HP48_KEY_G )
                                       ? UI4X_COLOR_UPPER_FACEPLATE
                                       : UI4X_COLOR_FACEPLATE );
        }

        // Bottom label: the only one is the cancel button
        if ( BUTTONS[ i ].below != ( char* )0 ) {
            x = OFFSET_X_KEYBOARD + BUTTONS[ i ].x +
                ( 1 + BUTTONS[ i ].w - SmallTextWidth( BUTTONS[ i ].below, strlen( BUTTONS[ i ].below ) ) ) / 2;
            y = OFFSET_Y_KEYBOARD + BUTTONS[ i ].y + BUTTONS[ i ].h + SMALL_ASCENT + 1;

            write_with_small_font( x, y, BUTTONS[ i ].below, UI4X_COLOR_LABEL, UI4X_COLOR_FACEPLATE );
        }

        // Draw top labels
        left_label_width =
            ( BUTTONS[ i ].left_sdl == ( char* )0 ) ? 0 : SmallTextWidth( BUTTONS[ i ].left_sdl, strlen( BUTTONS[ i ].left_sdl ) );
        right_label_width =
            ( BUTTONS[ i ].right_sdl == ( char* )0 ) ? 0 : SmallTextWidth( BUTTONS[ i ].right_sdl, strlen( BUTTONS[ i ].right_sdl ) );
        total_top_labels_width = left_label_width + right_label_width;
        if ( left_label_width > 0 && right_label_width > 0 )
            total_top_labels_width += space_char_width;

        // Calculate both labels' x
        x = xr = OFFSET_X_KEYBOARD + BUTTONS[ i ].x;
        if ( left_label_width > 0 && right_label_width > 0 ) {
            // should draw both labels
            if ( total_top_labels_width > BUTTONS[ i ].w ) {
                // combination of labels are wider than the button so they'll be centered above it
                int xoffset = ( 1 + BUTTONS[ i ].w - total_top_labels_width ) / 2;
                x += xoffset;
                xr += xoffset + left_label_width + space_char_width;
            } else {
                // combination of labels are smaller than the button so they'll hug the sides
                x += 2;
                xr += BUTTONS[ i ].w - right_label_width;
            }
        } else if ( left_label_width > 0 && right_label_width == 0 )
            // draw only left label so center it
            x += ( BUTTONS[ i ].w - total_top_labels_width ) / 2;
        else if ( left_label_width == 0 && right_label_width > 0 )
            // draw only right label so center it
            xr += ( BUTTONS[ i ].w - total_top_labels_width ) / 2;

        // y is easier
        y = OFFSET_Y_KEYBOARD + BUTTONS[ i ].y - SMALL_DESCENT;

        // finally draw labels
        if ( left_label_width > 0 )
            write_with_small_font( x, y, BUTTONS[ i ].left_sdl, UI4X_COLOR_LEFTSHIFT,
                                   BUTTONS[ i ].highlight ? UI4X_COLOR_KEYPAD_HIGHLIGHT : UI4X_COLOR_FACEPLATE );
        if ( right_label_width > 0 )
            write_with_small_font( xr, y, BUTTONS[ i ].right_sdl, UI4X_COLOR_RIGHTSHIFT,
                                   BUTTONS[ i ].highlight ? UI4X_COLOR_KEYPAD_HIGHLIGHT : UI4X_COLOR_FACEPLATE );
    }

    for ( int i = 0; i < NB_KEYS; i++ )
        _draw_key( i );
}

static void _draw_bezel_LCD( void )
{
    for ( int i = 0; i < BEZEL_WIDTH_DISPLAY; i++ ) {
        __draw_line( OFFSET_X_DISPLAY - i, OFFSET_Y_DISPLAY + HEIGHT_DISPLAY + 2 * i, OFFSET_X_DISPLAY + WIDTH_DISPLAY + i,
                     OFFSET_Y_DISPLAY + HEIGHT_DISPLAY + 2 * i, UI4X_COLOR_UPPER_FACEPLATE_EDGE_TOP );
        __draw_line( OFFSET_X_DISPLAY - i, OFFSET_Y_DISPLAY + HEIGHT_DISPLAY + 2 * i + 1, OFFSET_X_DISPLAY + WIDTH_DISPLAY + i,
                     OFFSET_Y_DISPLAY + HEIGHT_DISPLAY + 2 * i + 1, UI4X_COLOR_UPPER_FACEPLATE_EDGE_TOP );
        __draw_line( OFFSET_X_DISPLAY + WIDTH_DISPLAY + i, OFFSET_Y_DISPLAY - i, OFFSET_X_DISPLAY + WIDTH_DISPLAY + i,
                     OFFSET_Y_DISPLAY + HEIGHT_DISPLAY + 2 * i, UI4X_COLOR_UPPER_FACEPLATE_EDGE_TOP );

        __draw_line( OFFSET_X_DISPLAY - i - 1, OFFSET_Y_DISPLAY - i - 1, OFFSET_X_DISPLAY + WIDTH_DISPLAY + i - 1, OFFSET_Y_DISPLAY - i - 1,
                     UI4X_COLOR_UPPER_FACEPLATE_EDGE_BOTTOM );
        __draw_line( OFFSET_X_DISPLAY - i - 1, OFFSET_Y_DISPLAY - i - 1, OFFSET_X_DISPLAY - i - 1,
                     OFFSET_Y_DISPLAY + HEIGHT_DISPLAY + 2 * i - 1, UI4X_COLOR_UPPER_FACEPLATE_EDGE_BOTTOM );
    }

    // round off corners
    __draw_line( OFFSET_X_DISPLAY - BEZEL_WIDTH_DISPLAY, OFFSET_Y_DISPLAY - BEZEL_WIDTH_DISPLAY, OFFSET_X_DISPLAY - BEZEL_WIDTH_DISPLAY + 3,
                 OFFSET_Y_DISPLAY - BEZEL_WIDTH_DISPLAY, UI4X_COLOR_UPPER_FACEPLATE );
    __draw_line( OFFSET_X_DISPLAY - BEZEL_WIDTH_DISPLAY, OFFSET_Y_DISPLAY - BEZEL_WIDTH_DISPLAY, OFFSET_X_DISPLAY - BEZEL_WIDTH_DISPLAY,
                 OFFSET_Y_DISPLAY - BEZEL_WIDTH_DISPLAY + 3, UI4X_COLOR_UPPER_FACEPLATE );
    __draw_pixel( OFFSET_X_DISPLAY - BEZEL_WIDTH_DISPLAY + 1, OFFSET_Y_DISPLAY - BEZEL_WIDTH_DISPLAY + 1, UI4X_COLOR_UPPER_FACEPLATE );

    __draw_line( OFFSET_X_DISPLAY + WIDTH_DISPLAY + BEZEL_WIDTH_DISPLAY - 4, OFFSET_Y_DISPLAY - BEZEL_WIDTH_DISPLAY,
                 OFFSET_X_DISPLAY + WIDTH_DISPLAY + BEZEL_WIDTH_DISPLAY - 1, OFFSET_Y_DISPLAY - BEZEL_WIDTH_DISPLAY,
                 UI4X_COLOR_UPPER_FACEPLATE );
    __draw_line( OFFSET_X_DISPLAY + WIDTH_DISPLAY + BEZEL_WIDTH_DISPLAY - 1, OFFSET_Y_DISPLAY - BEZEL_WIDTH_DISPLAY,
                 OFFSET_X_DISPLAY + WIDTH_DISPLAY + BEZEL_WIDTH_DISPLAY - 1, OFFSET_Y_DISPLAY - BEZEL_WIDTH_DISPLAY + 3,
                 UI4X_COLOR_UPPER_FACEPLATE );
    __draw_pixel( OFFSET_X_DISPLAY + WIDTH_DISPLAY + BEZEL_WIDTH_DISPLAY - 2, OFFSET_Y_DISPLAY - BEZEL_WIDTH_DISPLAY + 1,
                  UI4X_COLOR_UPPER_FACEPLATE );

    __draw_line( OFFSET_X_DISPLAY - BEZEL_WIDTH_DISPLAY, OFFSET_Y_DISPLAY + HEIGHT_DISPLAY + 2 * BEZEL_WIDTH_DISPLAY - 4,
                 OFFSET_X_DISPLAY - BEZEL_WIDTH_DISPLAY, OFFSET_Y_DISPLAY + HEIGHT_DISPLAY + 2 * BEZEL_WIDTH_DISPLAY - 1,
                 UI4X_COLOR_UPPER_FACEPLATE );
    __draw_line( OFFSET_X_DISPLAY - BEZEL_WIDTH_DISPLAY, OFFSET_Y_DISPLAY + HEIGHT_DISPLAY + 2 * BEZEL_WIDTH_DISPLAY - 1,
                 OFFSET_X_DISPLAY - BEZEL_WIDTH_DISPLAY + 3, OFFSET_Y_DISPLAY + HEIGHT_DISPLAY + 2 * BEZEL_WIDTH_DISPLAY - 1,
                 UI4X_COLOR_UPPER_FACEPLATE );
    __draw_pixel( OFFSET_X_DISPLAY - BEZEL_WIDTH_DISPLAY + 1, OFFSET_Y_DISPLAY + HEIGHT_DISPLAY + 2 * BEZEL_WIDTH_DISPLAY - 2,
                  UI4X_COLOR_UPPER_FACEPLATE );

    __draw_line( OFFSET_X_DISPLAY + WIDTH_DISPLAY + BEZEL_WIDTH_DISPLAY - 1,
                 OFFSET_Y_DISPLAY + HEIGHT_DISPLAY + 2 * BEZEL_WIDTH_DISPLAY - 4,
                 OFFSET_X_DISPLAY + WIDTH_DISPLAY + BEZEL_WIDTH_DISPLAY - 1,
                 OFFSET_Y_DISPLAY + HEIGHT_DISPLAY + 2 * BEZEL_WIDTH_DISPLAY - 1, UI4X_COLOR_UPPER_FACEPLATE );
    __draw_line( OFFSET_X_DISPLAY + WIDTH_DISPLAY + BEZEL_WIDTH_DISPLAY - 4,
                 OFFSET_Y_DISPLAY + HEIGHT_DISPLAY + 2 * BEZEL_WIDTH_DISPLAY - 1,
                 OFFSET_X_DISPLAY + WIDTH_DISPLAY + BEZEL_WIDTH_DISPLAY - 1,
                 OFFSET_Y_DISPLAY + HEIGHT_DISPLAY + 2 * BEZEL_WIDTH_DISPLAY - 1, UI4X_COLOR_UPPER_FACEPLATE );
    __draw_pixel( OFFSET_X_DISPLAY + WIDTH_DISPLAY + BEZEL_WIDTH_DISPLAY - 2,
                  OFFSET_Y_DISPLAY + HEIGHT_DISPLAY + 2 * BEZEL_WIDTH_DISPLAY - 2, UI4X_COLOR_UPPER_FACEPLATE );

    // simulate rounded lcd corners
    __draw_line( OFFSET_X_DISPLAY - 1, OFFSET_Y_DISPLAY + 1, OFFSET_X_DISPLAY - 1, OFFSET_Y_DISPLAY + HEIGHT_DISPLAY - 2, COLOR_LCD_BG );
    __draw_line( OFFSET_X_DISPLAY + 1, OFFSET_Y_DISPLAY - 1, OFFSET_X_DISPLAY + WIDTH_DISPLAY - 2, OFFSET_Y_DISPLAY - 1, COLOR_LCD_BG );
    __draw_line( OFFSET_X_DISPLAY + 1, OFFSET_Y_DISPLAY + HEIGHT_DISPLAY, OFFSET_X_DISPLAY + WIDTH_DISPLAY - 2,
                 OFFSET_Y_DISPLAY + HEIGHT_DISPLAY, COLOR_LCD_BG );
    __draw_line( OFFSET_X_DISPLAY + WIDTH_DISPLAY, OFFSET_Y_DISPLAY + 1, OFFSET_X_DISPLAY + WIDTH_DISPLAY,
                 OFFSET_Y_DISPLAY + HEIGHT_DISPLAY - 2, COLOR_LCD_BG );
}

static void _draw_background( int width, int height, int w_top, int h_top )
{
    __draw_rect( 0, 0, w_top, h_top, UI4X_COLOR_FACEPLATE );
    __draw_rect( 0, 0, width, height, UI4X_COLOR_UPPER_FACEPLATE );
}

static void _draw_background_LCD( void ) { __draw_rect( OFFSET_X_DISPLAY, OFFSET_Y_DISPLAY, WIDTH_DISPLAY, HEIGHT_DISPLAY, COLOR_LCD_BG ); }

// Show the hp key which is being pressed
static void _show_key( int hpkey )
{
    if ( ui4x_config.chromeless || hpkey < 0 )
        return;

    SDL_SetRenderTarget( renderer, main_texture );

    _draw_key( hpkey );

    SDL_SetRenderTarget( renderer, NULL );
    SDL_RenderTexture( renderer, main_texture, NULL, NULL );
    SDL_RenderPresent( renderer );

    return;
}

static void _draw_serial_devices_path( void )
{
    char text[ 1024 ] = "";

    if ( ui4x_config.wire_name ) {
        strcat( text, "wire: " );
        strcat( text, ui4x_config.wire_name );
    }
    if ( ui4x_config.ir_name ) {
        if ( strlen( text ) > 0 )
            strcat( text, " | " );

        strcat( text, "IR: " );
        strcat( text, ui4x_config.ir_name );
    }

    if ( strlen( text ) > 0 )
        write_with_small_font(
            ( ( PADDING_SIDE * 1.5 ) + WIDTH_DISPLAY ) - SmallTextWidth( text, strlen( text ) ),
            ( ui4x_config.model == MODEL_49G ? PADDING_TOP - 12 : OFFSET_Y_KEYBOARD - ( PADDING_BETWEEN_DISPLAY_AND_KEYBOARD / 2 ) ), text,
            COLOR_LCD_BG, UI4X_COLOR_UPPER_FACEPLATE );
}

static int sdl_emulator_press_key( int hpkey )
{
    if ( hpkey == -1 || ui4x_emulator_api.is_key_pressed( hpkey ) )
        return -1;

    ui4x_emulator_api.press_key( hpkey );
    _show_key( hpkey );

    return hpkey;
}

static int sdl_emulator_release_key( int hpkey )
{
    if ( hpkey == -1 || !ui4x_emulator_api.is_key_pressed( hpkey ) )
        return -1;

    ui4x_emulator_api.release_key( hpkey );
    _show_key( hpkey );

    return hpkey;
}

static void blank_lcd( void ) { memset( display_buffer_grayscale, 0, sizeof( display_buffer_grayscale ) ); }

static void sdl_update_annunciators( void )
{
    int annunciators = ui4x_emulator_api.get_annunciators();

    if ( last_annunciators == annunciators )
        return;

    last_annunciators = annunciators;

    SDL_SetRenderTarget( renderer, main_texture );

    for ( int i = 0; i < NB_ANNUNCIATORS; i++ )
        __draw_texture( OFFSET_X_DISPLAY + annunciators_ui[ i ].x, OFFSET_Y_DISPLAY + annunciators_ui[ i ].y, annunciators_ui[ i ].width,
                        annunciators_ui[ i ].height,
                        ( ( annunciators >> i ) & 0x01 ) ? annunciators_textures[ i ].up : annunciators_textures[ i ].down );

    // Always immediately update annunciators
    SDL_SetRenderTarget( renderer, NULL );
    SDL_RenderTexture( renderer, main_texture, NULL, NULL );
    SDL_RenderPresent( renderer );
}

static int apply_contrast( int value, int contrast )
{
    int max = 19;
    int min = 3;

    int contrasted = ( value / ( max - min ) ) * ( max - contrast );
    return contrasted;
}

static void setup_colors( void )
{
    int contrast = ui4x_emulator_api.get_contrast();

    if ( last_contrast == contrast )
        return;

    if ( contrast < 3 )
        contrast = 3;
    if ( contrast > 19 )
        contrast = 19;

    last_contrast = contrast;

    for ( unsigned i = 0; i < NB_COLORS; i++ ) {
        colors[ i ] = COLORS[ i ];

        if ( ui4x_config.mono ) {
            colors[ i ].r = colors[ i ].mono_rgb;
            colors[ i ].g = colors[ i ].mono_rgb;
            colors[ i ].b = colors[ i ].mono_rgb;
        } else {
            if ( ui4x_config.gray ) {
                colors[ i ].r = colors[ i ].gray_rgb;
                colors[ i ].g = colors[ i ].gray_rgb;
                colors[ i ].b = colors[ i ].gray_rgb;
            } else {
                colors[ i ].r = ( colors[ i ].rgb >> 16 ) & 0xff;
                colors[ i ].g = ( colors[ i ].rgb >> 8 ) & 0xff;
                colors[ i ].b = colors[ i ].rgb & 0xff;
            }
            if ( i == COLOR_PIXEL_ON ) {
                // COLOR_PIXEL_ON is computed based on COLOR_LCD_BG and contrast
                colors[ i ].r = apply_contrast( colors[ COLOR_LCD_BG ].r, contrast );
                colors[ i ].g = apply_contrast( colors[ COLOR_LCD_BG ].g, contrast );

                if ( ui4x_config.black_lcd )
                    colors[ i ].b = apply_contrast( colors[ COLOR_LCD_BG ].b, contrast );
                else
                    colors[ i ].b = 128 - ( ( 19 - contrast ) * ( ( 128 - colors[ COLOR_LCD_BG ].b ) / 16 ) );
            }
        }
    }

    pixel_colors[ 0 ] = colors[ COLOR_LCD_BG ];
    int contrast_value_for_shade;
    for ( int i = 1; i < N_LEVELS_OF_GRAY; i++ ) {
        contrast_value_for_shade = 3 + ( N_LEVELS_OF_GRAY == 16 ? i : i * 4 );
        pixel_colors[ i ].a = 0xff;
        pixel_colors[ i ].r = apply_contrast( colors[ COLOR_LCD_BG ].r, contrast_value_for_shade );
        pixel_colors[ i ].g = apply_contrast( colors[ COLOR_LCD_BG ].g, contrast_value_for_shade );

        if ( ui4x_config.black_lcd )
            pixel_colors[ i ].b = apply_contrast( colors[ COLOR_LCD_BG ].b, contrast_value_for_shade );
        else
            pixel_colors[ i ].b = 128 - ( ( 19 - contrast_value_for_shade ) * ( ( 128 - colors[ COLOR_LCD_BG ].b ) / 16 ) );
    }
    pixel_colors[ N_LEVELS_OF_GRAY ] = colors[ COLOR_PIXEL_ON ];

    // re-create annunciators textures
    last_annunciators = -1;
    create_annunciators_textures();
}

/**********/
/* public */
/**********/
void sdl_ui_handle_pending_inputs( void )
{
    SDL_Event event;
    int hpkey = -1;
    static int lasthpkey = -1; // last key that was pressed or -1 for none
    static int lastticks = -1; // time at which a key was pressed or -1 if timer expired

    // Iterate as long as there are events
    while ( SDL_PollEvent( &event ) ) {
        switch ( event.type ) {
            case SDL_EVENT_QUIT:
                // please_exit = true;
                ui4x_emulator_api.do_stop();
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                hpkey = mouse_click_to_hpkey( event.button.x, event.button.y );
                if ( sdl_emulator_press_key( hpkey ) != -1 ) {
                    if ( lasthpkey == -1 ) {
                        lasthpkey = hpkey;
                        // Start timer
                        lastticks = SDL_GetTicks();
                    } else
                        lasthpkey = lastticks = -1;
                }

                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                hpkey = mouse_click_to_hpkey( event.button.x, event.button.y );
                if ( lasthpkey != hpkey || lastticks == -1 || ( SDL_GetTicks() - lastticks < LONG_PRESS_THR ) )
                    sdl_emulator_release_key( hpkey );

                lasthpkey = lastticks = -1;
                break;

            case SDL_EVENT_KEY_DOWN:
                sdl_emulator_press_key( sdlkey_to_hpkey( EVENT_KEY( event ) ) );
                break;
            case SDL_EVENT_KEY_UP:
                sdl_emulator_release_key( sdlkey_to_hpkey( EVENT_KEY( event ) ) );
                break;
        }
    }
}

void sdl_ui_refresh_lcd( void )
{
    setup_colors();

    if ( ui4x_emulator_api.is_display_on() )
        ui4x_emulator_api.get_lcd_buffer( display_buffer_grayscale );
    else
        blank_lcd();

    SDL_SetRenderTarget( renderer, display_texture );

    int pxl_val;
    for ( int y = 0; y < LCD_HEIGHT; ++y ) {
        for ( int x = 0; x < LCD_WIDTH; ++x ) {
            pxl_val = display_buffer_grayscale[ ( y * LCD_WIDTH ) + x ];

            __draw_pixel_rgba( x, y, pixel_colors[ pxl_val ].r, pixel_colors[ pxl_val ].g, pixel_colors[ pxl_val ].b,
                               pixel_colors[ pxl_val ].a );
        }
    }

    SDL_SetRenderTarget( renderer, NULL );
    SDL_SetTextureScaleMode( display_texture, SDL_SCALEMODE_NEAREST );
    SDL_RenderTexture( renderer, display_texture, NULL, NULL );

    SDL_SetRenderTarget( renderer, main_texture );

    __draw_texture( OFFSET_X_DISPLAY + 5, OFFSET_Y_DISPLAY + 20, LCD_WIDTH * 2, LCD_HEIGHT * 2, display_texture );

    SDL_SetRenderTarget( renderer, NULL );
    SDL_RenderTexture( renderer, main_texture, NULL, NULL );
    SDL_RenderPresent( renderer );

    sdl_update_annunciators();
}

void sdl_init_ui( void )
{
    blank_lcd();

    // Initialize SDL
    if ( !SDL_Init( SDL_INIT_VIDEO ) ) {
        printf( "Couldn't initialize SDL: %s\n", SDL_GetError() );
        exit( 1 );
    }

    // On exit: clean SDL
    atexit( SDL_Quit );

    unsigned int width =
        ui4x_config.chromeless ? WIDTH_DISPLAY : ( ( BUTTONS[ NB_KEYS - 1 ].x + BUTTONS[ NB_KEYS - 1 ].w ) + 2 * PADDING_SIDE );
    unsigned int height = ui4x_config.chromeless ? HEIGHT_DISPLAY
                                                 : ( OFFSET_Y_DISPLAY + HEIGHT_DISPLAY + PADDING_BETWEEN_DISPLAY_AND_KEYBOARD +
                                                     BUTTONS[ NB_KEYS - 1 ].y + BUTTONS[ NB_KEYS - 1 ].h + PADDING_BOTTOM );

    uint32_t window_flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if ( ui4x_config.fullscreen )
        window_flags |= SDL_WINDOW_FULLSCREEN;
    else
        window_flags |= SDL_WINDOW_RESIZABLE;

    window = CREATE_WINDOW( ui4x_config.progname, width * ui4x_config.zoom, height * ui4x_config.zoom, window_flags );
    if ( window == NULL ) {
        printf( "Couldn't create window: %s\n", SDL_GetError() );
        exit( 1 );
    }

    renderer = CREATE_RENDERER( window );
    if ( renderer == NULL )
        exit( 2 );

    // SDL_SetRenderLogicalPresentation( renderer, width, height, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE );
    SET_RENDER_LOGICAL_PRESENTATION( renderer, width, height );

    main_texture = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height );

    display_texture = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, LCD_WIDTH, LCD_HEIGHT );

    SDL_SetRenderTarget( renderer, main_texture );

    setup_colors();

    if ( !ui4x_config.chromeless ) {
        int cut = BUTTONS[ HP48_KEY_G ].y + OFFSET_Y_KEYBOARD - 19;

        create_buttons_textures();

        _draw_background( width, cut, width, height );
        _draw_bezel( cut, OFFSET_Y_KEYBOARD, width, height );
        _draw_header();
        _draw_bezel_LCD();
        _draw_keypad();

        _draw_serial_devices_path();
    }

    _draw_background_LCD();

    SDL_SetRenderTarget( renderer, NULL );
    SDL_RenderTexture( renderer, main_texture, NULL, NULL );
    SDL_RenderPresent( renderer );
}

void sdl_exit_ui( void )
{
    for ( int i = 0; i < NB_ANNUNCIATORS; i++ ) {
        SDL_DestroyTexture( annunciators_textures[ i ].up );
        SDL_DestroyTexture( annunciators_textures[ i ].down );
    }

    for ( int i = 0; i < NB_KEYS; i++ ) {
        SDL_DestroyTexture( buttons_textures[ i ].up );
        SDL_DestroyTexture( buttons_textures[ i ].down );
    }

    SDL_DestroyTexture( main_texture );
    SDL_DestroyTexture( display_texture );
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
}
