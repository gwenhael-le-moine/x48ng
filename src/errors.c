#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "resources.h"

char errbuf[ 1024 ] = {
    0,
};
char fixbuf[ 1024 ] = {
    0,
};

void fatal_exit() {
    if ( quiet )
        exit( 1 );

    if ( errbuf[ 0 ] == '\0' ) {
        fprintf( stderr, "%s: FATAL ERROR, exit.\n", progname );
        exit( 1 );
    }

    fprintf( stderr, "%s: FATAL ERROR, exit.\n  - %s\n", progname, errbuf );

    if ( fixbuf[ 0 ] != '\0' )
        fprintf( stderr, "  - %s\n", fixbuf );

    exit( 1 );
}
