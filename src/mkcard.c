#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

unsigned char* mem;

int write_mem_file( char* name, unsigned char* mem, int size ) {
    FILE* fp;

    if ( NULL == ( fp = fopen( name, "w" ) ) ) {
        fprintf( stderr, "can\'t open %s\n", name );
        return 0;
    }

    if ( fwrite( mem, 1, ( size_t )size, fp ) != size ) {
        fprintf( stderr, "can\'t write %s\n", name );
        fclose( fp );
        return 0;
    }

    fclose( fp );
    return 1;
}

int main( int argc, char** argv ) {
    long size;
    char* name;
    char* asize;
    unsigned char* core;

    if ( argc < 2 ) {
        fprintf( stderr, "usage: %s [32K | 128K | 1M | 2M | 4M] file-name\n",
                 argv[ 0 ] );
        exit( 1 );
    }

    name = argv[ 2 ];
    asize = argv[ 1 ];
    if ( !strcmp( asize, "32K" ) )
        size = 0x8000;
    else if ( !strcmp( asize, "128K" ) )
        size = 0x20000;
    else if ( !strcmp( asize, "256K" ) )
        size = 0x40000;
    else if ( !strcmp( asize, "512K" ) )
        size = 0x80000;
    else if ( !strcmp( asize, "1M" ) )
        size = 0x100000;
    else if ( !strcmp( asize, "2M" ) )
        size = 0x200000;
    else if ( !strcmp( asize, "4M" ) )
        size = 0x400000;
    else {
        fprintf(
            stderr,
            "%s: size must be one of 32K, 128K, 256K, 512K, 1M, 2M, or 4M\n",
            argv[ 0 ] );
        exit( 1 );
    }

    if ( ( core = ( unsigned char* )malloc( size ) ) == NULL ) {
        fprintf( stderr, "%s: can\'t malloc %ld bytes\n", argv[ 0 ], size );
        exit( 1 );
    }
    memset( core, 0, size );

    if ( !write_mem_file( name, core, size ) ) {
        fprintf( stderr, "%s: can\'t write to %s\n", argv[ 0 ], name );
        exit( 1 );
    }

    exit( 0 );
}
