#ifndef _X48_GUI_H
#define _X48_GUI_H 1

#include "emulator.h" /* word_4; word_20; */

/*************/
/* functions */
/*************/
extern void ui_sdl__disp_draw_nibble( word_20 addr, word_4 val );
extern void ui_x11__disp_draw_nibble( word_20 addr, word_4 val );

extern void ui_sdl__menu_draw_nibble( word_20 addr, word_4 val );
extern void ui_x11__menu_draw_nibble( word_20 addr, word_4 val );

extern int ui_sdl__get_event( void );
extern int ui_x11__get_event( void );

extern void ui_sdl__update_LCD( void );
extern void ui_x11__update_LCD( void );

extern void ui_sdl__adjust_contrast( void );
extern void ui_x11__adjust_contrast( void );

extern void ui_sdl__draw_annunc( void );
extern void ui_x11__draw_annunc( void );

extern void ui_sdl__init_LCD( void );
extern void ui_x11__init_LCD( void );

extern void init_sdl_ui( int argc, char** argv );
extern void init_x11_ui( int argc, char** argv );

/*************************************************/
/* public API: if it's there it's used elsewhere */
/*************************************************/
/*************************/
/* used in: emu_memory.c */
/*************************/
extern void ( *ui__disp_draw_nibble )( word_20 addr, word_4 val );
extern void ( *ui__menu_draw_nibble )( word_20 addr, word_4 val );

/*****************************************/
/* used in: emu_actions.c, emu_emulate.c */
/*****************************************/
extern int ( *ui__get_event )( void );

/*****************************************************/
/* used in: emu_actions.c, emu_emulate.c, debugger.c */
/*****************************************************/
extern void ( *ui__update_LCD )( void );

/**************************/
/* used in: emu_emulate.c */
/**************************/
extern void ( *ui__adjust_contrast )( void );
extern void ( *ui__draw_annunc )( void );

/*********************************/
/* used in: debugger.c, ui_sdl.c */
/*********************************/
extern void ( *ui__init_LCD )( void );

/*******************/
/* used in: main.c */
/*******************/
extern void ( *init_ui )( int argc, char** argv );

#endif /* !_X48_GUI_H */
