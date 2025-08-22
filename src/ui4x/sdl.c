#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>

#include "../emulator.h"
#include "../options.h"
#include "common.h"
#include "inner.h"

#define COLORS                                                                                                                             \
    ( __config.model == MODEL_48GX                                                                                                         \
          ? colors_48gx                                                                                                                    \
          : ( __config.model == MODEL_48SX ? colors_48sx : ( __config.model == MODEL_49G ? colors_49g : colors_50g ) ) )
#define BUTTONS                                                                                                                            \
    ( __config.model == MODEL_48GX                                                                                                         \
          ? buttons_48gx                                                                                                                   \
          : ( __config.model == MODEL_48SX ? buttons_48sx : ( __config.model == MODEL_49G ? buttons_49g : buttons_50g ) ) )

#define COLOR_PIXEL_OFF ( __config.black_lcd ? UI4X_COLOR_BLACK_PIXEL_OFF : UI4X_COLOR_PIXEL_OFF )
#define COLOR_PIXEL_GREY_1 ( __config.black_lcd ? UI4X_COLOR_BLACK_PIXEL_GREY_1 : UI4X_COLOR_PIXEL_GREY_1 )
#define COLOR_PIXEL_GREY_2 ( __config.black_lcd ? UI4X_COLOR_BLACK_PIXEL_GREY_2 : UI4X_COLOR_PIXEL_GREY_2 )
#define COLOR_PIXEL_ON ( __config.black_lcd ? UI4X_COLOR_BLACK_PIXEL_ON : UI4X_COLOR_PIXEL_ON )

#define PADDING_TOP ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? 48 : 65 )
#define PADDING_SIDE 20
#define PADDING_BOTTOM 25
#define PADDING_BETWEEN_DISPLAY_AND_KEYBOARD ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? 32 : 65 )
#define OFFSET_Y_48GSX_INDENT_BELOW_MENU_KEYS 25

#define WIDTH_DISPLAY ( ( ( LCD_WIDTH + 1 ) * 2 ) + 8 )
#define HEIGHT_DISPLAY ( ( LCD_HEIGHT * 2 ) + 16 + 8 )
#define OFFSET_X_DISPLAY ( __config.chromeless ? 0 : ( PADDING_SIDE + ( 286 - WIDTH_DISPLAY ) / 2 ) )
#define OFFSET_Y_DISPLAY ( __config.chromeless ? 0 : PADDING_TOP )

#define BEZEL_WIDTH_DISPLAY 8

#define OFFSET_X_KEYBOARD PADDING_SIDE
#define OFFSET_Y_KEYBOARD ( PADDING_TOP + HEIGHT_DISPLAY + PADDING_BETWEEN_DISPLAY_AND_KEYBOARD )

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

static int display_buffer_current[ LCD_WIDTH * 80 ];
static int display_buffer_past_one[ LCD_WIDTH * 80 ];
static int display_buffer_past_two[ LCD_WIDTH * 80 ];
static int display_buffer_grayscale[ LCD_WIDTH * 80 ];
static int last_annunciators = -1;
static int last_contrast = -1;

static color_t colors[ NB_COLORS ];
static on_off_sdl_textures_struct_t buttons_textures[ NB_HP49_KEYS ];
static on_off_sdl_textures_struct_t annunciators_textures[ NB_ANNUNCIATORS ];

static SDL_Window* window;
static SDL_Renderer* renderer;
static SDL_Texture* main_texture;
static SDL_Texture* display_texture;

static config_t __config;

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
    SDL_Surface* surf = SDL_CreateSurface( w, h, SDL_GetPixelFormatForMasks( 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 ) );

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

static void __draw_pixel( int x, int y, int color )
{
    SDL_SetRenderDrawColor( renderer, colors[ color ].r, colors[ color ].g, colors[ color ].b, colors[ color ].a );
    SDL_RenderPoint( renderer, x, y );
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
                                                           annunciators_ui[ i ].bits, COLOR_PIXEL_ON, COLOR_PIXEL_OFF );
        annunciators_textures[ i ].down = bitmap_to_texture( annunciators_ui[ i ].width, annunciators_ui[ i ].height,
                                                             annunciators_ui[ i ].bits, COLOR_PIXEL_OFF, COLOR_PIXEL_OFF );
    }
}

