#include <stdio.h>
#include <stdlib.h>

#include "emulator.h"
#include "emulator_inner.h"

extern long nibble_masks[ 16 ];

int start_fields[] = { -1, 0, 2, 0, 15, 3, 0, 0, -1, 0, 2, 0, 15, 3, 0, 0, 0, 0, 0 };

int end_fields[] = { -1, -1, 2, 2, 15, 14, 1, 15, -1, -1, 2, 2, 15, 14, 1, 4, 3, 2, 0 };

int get_start( int code )
{
    int s;

    if ( ( s = start_fields[ code ] ) == -1 )
        s = saturn.P;

    return s; /* FIXME: potentially return uninitialized s ? */
}

int get_end( int code )
{
    int e;

    if ( ( e = end_fields[ code ] ) == -1 )
        e = saturn.P;

    return e; /* FIXME: potentially return uninitialized e ? */
}

void add_register( unsigned char* res, unsigned char* r1, unsigned char* r2, int code )
{
    int t;
    int s = get_start( code );
    int e = get_end( code );
    int c = 0;

    for ( int i = s; i <= e; i++ ) {
        t = r1[ i ] + r2[ i ] + c;
        if ( t < ( int )saturn.hexmode ) {
            res[ i ] = t & 0xf;
            c = 0;
        } else {
            res[ i ] = ( t - saturn.hexmode ) & 0xf;
            c = 1;
        }
    }

    saturn.CARRY = c ? 1 : 0;
}

void add_p_plus_one( unsigned char* r )
{
    int t;
    int s = 0;
    int e = 4;
    int c = saturn.P + 1;

    for ( int i = s; i <= e; i++ ) {
        t = r[ i ] + c;
        if ( t < 16 ) {
            r[ i ] = t & 0xf;
            c = 0;
        } else {
            r[ i ] = ( t - 16 ) & 0xf;
            c = 1;
        }
    }

    saturn.CARRY = c ? 1 : 0;
}

void sub_register( unsigned char* res, unsigned char* r1, unsigned char* r2, int code )
{
    int t;
    int s = get_start( code );
    int e = get_end( code );
    int c = 0;

    for ( int i = s; i <= e; i++ ) {
        t = r1[ i ] - r2[ i ] - c;
        if ( t >= 0 ) {
            res[ i ] = t & 0xf;
            c = 0;
        } else {
            res[ i ] = ( t + saturn.hexmode ) & 0xf;
            c = 1;
        }
    }
    saturn.CARRY = c ? 1 : 0;
}

void complement_2_register( unsigned char* r, int code )
{
    int t;
    int s = get_start( code );
    int e = get_end( code );
    int c = 1;
    int carry = 0;

    for ( int i = s; i <= e; i++ ) {
        t = ( saturn.hexmode - 1 ) - r[ i ] + c;
        if ( t < ( int )saturn.hexmode ) {
            r[ i ] = t & 0xf;
            c = 0;
        } else {
            r[ i ] = ( t - saturn.hexmode ) & 0xf;
            c = 1;
        }
        carry += r[ i ];
    }

    saturn.CARRY = carry ? 1 : 0;
}

void complement_1_register( unsigned char* r, int code )
{
    int t;
    int s = get_start( code );
    int e = get_end( code );

    for ( int i = s; i <= e; i++ ) {
        t = ( saturn.hexmode - 1 ) - r[ i ];
        r[ i ] = t & 0xf;
    }
    saturn.CARRY = 0;
}

void inc_register( unsigned char* r, int code )
{
    int t;
    int s = get_start( code );
    int e = get_end( code );
    int c = 1;

    for ( int i = s; i <= e; i++ ) {
        t = r[ i ] + c;
        if ( t < ( int )saturn.hexmode ) {
            r[ i ] = t & 0xf;
            c = 0;
            break;
        } else {
            r[ i ] = ( t - saturn.hexmode ) & 0xf;
            c = 1;
        }
    }

    saturn.CARRY = c ? 1 : 0;
}

