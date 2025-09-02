#include "../options.h"
#include "api.h"
#include "common.h"
#include "inner.h"

static config_t __config;

void ui_get_event_gtk( void ) {}

void ui_update_display_gtk( void ) {}

void ui_start_gtk( config_t* conf ) { __config = *conf; }

void ui_stop_gtk( void ) {}

void setup_frontend_gtk( void )
{
    ui_get_event = ui_get_event_gtk;
    ui_update_display = ui_update_display_gtk;
    ui_start = ui_start_gtk;
    ui_stop = ui_stop_gtk;
}
