#include <stdlib.h>

#include "api.h"
#if defined( HAS_SDL )
#  include "sdl.h"
#endif
#if defined( HAS_GTK )
#  include "gtk.h"
#endif
#include "ncurses.h"

void ( *ui_get_event )( void );
void ( *ui_update_display )( void );
void ( *ui_start )( void );
void ( *ui_stop )( void );

ui4x_config_t ui4x_config;

static void ( *emulator_exit )( void );

void setup_ui( ui4x_config_t* conf, void ( *emulator_api_press_key )( int hpkey ), void ( *emulator_api_release_key )( int hpkey ),
               bool ( *emulator_api_is_key_pressed )( int hpkey ), unsigned char ( *emulator_api_get_annunciators )( void ),
               bool ( *emulator_api_get_display_state )( void ), void ( *emulator_api_get_lcd_buffer )( int* target ),
               int ( *emulator_api_get_contrast )( void ), void ( *exit_emulator )( void ) )
{
    ui4x_config = *conf;

    emulator_exit = exit_emulator;

    switch ( conf->frontend ) {
#if defined( HAS_GTK )
        case FRONTEND_GTK:
#  if !defined( HAS_SDL )
        default:
#  endif
            setup_frontend_gtk( emulator_api_press_key, emulator_api_release_key, emulator_api_is_key_pressed,
                                emulator_api_get_annunciators, emulator_api_get_display_state, emulator_api_get_lcd_buffer,
                                emulator_api_get_contrast );
            break;
#endif
#if defined( HAS_SDL )
        case FRONTEND_SDL:
#  if !defined( HAS_GTK )
        default:
#  endif
            setup_frontend_sdl( emulator_api_press_key, emulator_api_release_key, emulator_api_is_key_pressed,
                                emulator_api_get_annunciators, emulator_api_get_display_state, emulator_api_get_lcd_buffer,
                                emulator_api_get_contrast );
            break;
#endif
        case FRONTEND_NCURSES:
#if !defined( HAS_SDL ) && !defined( HAS_GTK )
        default:
#endif
            setup_frontend_ncurses( emulator_api_press_key, emulator_api_release_key, emulator_api_is_key_pressed,
                                    emulator_api_get_annunciators, emulator_api_get_display_state, emulator_api_get_lcd_buffer,
                                    emulator_api_get_contrast );
            break;
    }
}

void close_and_exit( void )
{
    emulator_exit();

    ui_stop();

    exit( 0 );
}
