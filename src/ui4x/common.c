#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "inner.h"
#if defined( HAS_SDL )
#  include "sdl.h"
#endif
#if defined( HAS_GTK )
#  include "gtk.h"
#endif
#include "ncurses.h"

void ( *ui_get_event )( void );
void ( *ui_update_display )( void );
void ( *ui_start )( config_t* conf );
void ( *ui_stop )( void );

void setup_ui( config_t* conf )
{
    switch ( conf->frontend ) {
#if defined( HAS_GTK )
        case FRONTEND_GTK:
#  if !defined( HAS_SDL )
        default:
#  endif
            setup_frontend_gtk();
            break;
#endif
#if defined( HAS_SDL )
        case FRONTEND_SDL:
#  if !defined( HAS_GTK )
        default:
#  endif
            setup_frontend_sdl();
            break;
#endif
        case FRONTEND_NCURSES:
#if !defined( HAS_SDL ) && !defined( HAS_GTK )
        default:
#endif
            setup_frontend_ncurses();
            break;
    }
}

void close_and_exit( void )
{
    exit_emulator();

    ui_stop();

    exit( 0 );
}
