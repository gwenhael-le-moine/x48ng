#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "options.h"
#include "romio.h"

#include "core/emulate.h"
#include "core/init.h"
#include "core/memory.h"

#include "ui4x/src/api.h"

#define KEYBOARD keyboard48

#define IN_VAL( keycode ) ( 1 << ( keycode & 0xf ) )
#define OUT_BIT( keycode ) ( keycode >> 4 )

typedef struct hpkey_t {
    int code;
    bool pressed;
} hpkey_t;

static hpkey_t keyboard48[ NB_HP48_KEYS ] = {
    /* From top left to bottom right */
    {0x14,   false},
    {0x84,   false},
    {0x83,   false},
    {0x82,   false},
    {0x81,   false},
    {0x80,   false},

    {0x24,   false},
    {0x74,   false},
    {0x73,   false},
    {0x72,   false},
    {0x71,   false},
    {0x70,   false},

    {0x04,   false},
    {0x64,   false},
    {0x63,   false},
    {0x62,   false},
    {0x61,   false},
    {0x60,   false},

    {0x34,   false},
    {0x54,   false},
    {0x53,   false},
    {0x52,   false},
    {0x51,   false},
    {0x50,   false},

    {0x44,   false},
    {0x43,   false},
    {0x42,   false},
    {0x41,   false},
    {0x40,   false},

    {0x35,   false},
    {0x33,   false},
    {0x32,   false},
    {0x31,   false},
    {0x30,   false},

    {0x25,   false},
    {0x23,   false},
    {0x22,   false},
    {0x21,   false},
    {0x20,   false},

    {0x15,   false},
    {0x13,   false},
    {0x12,   false},
    {0x11,   false},
    {0x10,   false},

    {0x8000, false},
    {0x03,   false},
    {0x02,   false},
    {0x01,   false},
    {0x00,   false},
};

static config_t __config;

void press_key( int hpkey )
{
    if ( hpkey < 0 || hpkey > ui_get_nb_keys() )
        return;
    // Check not already pressed (may be important: avoids a useless do_kbd_int)
    if ( KEYBOARD[ hpkey ].pressed )
        return;

    KEYBOARD[ hpkey ].pressed = true;

    if ( KEYBOARD[ hpkey ].code == 0x8000 ) { /* HPKEY_ON */
        for ( int i = 0; i < KEYS_BUFFER_SIZE; i++ )
            saturn.keybuf[ i ] |= 0x8000;
        do_kbd_int();
    } else {
        if ( ( saturn.keybuf[ OUT_BIT( KEYBOARD[ hpkey ].code ) ] & IN_VAL( KEYBOARD[ hpkey ].code ) ) == 0 ) {
            if ( saturn.kbd_ien )
                do_kbd_int();
            if ( ( saturn.keybuf[ OUT_BIT( KEYBOARD[ hpkey ].code ) ] & IN_VAL( KEYBOARD[ hpkey ].code ) ) )
                fprintf( stderr, "bug\n" );

            saturn.keybuf[ OUT_BIT( KEYBOARD[ hpkey ].code ) ] |= IN_VAL( KEYBOARD[ hpkey ].code );
        }
    }
}

void release_key( int hpkey )
{
    if ( hpkey < 0 || hpkey > ui_get_nb_keys() )
        return;
    // Check not already released (not critical)
    if ( !KEYBOARD[ hpkey ].pressed )
        return;

    KEYBOARD[ hpkey ].pressed = false;

    if ( KEYBOARD[ hpkey ].code == 0x8000 ) {
        for ( int i = 0; i < KEYS_BUFFER_SIZE; i++ )
            saturn.keybuf[ i ] &= ~0x8000;
    } else
        saturn.keybuf[ OUT_BIT( KEYBOARD[ hpkey ].code ) ] &= ~( IN_VAL( KEYBOARD[ hpkey ].code ) );
}

bool is_key_pressed( int hpkey )
{
    if ( hpkey < 0 || hpkey > ui_get_nb_keys() )
        return false;

    return KEYBOARD[ hpkey ].pressed;
}

// unsigned char get_annunciators( void ) { return saturn.annunc; }

typedef enum {
    ANN_LEFT = 0x81,
    ANN_RIGHT = 0x82,
    ANN_ALPHA = 0x84,
    ANN_BATTERY = 0x88,
    ANN_BUSY = 0x90,
    ANN_IO = 0xa0,
} annunciators_bits_t;
unsigned char get_annunciators( void )
{
    const int annunciators_bits[ NB_ANNUNCIATORS ] = { ANN_LEFT, ANN_RIGHT, ANN_ALPHA, ANN_BATTERY, ANN_BUSY, ANN_IO };
    char annunciators = 0;

    for ( int i = 0; i < NB_ANNUNCIATORS; i++ )
        if ( ( annunciators_bits[ i ] & saturn.annunc ) == annunciators_bits[ i ] )
            annunciators |= 0x01 << i;

    return annunciators;
}

bool get_display_state( void ) { return display.on; }

void get_lcd_buffer( int* target )
{
    Address addr = display.disp_start; // mod_status.hdw.lcd_base_addr;
    int x, y;
    Nibble v;

    /* Scan active display rows */
    for ( y = 0; y <= display.lines /* mod_status.hdw.lcd_vlc */; y++ ) {
        /* Scan columns */
        for ( x = 0; x < NIBBLES_PER_ROW; x++ ) {
            v = bus_fetch_nibble( addr++ );

            // split nibble
            for ( int nx = 0; nx < 4; nx++ )
                target[ ( y * LCD_WIDTH ) + ( x * 4 ) + nx ] = ( v & ( 1 << ( nx & 3 ) ) ) > 0 ? 4 : 0;
        }

        addr += display.offset; // mod_status.hdw.lcd_line_offset;
    }

    /* Scan menu display rows */
    addr = display.menu_start; // mod_status.hdw.lcd_menu_addr;
    for ( ; y < ui_get_lcd_height(); y++ ) {
        /* Scan columns */
        for ( x = 0; x < NIBBLES_PER_ROW; x++ ) {
            v = bus_fetch_nibble( addr++ );

            // split nibble
            for ( int nx = 0; nx < 4; nx++ )
                target[ ( y * LCD_WIDTH ) + ( x * 4 ) + nx ] = ( v & ( 1 << ( nx & 3 ) ) ) > 0 ? 4 : 0;
        }
    }
}

int get_contrast( void ) { return display.contrast; }

void init_emulator( config_t* conf )
{
    __config = *conf;

    start_emulator();

    conf->model = opt_gx ? MODEL_48GX : MODEL_48SX;
}

void exit_emulator( void )
{
    stop_emulator();

    exit_ui();

    exit( EXIT_SUCCESS );
}
