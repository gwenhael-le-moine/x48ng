#include <stdio.h>

#include "emulator.h"
#include "emulator_inner.h"

void press_key( int hpkey )
{
    // Check not already pressed (may be important: avoids a useless do_kbd_int)
    if ( keyboard[ hpkey ].pressed )
        return;

    keyboard[ hpkey ].pressed = true;

    int code = keyboard[ hpkey ].code;
    if ( code == 0x8000 ) { /* HPKEY_ON */
        for ( int i = 0; i < 9; i++ )
            saturn.keybuf.rows[ i ] |= 0x8000;
        do_kbd_int();
    } else {
        int r = code >> 4;
        int c = 1 << ( code & 0xf );
        if ( ( saturn.keybuf.rows[ r ] & c ) == 0 ) {
            if ( saturn.kbd_ien )
                do_kbd_int();
            if ( ( saturn.keybuf.rows[ r ] & c ) )
                fprintf( stderr, "bug\n" );

            saturn.keybuf.rows[ r ] |= c;
        }
    }
}

void release_key( int hpkey )
{
    // Check not already released (not critical)
    if ( !keyboard[ hpkey ].pressed )
        return;

    keyboard[ hpkey ].pressed = false;

    int code = keyboard[ hpkey ].code;
    if ( code == 0x8000 ) {
        for ( int i = 0; i < 9; i++ )
            saturn.keybuf.rows[ i ] &= ~0x8000;
    } else {
        int r = code >> 4;
        int c = 1 << ( code & 0xf );
        saturn.keybuf.rows[ r ] &= ~c;
    }
}

void release_all_keys( void )
{
    for ( int hpkey = FIRST_HPKEY; hpkey <= LAST_HPKEY; hpkey++ )
        if ( keyboard[ hpkey ].pressed )
            release_key( hpkey );
}
