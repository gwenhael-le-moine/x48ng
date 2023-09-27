#ifndef _X48_GUI_H
#define _X48_GUI_H 1

#include "emulator.h" /* word_4; word_20; */

#define DISP_ROWS 64
#define NIBS_PER_BUFFER_ROW ( NIBBLES_PER_ROW + 2 )

/***********/
/* typedef */
/***********/
typedef struct letter_t {
    unsigned int w, h;
    unsigned char* bits;
} letter_t;

/*************/
/* variables */
/*************/
extern int last_annunc_state;

extern unsigned char disp_buf[ DISP_ROWS ][ NIBS_PER_BUFFER_ROW ];
extern unsigned char lcd_nibbles_buffer[ DISP_ROWS ][ NIBS_PER_BUFFER_ROW ];

extern letter_t small_font[ 128 ];

/*************/
/* functions */
/*************/
extern void x11_disp_draw_nibble( word_20 addr, word_4 val );
extern void x11_menu_draw_nibble( word_20 addr, word_4 val );
extern int x11_get_event( void );
extern void x11_update_LCD( void );
extern void x11_refresh_LCD( void );
extern void x11_adjust_contrast( void );
extern void x11_draw_annunc( void );
extern void x11_init_LCD( void );
extern void init_x11_ui( int argc, char** argv );

extern void sdl_disp_draw_nibble( word_20 addr, word_4 val );
extern void sdl_menu_draw_nibble( word_20 addr, word_4 val );
extern int sdl_get_event( void );
extern void sdl_update_LCD( void );
extern void sdl_refresh_LCD( void );
extern void sdl_adjust_contrast( void );
extern void sdl_draw_annunc( void );
extern void sdl_init_LCD( void );
extern void init_sdl_ui( int argc, char** argv );

extern void text_disp_draw_nibble( word_20 addr, word_4 val );
extern void text_menu_draw_nibble( word_20 addr, word_4 val );
extern int text_get_event( void );
extern void text_update_LCD( void );
extern void text_refresh_LCD( void );
extern void text_adjust_contrast( void );
extern void text_draw_annunc( void );
extern void text_init_LCD( void );
extern void init_text_ui( int argc, char** argv );

/*************************************************/
/* public API: if it's there it's used elsewhere */
/*************************************************/
/*************************/
/* used in: emu_memory.c */
/*************************/
extern void ( *ui_disp_draw_nibble )( word_20 addr, word_4 val );
extern void ( *ui_menu_draw_nibble )( word_20 addr, word_4 val );

/*****************************************/
/* used in: emu_actions.c, emu_emulate.c */
/*****************************************/
extern int ( *ui_get_event )( void );

/*****************************************************/
/* used in: emu_actions.c, emu_emulate.c, debugger.c */
/*****************************************************/
extern void ( *ui_update_LCD )( void );
extern void ( *ui_refresh_LCD )( void );

/**************************/
/* used in: emu_emulate.c */
/**************************/
extern void ( *ui_adjust_contrast )( void );
extern void ( *ui_draw_annunc )( void );

/*********************************/
/* used in: debugger.c, ui_sdl.c */
/*********************************/
extern void ( *ui_init_LCD )( void );

/*******************/
/* used in: main.c */
/*******************/
extern void ( *init_ui )( int argc, char** argv );

extern void setup_frontend( void );
#endif /* !_X48_GUI_H */
