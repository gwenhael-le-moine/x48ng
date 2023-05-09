#include <stdio.h>
#include <stdlib.h>

#include "resources.h"

char errbuf[ 1024 ] = {
    0,
};
char fixbuf[ 1024 ] = {
    0,
};

void fatal_exit( void ) {
    if ( quiet )
        exit( 1 );

    if ( errbuf[ 0 ] == '\0' ) {
        fprintf( stderr, "FATAL ERROR, exit.\n" );
        exit( 1 );
    }

    fprintf( stderr, "FATAL ERROR, exit.\n  - %s\n", errbuf );

    if ( fixbuf[ 0 ] != '\0' )
        fprintf( stderr, "  - %s\n", fixbuf );

    exit( 1 );
}
