#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

unsigned char* core;

#define DEFAULT_ROM_FILE "rom.dump"

bool write_mem_file( const char* name, unsigned char* mem, size_t size )
{
    FILE* fp;
    unsigned char* tmp_mem;
    unsigned char byte;
    int i, j;

    if ( NULL == ( fp = fopen( name, "w" ) ) ) {
        fprintf( stderr, "can\'t open %s\n", name );
        return 0;
    }

    if ( NULL == ( tmp_mem = ( unsigned char* )malloc( size / 2 ) ) ) {
        for ( i = 0, j = 0; i < ( int )size / 2; i++ ) {
            byte = ( mem[ j++ ] & 0x0f );
            byte |= ( unsigned char )( ( mem[ j++ ] << 4 ) & 0xf0 );
            if ( 1 != fwrite( &byte, 1, 1, fp ) ) {
                fprintf( stderr, "can\'t write %s\n", name );
                fclose( fp );
                return false;
            }
        }
    } else {
        for ( i = 0, j = 0; i < ( int )size / 2; i++ ) {
            tmp_mem[ i ] = ( mem[ j++ ] & 0x0f );
            tmp_mem[ i ] |= ( unsigned char )( ( mem[ j++ ] << 4 ) & 0xf0 );
        }

        if ( fwrite( tmp_mem, 1, ( size_t )size / 2, fp ) != size / 2 ) {
            fprintf( stderr, "can\'t write %s\n", name );
            fclose( fp );
            free( tmp_mem );
            return false;
        }

        free( tmp_mem );
    }

    fclose( fp );
    return true;
}

int main( int argc, char** argv )
{
    FILE* dump;
    long addr;
    size_t size;
    int ch, i;
    bool gx, error;

    if ( argc < 2 ) {
        fprintf( stderr, "usage: %s hp48-dump-file\n", argv[ 0 ] );
        exit( 1 );
    }

    if ( ( dump = fopen( argv[ 1 ], "r" ) ) == NULL ) {
        fprintf( stderr, "%s: can\'t open %s\n", argv[ 0 ], argv[ 1 ] );
        exit( 1 );
    }

    if ( ( core = ( unsigned char* )malloc( 0x100000 ) ) == NULL ) {
        fprintf( stderr, "%s: can\'t malloc %d bytes\n", argv[ 0 ], 0x100000 );
        exit( 1 );
    }
    memset( core, 0, 0x100000 );

    gx = false;
    error = false;
    while ( 1 ) {
        addr = 0;
        for ( i = 0; i < 5; i++ ) {
            addr <<= 4;
            if ( ( ch = fgetc( dump ) ) < 0 ) {
                error = true;
                break;
            }
            if ( ch >= '0' && ch <= '9' ) {
                addr |= ch - '0';
            } else if ( ch >= 'A' && ch <= 'F' ) {
                addr |= ch - 'A' + 10;
            } else {
                fprintf( stderr, "%s: Illegal char %c at %lx\n", argv[ 0 ], ch, addr );
                error = true;
                break;
            }
        }
        if ( error )
            break;
        if ( addr >= 0x80000 )
            gx = true;
        if ( ( ch = fgetc( dump ) ) < 0 ) {
            fprintf( stderr, "%s: Unexpected EOF at %lx\n", argv[ 0 ], addr );
            break;
        }
        if ( ch != ':' ) {
            fprintf( stderr, "%s: Illegal char %c, expected \':\' at %lx\n", argv[ 0 ], ch, addr );
            break;
        }
        for ( i = 0; i < 16; i++ ) {
            if ( ( ch = fgetc( dump ) ) < 0 ) {
                fprintf( stderr, "%s: Unexpected EOF at %lx\n", argv[ 0 ], addr );
                error = true;
                break;
            }
            if ( ch >= '0' && ch <= '9' ) {
                core[ addr++ ] = ( unsigned char )( ch - '0' );
            } else if ( ch >= 'A' && ch <= 'F' ) {
                core[ addr++ ] = ( unsigned char )( ch - 'A' + 10 );
            } else {
                fprintf( stderr, "%s: Illegal char %c at %lx\n", argv[ 0 ], ch, addr );
                error = true;
                break;
            }
        }
        if ( error )
            break;
        if ( ( ch = fgetc( dump ) ) < 0 )
            break;
        if ( ch != '\n' ) {
            fprintf( stderr, "%s: Illegal char %c, expected \'\\n\' at %lx\n", argv[ 0 ], ch, addr );
            break;
        }
    }

    if ( !gx && core[ 0x29 ] == 0x0 )
        gx = true;

    if ( gx )
        size = 0x100000;
    else
        size = 0x80000;
    if ( !write_mem_file( DEFAULT_ROM_FILE, core, size ) ) {
        fprintf( stderr, "%s: can\'t write to %s\n", argv[ 0 ], DEFAULT_ROM_FILE );
        exit( 1 );
    }

    exit( 0 );
}
