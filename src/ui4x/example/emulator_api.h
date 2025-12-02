#ifndef EMULATOR_API_H
#define EMULATOR_API_H

#include <stdbool.h>

#include "../src/api.h"

extern void press_key( int hpkey );
extern void release_key( int hpkey );
extern bool is_key_pressed( int hpkey );

extern unsigned char get_annunciators( void );
extern bool get_display_state( void );
extern void get_lcd_buffer( int* target );
extern int get_contrast( void );

extern void init_emulator( ui4x_config_t* conf );
extern void exit_emulator( void );
extern void emulator_stop( void );

#endif /* EMULATOR_API_H */
