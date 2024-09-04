#include <ctype.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h> /* lineColor(); pixelColor(); rectangleColor();stringColor(); */

#include "romio.h" /* opt_gx */
#include "config.h"
#include "ui.h"
#include "ui_inner.h"

#define KEYBOARD_HEIGHT ( BUTTONS[ LAST_HPKEY ].y + BUTTONS[ LAST_HPKEY ].h )
#define KEYBOARD_WIDTH ( BUTTONS[ LAST_HPKEY ].x + BUTTONS[ LAST_HPKEY ].w )

#define TOP_SKIP 65
#define SIDE_SKIP 20
#define BOTTOM_SKIP 25
#define DISP_KBD_SKIP 65
#define KBD_UPLINE 25

#define DISPLAY_WIDTH ( 264 + 8 )
#define DISPLAY_HEIGHT ( 128 + 16 + 8 )
#define DISPLAY_OFFSET_X ( SIDE_SKIP + ( 286 - DISPLAY_WIDTH ) / 2 )
#define DISPLAY_OFFSET_Y TOP_SKIP

#define DISP_FRAME 8

#define KEYBOARD_OFFSET_X SIDE_SKIP
#define KEYBOARD_OFFSET_Y ( TOP_SKIP + DISPLAY_HEIGHT + DISP_KBD_SKIP )

/***********/
/* typedef */
/***********/
typedef struct sdl_surfaces_textures_on_off_struct_t {
    SDL_Texture* textureon;
    SDL_Surface* surfaceon;
    SDL_Texture* textureoff;
    SDL_Surface* surfaceoff;
} sdl_surfaces_textures_on_off_struct_t;

/*************/
/* variables */
/*************/
static int display_offset_x, display_offset_y;

color_t colors[ NB_COLORS ];
/* static sdl_surfaces_on_off_struct_t buttons_surfaces[ NB_KEYS ]; */
static sdl_surfaces_textures_on_off_struct_t annunciators_surfaces_textures[ NB_ANNUNCIATORS ];

// State to displayed zoomed last pressed key
/* static SDL_Surface* showkeylastsurf = 0; */
/* static int showkeylastx, showkeylasty, showkeylastkey; */

static SDL_Window* window;
static SDL_Renderer* renderer;
static SDL_Texture* main_texture;

/****************************/
/* functions implementation */
/****************************/
static inline unsigned color2argb( int color )
{
    return 0x000000ff | ( colors[ color ].r << 24 ) | ( colors[ color ].g << 16 ) | ( colors[ color ].b << 8 );
}
static inline unsigned color2bgra( int color )
{
    return 0xff000000 | ( colors[ color ].r << 16 ) | ( colors[ color ].g << 8 ) | colors[ color ].b;
}

/*
        Create a surface from binary bitmap data
*/
static SDL_Surface* bitmap_to_surface( unsigned int w, unsigned int h, unsigned char* data, unsigned int coloron, unsigned int coloroff )
{
    SDL_Surface* surf;

    surf = SDL_CreateRGBSurface( SDL_SWSURFACE, w, h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 );

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

            lineptr[ x ] = ( b ) ? coloron : coloroff;
        }
    }

    SDL_UnlockSurface( surf );

    return surf;
}

/* static void write_text( int x, int y, const char* string, unsigned int length, unsigned int coloron, unsigned int coloroff ) */
/* { */
/*     int w, h; */

/*     for ( unsigned int i = 0; i < length; i++ ) { */
/*         if ( small_font[ ( int )string[ i ] ].h != 0 ) { */
/*             w = small_font[ ( int )string[ i ] ].w; */
/*             h = small_font[ ( int )string[ i ] ].h; */

/*             SDL_Surface* surf = bitmap_to_surface( w, h, small_font[ ( int )string[ i ] ].bits, coloron, coloroff ); */

/*             SDL_Rect srect; */
/*             SDL_Rect drect; */
/*             srect.x = 0; */
/*             srect.y = 0; */
/*             srect.w = w; */
/*             srect.h = h; */
/*             drect.x = x; */
/*             drect.y = ( int )( y - small_font[ ( int )string[ i ] ].h ); */
/*             drect.w = w; */
/*             drect.h = h; */
/*             SDL_BlitSurface( surf, &srect, window, &drect ); */
/*             SDL_FreeSurface( surf ); */
/*         } */

/*         x += SmallTextWidth( &string[ i ], 1 ); */
/*     } */
/* } */

static void colors_setup( void )
{
    // Adjust the LCD color according to the contrast
    int contrast = display.contrast;
    if ( contrast < 0x3 )
        contrast = 0x3;
    if ( contrast > 0x13 )
        contrast = 0x13;

    for ( unsigned i = FIRST_COLOR; i < LAST_COLOR; i++ ) {
        colors[ i ] = COLORS[ i ];
        if ( config.mono ) {
            colors[ i ].r = colors[ i ].mono_rgb;
            colors[ i ].g = colors[ i ].mono_rgb;
            colors[ i ].b = colors[ i ].mono_rgb;
        } else if ( config.gray ) {
            colors[ i ].r = colors[ i ].gray_rgb;
            colors[ i ].g = colors[ i ].gray_rgb;
            colors[ i ].b = colors[ i ].gray_rgb;
        }

        if ( !config.mono && i == PIXEL ) {
            colors[ i ].r = ( 0x13 - contrast ) * ( colors[ LCD ].r / 0x10 );
            colors[ i ].g = ( 0x13 - contrast ) * ( colors[ LCD ].g / 0x10 );
            colors[ i ].b = 128 - ( ( 0x13 - contrast ) * ( ( 128 - colors[ LCD ].b ) / 0x10 ) );
        }
    }
}

// This should be called once to setup the surfaces. Calling it multiple
// times is fine, it won't do anything on subsequent calls.
static void create_annunciators_surfaces_textures( void )
{
    for ( int i = 0; i < NB_ANNUNCIATORS; i++ ) {
        // If the SDL surface does not exist yet, we create it on the fly
        if ( annunciators_surfaces_textures[ i ].surfaceon ) {
            SDL_FreeSurface( annunciators_surfaces_textures[ i ].surfaceon );
            annunciators_surfaces_textures[ i ].surfaceon = 0;
        }

        annunciators_surfaces_textures[ i ].surfaceon =
            bitmap_to_surface( ann_tbl[ i ].width, ann_tbl[ i ].height, ann_tbl[ i ].bits, color2bgra( PIXEL ), color2bgra( LCD ) );
        annunciators_surfaces_textures[ i ].textureon =
            SDL_CreateTextureFromSurface( renderer, annunciators_surfaces_textures[ i ].surfaceon );

        if ( annunciators_surfaces_textures[ i ].surfaceoff ) {
            SDL_FreeSurface( annunciators_surfaces_textures[ i ].surfaceoff );
            annunciators_surfaces_textures[ i ].surfaceoff = 0;
        }

        annunciators_surfaces_textures[ i ].surfaceoff =
            bitmap_to_surface( ann_tbl[ i ].width, ann_tbl[ i ].height, ann_tbl[ i ].bits, color2bgra( LCD ), color2bgra( LCD ) );
        annunciators_surfaces_textures[ i ].textureoff =
            SDL_CreateTextureFromSurface( renderer, annunciators_surfaces_textures[ i ].surfaceoff );
    }
}

// Find which key is pressed, if any.
// Returns -1 is no key is pressed
static int mouse_click_to_hpkey( unsigned int x, unsigned int y )
{
    /* return immediatly if the click isn't even in the keyboard area */
    if ( y < KEYBOARD_OFFSET_Y )
        return -1;

    int row = ( y - KEYBOARD_OFFSET_Y ) / ( KEYBOARD_HEIGHT / 9 );
    int column;
    switch ( row ) {
        case 0:
        case 1:
        case 2:
        case 3:
            column = ( x - KEYBOARD_OFFSET_X ) / ( KEYBOARD_WIDTH / 6 );
            return ( row * 6 ) + column;
        case 4: /* with [ENTER] key */
            column = ( ( x - KEYBOARD_OFFSET_X ) / ( KEYBOARD_WIDTH / 5 ) ) - 1;
            if ( column < 0 )
                column = 0;
            return ( 4 * 6 ) + column;
        case 5:
        case 6:
        case 7:
        case 8:
            column = ( x - KEYBOARD_OFFSET_X ) / ( KEYBOARD_WIDTH / 5 );
            return ( 4 * 6 ) + 5 + ( ( row - 5 ) * 5 ) + column;

        default:
            return -1;
    }

    return -1;
}

