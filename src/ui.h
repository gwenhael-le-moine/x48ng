#ifndef _X48_GUI_H
#define _X48_GUI_H 1

#include "emulator.h" /* word_4; word_20; */

/*************************************************/
/* public API: if it's there it's used elsewhere */
/*************************************************/
/*************************/
/* used in: emu_memory.c */
/*************************/
extern void ui__disp_draw_nibble( word_20 addr, word_4 val );
extern void ui__menu_draw_nibble( word_20 addr, word_4 val );

/* extern void ui__draw_nibble( int c, int r, int val ); */

/*****************************************/
/* used in: emu_actions.c, emu_emulate.c */
/*****************************************/
extern int ui__get_event( void );

/*****************************************************/
/* used in: emu_actions.c, emu_emulate.c, debugger.c */
/*****************************************************/
extern void ui__update_LCD( void );

/**************************/
/* used in: emu_emulate.c */
/**************************/
extern void ui__adjust_contrast( void );
extern void ui__draw_annunc( void );

/*********************************/
/* used in: debugger.c, ui_sdl.c */
/*********************************/
extern void ui__init_LCD( void );

/*******************/
/* used in: main.c */
/*******************/
extern void init_ui( void );

#endif /* !_X48_GUI_H */
