#include "ui.h"

void ( *ui__disp_draw_nibble )( word_20 addr, word_4 val );
void ( *ui__menu_draw_nibble )( word_20 addr, word_4 val );
int ( *ui__get_event )( void );
void ( *ui__update_LCD )( void );
void ( *ui__adjust_contrast )( void );
void ( *ui__draw_annunc )( void );
void ( *ui__init_LCD )( void );
void ( *init_ui )( int argc, char** argv );
