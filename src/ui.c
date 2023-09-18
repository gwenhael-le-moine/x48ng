#include "ui.h"
#include "runtime_options.h"

void ( *ui__disp_draw_nibble )( word_20 addr, word_4 val );
void ( *ui__menu_draw_nibble )( word_20 addr, word_4 val );
int ( *ui__get_event )( void );
void ( *ui__update_LCD )( void );
void ( *ui__adjust_contrast )( void );
void ( *ui__draw_annunc )( void );
void ( *ui__init_LCD )( void );
void ( *init_ui )( int argc, char** argv );

void setup_frontend( void ) {
    switch ( frontend_type ) {
        case FRONTEND_SDL:
            ui__disp_draw_nibble = ui_sdl__disp_draw_nibble;
            ui__menu_draw_nibble = ui_sdl__menu_draw_nibble;
            ui__get_event = ui_sdl__get_event;
            ui__update_LCD = ui_sdl__update_LCD;
            ui__adjust_contrast = ui_sdl__adjust_contrast;
            ui__draw_annunc = ui_sdl__draw_annunc;
            ui__init_LCD = ui_sdl__init_LCD;
            init_ui = init_sdl_ui;
            break;

        case FRONTEND_X11:
        default:
            ui__disp_draw_nibble = ui_x11__disp_draw_nibble;
            ui__menu_draw_nibble = ui_x11__menu_draw_nibble;
            ui__get_event = ui_x11__get_event;
            ui__update_LCD = ui_x11__update_LCD;
            ui__adjust_contrast = ui_x11__adjust_contrast;
            ui__draw_annunc = ui_x11__draw_annunc;
            ui__init_LCD = ui_x11__init_LCD;
            init_ui = init_x11_ui;
            break;
    }
}
