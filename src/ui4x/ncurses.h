#ifndef _UI4x_NCURSES_H
#  define _UI4x_NCURSES_H 1

#  include <stdbool.h>

extern void setup_frontend_ncurses( void ( *emulator_api_press_key )( int hpkey ), void ( *emulator_api_release_key )( int hpkey ),
                                    bool ( *emulator_api_is_key_pressed )( int hpkey ),
                                    unsigned char ( *emulator_api_get_annunciators )( void ),
                                    bool ( *emulator_api_get_display_state )( void ), void ( *emulator_api_get_lcd_buffer )( int* target ),
                                    int ( *emulator_api_get_contrast )( void ) );

#endif /* _UI4x_NCURSES_H */
