#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>

#include <sys/time.h>

#include "../src/api.h"

#include "emulator_api.h"
#include "options.h"

#define UI_REFRESH_RATE_Hz 64

bool please_exit = false;

void signal_handler( int sig )
{
    switch ( sig ) {
        case SIGALRM:
            ui_handle_pending_inputs();
            ui_refresh_output();
            break;
        case SIGPIPE:
            please_exit = true;
        default:
            break;
    }
}

int main( int argc, char** argv )
{
    ui4x_config_t config = *config_init( argc, argv );

    init_emulator( &config );

    /* (G)UI */
    ui4x_emulator_api_t emulator_api = { .press_key = press_key,
                                         .release_key = release_key,
                                         .is_key_pressed = is_key_pressed,
                                         .is_display_on = get_display_state,
                                         .get_annunciators = get_annunciators,
                                         .get_lcd_buffer = get_lcd_buffer,
                                         .get_contrast = get_contrast,
                                         .do_stop = emulator_stop };
    init_ui( &config, &emulator_api );
    /* ui_start(); */

    sigset_t set;
    struct sigaction sa;
    sigemptyset( &set );
    sigaddset( &set, SIGALRM );
    sa.sa_handler = signal_handler;
    sa.sa_mask = set;
#ifdef SA_RESTART
    sa.sa_flags = SA_RESTART;
#endif
    sigaction( SIGALRM, &sa, ( struct sigaction* )0 );

    /************************************/
    /* set the real time interval timer */
    /************************************/
    /*
      Every <interval>µs setitimer will trigger a SIGALRM
      which will getUI events and refresh UI in signal_handler
     */
    struct itimerval it;
    it.it_interval.tv_sec = 0;
    it.it_interval.tv_usec = 1000000 / UI_REFRESH_RATE_Hz;
    it.it_value.tv_sec = it.it_interval.tv_sec;
    it.it_value.tv_usec = it.it_interval.tv_usec;
    setitimer( ITIMER_REAL, &it, ( struct itimerval* )0 );

    while ( !please_exit ) {
        // fprintf( stderr, "." );
    }

    exit_ui();

    return 0;
}
