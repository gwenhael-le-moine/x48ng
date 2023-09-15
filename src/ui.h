#ifndef _X48_GUI_H
#define _X48_GUI_H 1

#include "emulator.h" /* word_4; word_20; */

/**************/
/* public API */
/**************/
/* used in: hp48emu_memory.c */
typedef struct disp_t {
    unsigned int w, h;
    short mapped;
    int offset;
    int lines;
} disp_t;
extern disp_t disp;

extern void ui__disp_draw_nibble( word_20 addr, word_4 val );
extern void ui__menu_draw_nibble( word_20 addr, word_4 val );

/* used in: hp48emu_actions.c, hp48_emulate.c */
extern int ui__get_event( void );

/* used in: hp48emu_actions.c, hp48_emulate.c, debugger.c */
extern void ui__update_LCD( void );

/* used in: hp48_emulate.c */
extern void ui__adjust_contrast();
extern void ui__draw_annunc( void );

/* used in: debugger.c, ui_sdl.c */
extern void ui__init_LCD( void );

/* used in: main.c */
extern void ui__init( void );

#endif /* !_X48_GUI_H */