void add_register_constant( unsigned char* r, int code, int val )
{
    int t;
    int s = get_start( code );
    int e = get_end( code );
    int c = val;

    for ( int i = s; i <= e; i++ ) {
        t = r[ i ] + c;
        if ( t < 16 ) {
            r[ i ] = t & 0xf;
            c = 0;
            break;
        } else {
            r[ i ] = ( t - 16 ) & 0xf;
            c = 1;
        }
    }

    saturn.CARRY = c ? 1 : 0;
}

void dec_register( unsigned char* r, int code )
{
    int t;
    int s = get_start( code );
    int e = get_end( code );
    int c = 1;

    for ( int i = s; i <= e; i++ ) {
        t = r[ i ] - c;
        if ( t >= 0 ) {
            r[ i ] = t & 0xf;
            c = 0;
            break;
        } else {
            r[ i ] = ( t + saturn.hexmode ) & 0xf;
            c = 1;
        }
    }

    saturn.CARRY = c ? 1 : 0;
}

void sub_register_constant( unsigned char* r, int code, int val )
{
    int t;
    int s = get_start( code );
    int e = get_end( code );
    int c = val;

    for ( int i = s; i <= e; i++ ) {
        t = r[ i ] - c;
        if ( t >= 0 ) {
            r[ i ] = t & 0xf;
            c = 0;
            break;
        } else {
            r[ i ] = ( t + 16 ) & 0xf;
            c = 1;
        }
    }

    saturn.CARRY = c ? 1 : 0;
}

void zero_register( unsigned char* r, int code )
{
    int s = get_start( code );
    int e = get_end( code );

    for ( int i = s; i <= e; i++ )
        r[ i ] = 0;
}

void or_register( unsigned char* res, unsigned char* r1, unsigned char* r2, int code )
{
    int s = get_start( code );
    int e = get_end( code );

    for ( int i = s; i <= e; i++ )
        res[ i ] = ( r1[ i ] | r2[ i ] ) & 0xf;
}

void and_register( unsigned char* res, unsigned char* r1, unsigned char* r2, int code )
{
    int s = get_start( code );
    int e = get_end( code );

    for ( int i = s; i <= e; i++ )
        res[ i ] = ( r1[ i ] & r2[ i ] ) & 0xf;
}

void copy_register( unsigned char* to, unsigned char* from, int code )
{
    int s = get_start( code );
    int e = get_end( code );

    for ( int i = s; i <= e; i++ )
        to[ i ] = from[ i ];
}

void exchange_register( unsigned char* r1, unsigned char* r2, int code )
{
    int t;
    int s = get_start( code );
    int e = get_end( code );

    for ( int i = s; i <= e; i++ ) {
        t = r1[ i ];
        r1[ i ] = r2[ i ];
        r2[ i ] = t;
    }
}

void exchange_reg( unsigned char* r, word_20* d, int code )
{
    int t;
    int s = get_start( code );
    int e = get_end( code );

    for ( int i = s; i <= e; i++ ) {
        t = r[ i ];
        r[ i ] = ( *d >> ( i * 4 ) ) & 0x0f;
        *d &= ~nibble_masks[ i ];
        *d |= t << ( i * 4 );
    }
}

void shift_left_register( unsigned char* r, int code )
{
    int s = get_start( code );
    int e = get_end( code );

    for ( int i = e; i > s; i-- )
        r[ i ] = r[ i - 1 ] & 0x0f;

    r[ s ] = 0;
}

void shift_left_circ_register( unsigned char* r, int code )
{
    int s = get_start( code );
    int e = get_end( code );
    int t = r[ e ] & 0x0f;

    for ( int i = e; i > s; i-- )
        r[ i ] = r[ i - 1 ] & 0x0f;

    r[ s ] = t;
}

void shift_right_register( unsigned char* r, int code )
{
    int s = get_start( code );
    int e = get_end( code );

    if ( r[ s ] & 0x0f )
        saturn.SB = 1;

    for ( int i = s; i < e; i++ )
        r[ i ] = r[ i + 1 ] & 0x0f;

    r[ e ] = 0;
}

void shift_right_circ_register( unsigned char* r, int code )
{
    int s = get_start( code );
    int e = get_end( code );
    int t = r[ s ] & 0x0f;

    for ( int i = s; i < e; i++ )
        r[ i ] = r[ i + 1 ] & 0x0f;

    r[ e ] = t;
    if ( t )
        saturn.SB = 1;
}

