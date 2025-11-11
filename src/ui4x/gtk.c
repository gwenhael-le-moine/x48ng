#include "../options.h"

#include "api.h"

static config_t __config;
static void ( *press_key )( int hpkey );
static void ( *release_key )( int hpkey );
static bool ( *is_key_pressed )( int hpkey );

static unsigned char ( *get_annunciators )( void );
static bool ( *get_display_state )( void );
static void ( *get_lcd_buffer )( int* target );
static int ( *get_contrast )( void );

void ui_get_event_gtk( void ) {}

void ui_update_display_gtk( void ) {}

void ui_start_gtk( config_t* conf ) { __config = *conf; }

void ui_stop_gtk( void ) {}

void setup_frontend_gtk( void ( *emulator_api_press_key )( int hpkey ), void ( *emulator_api_release_key )( int hpkey ),
                         bool ( *emulator_api_is_key_pressed )( int hpkey ), unsigned char ( *emulator_api_get_annunciators )( void ),
                         bool ( *emulator_api_get_display_state )( void ), void ( *emulator_api_get_lcd_buffer )( int* target ),
                         int ( *emulator_api_get_contrast )( void ) )
{
    press_key = emulator_api_press_key;
    release_key = emulator_api_release_key;
    is_key_pressed = emulator_api_is_key_pressed;
    get_annunciators = emulator_api_get_annunciators;
    get_display_state = emulator_api_get_display_state;
    get_lcd_buffer = emulator_api_get_lcd_buffer;
    get_contrast = emulator_api_get_contrast;

    ui_get_event = ui_get_event_gtk;
    ui_update_display = ui_update_display_gtk;
    ui_start = ui_start_gtk;
    ui_stop = ui_stop_gtk;
}
