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

#include "romio.h" /* opt_gx */
#include "options.h"
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
typedef struct on_off_sdl_textures_struct_t {
    SDL_Texture* up;
    SDL_Texture* down;
} on_off_sdl_textures_struct_t;

/*************/
/* variables */
/*************/
static int display_offset_x, display_offset_y;

color_t colors[ NB_COLORS ];
static on_off_sdl_textures_struct_t buttons_textures[ NB_KEYS ];
static on_off_sdl_textures_struct_t annunciators_textures[ NB_ANNUNCIATORS ];

static SDL_Window* window;
static SDL_Renderer* renderer;
static SDL_Texture* main_texture;

/****************************/
/* functions implementation */
/****************************/
static inline unsigned color2bgra( int color )
{
    return 0xff000000 | ( colors[ color ].r << 16 ) | ( colors[ color ].g << 8 ) | colors[ color ].b;
}

/*
        Create a SDL_Texture from binary bitmap data
*/
static SDL_Texture* bitmap_to_texture( unsigned int w, unsigned int h, unsigned char* data, int color_fg, int color_bg )
{
    SDL_Surface* surf = SDL_CreateRGBSurface( SDL_SWSURFACE, w, h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 );

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
    SDL_FreeSurface( surf );

    return tex;
}

static void __draw_pixel( int x, int y, int color )
{
    SDL_SetRenderDrawColor( renderer, colors[ color ].r, colors[ color ].g, colors[ color ].b, 0xff );
    SDL_RenderDrawPoint( renderer, x, y );
}

static void __draw_line( int x1, int y1, int x2, int y2, int color )
{
    SDL_SetRenderDrawColor( renderer, colors[ color ].r, colors[ color ].g, colors[ color ].b, 0xff );
    SDL_RenderDrawLine( renderer, x1, y1, x2, y2 );
}

static void __draw_rect( int x, int y, int w, int h, int color )
{
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;

    SDL_SetRenderDrawColor( renderer, colors[ color ].r, colors[ color ].g, colors[ color ].b, 0xff );
    SDL_RenderFillRect( renderer, &rect );
}

