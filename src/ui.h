#ifndef _UI_H
#define _UI_H 1

#include "emulator.h" /* word_4; word_20; */

#define DISP_ROWS 64
#define NIBS_PER_BUFFER_ROW ( NIBBLES_PER_ROW + 2 )

/*************/
/* variables */
/*************/
extern int last_annunc_state;

extern unsigned char lcd_nibbles_buffer[ DISP_ROWS ][ NIBS_PER_BUFFER_ROW ];

/*************/
/* functions */
/*************/
#ifdef HAS_SDL
extern void init_sdl2_ui( int argc, char** argv );
extern void sdl2_ui_stop( void );
#endif

extern void init_text_ui( int argc, char** argv );
extern void text_ui_stop( void );

/*************************************************/
/* public API: if it's there it's used elsewhere */
/*************************************************/
/*************************/
/* used in: emu_memory.c */
/*************************/
extern void ( *ui_disp_draw_nibble )( word_20 addr, word_4 val );
extern void ( *ui_menu_draw_nibble )( word_20 addr, word_4 val );

/*****************************************/
/* used in: emu_emulate.c */
/*****************************************/
extern void ( *ui_get_event )( void );
extern void ( *ui_adjust_contrast )( void );
extern void ( *ui_draw_annunc )( void );

/*****************************************************/
/* used in: emu_emulate.c, debugger.c */
/*****************************************************/
extern void ( *ui_update_LCD )( void );
extern void ( *ui_refresh_LCD )( void );

/*******************/
/* used in: main.c */
/*******************/
extern void ui_stop( void );
extern void start_UI( int argc, char** argv );

extern void close_and_exit( void );

#endif /* !_UI_H */
