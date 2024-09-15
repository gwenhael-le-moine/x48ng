#ifndef _UI_INNER_H
#define _UI_INNER_H 1

#include "emulator.h"
#include "ui_bitmaps_misc.h"
#include "ui_bitmaps_small_font.h"
#include "ui_bitmaps_big_font.h"

// Colors
/*                          SX      GX     */
#define WHITE 0         /* #ffffff #ffffff */
#define LEFT 1          /* #ffa600 #ffbaff */
#define RIGHT 2         /* #00d2ff #00ffcc */
#define BUT_TOP 3       /* #6d5d5d #646464 */
#define BUTTON 4        /* #5a4d4d #585858 */
#define BUT_BOT 5       /* #4c4141 #4a4a4a */
#define LCD 6           /* #cadd5c #cadd5c */
#define PIXEL 7         /* #000080 #000080 */
#define PAD_TOP 8       /* #6d4e4e #585858 */
#define PAD 9           /* #5a4040 #4a4a4a */
#define PAD_BOT 10      /* #4c3636 #404040 */
#define DISP_PAD_TOP 11 /* #9b7654 #808080 */
#define DISP_PAD 12     /* #7c5e43 #68686e */
#define DISP_PAD_BOT 13 /* #644b35 #54545a */
#define LOGO 14         /* #cca96b #b0b0b8 */
#define LOGO_BACK 15    /* #404040 #68686e */
#define LABEL 16        /* #cab890 #f0f0f0 */
#define FRAME 17        /* #000000 #000000 */
#define UNDERLAY 18     /* #3c2a2a #68686e */
#define BLACK 19        /* #000000 #000000 */

#define FIRST_COLOR WHITE
#define LAST_COLOR BLACK
#define NB_COLORS ( LAST_COLOR + 1 )

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
