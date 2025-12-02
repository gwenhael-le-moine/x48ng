#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/api.h"

#define KEYBOARD ( __config.model == MODEL_48GX || __config.model == MODEL_48SX ? keyboard48 : keyboard49 )

typedef struct hpkey_t {
    bool pressed;
} hpkey_t;

static hpkey_t keyboard48[ NB_HP48_KEYS ] = {
    /* From top left to bottom right */
    { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false },
    { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false },
    { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false },
    { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false },
    { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false },
    { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false },
    { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false },
    { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false },
    { .pressed = false },
};

static hpkey_t keyboard49[ NB_HP4950_KEYS ] = {
    /* From top left to bottom right */
    { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false },
    { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false },
    { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false },
    { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false },
    { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false },
    { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false },
    { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false },
    { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false }, { .pressed = false },
    { .pressed = false }, { .pressed = false }, { .pressed = false },
};

static ui4x_config_t __config;

void press_key( int hpkey )
{
    if ( hpkey < 0 || hpkey > NB_HP4950_KEYS )
        return;
    // Check not already pressed (may be important: avoids a useless do_kbd_int)
    if ( KEYBOARD[ hpkey ].pressed )
        return;

    KEYBOARD[ hpkey ].pressed = true;
}

void release_key( int hpkey )
{
    if ( hpkey < 0 || hpkey > NB_HP4950_KEYS )
        return;
    // Check not already released (not critical)
    if ( !KEYBOARD[ hpkey ].pressed )
        return;

    KEYBOARD[ hpkey ].pressed = false;
}

bool is_key_pressed( int hpkey )
{
    if ( hpkey < 0 || hpkey > NB_HP4950_KEYS )
        return false;

    return KEYBOARD[ hpkey ].pressed;
}

unsigned char get_annunciators( void ) { return 0b111111; }

bool get_display_state( void ) { return true /* mod_status.hdw.lcd_on */; }

void get_lcd_buffer( int* target )
{
    for ( int y = 0; y < ui_get_lcd_height(); ++y )
        for ( int x = 0; x < LCD_WIDTH; ++x )
            target[ ( LCD_WIDTH * y ) + x ] = x % ( __config.model == MODEL_50G ? 16 : 4 );
}

static int contrast = 19;
int get_contrast( void ) { return contrast; }

void init_emulator( ui4x_config_t* conf )
{
    __config = *conf;

    conf->wire_name = ( char* )"dummy wire" /* SerialInit() */;
}

extern bool please_exit; /* main.c */
void exit_emulator( void ) { please_exit = true; }

void emulator_stop( void ) { exit_emulator(); }
