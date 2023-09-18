#include "ui.h"
#include "runtime_options.h"

display_t display;

extern void ui_sdl__disp_draw_nibble( word_20 addr, word_4 val );
extern void ui_x11__disp_draw_nibble( word_20 addr, word_4 val );
void ui__disp_draw_nibble( word_20 addr, word_4 val ) {
    switch ( frontend_type ) {
        case FRONTEND_SDL:
            return ui_sdl__disp_draw_nibble( addr, val );
        case FRONTEND_X11:
            return ui_x11__disp_draw_nibble( addr, val );
    }
}

extern void ui_sdl__menu_draw_nibble( word_20 addr, word_4 val );
extern void ui_x11__menu_draw_nibble( word_20 addr, word_4 val );
void ui__menu_draw_nibble( word_20 addr, word_4 val ) {
    switch ( frontend_type ) {
        case FRONTEND_SDL:
            return ui_sdl__menu_draw_nibble( addr, val );
        case FRONTEND_X11:
            return ui_x11__menu_draw_nibble( addr, val );
    }
}

extern int ui_sdl__get_event( void );
extern int ui_x11__get_event( void );
int ui__get_event( void ) {
    switch ( frontend_type ) {
        case FRONTEND_SDL:
            return ui_sdl__get_event();
        case FRONTEND_X11:
            return ui_x11__get_event();
    }
    return -1;
}

extern void ui_sdl__update_LCD( void );
extern void ui_x11__update_LCD( void );
void ui__update_LCD( void ) {
    switch ( frontend_type ) {
        case FRONTEND_SDL:
            return ui_sdl__update_LCD();
        case FRONTEND_X11:
            return ui_x11__update_LCD();
    }
}

extern void ui_sdl__adjust_contrast( void );
extern void ui_x11__adjust_contrast( void );
void ui__adjust_contrast( void ) {
    switch ( frontend_type ) {
        case FRONTEND_SDL:
            return ui_sdl__adjust_contrast();
        case FRONTEND_X11:
            return ui_x11__adjust_contrast();
    }
}

extern void ui_sdl__draw_annunc( void );
extern void ui_x11__draw_annunc( void );
void ui__draw_annunc( void ) {
    switch ( frontend_type ) {
        case FRONTEND_SDL:
            return ui_sdl__draw_annunc();
        case FRONTEND_X11:
            return ui_x11__draw_annunc();
    }
}

extern void ui_sdl__init_LCD( void );
extern void ui_x11__init_LCD( void );
void ui__init_LCD( void ) {
    switch ( frontend_type ) {
        case FRONTEND_SDL:
            return ui_sdl__init_LCD();
        case FRONTEND_X11:
            return ui_x11__init_LCD();
    }
}

extern void init_sdl_ui( int argc, char** argv );
extern void init_x11_ui( int argc, char** argv );
void init_ui( int argc, char** argv ) {
    switch ( frontend_type ) {
        case FRONTEND_SDL:
            return init_sdl_ui( argc, argv );
        case FRONTEND_X11:
            return init_x11_ui( argc, argv );
    }
}