void shift_right_bit_register( unsigned char* r, int code )
{
    int t;
    int s = get_start( code );
    int e = get_end( code );
    int sb = 0;

    for ( int i = e; i >= s; i-- ) {
        t = ( ( ( r[ i ] >> 1 ) & 7 ) | ( sb << 3 ) ) & 0x0f;
        sb = r[ i ] & 1;
        r[ i ] = t;
    }
    if ( sb )
        saturn.SB = 1;
}

int is_zero_register( unsigned char* r, int code )
{
    int s = get_start( code );
    int e = get_end( code );
    int z = 1;

    for ( int i = s; i <= e; i++ )
        if ( ( r[ i ] & 0xf ) != 0 ) {
            z = 0;
            break;
        }

    return z;
}

int is_not_zero_register( unsigned char* r, int code )
{
    int s = get_start( code );
    int e = get_end( code );
    int z = 0;

    for ( int i = s; i <= e; i++ )
        if ( ( r[ i ] & 0xf ) != 0 ) {
            z = 1;
            break;
        }

    return z;
}

int is_equal_register( unsigned char* r1, unsigned char* r2, int code )
{
    int s = get_start( code );
    int e = get_end( code );
    int z = 1;

    for ( int i = s; i <= e; i++ )
        if ( ( r1[ i ] & 0xf ) != ( r2[ i ] & 0xf ) ) {
            z = 0;
            break;
        }

    return z;
}

int is_not_equal_register( unsigned char* r1, unsigned char* r2, int code )
{
    int s = get_start( code );
    int e = get_end( code );
    int z = 0;

    for ( int i = s; i <= e; i++ )
        if ( ( r1[ i ] & 0xf ) != ( r2[ i ] & 0xf ) ) {
            z = 1;
            break;
        }

    return z;
}

int is_less_register( unsigned char* r1, unsigned char* r2, int code )
{
    int s = get_start( code );
    int e = get_end( code );
    int z = 0;

    for ( int i = e; i >= s; i-- ) {
        if ( ( int )( r1[ i ] & 0xf ) < ( int )( r2[ i ] & 0xf ) ) {
            z = 1;
            break;
        }
        if ( ( int )( r1[ i ] & 0xf ) > ( int )( r2[ i ] & 0xf ) ) {
            z = 0;
            break;
        }
    }

    return z;
}

int is_less_or_equal_register( unsigned char* r1, unsigned char* r2, int code )
{
    int s = get_start( code );
    int e = get_end( code );
    int z = 1;

    for ( int i = e; i >= s; i-- ) {
        if ( ( int )( r1[ i ] & 0xf ) < ( int )( r2[ i ] & 0xf ) ) {
            z = 1;
            break;
        }
        if ( ( int )( r1[ i ] & 0xf ) > ( int )( r2[ i ] & 0xf ) ) {
            z = 0;
            break;
        }
    }

    return z;
}

int is_greater_register( unsigned char* r1, unsigned char* r2, int code )
{
    int s = get_start( code );
    int e = get_end( code );
    int z = 0;

    for ( int i = e; i >= s; i-- ) {
        if ( ( int )( r1[ i ] & 0xf ) > ( int )( r2[ i ] & 0xf ) ) {
            z = 1;
            break;
        }
        if ( ( int )( r1[ i ] & 0xf ) < ( int )( r2[ i ] & 0xf ) ) {
            z = 0;
            break;
        }
    }

    return z;
}

int is_greater_or_equal_register( unsigned char* r1, unsigned char* r2, int code )
{
    int s = get_start( code );
    int e = get_end( code );
    int z = 1;

    for ( int i = e; i >= s; i-- ) {
        if ( ( int )( r1[ i ] & 0xf ) < ( int )( r2[ i ] & 0xf ) ) {
            z = 0;
            break;
        }
        if ( ( int )( r1[ i ] & 0xf ) > ( int )( r2[ i ] & 0xf ) ) {
            z = 1;
            break;
        }
    }

    return z;
}
