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

extern unsigned char lcd_nibbles_buffer_2[ DISP_ROWS ][ NIBS_PER_BUFFER_ROW ];
extern unsigned char lcd_nibbles_buffer_1[ DISP_ROWS ][ NIBS_PER_BUFFER_ROW ];
extern unsigned char lcd_nibbles_buffer_0[ DISP_ROWS ][ NIBS_PER_BUFFER_ROW ];
extern unsigned char greyscale_lcd_buffer[ DISP_ROWS ][ NIBS_PER_BUFFER_ROW * NIBBLES_NB_BITS ];

extern letter_t small_font[ 128 ];

/*************/
/* functions */
/*************/
#ifdef HAS_X11
extern void init_x11_ui( int argc, char** argv );
#endif

#ifdef HAS_SDL
extern void init_sdl_ui( int argc, char** argv );
#endif

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

/*******************/
/* used in: main.c */
/*******************/
extern void ( *init_ui )( int argc, char** argv );

/*********************************/
/* used in: debugger.c, ui_sdl.c */
/*********************************/
extern void ui_init_LCD( void );

extern void setup_frontend( void );
#endif /* !_X48_GUI_H */
