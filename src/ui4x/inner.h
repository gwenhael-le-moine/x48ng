#ifndef _UI4x_INNER_H
#define _UI4x_INNER_H 1

#include "../emulator_ui4x_api.h"

#include "bitmaps_misc.h"

/* 4.1.1.1: When defined, this symbol represents the threshold of the long
   key pression.  When the mouse button is kept pressed on a calculator's key
   for more than LONG_PRESS_THR milliseconds, the key stays pressed after
   release.
*/
#define LONG_PRESS_THR 750

// Colors
typedef enum {
    UI4X_COLOR_HP_LOGO = 0,
    UI4X_COLOR_HP_LOGO_BG,
    UI4X_COLOR_48GX_128K_RAM,
    UI4X_COLOR_FRAME,
    UI4X_COLOR_UPPER_FACEPLATE_EDGE_TOP,
    UI4X_COLOR_UPPER_FACEPLATE,
    UI4X_COLOR_UPPER_FACEPLATE_EDGE_BOTTOM,
    UI4X_COLOR_FACEPLATE_EDGE_TOP,
    UI4X_COLOR_FACEPLATE,
    UI4X_COLOR_FACEPLATE_EDGE_BOTTOM,
    UI4X_COLOR_KEYPAD_HIGHLIGHT,
    UI4X_COLOR_BUTTON_EDGE_TOP,
    UI4X_COLOR_BUTTON,
    UI4X_COLOR_BUTTON_EDGE_BOTTOM,
    UI4X_COLOR_PIXEL_OFF,
    UI4X_COLOR_PIXEL_GREY_1,
    UI4X_COLOR_PIXEL_GREY_2,
    UI4X_COLOR_PIXEL_ON,
    UI4X_COLOR_BLACK_PIXEL_OFF,
    UI4X_COLOR_BLACK_PIXEL_GREY_1,
    UI4X_COLOR_BLACK_PIXEL_GREY_2,
    UI4X_COLOR_BLACK_PIXEL_ON,
    UI4X_COLOR_LABEL,
    UI4X_COLOR_ALPHA,
    UI4X_COLOR_SHIFT_LEFT,
    UI4X_COLOR_SHIFT_RIGHT,
    NB_COLORS,
} colors_t;

/***********/
/* typedef */
/***********/
typedef struct letter_t {
    unsigned int w, h;
    unsigned char* bits;
} letter_t;

typedef struct color_t {
    int r, g, b, a;
    int mono_rgb;
    int gray_rgb;
    int rgb;
} color_t;

typedef struct button_t {
    int x, y;
    int w, h;

    bool highlight;

    /* label on the button (text or bitmap) */
    int label_color;
    const char* label_text;
    unsigned char* label_graphic;
    unsigned int label_graphic_w, label_graphic_h;

    /* labels around the button */
    const char* letter;
    const char* left;
    const char* right;
    const char* sub;

    /* unused */
    short font_size;
} button_t;

/*************/
/* variables */
/*************/
extern letter_t small_font[ 128 ];
extern letter_t big_font[ 128 ];

extern color_t colors_48sx[ NB_COLORS ];
extern color_t colors_48gx[ NB_COLORS ];
extern color_t colors_49g[ NB_COLORS ];
extern color_t colors_50g[ NB_COLORS ];

extern button_t buttons_48sx[ NB_HP48_KEYS ];
extern button_t buttons_48gx[ NB_HP48_KEYS ];
extern button_t buttons_49g[ NB_HP49_KEYS ];
extern button_t buttons_50g[ NB_HP49_KEYS ];

/*************/
/* functions */
/*************/
extern int SmallTextWidth( const char* string, unsigned int length );
extern int BigTextWidth( const char* string, unsigned int length );

#endif /* _UI4x_INNER_H */
