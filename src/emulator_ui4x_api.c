#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "emulate.h"
#include "emulator_ui4x_api.h"
#include "init.h"
#include "memory.h"
#include "options.h"
#include "romio.h"

#define KEYBOARD keyboard48

typedef struct hpkey_t {
    int code;
    bool pressed;
    char* inOut;
} hpkey_t;

static hpkey_t keyboard48[ NB_HP48_KEYS ] = {
    /* From top left to bottom right */
    {0x14,   false, ( char* )"1/10"},
    {0x84,   false, ( char* )"8/10"},
    {0x83,   false, ( char* )"8/08"},
    {0x82,   false, ( char* )"8/04"},
    {0x81,   false, ( char* )"8/02"},
    {0x80,   false, ( char* )"8/01"},

    {0x24,   false, ( char* )"2/10"},
    {0x74,   false, ( char* )"7/10"},
    {0x73,   false, ( char* )"7/08"},
    {0x72,   false, ( char* )"7/04"},
    {0x71,   false, ( char* )"7/02"},
    {0x70,   false, ( char* )"7/01"},

    {0x04,   false, ( char* )"0/10"},
    {0x64,   false, ( char* )"6/10"},
    {0x63,   false, ( char* )"6/08"},
    {0x62,   false, ( char* )"6/04"},
    {0x61,   false, ( char* )"6/02"},
    {0x60,   false, ( char* )"6/01"},

    {0x34,   false, ( char* )"3/10"},
    {0x54,   false, ( char* )"5/10"},
    {0x53,   false, ( char* )"5/08"},
    {0x52,   false, ( char* )"5/04"},
    {0x51,   false, ( char* )"5/02"},
    {0x50,   false, ( char* )"5/01"},

    {0x44,   false, ( char* )"4/10"},
    {0x43,   false, ( char* )"4/08"},
    {0x42,   false, ( char* )"4/04"},
    {0x41,   false, ( char* )"4/02"},
    {0x40,   false, ( char* )"4/01"},

    {0x35,   false, ( char* )"3/20"},
    {0x33,   false, ( char* )"3/08"},
    {0x32,   false, ( char* )"3/04"},
    {0x31,   false, ( char* )"3/02"},
    {0x30,   false, ( char* )"3/01"},

    {0x25,   false, ( char* )"2/20"},
    {0x23,   false, ( char* )"2/08"},
    {0x22,   false, ( char* )"2/04"},
    {0x21,   false, ( char* )"2/02"},
    {0x20,   false, ( char* )"2/01"},

    {0x15,   false, ( char* )"1/20"},
    {0x13,   false, ( char* )"1/08"},
    {0x12,   false, ( char* )"1/04"},
    {0x11,   false, ( char* )"1/02"},
    {0x10,   false, ( char* )"1/01"},

    {0x8000, false, ( char* )"*"   },
    {0x03,   false, ( char* )"0/08"},
    {0x02,   false, ( char* )"0/04"},
    {0x01,   false, ( char* )"0/02"},
    {0x00,   false, ( char* )"0/01"},
};

void press_key( int hpkey )
{
    if ( hpkey < 0 || hpkey > NB_KEYS )
        return;
    // Check not already pressed (may be important: avoids a useless do_kbd_int)
    if ( KEYBOARD[ hpkey ].pressed )
        return;

    KEYBOARD[ hpkey ].pressed = true;

    // press_key( hpkey );
    KEYBOARD[ hpkey ].pressed = true;

    int code = KEYBOARD[ hpkey ].code;
    if ( code == 0x8000 ) { /* HPKEY_ON */
        for ( int i = 0; i < KEYS_BUFFER_SIZE; i++ )
            saturn.keybuf[ i ] |= 0x8000;
        do_kbd_int();
    } else {
        int r = code >> 4;
        int c = 1 << ( code & 0xf );
        if ( ( saturn.keybuf[ r ] & c ) == 0 ) {
            if ( saturn.kbd_ien )
                do_kbd_int();
            if ( ( saturn.keybuf[ r ] & c ) )
                fprintf( stderr, "bug\n" );

            saturn.keybuf[ r ] |= c;
        }
    }
}

void release_key( int hpkey )
{
    if ( hpkey < 0 || hpkey > NB_KEYS )
        return;
    // Check not already released (not critical)
    if ( !KEYBOARD[ hpkey ].pressed )
        return;

    KEYBOARD[ hpkey ].pressed = false;

    // release_key( hpkey );
    KEYBOARD[ hpkey ].pressed = false;

    int code = KEYBOARD[ hpkey ].code;
    if ( code == 0x8000 ) {
        for ( int i = 0; i < KEYS_BUFFER_SIZE; i++ )
            saturn.keybuf[ i ] &= ~0x8000;
    } else {
        int r = code >> 4;
        int c = 1 << ( code & 0xf );
        saturn.keybuf[ r ] &= ~c;
    }
}

bool is_key_pressed( int hpkey )
{
    if ( hpkey < 0 || hpkey > NB_KEYS )
        return false;

    return KEYBOARD[ hpkey ].pressed;
}

unsigned char get_annunciators( void ) { return saturn.annunc; }

bool get_display_state( void ) { return display.on; }

void get_lcd_buffer( int* target ) { memcpy( target, &lcd_pixels_buffer, sizeof( lcd_pixels_buffer ) ); }

int get_contrast( void ) { return display.contrast; }

void init_emulator( config_t* conf )
{
    start_emulator();
    conf->model = opt_gx ? MODEL_48GX : MODEL_48SX;
}

void exit_emulator( void ) { stop_emulator(); }
