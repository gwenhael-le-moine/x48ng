#ifndef _UI_INNER_H
#define _UI_INNER_H 1

#include "emulator_core.h"
#include "ui_bitmaps_misc.h"
#include "ui_bitmaps_small_font.h"
#include "ui_bitmaps_big_font.h"

// Colors
/*                          SX      GX     */
typedef enum {
    WHITE,        /* #ffffff #ffffff */
    LEFT,         /* #ffa600 #ffbaff */
    RIGHT,        /* #00d2ff #00ffcc */
    BUT_TOP,      /* #6d5d5d #646464 */
    BUTTON,       /* #5a4d4d #585858 */
    BUT_BOT,      /* #4c4141 #4a4a4a */
    LCD,          /* #cadd5c #cadd5c */
    PIXEL,        /* #000080 #000080 */
    PAD_TOP,      /* #6d4e4e #585858 */
    PAD,          /* #5a4040 #4a4a4a */
    PAD_BOT,      /* #4c3636 #404040 */
    DISP_PAD_TOP, /* #9b7654 #808080 */
    DISP_PAD,     /* #7c5e43 #68686e */
    DISP_PAD_BOT, /* #644b35 #54545a */
    LOGO,         /* #cca96b #b0b0b8 */
    LOGO_BACK,    /* #404040 #68686e */
    LABEL,        /* #cab890 #f0f0f0 */
    FRAME,        /* #000000 #000000 */
    UNDERLAY,     /* #3c2a2a #68686e */
    BLACK,        /* #000000 #000000 */
    NB_COLORS
} ui_colors_t;

/***********/
/* typedef */
/***********/
typedef struct letter_t {
    unsigned int w, h;
    unsigned char* bits;
} letter_t;

typedef struct color_t {
    const char* name;
    int r, g, b;
    int mono_rgb;
    int gray_rgb;
} color_t;

typedef struct button_t {
    const char* name;

    int x, y;
    int w, h;

    int lc;
    const char* label;
    short font_size;
    unsigned int lw, lh;
    unsigned char* lb;

    const char* letter;

    const char* left;
    short is_menu;
    const char* right;
    const char* sub;
} button_t;

typedef struct ann_struct_t {
    int x;
    int y;
    unsigned int width;
    unsigned int height;
    unsigned char* bits;
} ann_struct_t;

/*************/
/* variables */
/*************/
extern letter_t small_font[ 128 ];
extern letter_t big_font[ 128 ];

extern color_t colors_sx[ NB_COLORS ];
extern color_t colors_gx[ NB_COLORS ];
#define COLORS ( opt_gx ? colors_gx : colors_sx )

extern button_t buttons_sx[ NB_KEYS ];
extern button_t buttons_gx[ NB_KEYS ];
#define BUTTONS ( opt_gx ? buttons_gx : buttons_sx )

extern ann_struct_t ann_tbl[ NB_ANNUNCIATORS ];

#define small_ascent 8
#define small_descent 4

/*************/
/* functions */
/*************/
extern int SmallTextWidth( const char* string, unsigned int length );
extern int BigTextWidth( const char* string, unsigned int length );
extern void ui_init_LCD( void );

#endif /* _UI_INNER_H */
