#ifndef _X48_GUI_H
#define _X48_GUI_H 1

#include "hp48.h" /* word_4; word_20; */

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

extern void disp_draw_nibble( word_20 addr, word_4 val );
extern void menu_draw_nibble( word_20 addr, word_4 val );

/* used in: hp48emu_actions.c, hp48_emulate.c */
extern int get_ui_event( void );
extern void update_display( void );

/* used in: hp48_emulate.c */
extern void adjust_contrast();
extern void draw_annunc( void );

/* used in: main.c */
extern void SDLInit( void );
extern void SDLCreateHP( void );
extern void init_display( void );

#endif /* !_X48_GUI_H */
