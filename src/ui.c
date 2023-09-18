#include "ui.h"
#include "runtime_options.h"

void ( *ui_disp_draw_nibble )( word_20 addr, word_4 val );
void ( *ui_menu_draw_nibble )( word_20 addr, word_4 val );
int ( *ui_get_event )( void );
void ( *ui_update_LCD )( void );
void ( *ui_adjust_contrast )( void );
void ( *ui_draw_annunc )( void );
void ( *ui_init_LCD )( void );
void ( *init_ui )( int argc, char** argv );

void setup_frontend( void ) {
    switch ( frontend_type ) {
        case FRONTEND_SDL:
            ui_disp_draw_nibble = sdl_disp_draw_nibble;
            ui_menu_draw_nibble = sdl_menu_draw_nibble;
            ui_get_event = sdl_get_event;
            ui_update_LCD = sdl_update_LCD;
            ui_adjust_contrast = sdl_adjust_contrast;
            ui_draw_annunc = sdl_draw_annunc;
            ui_init_LCD = sdl_init_LCD;
            init_ui = init_sdl_ui;
            break;

        case FRONTEND_X11:
        default:
            ui_disp_draw_nibble = x11_disp_draw_nibble;
            ui_menu_draw_nibble = x11_menu_draw_nibble;
            ui_get_event = x11_get_event;
            ui_update_LCD = x11_update_LCD;
            ui_adjust_contrast = x11_adjust_contrast;
            ui_draw_annunc = x11_draw_annunc;
            ui_init_LCD = x11_init_LCD;
            init_ui = init_x11_ui;
            break;
    }
}
