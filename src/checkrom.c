#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "config.h"
#include "romio.h"

unsigned char* rom;
unsigned short rom_crc, crc;

int verbose = 0;
char* progname;

#define calc_crc( n )                                                          \
    ( crc = ( ( crc >> 4 ) ^ ( ( ( crc ^ n ) & 0xf ) * 0x1081 ) ) )

int main( int argc, char** argv ) {
    unsigned char version[ 7 ];
    long ver_addr;
    int i, a, c, d, d0, d1, D0, D1;
    int fail;

    if ( argc < 2 ) {
        fprintf( stderr, "usage: %s rom-file\n", argv[ 0 ] );
        exit( 1 );
    }

    if ( !read_rom_file( argv[ 1 ], &rom, &rom_size ) ) {
        fprintf( stderr, "%s: can\'t read ROM from %s\n", argv[ 0 ],
                 argv[ 1 ] );
        exit( 1 );
    }

    if ( opt_gx != 0 )
        ver_addr = 0x7ffbf;
    else
        ver_addr = 0x7fff0;

    for ( i = 0; i < 6; i++ ) {
        version[ i ] = rom[ ver_addr + 2 * i + 1 ] << 4;
        version[ i ] |= rom[ ver_addr + 2 * i ];
    }
    version[ 6 ] = '\0';
    printf( "ROM Version is %s\n", version );

    for ( i = 0x100; i < 0x140; i++ ) {
        rom[ i ] = 0x0;
    }

    fail = a = 0;
    D0 = 0x00000;
    D1 = 0x40000;
    for ( d = 1; d <= rom_size / 0x80000; d++ ) {

        crc = 0x0000;
        rom_crc = 0;
        for ( i = 0; i < 4; i++ ) {
            rom_crc <<= 4;
            rom_crc |= ( rom[ 0x80000 * d - i - 1 ] & 0x0f );
        }

        if ( opt_gx )
            printf( "ROM CRC %d reads 0x%.4x\n", d, rom_crc );
        else
            printf( "ROM CRC reads 0x%.4x\n", rom_crc );

        d0 = D0;
        d1 = D1;
        for ( c = 0x3fff; c >= 0x0000; c-- ) {
            for ( i = 0; i < 16; i++ ) {
                calc_crc( rom[ d0 + i ] );
            }
            d0 += 16;
            for ( i = 0; i < 16; i++ ) {
                calc_crc( rom[ d1 + i ] );
            }
            d1 += 16;
        }
        D0 += 0x80000;
        D1 += 0x80000;
        a = crc;
        a = ( ( a | 0xf0000 ) + 1 ) & 0xfffff;

        if ( a != 0x00000 ) {
            fail++;
        }
    }

    if ( fail != 0 )
        printf( "IROM %.4x: ROM CRC test FAILED !!!\n", a & 0xffff );
    else
        printf( "IROM OK: ROM CRC test passed.\n" );

    return 0;
}