// Find which key is pressed, if any.
// Returns -1 is no key is pressed
static int mouse_click_to_hpkey( int x, int y )
{
    /* return immediatly if the click isn't even in the keyboard area */
    if ( y < OFFSET_Y_KEYBOARD )
        return -1;

    x -= OFFSET_X_KEYBOARD;
    y -= OFFSET_Y_KEYBOARD;

    for ( int i = 0; i < NB_KEYS; i++ )
        if ( ( BUTTONS[ i ].x < x && ( BUTTONS[ i ].x + BUTTONS[ i ].w ) > x ) &&
             ( BUTTONS[ i ].y < y && ( BUTTONS[ i ].y + BUTTONS[ i ].h ) > y ) )
            return i;

    return -1;
}

// Map the keyboard keys to the HP keys
// Returns -1 if there is no mapping
static int sdlkey_to_hpkey( SDL_Keycode k )
{
    switch ( k ) {
        case SDLK_0:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_0 : HP48_KEY_0 );
        case SDLK_1:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_1 : HP48_KEY_1 );
        case SDLK_2:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_2 : HP48_KEY_2 );
        case SDLK_3:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_3 : HP48_KEY_3 );
        case SDLK_4:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_4 : HP48_KEY_4 );
        case SDLK_5:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_5 : HP48_KEY_5 );
        case SDLK_6:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_6 : HP48_KEY_6 );
        case SDLK_7:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_7 : HP48_KEY_7 );
        case SDLK_8:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_8 : HP48_KEY_8 );
        case SDLK_9:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_9 : HP48_KEY_9 );
        case SDLK_KP_0:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_0 : HP48_KEY_0 );
        case SDLK_KP_1:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_1 : HP48_KEY_1 );
        case SDLK_KP_2:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_2 : HP48_KEY_2 );
        case SDLK_KP_3:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_3 : HP48_KEY_3 );
        case SDLK_KP_4:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_4 : HP48_KEY_4 );
        case SDLK_KP_5:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_5 : HP48_KEY_5 );
        case SDLK_KP_6:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_6 : HP48_KEY_6 );
        case SDLK_KP_7:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_7 : HP48_KEY_7 );
        case SDLK_KP_8:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_8 : HP48_KEY_8 );
        case SDLK_KP_9:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_9 : HP48_KEY_9 );
        case SDLK_A:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_A : HP48_KEY_A );
        case SDLK_B:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_B : HP48_KEY_B );
        case SDLK_C:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_C : HP48_KEY_C );
        case SDLK_D:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_D : HP48_KEY_D );
        case SDLK_E:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_E : HP48_KEY_E );
        case SDLK_F:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_F : HP48_KEY_F );
        case SDLK_G:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_APPS : HP48_KEY_MTH );
        case SDLK_H:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_MODE : HP48_KEY_PRG );
        case SDLK_I:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_TOOL : HP48_KEY_CST );
        case SDLK_J:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_VAR : HP48_KEY_VAR );
        case SDLK_K:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_STO : HP48_KEY_UP );
        case SDLK_UP:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_UP : HP48_KEY_UP );
        case SDLK_L:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_NXT : HP48_KEY_NXT );
        case SDLK_M:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_HIST : HP48_KEY_QUOTE );
        case SDLK_N:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_CAT : HP48_KEY_STO );
        case SDLK_O:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_EQW : HP48_KEY_EVAL );
        case SDLK_P:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_SYMB : HP48_KEY_LEFT );
        case SDLK_LEFT:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_LEFT : HP48_KEY_LEFT );
        case SDLK_Q:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_POWER : HP48_KEY_DOWN );
        case SDLK_DOWN:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_DOWN : HP48_KEY_DOWN );
        case SDLK_R:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_SQRT : HP48_KEY_RIGHT );
        case SDLK_RIGHT:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_RIGHT : HP48_KEY_RIGHT );
        case SDLK_S:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_SIN : HP48_KEY_SIN );
        case SDLK_T:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_COS : HP48_KEY_COS );
        case SDLK_U:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_TAN : HP48_KEY_TAN );
        case SDLK_V:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_EEX : HP48_KEY_SQRT );
        case SDLK_W:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_NEG : HP48_KEY_POWER );
        case SDLK_X:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_X : HP48_KEY_INV );
        case SDLK_Y:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_INV : HP48_KEY_NEG );
        case SDLK_Z:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_DIV : HP48_KEY_EEX );
        case SDLK_SPACE:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_SPC : HP48_KEY_SPC );
        case SDLK_F1:
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_ENTER : HP48_KEY_ENTER );
        case SDLK_BACKSPACE:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_BS : HP48_KEY_BS );
        case SDLK_DELETE:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? -1 : HP48_KEY_DEL );
        case SDLK_PERIOD:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_PERIOD : HP48_KEY_PERIOD );
        case SDLK_KP_PERIOD:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_PERIOD : HP48_KEY_PERIOD );
        case SDLK_PLUS:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_PLUS : HP48_KEY_PLUS );
        case SDLK_KP_PLUS:
        case SDLK_EQUALS:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_PLUS : HP48_KEY_PLUS );
        case SDLK_MINUS:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_MINUS : HP48_KEY_MINUS );
        case SDLK_KP_MINUS:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_MINUS : HP48_KEY_MINUS );
        case SDLK_ASTERISK:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_MUL : HP48_KEY_MUL );
        case SDLK_KP_MULTIPLY:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_MUL : HP48_KEY_MUL );
        case SDLK_SLASH:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_DIV : HP48_KEY_DIV );
        case SDLK_KP_DIVIDE:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_DIV : HP48_KEY_DIV );
        case SDLK_F5:
        case SDLK_ESCAPE:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_ON : HP48_KEY_ON );
        case SDLK_LSHIFT:
            if ( !__config.shiftless )
                return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_SHL : HP48_KEY_SHL );
            break;
        case SDLK_RSHIFT:
            if ( !__config.shiftless )
                return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_SHR : HP48_KEY_SHR );
            break;
        case SDLK_F2:
        case SDLK_RCTRL:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_SHL : HP48_KEY_SHL );
        case SDLK_F3:
        case SDLK_LCTRL:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_SHR : HP48_KEY_SHR );
        case SDLK_F4:
        case SDLK_LALT:
        case SDLK_RALT:
            return ( ( __config.model == MODEL_49G || __config.model == MODEL_50G ) ? HP49_KEY_ALPHA : HP48_KEY_ALPHA );
        case SDLK_F7:
        case SDLK_F10:
            close_and_exit();
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

    if ( __config.model == MODEL_48GX || __config.model == MODEL_48SX ) {
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
    if ( __config.model == MODEL_48GX )
        x -= 6;
    /* if ( __config.model == MODEL_49G ) */
    /*     x += ( WIDTH_DISPLAY / 2 ) - ( hp_width / 2 ); */

    __draw_bitmap( x, 10, hp_width, hp_height, hp_bitmap, UI4X_COLOR_HP_LOGO, UI4X_COLOR_HP_LOGO_BG );

    if ( __config.model == MODEL_49G )
        write_with_big_font( WIDTH_DISPLAY - PADDING_SIDE, 10, "HP 49GX", UI4X_COLOR_HP_LOGO, UI4X_COLOR_UPPER_FACEPLATE );

    if ( __config.model == MODEL_50G )
        write_with_big_font( WIDTH_DISPLAY - PADDING_SIDE, 10, "HP 50G", UI4X_COLOR_HP_LOGO, UI4X_COLOR_UPPER_FACEPLATE );

    // write the name of it
    if ( __config.model == MODEL_48GX ) {
        x = OFFSET_X_DISPLAY + WIDTH_DISPLAY - gx_128K_ram_width + gx_128K_ram_x_hot + 2;
        y = 10 + gx_128K_ram_y_hot;
        __draw_bitmap( x, y, gx_128K_ram_width, gx_128K_ram_height, gx_128K_ram_bitmap, UI4X_COLOR_48GX_128K_RAM,
                       UI4X_COLOR_UPPER_FACEPLATE );

        x = OFFSET_X_DISPLAY + hp_width;
        y = hp_height + 8 - hp48gx_height;
        __draw_bitmap( x, y, hp48gx_width, hp48gx_height, hp48gx_bitmap, UI4X_COLOR_HP_LOGO, UI4X_COLOR_UPPER_FACEPLATE );

        x = OFFSET_X_DISPLAY + WIDTH_DISPLAY - gx_128K_ram_width + gx_green_x_hot + 2;
        y = 10 + gx_green_y_hot;
        __draw_bitmap( x, y, gx_green_width, gx_green_height, gx_green_bitmap, UI4X_COLOR_SHIFT_RIGHT, UI4X_COLOR_UPPER_FACEPLATE );

        x = OFFSET_X_DISPLAY + WIDTH_DISPLAY - gx_128K_ram_width + gx_silver_x_hot + 2;
        y = 10 + gx_silver_y_hot;
        __draw_bitmap( x, y, gx_silver_width, gx_silver_height, gx_silver_bitmap, UI4X_COLOR_HP_LOGO,
                       UI4X_COLOR_LABEL ); // Background transparent: draw only silver line
    }
    if ( __config.model == MODEL_48SX ) {
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
    }
}

