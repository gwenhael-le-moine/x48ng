#include <stdio.h>
#include <stdlib.h>

void fatal_exit( char* error, char* advice ) {
    fprintf( stderr, "FATAL ERROR, exit.\n" );

    if ( error[ 0 ] != '\0' )
        fprintf( stderr, "  - %s\n", error );

    if ( advice[ 0 ] != '\0' )
        fprintf( stderr, "  - %s\n", advice );

    exit( 1 );
}