static void __draw_texture( int x, int y, unsigned int w, unsigned int h, SDL_Texture* texture )
{
    SDL_Rect drect;
    drect.x = x;
    drect.y = y;
    drect.w = w;
    drect.h = h;

    SDL_RenderCopy( renderer, texture, NULL, &drect );
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
static void create_annunciators_textures( void )
{
    for ( int i = 0; i < NB_ANNUNCIATORS; i++ ) {
        annunciators_textures[ i ].up = bitmap_to_texture( ann_tbl[ i ].width, ann_tbl[ i ].height, ann_tbl[ i ].bits, PIXEL, LCD );
        annunciators_textures[ i ].down = bitmap_to_texture( ann_tbl[ i ].width, ann_tbl[ i ].height, ann_tbl[ i ].bits, LCD, LCD );
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
            return HPKEY_QUOTE;
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
    __draw_line( 1, keypad_height - 1, keypad_width - 1, keypad_height - 1, PAD_TOP );
    __draw_line( 2, keypad_height - 2, keypad_width - 2, keypad_height - 2, PAD_TOP );

    // right lines
    __draw_line( keypad_width - 1, keypad_height - 1, keypad_width - 1, cut, PAD_TOP );
    __draw_line( keypad_width - 2, keypad_height - 2, keypad_width - 2, cut, PAD_TOP );

    // right lines
    __draw_line( keypad_width - 1, cut - 1, keypad_width - 1, 1, DISP_PAD_TOP );
    __draw_line( keypad_width - 2, cut - 1, keypad_width - 2, 2, DISP_PAD_TOP );

    // top lines
    __draw_line( 0, 0, keypad_width - 2, 0, DISP_PAD_BOT );
    __draw_line( 1, 1, keypad_width - 3, 1, DISP_PAD_BOT );

    // left lines
    __draw_line( 0, cut - 1, 0, 0, DISP_PAD_BOT );
    __draw_line( 1, cut - 1, 1, 1, DISP_PAD_BOT );

    // left lines
    __draw_line( 0, keypad_height - 2, 0, cut, PAD_BOT );
    __draw_line( 1, keypad_height - 3, 1, cut, PAD_BOT );

    // lower the menu BUTTONS

    // bottom lines
    __draw_line( 3, keypad_height - 3, keypad_width - 3, keypad_height - 3, PAD_TOP );
    __draw_line( 4, keypad_height - 4, keypad_width - 4, keypad_height - 4, PAD_TOP );

    // right lines
    __draw_line( keypad_width - 3, keypad_height - 3, keypad_width - 3, cut, PAD_TOP );
    __draw_line( keypad_width - 4, keypad_height - 4, keypad_width - 4, cut, PAD_TOP );

    // right lines
    __draw_line( keypad_width - 3, cut - 1, keypad_width - 3, offset_y - ( KBD_UPLINE - 1 ), DISP_PAD_TOP );
    __draw_line( keypad_width - 4, cut - 1, keypad_width - 4, offset_y - ( KBD_UPLINE - 2 ), DISP_PAD_TOP );

    // top lines
    __draw_line( 2, offset_y - ( KBD_UPLINE - 0 ), keypad_width - 4, offset_y - ( KBD_UPLINE - 0 ), DISP_PAD_BOT );
    __draw_line( 3, offset_y - ( KBD_UPLINE - 1 ), keypad_width - 5, offset_y - ( KBD_UPLINE - 1 ), DISP_PAD_BOT );

    // left lines
    __draw_line( 2, cut - 1, 2, offset_y - ( KBD_UPLINE - 1 ), DISP_PAD_BOT );
    __draw_line( 3, cut - 1, 3, offset_y - ( KBD_UPLINE - 2 ), DISP_PAD_BOT );

    // left lines
    __draw_line( 2, keypad_height - 4, 2, cut, PAD_BOT );
    __draw_line( 3, keypad_height - 5, 3, cut, PAD_BOT );

    // lower the keyboard

    // bottom lines
    __draw_line( 5, keypad_height - 5, keypad_width - 3, keypad_height - 5, PAD_TOP );
    __draw_line( 6, keypad_height - 6, keypad_width - 4, keypad_height - 6, PAD_TOP );

    // right lines
    __draw_line( keypad_width - 5, keypad_height - 5, keypad_width - 5, cut + 1, PAD_TOP );
    __draw_line( keypad_width - 6, keypad_height - 6, keypad_width - 6, cut + 2, PAD_TOP );

    // top lines
    __draw_line( 4, cut, keypad_width - 6, cut, DISP_PAD_BOT );
    __draw_line( 5, cut + 1, keypad_width - 7, cut + 1, DISP_PAD_BOT );

    // left lines
    __draw_line( 4, keypad_height - 6, 4, cut + 1, PAD_BOT );
    __draw_line( 5, keypad_height - 7, 5, cut + 2, PAD_BOT );

    // round off the bottom edge
    __draw_line( keypad_width - 7, keypad_height - 7, keypad_width - 7, keypad_height - 14, PAD_TOP );
    __draw_line( keypad_width - 8, keypad_height - 8, keypad_width - 8, keypad_height - 11, PAD_TOP );
    __draw_line( keypad_width - 7, keypad_height - 7, keypad_width - 14, keypad_height - 7, PAD_TOP );
    __draw_line( keypad_width - 7, keypad_height - 8, keypad_width - 11, keypad_height - 8, PAD_TOP );
    __draw_pixel( keypad_width - 9, keypad_height - 9, PAD_TOP );

    __draw_line( 7, keypad_height - 7, 13, keypad_height - 7, PAD_TOP );
    __draw_line( 8, keypad_height - 8, 10, keypad_height - 8, PAD_TOP );

    __draw_line( 6, keypad_height - 8, 6, keypad_height - 14, PAD_BOT );
    __draw_line( 7, keypad_height - 9, 7, keypad_height - 11, PAD_BOT );
}

static void _draw_header( void )
{
    int x = display_offset_x;
    int y;

    // insert the HP Logo
    if ( opt_gx )
        x -= 6;

    __draw_bitmap( x, 10, hp_width, hp_height, hp_bitmap, LOGO, LOGO_BACK );

    if ( !opt_gx ) {
        __draw_line( display_offset_x, 9, display_offset_x + hp_width - 1, 9, FRAME );
        __draw_line( display_offset_x - 1, 10, display_offset_x - 1, 10 + hp_height - 1, FRAME );
        __draw_line( display_offset_x, 10 + hp_height, display_offset_x + hp_width - 1, 10 + hp_height, FRAME );
        __draw_line( display_offset_x + hp_width, 10, display_offset_x + hp_width, 10 + hp_height - 1, FRAME );
    }

    // write the name of it
    if ( opt_gx ) {
        x = display_offset_x + DISPLAY_WIDTH - gx_128K_ram_width + gx_128K_ram_x_hot + 2;
        y = 10 + gx_128K_ram_y_hot;
        __draw_bitmap( x, y, gx_128K_ram_width, gx_128K_ram_height, gx_128K_ram_bitmap, LABEL, DISP_PAD );

        x = display_offset_x + hp_width;
        y = hp_height + 8 - hp48gx_height;
        __draw_bitmap( x, y, hp48gx_width, hp48gx_height, hp48gx_bitmap, LOGO, DISP_PAD );

        x = display_offset_x + DISPLAY_WIDTH - gx_128K_ram_width + gx_green_x_hot + 2;
        y = 10 + gx_green_y_hot;
        __draw_bitmap( x, y, gx_green_width, gx_green_height, gx_green_bitmap, RIGHT, DISP_PAD );

        x = display_offset_x + DISPLAY_WIDTH - gx_128K_ram_width + gx_silver_x_hot + 2;
        y = 10 + gx_silver_y_hot;
        __draw_bitmap( x, y, gx_silver_width, gx_silver_height, gx_silver_bitmap, LOGO,
                       0 ); // Background transparent: draw only silver line
    } else {
        x = display_offset_x;
        y = TOP_SKIP - DISP_FRAME - hp48sx_height - 3;
        __draw_bitmap( x, y, hp48sx_width, hp48sx_height, hp48sx_bitmap, LOGO, DISP_PAD );

        x = display_offset_x + DISPLAY_WIDTH - 1 - science_width;
        y = TOP_SKIP - DISP_FRAME - science_height - 4;
        __draw_bitmap( x, y, science_width, science_height, science_bitmap, LOGO, DISP_PAD );
    }
}

static SDL_Texture* create_button_texture( int hpkey, bool is_up )
{
    bool is_down = !is_up;
    int x, y;
    int on_key_offset_y = ( hpkey == HPKEY_ON ) ? 1 : 0;
    SDL_Texture* texture =
        SDL_CreateTexture( renderer, SDL_PIXELFORMAT_RGB444, SDL_TEXTUREACCESS_TARGET, BUTTONS[ hpkey ].w, BUTTONS[ hpkey ].h );
    SDL_SetRenderTarget( renderer, texture );

    // Fill the button and outline
    // fix outer-corners color
    int outer_color = PAD;
    if ( BUTTONS[ hpkey ].is_menu )
        outer_color = UNDERLAY;
    if ( hpkey < HPKEY_MTH )
        outer_color = DISP_PAD;
    __draw_rect( 0, 0, BUTTONS[ hpkey ].w, BUTTONS[ hpkey ].h, outer_color );
    __draw_rect( 1, 1, BUTTONS[ hpkey ].w - 2, BUTTONS[ hpkey ].h - 2, BUTTON );

    // draw label in button
    if ( BUTTONS[ hpkey ].label != ( char* )0 ) {
        /* Button has a text label */
        x = strlen( BUTTONS[ hpkey ].label ) - 1;
        x += ( ( BUTTONS[ hpkey ].w - BigTextWidth( BUTTONS[ hpkey ].label, strlen( BUTTONS[ hpkey ].label ) ) ) / 2 );
        y = ( BUTTONS[ hpkey ].h + 1 ) / 2 - 6;
        if ( is_down )
            y -= 1;

        write_with_big_font( x, y, BUTTONS[ hpkey ].label, WHITE, BUTTON );
    } else if ( BUTTONS[ hpkey ].lw != 0 ) {
        /* Button has a texture */
        x = ( 1 + BUTTONS[ hpkey ].w - BUTTONS[ hpkey ].lw ) / 2;
        y = ( 1 + BUTTONS[ hpkey ].h - BUTTONS[ hpkey ].lh ) / 2;
        if ( is_up )
            y += 1;

        __draw_bitmap( x, y, BUTTONS[ hpkey ].lw, BUTTONS[ hpkey ].lh, BUTTONS[ hpkey ].lb, BUTTONS[ hpkey ].lc, BUTTON );
    }

    // draw edge of button
    // top
    __draw_line( 1, 1, BUTTONS[ hpkey ].w - 2, 1, BUT_TOP );
    __draw_line( 2, 2, BUTTONS[ hpkey ].w - 3, 2, BUT_TOP );
    if ( is_up ) {
        __draw_line( 3, 3, BUTTONS[ hpkey ].w - 4, 3, BUT_TOP );
        __draw_line( 4, 4, BUTTONS[ hpkey ].w - 5, 4, BUT_TOP );
    }
    // top-left
    __draw_pixel( 4, 3 + ( is_up ? 2 : 0 ), BUT_TOP );
    // left
    __draw_line( 1, 1, 1, BUTTONS[ hpkey ].h - 2, BUT_TOP );
    __draw_line( 2, 2, 2, BUTTONS[ hpkey ].h - 3, BUT_TOP );
    __draw_line( 3, 3, 3, BUTTONS[ hpkey ].h - 4, BUT_TOP );
    // right
    __draw_line( BUTTONS[ hpkey ].w - 2, BUTTONS[ hpkey ].h - 2, BUTTONS[ hpkey ].w - 2, 3, BUT_BOT );
    __draw_line( BUTTONS[ hpkey ].w - 3, BUTTONS[ hpkey ].h - 3, BUTTONS[ hpkey ].w - 3, 4, BUT_BOT );
    __draw_line( BUTTONS[ hpkey ].w - 4, BUTTONS[ hpkey ].h - 4, BUTTONS[ hpkey ].w - 4, 5, BUT_BOT );
    __draw_pixel( BUTTONS[ hpkey ].w - 5, BUTTONS[ hpkey ].h - 4, BUT_BOT );
    // bottom
    __draw_line( 3, BUTTONS[ hpkey ].h - 2, BUTTONS[ hpkey ].w - 2, BUTTONS[ hpkey ].h - 2, BUT_BOT );
    __draw_line( 4, BUTTONS[ hpkey ].h - 3, BUTTONS[ hpkey ].w - 3, BUTTONS[ hpkey ].h - 3, BUT_BOT );

    // draw black frame around button
    // top
    __draw_line( 2, 0, BUTTONS[ hpkey ].w - 3, 0, FRAME );
    // left
    __draw_line( 0, 2, 0, BUTTONS[ hpkey ].h - 3, FRAME );
    // right
    __draw_line( BUTTONS[ hpkey ].w - 1, BUTTONS[ hpkey ].h - 3, BUTTONS[ hpkey ].w - 1, 2, FRAME );
    // bottom
    __draw_line( 2, BUTTONS[ hpkey ].h - 1, BUTTONS[ hpkey ].w - 3, BUTTONS[ hpkey ].h - 1, FRAME );
    // top-left
    __draw_pixel( 1, 1, FRAME );
    // top-right
    __draw_pixel( BUTTONS[ hpkey ].w - 2, 1, FRAME );
    // bottom-left
    __draw_pixel( 1, BUTTONS[ hpkey ].h - 2, FRAME );
    // bottom-right
    __draw_pixel( BUTTONS[ hpkey ].w - 2, BUTTONS[ hpkey ].h - 2, FRAME );
    if ( hpkey == HPKEY_ON ) {
        // top
        __draw_line( 2, 1, BUTTONS[ hpkey ].w - 3, 1, FRAME );
        // top-left
        __draw_pixel( 1, 1 + on_key_offset_y, FRAME );
        // top-right
        __draw_pixel( BUTTONS[ hpkey ].w - 2, 1 + on_key_offset_y, FRAME );
    }

    if ( is_down ) {
        // top
        __draw_line( 2, 1 + on_key_offset_y, BUTTONS[ hpkey ].w - 3, 1 + on_key_offset_y, FRAME );
        // left
        __draw_line( 1, 2, 1, BUTTONS[ hpkey ].h, FRAME );
        // right
        __draw_line( BUTTONS[ hpkey ].w - 2, 2, BUTTONS[ hpkey ].w - 2, BUTTONS[ hpkey ].h, FRAME );
        // top-left
        __draw_pixel( 2, 2 + on_key_offset_y, FRAME );
        // top-right
        __draw_pixel( BUTTONS[ hpkey ].w - 3, 2 + on_key_offset_y, FRAME );
    }

    return texture;
}

static void create_buttons_textures( void )
{
    for ( int i = FIRST_HPKEY; i <= LAST_HPKEY; i++ ) {
        buttons_textures[ i ].up = create_button_texture( i, true );
        buttons_textures[ i ].down = create_button_texture( i, false );
    }

    // Give back to renderer as it was
    SDL_SetRenderTarget( renderer, main_texture );
}

static void _draw_key( int hpkey )
{
    __draw_texture( KEYBOARD_OFFSET_X + BUTTONS[ hpkey ].x, KEYBOARD_OFFSET_Y + BUTTONS[ hpkey ].y, BUTTONS[ hpkey ].w, BUTTONS[ hpkey ].h,
                    keyboard[ hpkey ].pressed ? buttons_textures[ hpkey ].down : buttons_textures[ hpkey ].up );
}

static void _draw_keypad( void )
{
    int x, y;
    int pw = opt_gx ? 58 : 44;
    int ph = opt_gx ? 48 : 9;
    int left_label_width, right_label_width;
    int space_char_width = SmallTextWidth( " ", 1 );
    int total_top_labels_width;

    for ( int i = FIRST_HPKEY; i <= LAST_HPKEY; i++ ) {
        // Background
        if ( BUTTONS[ i ].is_menu ) {
            x = KEYBOARD_OFFSET_X + BUTTONS[ i ].x;
            y = KEYBOARD_OFFSET_Y + BUTTONS[ i ].y - small_ascent - small_descent;

            if ( opt_gx ) {
                x -= 6;
                y -= 6;
            } else
                x += ( BUTTONS[ i ].w - pw ) / 2;

            __draw_rect( x, y, pw, ph, UNDERLAY );
        }

        // Letter (small character bottom right of key)
        if ( BUTTONS[ i ].letter != ( char* )0 ) {
            x = KEYBOARD_OFFSET_X + BUTTONS[ i ].x + BUTTONS[ i ].w;
            y = KEYBOARD_OFFSET_Y + BUTTONS[ i ].y + BUTTONS[ i ].h;

            if ( opt_gx ) {
                x += 3;
                y += 1;
            } else {
                x -= SmallTextWidth( BUTTONS[ i ].letter, 1 ) / 2 + 5;
                y -= 2;
            }

            write_with_small_font( x, y, BUTTONS[ i ].letter, WHITE, ( i < HPKEY_MTH ) ? DISP_PAD : PAD );
        }

        // Bottom label: the only one is the cancel button
        if ( BUTTONS[ i ].sub != ( char* )0 ) {
            x = KEYBOARD_OFFSET_X + BUTTONS[ i ].x +
                ( 1 + BUTTONS[ i ].w - SmallTextWidth( BUTTONS[ i ].sub, strlen( BUTTONS[ i ].sub ) ) ) / 2;
            y = KEYBOARD_OFFSET_Y + BUTTONS[ i ].y + BUTTONS[ i ].h + small_ascent + 2;
            write_with_small_font( x, y, BUTTONS[ i ].sub, WHITE, PAD );
        }

        total_top_labels_width = 0;
        // Draw the left labels
        if ( BUTTONS[ i ].left != ( char* )0 ) {
            x = KEYBOARD_OFFSET_X + BUTTONS[ i ].x;
            y = KEYBOARD_OFFSET_Y + BUTTONS[ i ].y - small_descent;

            left_label_width = SmallTextWidth( BUTTONS[ i ].left, strlen( BUTTONS[ i ].left ) );
            total_top_labels_width = left_label_width;

            if ( BUTTONS[ i ].right != ( char* )0 ) {
                // label to the left
                right_label_width = SmallTextWidth( BUTTONS[ i ].right, strlen( BUTTONS[ i ].right ) );
                total_top_labels_width += space_char_width + right_label_width;
            }

            x += ( 1 + BUTTONS[ i ].w - total_top_labels_width ) / 2;

            write_with_small_font( x, y, BUTTONS[ i ].left, LEFT, BUTTONS[ i ].is_menu ? UNDERLAY : PAD );
        }

        // draw the right labels ( .is_menu never have one )
        if ( BUTTONS[ i ].right != ( char* )0 ) {
            x = KEYBOARD_OFFSET_X + BUTTONS[ i ].x;
            y = KEYBOARD_OFFSET_Y + BUTTONS[ i ].y - small_descent;

            if ( BUTTONS[ i ].left == ( char* )0 ) {
                right_label_width = SmallTextWidth( BUTTONS[ i ].right, strlen( BUTTONS[ i ].right ) );
                total_top_labels_width = right_label_width;
            } else
                x += space_char_width + left_label_width;

            x += ( 1 + BUTTONS[ i ].w - total_top_labels_width ) / 2;

            write_with_small_font( x, y, BUTTONS[ i ].right, RIGHT, PAD );
        }
    }

    for ( int i = FIRST_HPKEY; i <= LAST_HPKEY; i++ )
        _draw_key( i );
}

static void _draw_bezel_LCD( void )
{
    for ( int i = 0; i < DISP_FRAME; i++ ) {
        __draw_line( display_offset_x - i, display_offset_y + DISPLAY_HEIGHT + 2 * i, display_offset_x + DISPLAY_WIDTH + i,
                     display_offset_y + DISPLAY_HEIGHT + 2 * i, DISP_PAD_TOP );
        __draw_line( display_offset_x - i, display_offset_y + DISPLAY_HEIGHT + 2 * i + 1, display_offset_x + DISPLAY_WIDTH + i,
                     display_offset_y + DISPLAY_HEIGHT + 2 * i + 1, DISP_PAD_TOP );
        __draw_line( display_offset_x + DISPLAY_WIDTH + i, display_offset_y - i, display_offset_x + DISPLAY_WIDTH + i,
                     display_offset_y + DISPLAY_HEIGHT + 2 * i, DISP_PAD_TOP );

        __draw_line( display_offset_x - i - 1, display_offset_y - i - 1, display_offset_x + DISPLAY_WIDTH + i - 1, display_offset_y - i - 1,
                     DISP_PAD_BOT );
        __draw_line( display_offset_x - i - 1, display_offset_y - i - 1, display_offset_x - i - 1,
                     display_offset_y + DISPLAY_HEIGHT + 2 * i - 1, DISP_PAD_BOT );
    }

    // round off corners
    __draw_line( display_offset_x - DISP_FRAME, display_offset_y - DISP_FRAME, display_offset_x - DISP_FRAME + 3,
                 display_offset_y - DISP_FRAME, DISP_PAD );
    __draw_line( display_offset_x - DISP_FRAME, display_offset_y - DISP_FRAME, display_offset_x - DISP_FRAME,
                 display_offset_y - DISP_FRAME + 3, DISP_PAD );
    __draw_pixel( display_offset_x - DISP_FRAME + 1, display_offset_y - DISP_FRAME + 1, DISP_PAD );

    __draw_line( display_offset_x + DISPLAY_WIDTH + DISP_FRAME - 4, display_offset_y - DISP_FRAME,
                 display_offset_x + DISPLAY_WIDTH + DISP_FRAME - 1, display_offset_y - DISP_FRAME, DISP_PAD );
    __draw_line( display_offset_x + DISPLAY_WIDTH + DISP_FRAME - 1, display_offset_y - DISP_FRAME,
                 display_offset_x + DISPLAY_WIDTH + DISP_FRAME - 1, display_offset_y - DISP_FRAME + 3, DISP_PAD );
    __draw_pixel( display_offset_x + DISPLAY_WIDTH + DISP_FRAME - 2, display_offset_y - DISP_FRAME + 1, DISP_PAD );

    __draw_line( display_offset_x - DISP_FRAME, display_offset_y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 4, display_offset_x - DISP_FRAME,
                 display_offset_y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 1, DISP_PAD );
    __draw_line( display_offset_x - DISP_FRAME, display_offset_y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 1, display_offset_x - DISP_FRAME + 3,
                 display_offset_y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 1, DISP_PAD );
    __draw_pixel( display_offset_x - DISP_FRAME + 1, display_offset_y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 2, DISP_PAD );

    __draw_line( display_offset_x + DISPLAY_WIDTH + DISP_FRAME - 1, display_offset_y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 4,
                 display_offset_x + DISPLAY_WIDTH + DISP_FRAME - 1, display_offset_y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 1, DISP_PAD );
    __draw_line( display_offset_x + DISPLAY_WIDTH + DISP_FRAME - 4, display_offset_y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 1,
                 display_offset_x + DISPLAY_WIDTH + DISP_FRAME - 1, display_offset_y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 1, DISP_PAD );
    __draw_pixel( display_offset_x + DISPLAY_WIDTH + DISP_FRAME - 2, display_offset_y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 2, DISP_PAD );

    // simulate rounded lcd corners
    __draw_line( display_offset_x - 1, display_offset_y + 1, display_offset_x - 1, display_offset_y + DISPLAY_HEIGHT - 2, LCD );
    __draw_line( display_offset_x + 1, display_offset_y - 1, display_offset_x + DISPLAY_WIDTH - 2, display_offset_y - 1, LCD );
    __draw_line( display_offset_x + 1, display_offset_y + DISPLAY_HEIGHT, display_offset_x + DISPLAY_WIDTH - 2,
                 display_offset_y + DISPLAY_HEIGHT, LCD );
    __draw_line( display_offset_x + DISPLAY_WIDTH, display_offset_y + 1, display_offset_x + DISPLAY_WIDTH,
                 display_offset_y + DISPLAY_HEIGHT - 2, LCD );
}

static void _draw_background( int width, int height, int w_top, int h_top )
{
    __draw_rect( 0, 0, w_top, h_top, PAD );
    __draw_rect( 0, 0, width, height, DISP_PAD );
}

static void _draw_background_LCD( void ) { __draw_rect( display_offset_x, display_offset_y, DISPLAY_WIDTH, DISPLAY_HEIGHT, LCD ); }

// Show the hp key which is being pressed
static void _show_key( int hpkey )
{
    if ( config.hide_chrome || hpkey < 0 )
        return;

    SDL_SetRenderTarget( renderer, main_texture );

    _draw_key( hpkey );

    SDL_SetRenderTarget( renderer, NULL );
    SDL_RenderCopy( renderer, main_texture, NULL, NULL );
    SDL_RenderPresent( renderer );

    return;
}

static void _draw_serial_devices_path( void )
{
    char text[ 1024 ] = "";

    if ( config.verbose ) {
        fprintf( stderr, "wire_name: %s\n", wire_name );
        fprintf( stderr, "ir_name: %s\n", ir_name );
    }

    if ( wire_name ) {
        strcat( text, "wire: " );
        strcat( text, wire_name );
    }
    if ( ir_name ) {
        if ( strlen( text ) > 0 )
            strcat( text, " | " );

        strcat( text, "IR: " );
        strcat( text, ir_name );
    }

    if ( strlen( text ) > 0 )
        write_with_small_font( SIDE_SKIP, KEYBOARD_OFFSET_Y - ( DISP_KBD_SKIP / 2 ), text, WHITE, DISP_PAD );
}

static void sdl_draw_nibble( int nx, int ny, int val )
{
    SDL_SetRenderTarget( renderer, main_texture );

    for ( int x = 0; x < 4; x++ ) {
        if ( nx + x >= 131 ) // Clip at 131 pixels
            break;

        char pixel = val & ( 1 << ( x & 3 ) );

        __draw_rect( display_offset_x + 5 + ( 2 * ( nx + x ) ), display_offset_y + 20 + ( 2 * ny ), 2, 2, pixel ? PIXEL : LCD );
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

    int x = col * 4;
    if ( row <= display.lines )
        x -= ( 2 * display.offset );

    sdl_draw_nibble( x, row, val );
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

static int sdl_press_key( int hpkey )
{
    if ( hpkey == -1 || keyboard[ hpkey ].pressed )
        return -1;

    press_key( hpkey );
    _show_key( hpkey );

    return hpkey;
}

static int sdl_release_key( int hpkey )
{
    if ( hpkey == -1 || !keyboard[ hpkey ].pressed )
        return -1;

    release_key( hpkey );
    _show_key( hpkey );

    return hpkey;
}

/**********/
/* public */
/**********/
void sdl_get_event( void )
{
    SDL_Event event;
    int hpkey = -1;
    static int lasthpkey = -1; // last key that was pressed or -1 for none
    static int lastticks = -1; // time at which a key was pressed or -1 if timer expired

    // Iterate as long as there are events
    while ( SDL_PollEvent( &event ) ) {
        switch ( event.type ) {
            case SDL_QUIT:
                // please_exit = true;
                close_and_exit();
                break;

            case SDL_MOUSEBUTTONDOWN:
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
            case SDL_MOUSEBUTTONUP:
                hpkey = mouse_click_to_hpkey( event.button.x, event.button.y );
                if ( lasthpkey != hpkey || lastticks == -1 || ( SDL_GetTicks() - lastticks < 750 ) )
                    sdl_release_key( hpkey );

                lasthpkey = lastticks = -1;
                break;

            case SDL_KEYDOWN:
                sdl_press_key( sdlkey_to_hpkey( event.key.keysym.sym ) );
                break;
            case SDL_KEYUP:
                sdl_release_key( sdlkey_to_hpkey( event.key.keysym.sym ) );
                break;
        }
    }
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

    bool annunc_state;
    last_annunc_state = saturn.annunc;

    SDL_SetRenderTarget( renderer, main_texture );

    for ( int i = 0; i < NB_ANNUNCIATORS; i++ ) {
        annunc_state = ( ( annunciators_bits[ i ] & saturn.annunc ) == annunciators_bits[ i ] );

        __draw_texture( display_offset_x + ann_tbl[ i ].x, display_offset_y + ann_tbl[ i ].y, ann_tbl[ i ].width, ann_tbl[ i ].height,
                        ( annunc_state ) ? annunciators_textures[ i ].up : annunciators_textures[ i ].down );
    }

    // Always immediately update annunciators
    SDL_SetRenderTarget( renderer, NULL );
    SDL_RenderCopy( renderer, main_texture, NULL, NULL );
    SDL_RenderPresent( renderer );
}

void sdl_adjust_contrast( void )
{
    colors_setup();

    // redraw LCD
    ui_init_LCD();
    sdl_update_LCD();

    // redraw annunc
    last_annunc_state = -1;

    create_annunciators_textures();
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

    window = SDL_CreateWindow( config.progname, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width * config.scale,
                               height * config.scale, window_flags );
    if ( window == NULL ) {
        printf( "Couldn't create window: %s\n", SDL_GetError() );
        exit( 1 );
    }

    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE );
    if ( renderer == NULL )
        exit( 2 );

    SDL_RenderSetLogicalSize( renderer, width, height );

    main_texture = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_RGB444, SDL_TEXTUREACCESS_TARGET, width, height );

    SDL_SetRenderTarget( renderer, main_texture );

    colors_setup();

    create_annunciators_textures();

    if ( !config.hide_chrome ) {
        int cut = BUTTONS[ HPKEY_MTH ].y + KEYBOARD_OFFSET_Y - 19;

        create_buttons_textures();

        _draw_background( width, cut, width, height );
        _draw_bezel( cut, KEYBOARD_OFFSET_Y, width, height );
        _draw_header();
        _draw_bezel_LCD();
        _draw_keypad();

        _draw_serial_devices_path();
    }

    _draw_background_LCD();

    SDL_SetRenderTarget( renderer, NULL );
    SDL_RenderCopy( renderer, main_texture, NULL, NULL );
    SDL_RenderPresent( renderer );
}