static SDL_Texture* create_button_texture( int hpkey, bool is_up )
{
    bool is_down = !is_up;
    int x, y;
    int on_key_offset_y = ( hpkey == HP48_KEY_ON || hpkey == HP49_KEY_ON ) ? 1 : 0;
    SDL_Texture* texture =
        SDL_CreateTexture( renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, BUTTONS[ hpkey ].w, BUTTONS[ hpkey ].h );
    SDL_SetRenderTarget( renderer, texture );

    // Fill the button and outline
    // fix outer-corners color
    int outer_color = UI4X_COLOR_FACEPLATE;
    if ( BUTTONS[ hpkey ].highlight )
        outer_color = UI4X_COLOR_KEYPAD_HIGHLIGHT;
    if ( ( __config.model == MODEL_48GX || __config.model == MODEL_48SX ) && hpkey < HP48_KEY_MTH )
        outer_color = UI4X_COLOR_UPPER_FACEPLATE;
    int inner_color = UI4X_COLOR_BUTTON;
    int edge_top_color = UI4X_COLOR_BUTTON_EDGE_TOP;
    int edge_bottom_color = UI4X_COLOR_BUTTON_EDGE_BOTTOM;
    if ( __config.model == MODEL_49G && ( hpkey == HP49_KEY_ALPHA || hpkey == HP49_KEY_SHL || hpkey == HP49_KEY_SHR ) ) {
        // inner_color = BUTTONS[ hpkey ].label_color;
        edge_top_color = BUTTONS[ hpkey ].label_color;
        edge_bottom_color = BUTTONS[ hpkey ].label_color;
    }

    __draw_rect( 0, 0, BUTTONS[ hpkey ].w, BUTTONS[ hpkey ].h, outer_color );
    __draw_rect( 1, 1, BUTTONS[ hpkey ].w - 2, BUTTONS[ hpkey ].h - 2, inner_color );

    // draw label in button
    int label_color = BUTTONS[ hpkey ].label_color;
    /* if ( __config.model == MODEL_49G && ( hpkey == HP49_KEY_ALPHA || hpkey == HP49_KEY_SHL || hpkey == HP49_KEY_SHR ) ) */
    /*     label_color = UI4X_COLOR_LABEL; */
    if ( BUTTONS[ hpkey ].label_text != ( char* )0 ) {
        /* Button has a text label */
        x = strlen( BUTTONS[ hpkey ].label_text ) - 1;
        x += ( ( BUTTONS[ hpkey ].w - BigTextWidth( BUTTONS[ hpkey ].label_text, strlen( BUTTONS[ hpkey ].label_text ) ) ) / 2 );
        y = ( BUTTONS[ hpkey ].h + 1 ) / 2 - 6;
        if ( is_down )
            y -= 1;

        write_with_big_font( x, y, BUTTONS[ hpkey ].label_text, label_color, inner_color );
    } else if ( BUTTONS[ hpkey ].label_graphic != ( unsigned char* )0 ) {
        /* Button has a texture */
        x = ( 1 + BUTTONS[ hpkey ].w - BUTTONS[ hpkey ].label_graphic_w ) / 2;
        y = ( 1 + BUTTONS[ hpkey ].h - BUTTONS[ hpkey ].label_graphic_h ) / 2;
        if ( is_up )
            y += 1;

        __draw_bitmap( x, y, BUTTONS[ hpkey ].label_graphic_w, BUTTONS[ hpkey ].label_graphic_h, BUTTONS[ hpkey ].label_graphic,
                       label_color, inner_color );
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
    if ( ( __config.model == MODEL_49G && hpkey == HP49_KEY_ON ) || hpkey == HP48_KEY_ON ) {
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
                    is_key_pressed( hpkey ) ? buttons_textures[ hpkey ].down : buttons_textures[ hpkey ].up );
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
    int pw = __config.model == MODEL_48GX ? 58 : 44;
    int ph = __config.model == MODEL_48GX ? 48 : 9;

    for ( int i = 0; i < NB_KEYS; i++ ) {
        // Background
        if ( BUTTONS[ i ].highlight ) {
            x = OFFSET_X_KEYBOARD + BUTTONS[ i ].x;
            y = OFFSET_Y_KEYBOARD + BUTTONS[ i ].y - SMALL_ASCENT - SMALL_DESCENT;

            if ( __config.model == MODEL_48GX ) {
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

            if ( __config.model == MODEL_48SX )
                y -= 2;

            write_with_small_font( x, y, BUTTONS[ i ].letter, UI4X_COLOR_ALPHA,
                                   ( ( __config.model == MODEL_48GX || __config.model == MODEL_48SX ) && i < HP48_KEY_MTH )
                                       ? UI4X_COLOR_UPPER_FACEPLATE
                                       : UI4X_COLOR_FACEPLATE );
        }

        // Bottom label: the only one is the cancel button
        if ( BUTTONS[ i ].sub != ( char* )0 ) {
            x = OFFSET_X_KEYBOARD + BUTTONS[ i ].x +
                ( 1 + BUTTONS[ i ].w - SmallTextWidth( BUTTONS[ i ].sub, strlen( BUTTONS[ i ].sub ) ) ) / 2;
            y = OFFSET_Y_KEYBOARD + BUTTONS[ i ].y + BUTTONS[ i ].h + SMALL_ASCENT + 1;

            write_with_small_font( x, y, BUTTONS[ i ].sub, UI4X_COLOR_LABEL, UI4X_COLOR_FACEPLATE );
        }

        total_top_labels_width = 0;
        // Draw the left labels
        if ( BUTTONS[ i ].left != ( char* )0 ) {
            x = OFFSET_X_KEYBOARD + BUTTONS[ i ].x;
            y = OFFSET_Y_KEYBOARD + BUTTONS[ i ].y - SMALL_DESCENT;

            left_label_width = SmallTextWidth( BUTTONS[ i ].left, strlen( BUTTONS[ i ].left ) );
            total_top_labels_width = left_label_width;

            if ( BUTTONS[ i ].right != ( char* )0 ) {
                // label to the left
                right_label_width = SmallTextWidth( BUTTONS[ i ].right, strlen( BUTTONS[ i ].right ) );
                total_top_labels_width += space_char_width + right_label_width;

                xr = OFFSET_X_KEYBOARD + BUTTONS[ i ].x;
            }
            if ( total_top_labels_width > BUTTONS[ i ].w || BUTTONS[ i ].right == ( char* )0 ) {
                x += ( 1 + BUTTONS[ i ].w - total_top_labels_width ) / 2;

                if ( BUTTONS[ i ].right != ( char* )0 ) {
                    xr += space_char_width + left_label_width;
                    xr += ( 1 + BUTTONS[ i ].w - total_top_labels_width ) / 2;
                }
            } else {
                x += 2;

                if ( BUTTONS[ i ].right != ( char* )0 )
                    xr = ( OFFSET_X_KEYBOARD + BUTTONS[ i ].x + BUTTONS[ i ].w ) - right_label_width;
            }

            write_with_small_font( x, y, BUTTONS[ i ].left, UI4X_COLOR_SHIFT_LEFT,
                                   BUTTONS[ i ].highlight ? UI4X_COLOR_KEYPAD_HIGHLIGHT : UI4X_COLOR_FACEPLATE );

            // draw the right labels ( .highlight never have one )
            if ( BUTTONS[ i ].right != ( char* )0 )
                write_with_small_font( xr, y, BUTTONS[ i ].right, UI4X_COLOR_SHIFT_RIGHT, UI4X_COLOR_FACEPLATE );
        }
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
    __draw_line( OFFSET_X_DISPLAY - 1, OFFSET_Y_DISPLAY + 1, OFFSET_X_DISPLAY - 1, OFFSET_Y_DISPLAY + HEIGHT_DISPLAY - 2, COLOR_PIXEL_OFF );
    __draw_line( OFFSET_X_DISPLAY + 1, OFFSET_Y_DISPLAY - 1, OFFSET_X_DISPLAY + WIDTH_DISPLAY - 2, OFFSET_Y_DISPLAY - 1, COLOR_PIXEL_OFF );
    __draw_line( OFFSET_X_DISPLAY + 1, OFFSET_Y_DISPLAY + HEIGHT_DISPLAY, OFFSET_X_DISPLAY + WIDTH_DISPLAY - 2,
                 OFFSET_Y_DISPLAY + HEIGHT_DISPLAY, COLOR_PIXEL_OFF );
    __draw_line( OFFSET_X_DISPLAY + WIDTH_DISPLAY, OFFSET_Y_DISPLAY + 1, OFFSET_X_DISPLAY + WIDTH_DISPLAY,
                 OFFSET_Y_DISPLAY + HEIGHT_DISPLAY - 2, COLOR_PIXEL_OFF );
}

static void _draw_background( int width, int height, int w_top, int h_top )
{
    __draw_rect( 0, 0, w_top, h_top, UI4X_COLOR_FACEPLATE );
    __draw_rect( 0, 0, width, height, UI4X_COLOR_UPPER_FACEPLATE );
}

static void _draw_background_LCD( void )
{
    __draw_rect( OFFSET_X_DISPLAY, OFFSET_Y_DISPLAY, WIDTH_DISPLAY, HEIGHT_DISPLAY, COLOR_PIXEL_OFF );
}

// Show the hp key which is being pressed
static void _show_key( int hpkey )
{
    if ( __config.chromeless || hpkey < 0 )
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

    if ( __config.verbose ) {
        fprintf( stderr, "wire_name: %s\n", __config.wire_name );
        fprintf( stderr, "ir_name: %s\n", __config.ir_name );
    }

    if ( __config.wire_name ) {
        strcat( text, "wire: " );
        strcat( text, __config.wire_name );
    }
    if ( __config.ir_name ) {
        if ( strlen( text ) > 0 )
            strcat( text, " | " );

        strcat( text, "IR: " );
        strcat( text, __config.ir_name );
    }

    if ( strlen( text ) > 0 )
        write_with_small_font(
            ( ( PADDING_SIDE * 1.5 ) + WIDTH_DISPLAY ) - SmallTextWidth( text, strlen( text ) ),
            ( __config.model == MODEL_49G ? PADDING_TOP - 12 : OFFSET_Y_KEYBOARD - ( PADDING_BETWEEN_DISPLAY_AND_KEYBOARD / 2 ) ), text,
            COLOR_PIXEL_OFF, UI4X_COLOR_UPPER_FACEPLATE );
}

static int sdl_press_key( int hpkey )
{
    if ( hpkey == -1 || is_key_pressed( hpkey ) )
        return -1;

    press_key( hpkey );
    _show_key( hpkey );

    return hpkey;
}

static int sdl_release_key( int hpkey )
{
    if ( hpkey == -1 || !is_key_pressed( hpkey ) )
        return -1;

    release_key( hpkey );
    _show_key( hpkey );

    return hpkey;
}

static void ui_init_LCD( void )
{
    memset( display_buffer_grayscale, 0, sizeof( display_buffer_grayscale ) );
    memset( display_buffer_past_two, 0, sizeof( display_buffer_past_two ) );
    memset( display_buffer_past_one, 0, sizeof( display_buffer_past_one ) );
    memset( display_buffer_current, 0, sizeof( display_buffer_current ) );
}

static void get_display_buffer( void )
{

    if ( get_display_state() ) {
        memcpy( &display_buffer_past_two, &display_buffer_past_one, LCD_WIDTH * 80 * sizeof( int ) );
        memcpy( &display_buffer_past_one, &display_buffer_current, LCD_WIDTH * 80 * sizeof( int ) );
        get_lcd_buffer( display_buffer_current );

        for ( int i = 0; i < LCD_WIDTH * 80; ++i )
            display_buffer_grayscale[ i ] = display_buffer_past_two[ i ] + display_buffer_past_one[ i ] + display_buffer_current[ i ];
    } else
        ui_init_LCD();
}

static void sdl_update_annunciators( void )
{
    const int annunciators_bits[ NB_ANNUNCIATORS ] = { ANN_LEFT, ANN_RIGHT, ANN_ALPHA, ANN_BATTERY, ANN_BUSY, ANN_IO };
    int annunciators = get_annunciators();

    if ( last_annunciators == annunciators )
        return;

    last_annunciators = annunciators;

    SDL_SetRenderTarget( renderer, main_texture );

    for ( int i = 0; i < NB_ANNUNCIATORS; i++ )
        __draw_texture( OFFSET_X_DISPLAY + annunciators_ui[ i ].x, OFFSET_Y_DISPLAY + annunciators_ui[ i ].y, annunciators_ui[ i ].width,
                        annunciators_ui[ i ].height,
                        ( ( ( annunciators_bits[ i ] & annunciators ) == annunciators_bits[ i ] ) ) ? annunciators_textures[ i ].up
                                                                                                    : annunciators_textures[ i ].down );

    // Always immediately update annunciators
    SDL_SetRenderTarget( renderer, NULL );
    SDL_RenderTexture( renderer, main_texture, NULL, NULL );
    SDL_RenderPresent( renderer );
}

static int apply_contrast( int value, int contrast )
{
    int max = 19;
    int min = 3;

    return ( value / ( max - min ) ) * ( max - contrast );
}

static void setup_colors( void )
{
    int contrast = get_contrast();

    if ( last_contrast == contrast )
        return;

    if ( contrast < 3 )
        contrast = 3;
    if ( contrast > 19 )
        contrast = 19;

    last_contrast = contrast;

    for ( unsigned i = 0; i < NB_COLORS; i++ ) {
        colors[ i ] = COLORS[ i ];

        if ( __config.mono ) {
            colors[ i ].r = colors[ i ].mono_rgb;
            colors[ i ].g = colors[ i ].mono_rgb;
            colors[ i ].b = colors[ i ].mono_rgb;
        } else {
            if ( __config.gray ) {
                colors[ i ].r = colors[ i ].gray_rgb;
                colors[ i ].g = colors[ i ].gray_rgb;
                colors[ i ].b = colors[ i ].gray_rgb;
            } else {
                if ( i == COLOR_PIXEL_ON || i == COLOR_PIXEL_GREY_2 || i == COLOR_PIXEL_GREY_1 ) {
                    // COLOR_PIXEL_ON, COLOR_PIXEL_GREY_2 and COLOR_PIXEL_GREY_1 are
                    // computed based on COLOR_PIXEL_OFF and contrast
                    colors[ i ].r = apply_contrast( colors[ COLOR_PIXEL_OFF ].r, contrast );
                    colors[ i ].g = apply_contrast( colors[ COLOR_PIXEL_OFF ].g, contrast );

                    if ( __config.black_lcd )
                        colors[ i ].b = apply_contrast( colors[ COLOR_PIXEL_OFF ].b, contrast );
                    else
                        colors[ i ].b = 128 - ( ( 19 - contrast ) * ( ( 128 - colors[ COLOR_PIXEL_OFF ].b ) / 16 ) );
                } else {
                    colors[ i ].r = ( colors[ i ].rgb >> 16 ) & 0xff;
                    colors[ i ].g = ( colors[ i ].rgb >> 8 ) & 0xff;
                    colors[ i ].b = colors[ i ].rgb & 0xff;
                }
            }
        }
    }

    // re-create annunciators textures
    last_annunciators = -1;
    create_annunciators_textures();
}

/**********/
/* public */
/**********/
void ui_get_event_sdl( void )
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
                close_and_exit();
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                hpkey = mouse_click_to_hpkey( event.button.x, event.button.y );
                if ( sdl_press_key( hpkey ) != -1 ) {
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
                    sdl_release_key( hpkey );

                lasthpkey = lastticks = -1;
                break;

            case SDL_EVENT_KEY_DOWN:
                sdl_press_key( sdlkey_to_hpkey( event.key.key ) );
                break;
            case SDL_EVENT_KEY_UP:
                sdl_release_key( sdlkey_to_hpkey( event.key.key ) );
                break;
        }
    }
}

void ui_update_display_sdl( void )
{
    setup_colors();

    get_display_buffer();

    SDL_SetRenderTarget( renderer, display_texture );

    int color = COLOR_PIXEL_OFF;
    for ( int y = 0; y < LCD_HEIGHT; ++y ) {
        for ( int x = 0; x < LCD_WIDTH; ++x ) {
            switch ( display_buffer_grayscale[ ( y * LCD_WIDTH ) + x ] ) {
                case 0:
                    color = COLOR_PIXEL_OFF;
                    break;
                case 1:
                    color = COLOR_PIXEL_GREY_1;
                    break;
                case 2:
                    color = COLOR_PIXEL_GREY_2;
                    break;
                case 3:
                default:
                    color = COLOR_PIXEL_ON;
                    break;
            }

            __draw_pixel( x, y, color );
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

void ui_start_sdl( config_t* conf )
{
    __config = *conf;

    ui_init_LCD();

    // Initialize SDL
    if ( !SDL_Init( SDL_INIT_VIDEO ) ) {
        printf( "Couldn't initialize SDL: %s\n", SDL_GetError() );
        exit( 1 );
    }

    // On exit: clean SDL
    atexit( SDL_Quit );

    unsigned int width =
        __config.chromeless ? WIDTH_DISPLAY : ( ( BUTTONS[ NB_KEYS - 1 ].x + BUTTONS[ NB_KEYS - 1 ].w ) + 2 * PADDING_SIDE );
    unsigned int height = __config.chromeless ? HEIGHT_DISPLAY
                                              : ( OFFSET_Y_DISPLAY + HEIGHT_DISPLAY + PADDING_BETWEEN_DISPLAY_AND_KEYBOARD +
                                                  BUTTONS[ NB_KEYS - 1 ].y + BUTTONS[ NB_KEYS - 1 ].h + PADDING_BOTTOM );

    uint32_t window_flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if ( __config.fullscreen )
        window_flags |= SDL_WINDOW_FULLSCREEN;
    else
        window_flags |= SDL_WINDOW_RESIZABLE;

    window = SDL_CreateWindow( __config.progname, width * __config.scale, height * __config.scale, window_flags );
    if ( window == NULL ) {
        printf( "Couldn't create window: %s\n", SDL_GetError() );
        exit( 1 );
    }

    renderer = SDL_CreateRenderer( window, NULL );
    if ( renderer == NULL )
        exit( 2 );

    // SDL_SetRenderLogicalPresentation( renderer, width, height, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE );
    SDL_SetRenderLogicalPresentation( renderer, width, height, SDL_LOGICAL_PRESENTATION_LETTERBOX );

    main_texture = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height );

    display_texture = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, LCD_WIDTH, LCD_HEIGHT );

    SDL_SetRenderTarget( renderer, main_texture );

    setup_colors();

    if ( !__config.chromeless ) {
        int cut = BUTTONS[ HP48_KEY_MTH ].y + OFFSET_Y_KEYBOARD - 19;

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

void ui_stop_sdl( void )
{
    SDL_DestroyTexture( main_texture );
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
}

void setup_frontend_sdl( void )
{
    ui_get_event = ui_get_event_sdl;
    ui_update_display = ui_update_display_sdl;
    ui_start = ui_start_sdl;
    ui_stop = ui_stop_sdl;
}