// Map the keyboard keys to the HP keys
// Returns -1 if there is no mapping
static int sdlkey_to_hpkey( SDL_Keycode k )
{
    switch ( k ) {
        case SDLK_0:
            return HPKEY_0;
        case SDLK_1:
            return HPKEY_1;
        case SDLK_2:
            return HPKEY_2;
        case SDLK_3:
            return HPKEY_3;
        case SDLK_4:
            return HPKEY_4;
        case SDLK_5:
            return HPKEY_5;
        case SDLK_6:
            return HPKEY_6;
        case SDLK_7:
            return HPKEY_7;
        case SDLK_8:
            return HPKEY_8;
        case SDLK_9:
            return HPKEY_9;
        case SDLK_KP_0:
            return HPKEY_0;
        case SDLK_KP_1:
            return HPKEY_1;
        case SDLK_KP_2:
            return HPKEY_2;
        case SDLK_KP_3:
            return HPKEY_3;
        case SDLK_KP_4:
            return HPKEY_4;
        case SDLK_KP_5:
            return HPKEY_5;
        case SDLK_KP_6:
            return HPKEY_6;
        case SDLK_KP_7:
            return HPKEY_7;
        case SDLK_KP_8:
            return HPKEY_8;
        case SDLK_KP_9:
            return HPKEY_9;
        case SDLK_a:
            return HPKEY_A;
        case SDLK_b:
            return HPKEY_B;
        case SDLK_c:
            return HPKEY_C;
        case SDLK_d:
            return HPKEY_D;
        case SDLK_e:
            return HPKEY_E;
        case SDLK_f:
            return HPKEY_F;
        case SDLK_g:
            return HPKEY_MTH;
        case SDLK_h:
            return HPKEY_PRG;
        case SDLK_i:
            return HPKEY_CST;
        case SDLK_j:
            return HPKEY_VAR;
        case SDLK_k:
            return HPKEY_UP;
        case SDLK_UP:
            return HPKEY_UP;
        case SDLK_l:
            return HPKEY_NXT;
        case SDLK_m:
            return HPKEY_COLON;
        case SDLK_n:
            return HPKEY_STO;
        case SDLK_o:
            return HPKEY_EVAL;
        case SDLK_p:
            return HPKEY_LEFT;
        case SDLK_LEFT:
            return HPKEY_LEFT;
        case SDLK_q:
            return HPKEY_DOWN;
        case SDLK_DOWN:
            return HPKEY_DOWN;
        case SDLK_r:
            return HPKEY_RIGHT;
        case SDLK_RIGHT:
            return HPKEY_RIGHT;
        case SDLK_s:
            return HPKEY_SIN;
        case SDLK_t:
            return HPKEY_COS;
        case SDLK_u:
            return HPKEY_TAN;
        case SDLK_v:
            return HPKEY_SQRT;
        case SDLK_w:
            return HPKEY_POWER;
        case SDLK_x:
            return HPKEY_INV;
        case SDLK_y:
            return HPKEY_NEG;
        case SDLK_z:
            return HPKEY_EEX;
        case SDLK_SPACE:
            return HPKEY_SPC;
        case SDLK_F1:
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            return HPKEY_ENTER;
        case SDLK_BACKSPACE:
            return HPKEY_BS;
        case SDLK_DELETE:
            return HPKEY_DEL;
        case SDLK_PERIOD:
            return HPKEY_PERIOD;
        case SDLK_KP_PERIOD:
            return HPKEY_PERIOD;
        case SDLK_PLUS:
            return HPKEY_PLUS;
        case SDLK_KP_PLUS:
            return HPKEY_PLUS;
        case SDLK_MINUS:
            return HPKEY_MINUS;
        case SDLK_KP_MINUS:
            return HPKEY_MINUS;
        case SDLK_ASTERISK:
            return HPKEY_MUL;
        case SDLK_KP_MULTIPLY:
            return HPKEY_MUL;
        case SDLK_SLASH:
            return HPKEY_DIV;
        case SDLK_KP_DIVIDE:
            return HPKEY_DIV;
        case SDLK_F5:
        case SDLK_ESCAPE:
            return HPKEY_ON;
        case SDLK_LSHIFT:
            if ( !config.leave_shift_keys )
                return HPKEY_SHL;
            break;
        case SDLK_RSHIFT:
            if ( !config.leave_shift_keys )
                return HPKEY_SHR;
            break;
        case SDLK_F2:
        case SDLK_RCTRL:
            return HPKEY_SHL;
        case SDLK_F3:
        case SDLK_LCTRL:
            return HPKEY_SHR;
        case SDLK_F4:
        case SDLK_LALT:
        case SDLK_RALT:
            return HPKEY_ALPHA;
        case SDLK_F7:
        case SDLK_F10:
            // please_exit = true;
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
    lineColor( renderer, 1, keypad_height - 1, keypad_width - 1, keypad_height - 1, color2argb( PAD_TOP ) );
    lineColor( renderer, 2, keypad_height - 2, keypad_width - 2, keypad_height - 2, color2argb( PAD_TOP ) );

    // right lines
    lineColor( renderer, keypad_width - 1, keypad_height - 1, keypad_width - 1, cut, color2argb( PAD_TOP ) );
    lineColor( renderer, keypad_width - 2, keypad_height - 2, keypad_width - 2, cut, color2argb( PAD_TOP ) );

    // right lines
    lineColor( renderer, keypad_width - 1, cut - 1, keypad_width - 1, 1, color2argb( DISP_PAD_TOP ) );
    lineColor( renderer, keypad_width - 2, cut - 1, keypad_width - 2, 2, color2argb( DISP_PAD_TOP ) );

    // top lines
    lineColor( renderer, 0, 0, keypad_width - 2, 0, color2argb( DISP_PAD_BOT ) );
    lineColor( renderer, 1, 1, keypad_width - 3, 1, color2argb( DISP_PAD_BOT ) );

    // left lines
    lineColor( renderer, 0, cut - 1, 0, 0, color2argb( DISP_PAD_BOT ) );
    lineColor( renderer, 1, cut - 1, 1, 1, color2argb( DISP_PAD_BOT ) );

    // left lines
    lineColor( renderer, 0, keypad_height - 2, 0, cut, color2argb( PAD_BOT ) );
    lineColor( renderer, 1, keypad_height - 3, 1, cut, color2argb( PAD_BOT ) );

    // lower the menu BUTTONS

    // bottom lines
    lineColor( renderer, 3, keypad_height - 3, keypad_width - 3, keypad_height - 3, color2argb( PAD_TOP ) );
    lineColor( renderer, 4, keypad_height - 4, keypad_width - 4, keypad_height - 4, color2argb( PAD_TOP ) );

    // right lines
    lineColor( renderer, keypad_width - 3, keypad_height - 3, keypad_width - 3, cut, color2argb( PAD_TOP ) );
    lineColor( renderer, keypad_width - 4, keypad_height - 4, keypad_width - 4, cut, color2argb( PAD_TOP ) );

    // right lines
    lineColor( renderer, keypad_width - 3, cut - 1, keypad_width - 3, offset_y - ( KBD_UPLINE - 1 ), color2argb( DISP_PAD_TOP ) );
    lineColor( renderer, keypad_width - 4, cut - 1, keypad_width - 4, offset_y - ( KBD_UPLINE - 2 ), color2argb( DISP_PAD_TOP ) );

    // top lines
    lineColor( renderer, 2, offset_y - ( KBD_UPLINE - 0 ), keypad_width - 4, offset_y - ( KBD_UPLINE - 0 ), color2argb( DISP_PAD_BOT ) );
    lineColor( renderer, 3, offset_y - ( KBD_UPLINE - 1 ), keypad_width - 5, offset_y - ( KBD_UPLINE - 1 ), color2argb( DISP_PAD_BOT ) );

    // left lines
    lineColor( renderer, 2, cut - 1, 2, offset_y - ( KBD_UPLINE - 1 ), color2argb( DISP_PAD_BOT ) );
    lineColor( renderer, 3, cut - 1, 3, offset_y - ( KBD_UPLINE - 2 ), color2argb( DISP_PAD_BOT ) );

    // left lines
    lineColor( renderer, 2, keypad_height - 4, 2, cut, color2argb( PAD_BOT ) );
    lineColor( renderer, 3, keypad_height - 5, 3, cut, color2argb( PAD_BOT ) );

    // lower the keyboard

    // bottom lines
    lineColor( renderer, 5, keypad_height - 5, keypad_width - 3, keypad_height - 5, color2argb( PAD_TOP ) );
    lineColor( renderer, 6, keypad_height - 6, keypad_width - 4, keypad_height - 6, color2argb( PAD_TOP ) );

    // right lines
    lineColor( renderer, keypad_width - 5, keypad_height - 5, keypad_width - 5, cut + 1, color2argb( PAD_TOP ) );
    lineColor( renderer, keypad_width - 6, keypad_height - 6, keypad_width - 6, cut + 2, color2argb( PAD_TOP ) );

    // top lines
    lineColor( renderer, 4, cut, keypad_width - 6, cut, color2argb( DISP_PAD_BOT ) );
    lineColor( renderer, 5, cut + 1, keypad_width - 7, cut + 1, color2argb( DISP_PAD_BOT ) );

    // left lines
    lineColor( renderer, 4, keypad_height - 6, 4, cut + 1, color2argb( PAD_BOT ) );
    lineColor( renderer, 5, keypad_height - 7, 5, cut + 2, color2argb( PAD_BOT ) );

    // round off the bottom edge

    lineColor( renderer, keypad_width - 7, keypad_height - 7, keypad_width - 7, keypad_height - 14, color2argb( PAD_TOP ) );
    lineColor( renderer, keypad_width - 8, keypad_height - 8, keypad_width - 8, keypad_height - 11, color2argb( PAD_TOP ) );
    lineColor( renderer, keypad_width - 7, keypad_height - 7, keypad_width - 14, keypad_height - 7, color2argb( PAD_TOP ) );
    lineColor( renderer, keypad_width - 7, keypad_height - 8, keypad_width - 11, keypad_height - 8, color2argb( PAD_TOP ) );
    pixelColor( renderer, keypad_width - 9, keypad_height - 9, color2argb( PAD_TOP ) );

    lineColor( renderer, 7, keypad_height - 7, 13, keypad_height - 7, color2argb( PAD_TOP ) );
    lineColor( renderer, 8, keypad_height - 8, 10, keypad_height - 8, color2argb( PAD_TOP ) );

    lineColor( renderer, 6, keypad_height - 8, 6, keypad_height - 14, color2argb( PAD_BOT ) );
    lineColor( renderer, 7, keypad_height - 9, 7, keypad_height - 11, color2argb( PAD_BOT ) );
}

static void _draw_header( void )
{
    /* int x, y; */
    /* SDL_Surface* surf; */

    /* int display_width = DISPLAY_WIDTH; */

    /* // insert the HP Logo */
    /* surf = bitmap_to_surface( hp_width, hp_height, hp_bitmap, ARGBColors[ LOGO ], ARGBColors[ LOGO_BACK ] ); */
    /* if ( opt_gx ) */
    /*     x = display_offset_x - 6; */
    /* else */
    /*     x = display_offset_x; */

    /* SDL_Rect srect; */
    /* SDL_Rect drect; */
    /* srect.x = 0; */
    /* srect.y = 0; */
    /* srect.w = hp_width; */
    /* srect.h = hp_height; */
    /* drect.x = x; */
    /* drect.y = 10; */
    /* drect.w = hp_width; */
    /* drect.h = hp_height; */
    /* SDL_BlitSurface( surf, &srect, window, &drect ); */
    /* SDL_FreeSurface( surf ); */

    if ( !opt_gx ) {
        lineColor( renderer, display_offset_x, 9, display_offset_x + hp_width - 1, 9, color2argb( FRAME ) );
        lineColor( renderer, display_offset_x - 1, 10, display_offset_x - 1, 10 + hp_height - 1, color2argb( FRAME ) );
        lineColor( renderer, display_offset_x, 10 + hp_height, display_offset_x + hp_width - 1, 10 + hp_height, color2argb( FRAME ) );
        lineColor( renderer, display_offset_x + hp_width, 10, display_offset_x + hp_width, 10 + hp_height - 1, color2argb( FRAME ) );
    }

    // write the name of it
    /*     if ( opt_gx ) { */
    /*         x = display_offset_x + display_width - gx_128K_ram_width + gx_128K_ram_x_hot + 2; */
    /*         y = 10 + gx_128K_ram_y_hot; */

    /*         surf = bitmap_to_surface( gx_128K_ram_width, gx_128K_ram_height, gx_128K_ram_bitmap, ARGBColors[ LABEL ], ARGBColors[
     * DISP_PAD ] */
    /* ); */
    /*         srect.x = 0; */
    /*         srect.y = 0; */
    /*         srect.w = gx_128K_ram_width; */
    /*         srect.h = gx_128K_ram_height; */
    /*         drect.x = x; */
    /*         drect.y = y; */
    /*         drect.w = gx_128K_ram_width; */
    /*         drect.h = gx_128K_ram_height; */
    /*         SDL_BlitSurface( surf, &srect, window, &drect ); */
    /*         SDL_FreeSurface( surf ); */

    /*         x = display_offset_x + hp_width; */
    /*         y = hp_height + 8 - hp48gx_height; */
    /*         surf = bitmap_to_surface( hp48gx_width, hp48gx_height, hp48gx_bitmap, ARGBColors[ LOGO ], ARGBColors[ DISP_PAD ] ); */
    /*         srect.x = 0; */
    /*         srect.y = 0; */
    /*         srect.w = hp48gx_width; */
    /*         srect.h = hp48gx_height; */
    /*         drect.x = x; */
    /*         drect.y = y; */
    /*         drect.w = hp48gx_width; */
    /*         drect.h = hp48gx_height; */
    /*         SDL_BlitSurface( surf, &srect, window, &drect ); */
    /*         SDL_FreeSurface( surf ); */

    /*         x = display_offset_x + DISPLAY_WIDTH - gx_128K_ram_width + gx_silver_x_hot + 2; */
    /*         y = 10 + gx_silver_y_hot; */
    /*         surf = bitmap_to_surface( gx_silver_width, gx_silver_height, gx_silver_bitmap, ARGBColors[ LOGO ], */
    /*                                   0 ); // Background transparent: draw only silver line */
    /*         srect.x = 0; */
    /*         srect.y = 0; */
    /*         srect.w = gx_silver_width; */
    /*         srect.h = gx_silver_height; */
    /*         drect.x = x; */
    /*         drect.y = y; */
    /*         drect.w = gx_silver_width; */
    /*         drect.h = gx_silver_height; */
    /*         SDL_BlitSurface( surf, &srect, window, &drect ); */
    /*         SDL_FreeSurface( surf ); */

    /*         x = display_offset_x + display_width - gx_128K_ram_width + gx_green_x_hot + 2; */
    /*         y = 10 + gx_green_y_hot; */
    /*         surf = bitmap_to_surface( gx_green_width, gx_green_height, gx_green_bitmap, ARGBColors[ RIGHT ], */
    /*                                   0 ); // Background transparent: draw only green menu */
    /*         srect.x = 0; */
    /*         srect.y = 0; */
    /*         srect.w = gx_green_width; */
    /*         srect.h = gx_green_height; */
    /*         drect.x = x; */
    /*         drect.y = y; */
    /*         drect.w = gx_green_width; */
    /*         drect.h = gx_green_height; */
    /*         SDL_BlitSurface( surf, &srect, window, &drect ); */
    /*         SDL_FreeSurface( surf ); */
    /*     } else { */
    /*         x = display_offset_x; */
    /*         y = TOP_SKIP - DISP_FRAME - hp48sx_height - 3; */
    /*         surf = bitmap_to_surface( hp48sx_width, hp48sx_height, hp48sx_bitmap, ARGBColors[ RIGHT ], */
    /*                                   0 ); // Background transparent: draw only green menu */
    /*         srect.x = 0; */
    /*         srect.y = 0; */
    /*         srect.w = hp48sx_width; */
    /*         srect.h = hp48sx_height; */
    /*         drect.x = x; */
    /*         drect.y = y; */
    /*         drect.w = hp48sx_width; */
    /*         drect.h = hp48sx_height; */
    /*         SDL_BlitSurface( surf, &srect, window, &drect ); */
    /*         SDL_FreeSurface( surf ); */

    /*         x = display_offset_x + display_width - 1 - science_width; */
    /*         y = TOP_SKIP - DISP_FRAME - science_height - 4; */
    /*         surf = bitmap_to_surface( science_width, science_height, science_bitmap, ARGBColors[ RIGHT ], */
    /*                                   0 ); // Background transparent: draw only green menu */
    /*         srect.x = 0; */
    /*         srect.y = 0; */
    /*         srect.w = science_width; */
    /*         srect.h = science_height; */
    /*         drect.x = x; */
    /*         drect.y = y; */
    /*         drect.w = science_width; */
    /*         drect.h = science_height; */
    /*         SDL_BlitSurface( surf, &srect, window, &drect ); */
    /*         SDL_FreeSurface( surf ); */
    /*     } */
}

/* static void __create_buttons( void ) */
/* { */
/*     unsigned i, x, y; */
/*     unsigned pixel; */

/*     for ( i = FIRST_HPKEY; i <= LAST_HPKEY; i++ ) { */
/*         // Create surfaces for each button */
/*         if ( !buttons_surfaces[ i ].surfaceon ) */
/*             buttons_surfaces[ i ].surfaceon = */
/*                 SDL_CreateRGBSurface( SDL_SWSURFACE, BUTTONS[ i ].w, BUTTONS[ i ].h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000
 * ); */

/*         if ( !buttons_surfaces[ i ].surfaceoff ) */
/*             buttons_surfaces[ i ].surfaceoff = */
/*                 SDL_CreateRGBSurface( SDL_SWSURFACE, BUTTONS[ i ].w, BUTTONS[ i ].h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000
 * ); */

/*         // Use alpha channel */
/*         pixel = 0x00000000; */
/*         // pixel = 0xffff0000; */

/*         // Fill the button and outline */
/*         SDL_FillRect( buttons_surfaces[ i ].surfaceon, 0, pixel ); */
/*         SDL_FillRect( buttons_surfaces[ i ].surfaceoff, 0, pixel ); */

/*         SDL_Rect rect; */
/*         rect.x = 1; */
/*         rect.y = 1; */
/*         rect.w = BUTTONS[ i ].w - 2; */
/*         rect.h = BUTTONS[ i ].h - 2; */
/*         SDL_FillRect( buttons_surfaces[ i ].surfaceon, &rect, ARGBColors[ BUTTON ] ); */
/*         SDL_FillRect( buttons_surfaces[ i ].surfaceoff, &rect, ARGBColors[ BUTTON ] ); */

/*         // draw the released button */
/*         // draw edge of button */
/*         lineColor( buttons_surfaces[ i ].surfaceon, 1, BUTTONS[ i ].h - 2, 1, 1, color2argb( BUT_TOP ) ); */
/*         lineColor( buttons_surfaces[ i ].surfaceon, 2, BUTTONS[ i ].h - 3, 2, 2, color2argb( BUT_TOP ) ); */
/*         lineColor( buttons_surfaces[ i ].surfaceon, 3, BUTTONS[ i ].h - 4, 3, 3, color2argb( BUT_TOP ) ); */

/*         lineColor( buttons_surfaces[ i ].surfaceon, 1, 1, BUTTONS[ i ].w - 2, 1, color2argb( BUT_TOP ) ); */
/*         lineColor( buttons_surfaces[ i ].surfaceon, 2, 2, BUTTONS[ i ].w - 3, 2, color2argb( BUT_TOP ) ); */
/*         lineColor( buttons_surfaces[ i ].surfaceon, 3, 3, BUTTONS[ i ].w - 4, 3, color2argb( BUT_TOP ) ); */
/*         lineColor( buttons_surfaces[ i ].surfaceon, 4, 4, BUTTONS[ i ].w - 5, 4, color2argb( BUT_TOP ) ); */

/*         pixelColor( buttons_surfaces[ i ].surfaceon, 4, 5, color2argb( BUT_TOP ) ); */

/*         lineColor( buttons_surfaces[ i ].surfaceon, 3, BUTTONS[ i ].h - 2, BUTTONS[ i ].w - 2, BUTTONS[ i ].h - 2, */
/*                    color2argb( BUT_BOT ) ); */
/*         lineColor( buttons_surfaces[ i ].surfaceon, 4, BUTTONS[ i ].h - 3, BUTTONS[ i ].w - 3, BUTTONS[ i ].h - 3, */
/*                    color2argb( BUT_BOT ) ); */

/*         lineColor( buttons_surfaces[ i ].surfaceon, BUTTONS[ i ].w - 2, BUTTONS[ i ].h - 2, BUTTONS[ i ].w - 2, 3, */
/*                    color2argb( BUT_BOT ) ); */
/*         lineColor( buttons_surfaces[ i ].surfaceon, BUTTONS[ i ].w - 3, BUTTONS[ i ].h - 3, BUTTONS[ i ].w - 3, 4, */
/*                    color2argb( BUT_BOT ) ); */
/*         lineColor( buttons_surfaces[ i ].surfaceon, BUTTONS[ i ].w - 4, BUTTONS[ i ].h - 4, BUTTONS[ i ].w - 4, 5, */
/*                    color2argb( BUT_BOT ) ); */
/*         pixelColor( buttons_surfaces[ i ].surfaceon, BUTTONS[ i ].w - 5, BUTTONS[ i ].h - 4, color2argb( BUT_BOT ) ); */

/*         // draw frame around button */

/*         lineColor( buttons_surfaces[ i ].surfaceon, 0, BUTTONS[ i ].h - 3, 0, 2, color2argb( FRAME ) ); */
/*         lineColor( buttons_surfaces[ i ].surfaceon, 2, 0, BUTTONS[ i ].w - 3, 0, color2argb( FRAME ) ); */
/*         lineColor( buttons_surfaces[ i ].surfaceon, 2, BUTTONS[ i ].h - 1, BUTTONS[ i ].w - 3, BUTTONS[ i ].h - 1, */
/*                    color2argb( FRAME ) ); */
/*         lineColor( buttons_surfaces[ i ].surfaceon, BUTTONS[ i ].w - 1, BUTTONS[ i ].h - 3, BUTTONS[ i ].w - 1, 2, */
/*                    color2argb( FRAME ) ); */
/*         if ( i == HPKEY_ON ) { */
/*             lineColor( buttons_surfaces[ i ].surfaceon, 1, 1, BUTTONS[ 1 ].w - 2, 1, color2argb( FRAME ) ); */
/*             pixelColor( buttons_surfaces[ i ].surfaceon, 1, 2, color2argb( FRAME ) ); */
/*             pixelColor( buttons_surfaces[ i ].surfaceon, BUTTONS[ i ].w - 2, 2, color2argb( FRAME ) ); */
/*         } else { */
/*             pixelColor( buttons_surfaces[ i ].surfaceon, 1, 1, color2argb( FRAME ) ); */
/*             pixelColor( buttons_surfaces[ i ].surfaceon, BUTTONS[ i ].w - 2, 1, color2argb( FRAME ) ); */
/*         } */
/*         pixelColor( buttons_surfaces[ i ].surfaceon, 1, BUTTONS[ i ].h - 2, color2argb( FRAME ) ); */
/*         pixelColor( buttons_surfaces[ i ].surfaceon, BUTTONS[ i ].w - 2, BUTTONS[ i ].h - 2, color2argb( FRAME ) ); */

/*         // draw the depressed button */

/*         // draw edge of button */
/*         lineColor( buttons_surfaces[ i ].surfaceoff, 2, BUTTONS[ i ].h - 4, 2, 2, color2argb( BUT_TOP ) ); */
/*         lineColor( buttons_surfaces[ i ].surfaceoff, 3, BUTTONS[ i ].h - 5, 3, 3, color2argb( BUT_TOP ) ); */
/*         lineColor( buttons_surfaces[ i ].surfaceoff, 2, 2, BUTTONS[ i ].w - 4, 2, color2argb( BUT_TOP ) ); */
/*         lineColor( buttons_surfaces[ i ].surfaceoff, 3, 3, BUTTONS[ i ].w - 5, 3, color2argb( BUT_TOP ) ); */
/*         pixelColor( buttons_surfaces[ i ].surfaceoff, 4, 4, color2argb( BUT_TOP ) ); */

/*         lineColor( buttons_surfaces[ i ].surfaceoff, 3, BUTTONS[ i ].h - 3, BUTTONS[ i ].w - 3, BUTTONS[ i ].h - 3, */
/*                    color2argb( BUT_BOT ) ); */
/*         lineColor( buttons_surfaces[ i ].surfaceoff, 4, BUTTONS[ i ].h - 4, BUTTONS[ i ].w - 4, BUTTONS[ i ].h - 4, */
/*                    color2argb( BUT_BOT ) ); */
/*         lineColor( buttons_surfaces[ i ].surfaceoff, BUTTONS[ i ].w - 3, BUTTONS[ i ].h - 3, BUTTONS[ i ].w - 3, 3, */
/*                    color2argb( BUT_BOT ) ); */
/*         lineColor( buttons_surfaces[ i ].surfaceoff, BUTTONS[ i ].w - 4, BUTTONS[ i ].h - 4, BUTTONS[ i ].w - 4, 4, */
/*                    color2argb( BUT_BOT ) ); */
/*         pixelColor( buttons_surfaces[ i ].surfaceoff, BUTTONS[ i ].w - 5, BUTTONS[ i ].h - 5, color2argb( BUT_BOT ) ); */

/*         // draw frame around button */
/*         lineColor( buttons_surfaces[ i ].surfaceoff, 0, BUTTONS[ i ].h - 3, 0, 2, color2argb( FRAME ) ); */
/*         lineColor( buttons_surfaces[ i ].surfaceoff, 2, 0, BUTTONS[ i ].w - 3, 0, color2argb( FRAME ) ); */
/*         lineColor( buttons_surfaces[ i ].surfaceoff, 2, BUTTONS[ i ].h - 1, BUTTONS[ i ].w - 3, BUTTONS[ i ].h - 1, */
/*                    color2argb( FRAME ) ); */
/*         lineColor( buttons_surfaces[ i ].surfaceoff, BUTTONS[ i ].w - 1, BUTTONS[ i ].h - 3, BUTTONS[ i ].w - 1, 2, */
/*                    color2argb( FRAME ) ); */

/*         if ( i == HPKEY_ON ) { */
/*             lineColor( buttons_surfaces[ i ].surfaceoff, 1, 1, BUTTONS[ i ].w - 2, 1, color2argb( FRAME ) ); */
/*             pixelColor( buttons_surfaces[ i ].surfaceoff, 1, 2, color2argb( FRAME ) ); */
/*             pixelColor( buttons_surfaces[ i ].surfaceoff, BUTTONS[ i ].w - 2, 2, color2argb( FRAME ) ); */
/*         } else { */
/*             pixelColor( buttons_surfaces[ i ].surfaceoff, 1, 1, color2argb( FRAME ) ); */
/*             pixelColor( buttons_surfaces[ i ].surfaceoff, BUTTONS[ i ].w - 2, 1, color2argb( FRAME ) ); */
/*         } */
/*         pixelColor( buttons_surfaces[ i ].surfaceoff, 1, BUTTONS[ i ].h - 2, color2argb( FRAME ) ); */
/*         pixelColor( buttons_surfaces[ i ].surfaceoff, BUTTONS[ i ].w - 2, BUTTONS[ i ].h - 2, color2argb( FRAME ) ); */
/*         if ( i == HPKEY_ON ) { */
/*             rectangleColor( buttons_surfaces[ i ].surfaceoff, 1, 2, 1 + BUTTONS[ i ].w - 3, 2 + BUTTONS[ i ].h - 4, */
/*                             color2argb( FRAME ) ); */
/*             pixelColor( buttons_surfaces[ i ].surfaceoff, 2, 3, color2argb( FRAME ) ); */
/*             pixelColor( buttons_surfaces[ i ].surfaceoff, BUTTONS[ i ].w - 3, 3, color2argb( FRAME ) ); */
/*         } else { */
/*             rectangleColor( buttons_surfaces[ i ].surfaceoff, 1, 1, 1 + BUTTONS[ i ].w - 3, 1 + BUTTONS[ i ].h - 3, */
/*                             color2argb( FRAME ) ); */
/*             pixelColor( buttons_surfaces[ i ].surfaceoff, 2, 2, color2argb( FRAME ) ); */
/*             pixelColor( buttons_surfaces[ i ].surfaceoff, BUTTONS[ i ].w - 3, 2, color2argb( FRAME ) ); */
/*         } */
/*         pixelColor( buttons_surfaces[ i ].surfaceoff, 2, BUTTONS[ i ].h - 3, color2argb( FRAME ) ); */
/*         pixelColor( buttons_surfaces[ i ].surfaceoff, BUTTONS[ i ].w - 3, BUTTONS[ i ].h - 3, color2argb( FRAME ) ); */

/*         if ( BUTTONS[ i ].label != ( char* )0 ) { */
/*             // Todo: use SDL_ttf to print "nice" fonts */

/*             // for the time being use SDL_gfxPrimitives' font */
/*             x = ( BUTTONS[ i ].w - strlen( BUTTONS[ i ].label ) * 8 ) / 2; */
/*             y = ( BUTTONS[ i ].h + 1 ) / 2 - 4; */
/*             stringColor( buttons_surfaces[ i ].surfaceon, x, y, BUTTONS[ i ].label, 0xffffffff ); */
/*             stringColor( buttons_surfaces[ i ].surfaceoff, x, y, BUTTONS[ i ].label, 0xffffffff ); */
/*         } */
/*         // Pixmap centered in button */
/*         if ( BUTTONS[ i ].lw != 0 ) { */
/*             // If there's a bitmap, try to plot this */
/*             unsigned colorbg = ARGBColors[ BUTTON ]; */
/*             unsigned colorfg = ARGBColors[ BUTTONS[ i ].lc ]; */

/*             // Blit the label surface to the button */
/*             SDL_Surface* surf; */
/*             surf = bitmap_to_surface( BUTTONS[ i ].lw, BUTTONS[ i ].lh, BUTTONS[ i ].lb, colorfg, colorbg ); */
/*             // Draw the surface on the center of the button */
/*             x = ( 1 + BUTTONS[ i ].w - BUTTONS[ i ].lw ) / 2; */
/*             y = ( 1 + BUTTONS[ i ].h - BUTTONS[ i ].lh ) / 2 + 1; */
/*             SDL_Rect srect; */
/*             SDL_Rect drect; */
/*             srect.x = 0; */
/*             srect.y = 0; */
/*             srect.w = BUTTONS[ i ].lw; */
/*             srect.h = BUTTONS[ i ].lh; */
/*             drect.x = x; */
/*             drect.y = y; */
/*             drect.w = BUTTONS[ i ].lw; */
/*             drect.h = BUTTONS[ i ].lh; */
/*             SDL_BlitSurface( surf, &srect, buttons_surfaces[ i ].surfaceoff, &drect ); */
/*             SDL_BlitSurface( surf, &srect, buttons_surfaces[ i ].surfaceon, &drect ); */
/*             SDL_FreeSurface( surf ); */
/*         } */
/*     } */
/* } */

/* static void __draw_buttons( void ) */
/* { */
/*     SDL_Rect srect, drect; */

/*     for ( int i = FIRST_HPKEY; i <= LAST_HPKEY; i++ ) { */
/*         // Blit the button surface to the screen */
/*         srect.x = 0; */
/*         srect.y = 0; */
/*         srect.w = BUTTONS[ i ].w; */
/*         srect.h = BUTTONS[ i ].h; */
/*         drect.x = KEYBOARD_OFFSET_X + BUTTONS[ i ].x; */
/*         drect.y = KEYBOARD_OFFSET_Y + BUTTONS[ i ].y; */
/*         drect.w = BUTTONS[ i ].w; */
/*         drect.h = BUTTONS[ i ].h; */
/*         if ( keyboard[ i ].pressed ) */
/*             SDL_BlitSurface( buttons_surfaces[ i ].surfaceoff, &srect, window, &drect ); */
/*         else */
/*             SDL_BlitSurface( buttons_surfaces[ i ].surfaceon, &srect, window, &drect ); */
/*     } */

/*     // Always update immediately BUTTONS */
/*     SDL_UpdateRect( window, KEYBOARD_OFFSET_X + BUTTONS[ 0 ].x, KEYBOARD_OFFSET_Y + BUTTONS[ 0 ].y, */
/*                     BUTTONS[ LAST_HPKEY ].x + BUTTONS[ LAST_HPKEY ].w - BUTTONS[ 0 ].x, */
/*                     BUTTONS[ LAST_HPKEY ].y + BUTTONS[ LAST_HPKEY ].h - BUTTONS[ 0 ].y ); */
/* } */

/* static void _draw_keypad( void ) */
/* { */
/*     int i, x, y; */
/*     int offset_y = KEYBOARD_OFFSET_Y; */
/*     int offset_x = KEYBOARD_OFFSET_X; */
/*     SDL_Rect rect; */
/*     unsigned color; */
/*     unsigned pw, ph; */
/*     unsigned colorbg, colorfg; */
/*     int wl, wr, ws; */

/*     __create_buttons(); */

/*     // SDLDrawKeyMenu(); */
/*     for ( i = FIRST_HPKEY; i <= LAST_HPKEY; i++ ) { */
/*         if ( BUTTONS[ i ].is_menu ) { */
/*             // draw the dark shade under the label */
/*             pw = opt_gx ? 58 : 44; */
/*             ph = opt_gx ? 48 : 9; */

/*             color = ARGBColors[ UNDERLAY ]; */

/*             // Set the coordinates to absolute */
/*             if ( opt_gx ) { */
/*                 x = offset_x + BUTTONS[ i ].x - 6; */
/*                 y = offset_y + BUTTONS[ i ].y - small_ascent - small_descent - 6; */
/*             } else { */
/*                 x = offset_x + BUTTONS[ i ].x + ( BUTTONS[ i ].w - pw ) / 2; */
/*                 y = offset_y + BUTTONS[ i ].y - small_ascent - small_descent; */
/*             } */

/*             rect.x = x; */
/*             rect.y = y; */
/*             rect.w = pw; */
/*             rect.h = ph; */
/*             SDL_FillRect( window, &rect, color ); */
/*         } */

/*         // SDLDrawKeysLetters(); */
/*         if ( i < HPKEY_MTH ) */
/*             colorbg = ARGBColors[ DISP_PAD ]; */
/*         else */
/*             colorbg = ARGBColors[ PAD ]; */

/*         // Letter ( small character bottom right of key) */
/*         if ( BUTTONS[ i ].letter != ( char* )0 ) { */
/*             if ( opt_gx ) { */
/*                 x = offset_x + BUTTONS[ i ].x + BUTTONS[ i ].w + 3; */
/*                 y = offset_y + BUTTONS[ i ].y + BUTTONS[ i ].h + 1; */
/*             } else { */
/*                 x = offset_x + BUTTONS[ i ].x + BUTTONS[ i ].w - SmallTextWidth( BUTTONS[ i ].letter, 1 ) / 2 + 5; */
/*                 y = offset_y + BUTTONS[ i ].y + BUTTONS[ i ].h - 2; */
/*             } */

/*             write_text( x, y, BUTTONS[ i ].letter, 1, 0xffffffff, colorbg ); */
/*         } */

/*         // SDLDrawKeysLabelsBottom(); */
/*         // Bottom label: the only one is the cancel button */
/*         if ( BUTTONS[ i ].sub != ( char* )0 ) { */
/*             colorfg = ARGBColors[ WHITE ]; */

/*             x = offset_x + BUTTONS[ i ].x + ( 1 + BUTTONS[ i ].w - SmallTextWidth( BUTTONS[ i ].sub, strlen( BUTTONS[ i ].sub ) ) ) / 2;
 */
/*             y = offset_y + BUTTONS[ i ].y + BUTTONS[ i ].h + small_ascent + 2; */
/*             write_text( x, y, BUTTONS[ i ].sub, strlen( BUTTONS[ i ].sub ), colorfg, colorbg ); */
/*         } */

/*         // SDLDrawKeysLabelsLeft(); */
/*         // Draw the left labels */
/*         if ( BUTTONS[ i ].left != ( char* )0 ) { */
/*             if ( BUTTONS[ i ].is_menu ) { */
/*                 // draw the dark shade under the label */
/*                 pw = opt_gx ? 58 : 46; */

/*                 colorbg = ARGBColors[ UNDERLAY ]; */
/*                 colorfg = ARGBColors[ LEFT ]; */

/*                 x = ( pw + 1 - SmallTextWidth( BUTTONS[ i ].left, strlen( BUTTONS[ i ].left ) ) ) / 2; */
/*                 y = opt_gx ? 14 : 9; */

/*                 // Set the coordinates to absolute */
/*                 if ( opt_gx ) { */
/*                     x += offset_x + BUTTONS[ i ].x - 6; */
/*                     y += offset_y + BUTTONS[ i ].y - small_ascent - small_descent - 6; */
/*                 } else { */
/*                     x += offset_x + BUTTONS[ i ].x + ( BUTTONS[ i ].w - pw ) / 2; */
/*                     y += offset_y + BUTTONS[ i ].y - small_ascent - small_descent; */
/*                 } */

/*                 write_text( x, y, BUTTONS[ i ].left, strlen( BUTTONS[ i ].left ), colorfg, colorbg ); */
/*             } else // is_menu */
/*             { */
/*                 colorbg = ARGBColors[ BLACK ]; */
/*                 colorfg = ARGBColors[ LEFT ]; */

/*                 if ( BUTTONS[ i ].right == ( char* )0 ) { */
/*                     // centered label */
/*                     x = offset_x + BUTTONS[ i ].x + */
/*                         ( 1 + BUTTONS[ i ].w - SmallTextWidth( BUTTONS[ i ].left, strlen( BUTTONS[ i ].left ) ) ) / 2; */
/*                 } else { */
/*                     // label to the left */
/*                     wl = SmallTextWidth( BUTTONS[ i ].left, strlen( BUTTONS[ i ].left ) ); */
/*                     wr = SmallTextWidth( BUTTONS[ i ].right, strlen( BUTTONS[ i ].right ) ); */
/*                     ws = SmallTextWidth( " ", 1 ); */

/*                     x = offset_x + BUTTONS[ i ].x + ( 1 + BUTTONS[ i ].w - ( wl + wr + ws ) ) / 2; */
/*                 } */

/*                 y = offset_y + BUTTONS[ i ].y - small_descent; */

/*                 write_text( x, y, BUTTONS[ i ].left, strlen( BUTTONS[ i ].left ), colorfg, colorbg ); */
/*             } // is_menu */
/*         } */

/*         // SDLDrawKeysLabelsRight(); */
/*         // draw the right labels */
/*         if ( BUTTONS[ i ].right != ( char* )0 ) { */
/*             if ( BUTTONS[ i ].is_menu ) { */
/*                 // draw the dark shade under the label */
/*                 pw = opt_gx ? 58 : 44; */

/*                 colorbg = ARGBColors[ UNDERLAY ]; */
/*                 colorfg = ARGBColors[ RIGHT ]; */

/*                 x = ( pw + 1 - SmallTextWidth( BUTTONS[ i ].right, strlen( BUTTONS[ i ].right ) ) ) / 2; */
/*                 y = opt_gx ? 14 : 8; */

/*                 // Set the coordinates to absolute */
/*                 if ( opt_gx ) { */
/*                     x += offset_x + BUTTONS[ i ].x - 6; */
/*                     y += offset_y + BUTTONS[ i ].y - small_ascent - small_descent - 6; */
/*                 } else { */
/*                     x += offset_x + BUTTONS[ i ].x + ( BUTTONS[ i ].w - pw ) / 2; */
/*                     y += offset_y + BUTTONS[ i ].y - small_ascent - small_descent; */
/*                 } */

/*                 write_text( x, y, BUTTONS[ i ].right, strlen( BUTTONS[ i ].right ), colorfg, colorbg ); */
/*             } // BUTTONS[i].is_menu */
/*             else { */
/*                 colorbg = ARGBColors[ BLACK ]; */
/*                 colorfg = ARGBColors[ RIGHT ]; */

/*                 if ( BUTTONS[ i ].left == ( char* )0 ) { */
/*                     // centered label */
/*                     x = offset_x + BUTTONS[ i ].x + */
/*                         ( 1 + BUTTONS[ i ].w - SmallTextWidth( BUTTONS[ i ].right, strlen( BUTTONS[ i ].right ) ) ) / 2; */
/*                 } else { */
/*                     // label to the right */
/*                     wl = SmallTextWidth( BUTTONS[ i ].left, strlen( BUTTONS[ i ].left ) ); */
/*                     wr = SmallTextWidth( BUTTONS[ i ].right, strlen( BUTTONS[ i ].right ) ); */
/*                     ws = SmallTextWidth( " ", 1 ); */

/*                     x = offset_x + BUTTONS[ i ].x + ( 1 + BUTTONS[ i ].w - ( wl + wr + ws ) ) / 2 + wl + ws; */
/*                 } */

/*                 y = offset_y + BUTTONS[ i ].y - small_descent; */

/*                 write_text( x, y, BUTTONS[ i ].right, strlen( BUTTONS[ i ].right ), colorfg, colorbg ); */
/*             } */
/*         } */
/*     } */

/*     __draw_buttons(); */
/* } */

static void _draw_bezel_LCD( void )
{
    for ( int i = 0; i < DISP_FRAME; i++ ) {
        lineColor( renderer, display_offset_x - i, display_offset_y + DISPLAY_HEIGHT + 2 * i, display_offset_x + DISPLAY_WIDTH + i,
                   display_offset_y + DISPLAY_HEIGHT + 2 * i, color2argb( DISP_PAD_TOP ) );
        lineColor( renderer, display_offset_x - i, display_offset_y + DISPLAY_HEIGHT + 2 * i + 1, display_offset_x + DISPLAY_WIDTH + i,
                   display_offset_y + DISPLAY_HEIGHT + 2 * i + 1, color2argb( DISP_PAD_TOP ) );
        lineColor( renderer, display_offset_x + DISPLAY_WIDTH + i, display_offset_y - i, display_offset_x + DISPLAY_WIDTH + i,
                   display_offset_y + DISPLAY_HEIGHT + 2 * i, color2argb( DISP_PAD_TOP ) );

        lineColor( renderer, display_offset_x - i - 1, display_offset_y - i - 1, display_offset_x + DISPLAY_WIDTH + i - 1,
                   display_offset_y - i - 1, color2argb( DISP_PAD_BOT ) );
        lineColor( renderer, display_offset_x - i - 1, display_offset_y - i - 1, display_offset_x - i - 1,
                   display_offset_y + DISPLAY_HEIGHT + 2 * i - 1, color2argb( DISP_PAD_BOT ) );
    }

    // round off corners
    lineColor( renderer, display_offset_x - DISP_FRAME, display_offset_y - DISP_FRAME, display_offset_x - DISP_FRAME + 3,
               display_offset_y - DISP_FRAME, color2argb( DISP_PAD ) );
    lineColor( renderer, display_offset_x - DISP_FRAME, display_offset_y - DISP_FRAME, display_offset_x - DISP_FRAME,
               display_offset_y - DISP_FRAME + 3, color2argb( DISP_PAD ) );
    pixelColor( renderer, display_offset_x - DISP_FRAME + 1, display_offset_y - DISP_FRAME + 1, color2argb( DISP_PAD ) );

    lineColor( renderer, display_offset_x + DISPLAY_WIDTH + DISP_FRAME - 4, display_offset_y - DISP_FRAME,
               display_offset_x + DISPLAY_WIDTH + DISP_FRAME - 1, display_offset_y - DISP_FRAME, color2argb( DISP_PAD ) );
    lineColor( renderer, display_offset_x + DISPLAY_WIDTH + DISP_FRAME - 1, display_offset_y - DISP_FRAME,
               display_offset_x + DISPLAY_WIDTH + DISP_FRAME - 1, display_offset_y - DISP_FRAME + 3, color2argb( DISP_PAD ) );
    pixelColor( renderer, display_offset_x + DISPLAY_WIDTH + DISP_FRAME - 2, display_offset_y - DISP_FRAME + 1, color2argb( DISP_PAD ) );

    lineColor( renderer, display_offset_x - DISP_FRAME, display_offset_y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 4,
               display_offset_x - DISP_FRAME, display_offset_y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 1, color2argb( DISP_PAD ) );
    lineColor( renderer, display_offset_x - DISP_FRAME, display_offset_y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 1,
               display_offset_x - DISP_FRAME + 3, display_offset_y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 1, color2argb( DISP_PAD ) );
    pixelColor( renderer, display_offset_x - DISP_FRAME + 1, display_offset_y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 2,
                color2argb( DISP_PAD ) );

    lineColor( renderer, display_offset_x + DISPLAY_WIDTH + DISP_FRAME - 1, display_offset_y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 4,
               display_offset_x + DISPLAY_WIDTH + DISP_FRAME - 1, display_offset_y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 1,
               color2argb( DISP_PAD ) );
    lineColor( renderer, display_offset_x + DISPLAY_WIDTH + DISP_FRAME - 4, display_offset_y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 1,
               display_offset_x + DISPLAY_WIDTH + DISP_FRAME - 1, display_offset_y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 1,
               color2argb( DISP_PAD ) );
    pixelColor( renderer, display_offset_x + DISPLAY_WIDTH + DISP_FRAME - 2, display_offset_y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 2,
                color2argb( DISP_PAD ) );

    // simulate rounded lcd corners

    lineColor( renderer, display_offset_x - 1, display_offset_y + 1, display_offset_x - 1, display_offset_y + DISPLAY_HEIGHT - 2,
               color2argb( LCD ) );
    lineColor( renderer, display_offset_x + 1, display_offset_y - 1, display_offset_x + DISPLAY_WIDTH - 2, display_offset_y - 1,
               color2argb( LCD ) );
    lineColor( renderer, display_offset_x + 1, display_offset_y + DISPLAY_HEIGHT, display_offset_x + DISPLAY_WIDTH - 2,
               display_offset_y + DISPLAY_HEIGHT, color2argb( LCD ) );
    lineColor( renderer, display_offset_x + DISPLAY_WIDTH, display_offset_y + 1, display_offset_x + DISPLAY_WIDTH,
               display_offset_y + DISPLAY_HEIGHT - 2, color2argb( LCD ) );
}

static void _draw_background( int width, int height, int w_top, int h_top )
{
    SDL_Rect rect;

    rect.x = 0;
    rect.y = 0;
    rect.w = w_top;
    rect.h = h_top;
    SDL_SetRenderDrawColor( renderer, colors[ PAD ].r, colors[ PAD ].g, colors[ PAD ].b, 255 );
    SDL_RenderFillRect( renderer, &rect );

    rect.w = width;
    rect.h = height;
    SDL_SetRenderDrawColor( renderer, colors[ DISP_PAD ].r, colors[ DISP_PAD ].g, colors[ DISP_PAD ].b, 255 );
    SDL_RenderFillRect( renderer, &rect );
}

static void _draw_background_LCD( void )
{
    SDL_Rect rect;

    rect.x = display_offset_x;
    rect.y = display_offset_y;
    rect.w = DISPLAY_WIDTH;
    rect.h = DISPLAY_HEIGHT;

    SDL_SetRenderDrawColor( renderer, colors[ LCD ].r, colors[ LCD ].g, colors[ LCD ].b, 255 );
    SDL_RenderFillRect( renderer, &rect );
}

/* // Show the hp key which is being pressed */
/* static void _show_key( int hpkey ) */
/* { */
/*     SDL_Rect srect, drect; */
/*     SDL_Surface* ssurf; */
/*     int x; */
/*     int y; */

/*     // If we're called with the same key as before, do nothing */
/*     if ( showkeylastkey == hpkey ) */
/*         return; */

/*     showkeylastkey = hpkey; */

/*     // Starts by hiding last */
/*     if ( showkeylastsurf != 0 ) { */
/*         drect.x = showkeylastx; */
/*         drect.y = showkeylasty; */
/*         SDL_BlitSurface( showkeylastsurf, 0, window, &drect ); */

/*         // Update */
/*         SDL_UpdateRect( window, showkeylastx, showkeylasty, showkeylastsurf->w, showkeylastsurf->h ); */

/*         // Free */
/*         SDL_FreeSurface( showkeylastsurf ); */
/*         showkeylastsurf = 0; */
/*     } */

/*     if ( hpkey == -1 ) */
/*         return; */

/*     // Which surface to show */
/*     ssurf = ( keyboard[ hpkey ].pressed ) ? buttons_surfaces[ hpkey ].surfaceoff : buttons_surfaces[ hpkey ].surfaceon; */

/*     // Background backup */
/*     showkeylastsurf = SDL_CreateRGBSurface( SDL_SWSURFACE, ssurf->w, ssurf->h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 ); */

/*     // Where to */
/*     x = KEYBOARD_OFFSET_X + BUTTONS[ hpkey ].x - ( ssurf->w - ssurf->w + 1 ) / 2; */
/*     y = KEYBOARD_OFFSET_Y + BUTTONS[ hpkey ].y - ( ssurf->h - ssurf->h + 1 ) / 2; */
/*     // blitting does not clip to screen, so if we are out of the screen we */
/*     // shift the button to fit */
/*     if ( x < 0 ) */
/*         x = 0; */
/*     if ( y < 0 ) */
/*         y = 0; */
/*     if ( x + ssurf->w > window->w ) */
/*         x = window->w - ssurf->w; */
/*     if ( y + ssurf->h > window->h ) */
/*         y = window->h - ssurf->h; */

/*     // Backup where to */
/*     showkeylastx = x; */
/*     showkeylasty = y; */

/*     // Backup old surface */
/*     srect.x = x; */
/*     srect.y = y; */
/*     srect.w = ssurf->w; */
/*     srect.h = ssurf->h; */
/*     drect.x = 0; */
/*     drect.y = 0; */
/*     SDL_BlitSurface( window, &srect, showkeylastsurf, &drect ); */

/*     // Blit the button */
/*     drect.x = x; */
/*     drect.y = y; */
/*     SDL_BlitSurface( ssurf, 0, window, &drect ); */

/*     // Update */
/*     SDL_UpdateRect( window, x, y, ssurf->w, ssurf->h ); */
/* } */

/* static void _draw_serial_devices_path( void ) */
/* { */
/*     char text[ 1024 ] = ""; */

/*     if ( config.verbose ) { */
/*         fprintf( stderr, "wire_name: %s\n", wire_name ); */
/*         fprintf( stderr, "ir_name: %s\n", ir_name ); */
/*     } */

/*     if ( wire_name ) { */
/*         strcat( text, "wire: " ); */
/*         strcat( text, wire_name ); */
/*     } */
/*     if ( ir_name ) { */
/*         if ( strlen( text ) > 0 ) */
/*             strcat( text, " | " ); */

/*         strcat( text, "ir: " ); */
/*         strcat( text, ir_name ); */
/*     } */

/*     if ( strlen( text ) > 0 ) */
/*         stringColor( window, 10, 240, text, 0xffffffff ); */
/* } */

static void sdl_draw_nibble( int nx, int ny, int val )
{
    int x, y;
    int xoffset = display_offset_x + 5;
    int yoffset = display_offset_y + 20;
    int color = LCD;
    SDL_Rect rect;

    SDL_SetRenderTarget( renderer, main_texture );

    for ( x = 0; x < 4; x++ ) {
        // Check if bit is on
        // bits in a byte are used (1 nibble per byte)
        if ( nx + x >= 131 ) // Clip at 131 pixels
            break;

        char c = val;
        char b = c & ( 1 << ( x & 3 ) );

        color = b ? PIXEL : LCD;
        SDL_SetRenderDrawColor( renderer, colors[ color ].r, colors[ color ].g, colors[ color ].b, 255 );

        rect.x = xoffset + ( 2 * ( nx + x ) );
        rect.y = yoffset + ( 2 * ny );
        rect.w = 2;
        rect.h = 2;

        SDL_RenderFillRect( renderer, &rect );
    }

    SDL_SetRenderTarget( renderer, NULL );
    SDL_RenderCopy( renderer, main_texture, NULL, NULL );
    SDL_RenderPresent( renderer );
}

static inline void draw_nibble( int col, int row, int val )
{
    val &= 0x0f;
    if ( val == lcd_nibbles_buffer[ row ][ col ] )
        return;

    lcd_nibbles_buffer[ row ][ col ] = val;

    int y = row;
    int x = col * 4;
    if ( row <= display.lines )
        x -= ( 2 * display.offset );

    sdl_draw_nibble( x, y, val );
}

/* Identical in all ui_*.c */
static inline void draw_row( long addr, int row )
{
    int line_length = NIBBLES_PER_ROW;

    if ( ( display.offset > 3 ) && ( row <= display.lines ) )
        line_length += 2;

    for ( int i = 0; i < line_length; i++ )
        draw_nibble( i, row, read_nibble( addr + i ) );
}

/**********/
/* public */
/**********/
void sdl_get_event( void )
{
    SDL_Event event;

    int hpkey = -1;
    static int lasthpkey = -1;           // last key that was pressed or -1 for none
    static int lastticks = -1;           // time at which a key was pressed or -1 if timer expired
    static bool lastislongpress = false; // last key press was a long press
    /* static int pressed_hpkey = -1;       // Indicate if a key is being held down by */
    /*                                      // a finger (not set for long presses) */

    // Check whether long pres on key
    if ( lastticks > 0 && ( SDL_GetTicks() - lastticks > 750 ) ) {
        // time elapsed
        lastticks = -1;

        // Check that the mouse is still on the same last key
        int x, y;
        int state = SDL_GetMouseState( &x, &y );

        if ( state & SDL_BUTTON( 1 ) && mouse_click_to_hpkey( x, y ) == lasthpkey )
            lastislongpress = true;
    }

    // Iterate as long as there are events
    while ( SDL_PollEvent( &event ) ) {
        switch ( event.type ) {
            case SDL_QUIT:
                // please_exit = true;
                close_and_exit();
                break;

            case SDL_MOUSEBUTTONDOWN:
                hpkey = mouse_click_to_hpkey( event.button.x, event.button.y );
                // React to mouse up/down when click over a button
                if ( hpkey == -1 || keyboard[ hpkey ].pressed )
                    break;

                /* pressed_hpkey = hpkey; */
                press_key( hpkey );
                lasthpkey = hpkey;
                // Start timer
                lastticks = SDL_GetTicks();
                break;
            case SDL_MOUSEBUTTONUP:
                hpkey = mouse_click_to_hpkey( event.button.x, event.button.y );
                // React to mouse up/down when click over a button
                if ( hpkey == -1 )
                    break;

                /* pressed_hpkey = -1; */
                if ( !lastislongpress ) {
                    release_all_keys();
                    lasthpkey = -1; // No key is pressed anymore
                }

                // Stop timer, clear long key press
                lastticks = -1;
                lastislongpress = false;
                break;

            case SDL_KEYDOWN:
                hpkey = sdlkey_to_hpkey( event.key.keysym.sym );
                if ( hpkey == -1 || keyboard[ hpkey ].pressed )
                    break;

                /* pressed_hpkey = hpkey; */
                press_key( hpkey );
                break;
            case SDL_KEYUP:
                hpkey = sdlkey_to_hpkey( event.key.keysym.sym );
                if ( hpkey == -1 )
                    break;

                /* pressed_hpkey = -1; */
                release_key( hpkey );
                break;
        }
    }

    // Display button being pressed, if any
    /* if ( !config.hide_chrome ) */
    /*     _show_key( pressed_hpkey ); */
}

void sdl_update_LCD( void )
{
    if ( display.on ) {
        int i;
        long addr = display.disp_start;
        static int old_offset = -1;
        static int old_lines = -1;

        if ( display.offset != old_offset ) {
            memset( lcd_nibbles_buffer, 0xf0, ( size_t )( ( display.lines + 1 ) * NIBS_PER_BUFFER_ROW ) );
            old_offset = display.offset;
        }
        if ( display.lines != old_lines ) {
            memset( &lcd_nibbles_buffer[ 56 ][ 0 ], 0xf0, ( size_t )( 8 * NIBS_PER_BUFFER_ROW ) );
            old_lines = display.lines;
        }
        for ( i = 0; i <= display.lines; i++ ) {
            draw_row( addr, i );
            addr += display.nibs_per_line;
        }
        if ( i < DISP_ROWS ) {
            addr = display.menu_start;
            for ( ; i < DISP_ROWS; i++ ) {
                draw_row( addr, i );
                addr += NIBBLES_PER_ROW;
            }
        }
    } else
        ui_init_LCD();
}

void sdl_refresh_LCD( void ) {}

void sdl_disp_draw_nibble( word_20 addr, word_4 val )
{
    long offset = ( addr - display.disp_start );
    int x = offset % display.nibs_per_line;

    if ( x < 0 || x > 35 )
        return;

    if ( display.nibs_per_line != 0 ) {
        int y = offset / display.nibs_per_line;
        if ( y < 0 || y > 63 )
            return;

        draw_nibble( x, y, val );
    } else
        for ( int y = 0; y < display.lines; y++ )
            draw_nibble( x, y, val );
}

void sdl_menu_draw_nibble( word_20 addr, word_4 val )
{
    long offset = ( addr - display.menu_start );
    int x = offset % NIBBLES_PER_ROW;
    int y = display.lines + ( offset / NIBBLES_PER_ROW ) + 1;

    draw_nibble( x, y, val );
}

void sdl_draw_annunc( void )
{
    if ( saturn.annunc == last_annunc_state )
        return;

    last_annunc_state = saturn.annunc;

    create_annunciators_surfaces_textures();

    bool annunc_state;

    SDL_SetRenderTarget( renderer, main_texture );
    for ( int i = 0; i < NB_ANNUNCIATORS; i++ ) {
        annunc_state = ( ( annunciators_bits[ i ] & saturn.annunc ) == annunciators_bits[ i ] );

        /* SDL_Rect srect; */
        SDL_Rect drect;
        /* srect.x = 0; */
        /* srect.y = 0; */
        /* srect.w = ann_tbl[ i ].width; */
        /* srect.h = ann_tbl[ i ].height; */
        drect.x = display_offset_x + ann_tbl[ i ].x;
        drect.y = display_offset_y + ann_tbl[ i ].y;
        drect.w = ann_tbl[ i ].width;
        drect.h = ann_tbl[ i ].height;

        /*     SDL_BlitSurface( ( annunc_state ) ? annunciators_surfaces[ i ].surfaceon : annunciators_surfaces[ i ].surfaceoff, &srect, */
        /* window, */
        /*                      &drect ); */
        SDL_RenderCopy( renderer,
                        ( annunc_state ) ? annunciators_surfaces_textures[ i ].textureon : annunciators_surfaces_textures[ i ].textureoff,
                        NULL, &drect );
    }

    // Always immediately update annunciators
    /* SDL_UpdateRect( window, display_offset_x + ann_tbl[ 0 ].x, display_offset_y + ann_tbl[ 0 ].y, */
    /*                 ann_tbl[ 5 ].x + ann_tbl[ 5 ].width - ann_tbl[ 0 ].x, ann_tbl[ 5 ].y + ann_tbl[ 5 ].height - ann_tbl[ 0 ].y ); */

    SDL_SetRenderTarget( renderer, NULL );
    SDL_RenderCopy( renderer, main_texture, NULL, NULL );
    SDL_RenderPresent( renderer );
}

void sdl_adjust_contrast( void )
{
    colors_setup();
    /* create_annunciators_surfaces_textures(); */

    // redraw LCD
    ui_init_LCD();
    sdl_update_LCD();

    // redraw annunc
    last_annunc_state = -1;

    sdl_draw_annunc();
}

void sdl2_ui_stop( void )
{
    SDL_DestroyTexture( main_texture );
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
}

void init_sdl2_ui( int argc, char** argv )
{
    if ( config.verbose )
        fprintf( stderr, "UI is sdl2\n" );

    /* Set public API to this UI's functions */
    ui_disp_draw_nibble = sdl_disp_draw_nibble;
    ui_menu_draw_nibble = sdl_menu_draw_nibble;
    ui_get_event = sdl_get_event;
    ui_update_LCD = sdl_update_LCD;
    ui_refresh_LCD = sdl_refresh_LCD;
    ui_adjust_contrast = sdl_adjust_contrast;
    ui_draw_annunc = sdl_draw_annunc;

    // Initialize SDL
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        printf( "Couldn't initialize SDL: %s\n", SDL_GetError() );
        exit( 1 );
    }

    // On exit: clean SDL
    atexit( SDL_Quit );

    unsigned int width, height;
    display_offset_x = DISPLAY_OFFSET_X;
    display_offset_y = DISPLAY_OFFSET_Y;
    width = ( BUTTONS[ LAST_HPKEY ].x + BUTTONS[ LAST_HPKEY ].w ) + 2 * SIDE_SKIP;
    height = display_offset_y + DISPLAY_HEIGHT + DISP_KBD_SKIP + BUTTONS[ LAST_HPKEY ].y + BUTTONS[ LAST_HPKEY ].h + BOTTOM_SKIP;

    if ( config.hide_chrome ) {
        display_offset_x = 0;
        display_offset_y = 0;
        width = DISPLAY_WIDTH;
        height = DISPLAY_HEIGHT;
    }

    uint32_t window_flags = SDL_WINDOW_ALLOW_HIGHDPI;
    if ( config.show_ui_fullscreen )
        window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    else
        window_flags |= SDL_WINDOW_RESIZABLE;

    window = SDL_CreateWindow( config.progname, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, window_flags );
    if ( window == NULL ) {
        printf( "Couldn't create window: %s\n", SDL_GetError() );
        exit( 1 );
    }

    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE );
    if ( renderer == NULL )
        exit( 2 );

    main_texture = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height );

    SDL_SetRenderTarget( renderer, main_texture );

    colors_setup();

    if ( !config.hide_chrome ) {
        int cut = BUTTONS[ HPKEY_MTH ].y + KEYBOARD_OFFSET_Y - 19;

        _draw_background( width, cut, width, height );
        _draw_bezel( cut, KEYBOARD_OFFSET_Y, width, height );
        _draw_header();
        _draw_bezel_LCD();
        /* _draw_keypad(); */

        /* _draw_serial_devices_path(); */
    }

    _draw_background_LCD();

    // SDL_UpdateRect( window, 0, 0, 0, 0 );

    SDL_SetRenderTarget( renderer, NULL );
    SDL_RenderCopy( renderer, main_texture, NULL, NULL );
    SDL_RenderPresent( renderer );
}
