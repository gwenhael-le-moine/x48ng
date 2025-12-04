#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sys/times.h>
#include <unistd.h>

#include "api.h"
#include "gtk.h"
#include "inner.h"
#include "ncurses.h"
#ifdef HAS_SDL
#  include "sdl.h"
#endif

char* ui_annunciators[ NB_ANNUNCIATORS ] = { "⮢", "⮣", "α", "🪫", "⌛", "⇄" };

ui4x_config_t ui4x_config;
ui4x_emulator_api_t ui4x_emulator_api;

/*************/
/* functions */
/*************/
static void newrplify_buttons_50g( void )
{
    if ( ui4x_config.model != MODEL_50G )
        return;

    // modify keys' labeling for newRPL
    for ( int i = HP4950_KEY_A; i <= HP4950_KEY_F; i++ ) {
        buttons_50g[ i ].left = "";

        buttons_50g[ i ].left_sdl = "";
    }

    for ( int i = HP4950_KEY_G; i <= HP4950_KEY_I; i++ ) {
        buttons_50g[ i ].label = "";
        buttons_50g[ i ].left = "";
        buttons_50g[ i ].right = NULL;

        buttons_50g[ i ].label_sdl = "";
        buttons_50g[ i ].left_sdl = "";
        buttons_50g[ i ].right_sdl = NULL;
    }

    for ( int i = HP4950_KEY_J; i <= HP4950_KEY_K; i++ ) {
        buttons_50g[ i ].label = "";
        buttons_50g[ i ].left = "";
        buttons_50g[ i ].right = NULL;

        buttons_50g[ i ].label_sdl = "";
        buttons_50g[ i ].left_sdl = "";
        buttons_50g[ i ].right_sdl = NULL;
    }

    buttons_50g[ HP4950_KEY_UP ].left = "UPDIR";
    buttons_50g[ HP4950_KEY_UP ].left_sdl = "UPDIR";

    buttons_50g[ HP4950_KEY_LEFT ].left = "BEG";
    buttons_50g[ HP4950_KEY_LEFT ].left_sdl = "BEG";
    buttons_50g[ HP4950_KEY_LEFT ].right = "COPY";
    buttons_50g[ HP4950_KEY_LEFT ].right_sdl = "COPY";

    buttons_50g[ HP4950_KEY_DOWN ].left = "CUT";
    buttons_50g[ HP4950_KEY_DOWN ].left_sdl = "CUT";

    buttons_50g[ HP4950_KEY_RIGHT ].left = "END";
    buttons_50g[ HP4950_KEY_RIGHT ].left_sdl = "END";
    buttons_50g[ HP4950_KEY_RIGHT ].right = "PASTE";
    buttons_50g[ HP4950_KEY_RIGHT ].right_sdl = "PASTE";

    buttons_50g[ HP4950_KEY_M ].label = "STO⏵";
    buttons_50g[ HP4950_KEY_M ].left = "RCL";
    buttons_50g[ HP4950_KEY_M ].right = "PREV.M";

    buttons_50g[ HP4950_KEY_M ].label_sdl = "STO";
    buttons_50g[ HP4950_KEY_M ].left_sdl = "RCL";
    buttons_50g[ HP4950_KEY_M ].right_sdl = "PREV.M";

    for ( int i = HP4950_KEY_N; i <= HP4950_KEY_O; i++ ) {
        buttons_50g[ i ].left = "";
        buttons_50g[ i ].left_sdl = "";
        buttons_50g[ i ].right = NULL;
        buttons_50g[ i ].right_sdl = NULL;
    }

    buttons_50g[ HP4950_KEY_P ].label = "MENU";
    buttons_50g[ HP4950_KEY_P ].label_sdl = "MENU";

    buttons_50g[ HP4950_KEY_BACKSPACE ].left = "";
    buttons_50g[ HP4950_KEY_BACKSPACE ].left_sdl = "";

    for ( int i = HP4950_KEY_S; i <= HP4950_KEY_U; i++ ) {
        buttons_50g[ i ].right = NULL;
        buttons_50g[ i ].right_sdl = NULL;
    }

    for ( int i = HP4950_KEY_ALPHA; i <= HP4950_KEY_9; i++ ) {
        buttons_50g[ i ].left = "";
        buttons_50g[ i ].left_sdl = "";
    }

    buttons_50g[ HP4950_KEY_8 ].right = NULL;

    for ( int i = HP4950_KEY_4; i <= HP4950_KEY_6; i++ ) {
        buttons_50g[ i ].left = "";
        buttons_50g[ i ].left_sdl = "";
        buttons_50g[ i ].right = NULL;
        buttons_50g[ i ].right_sdl = NULL;
    }

    buttons_50g[ HP4950_KEY_2 ].left = "";
    buttons_50g[ HP4950_KEY_2 ].left_sdl = "";

    buttons_50g[ HP4950_KEY_ON ].left = "";
    buttons_50g[ HP4950_KEY_ON ].left_sdl = "";
    buttons_50g[ HP4950_KEY_ON ].below = NULL;

    buttons_50g[ HP4950_KEY_ENTER ].left_sdl = "";
    buttons_50g[ HP4950_KEY_ENTER ].left = "";
}

/********************/
/* Public functions */
/********************/
int ui_get_lcd_height( void ) { return LCD_HEIGHT; }

int ui_get_nb_keys( void ) { return NB_KEYS; }

int ui_get_n_levels_of_gray( void ) { return N_LEVELS_OF_GRAY; }

void ui_handle_pending_inputs( void )
{
    switch ( ui4x_config.frontend ) {
        case FRONTEND_NCURSES:
            ncurses_handle_pending_inputs();
            break;
#ifdef HAS_SDL
        case FRONTEND_SDL:
            sdl_ui_handle_pending_inputs();
            break;
#endif
        case FRONTEND_GTK:
        default:
            gtk_ui_handle_pending_inputs();
            break;
    }
}

void ui_refresh_output( void )
{
    switch ( ui4x_config.frontend ) {
        case FRONTEND_NCURSES:
            ncurses_refresh_lcd();
            break;
#ifdef HAS_SDL
        case FRONTEND_SDL:
            sdl_ui_refresh_lcd();
            break;
#endif
        case FRONTEND_GTK:
        default:
            gtk_ui_refresh_lcd();
            break;
    }
}

void init_ui( ui4x_config_t* config, ui4x_emulator_api_t* emulator_api )
{
    ui4x_config = *config;
    ui4x_emulator_api = *emulator_api;

    if ( ui4x_config.newrpl_keyboard )
        newrplify_buttons_50g();

    switch ( ui4x_config.frontend ) {
        case FRONTEND_NCURSES:
            ncurses_init();
            break;
#ifdef HAS_SDL
        case FRONTEND_SDL:
            sdl_init_ui();
            break;
#endif
        case FRONTEND_GTK:
        default:
            gtk_init_ui();
            break;
    }
}

void exit_ui( void )
{
    switch ( ui4x_config.frontend ) {
        case FRONTEND_NCURSES:
            ncurses_exit();
            break;
#ifdef HAS_SDL
        case FRONTEND_SDL:
            sdl_exit_ui();
            break;
#endif
        case FRONTEND_GTK:
        default:
            gtk_exit_ui();
            break;
    }
}
