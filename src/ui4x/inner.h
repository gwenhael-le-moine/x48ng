#ifndef _UI4x_INNER_H
#  define _UI4x_INNER_H 1

#  include "api.h"
#  include "bitmaps_misc.h"
#  include "fonts.h"

#  define LCD_HEIGHT ( ui4x_config.model == MODEL_50G ? 80 : 64 )

#  define BUTTONS                                                                                                                          \
      ( ui4x_config.model == MODEL_48GX                                                                                                    \
            ? buttons_48gx                                                                                                                 \
            : ( ui4x_config.model == MODEL_48SX ? buttons_48sx : ( ui4x_config.model == MODEL_49G ? buttons_49g : buttons_50g ) ) )

#  define NB_KEYS ( ui4x_config.model == MODEL_48GX || ui4x_config.model == MODEL_48SX ? NB_HP48_KEYS : NB_HP4950_KEYS )

#  define UI4X_KEY_0 ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_0 : HP48_KEY_0 )
#  define UI4X_KEY_1 ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_1 : HP48_KEY_1 )
#  define UI4X_KEY_2 ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_2 : HP48_KEY_2 )
#  define UI4X_KEY_3 ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_3 : HP48_KEY_3 )
#  define UI4X_KEY_4 ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_4 : HP48_KEY_4 )
#  define UI4X_KEY_5 ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_5 : HP48_KEY_5 )
#  define UI4X_KEY_6 ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_6 : HP48_KEY_6 )
#  define UI4X_KEY_7 ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_7 : HP48_KEY_7 )
#  define UI4X_KEY_8 ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_8 : HP48_KEY_8 )
#  define UI4X_KEY_9 ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_9 : HP48_KEY_9 )
#  define UI4X_KEY_A ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_A : HP48_KEY_A )
#  define UI4X_KEY_B ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_B : HP48_KEY_B )
#  define UI4X_KEY_C ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_C : HP48_KEY_C )
#  define UI4X_KEY_D ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_D : HP48_KEY_D )
#  define UI4X_KEY_E ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_E : HP48_KEY_E )
#  define UI4X_KEY_F ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_F : HP48_KEY_F )
#  define UI4X_KEY_G ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_G : HP48_KEY_G )
#  define UI4X_KEY_H ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_H : HP48_KEY_H )
#  define UI4X_KEY_I ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_I : HP48_KEY_I )
#  define UI4X_KEY_J ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_J : HP48_KEY_J )
#  define UI4X_KEY_K ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_K : HP48_KEY_K )
#  define UI4X_KEY_L ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_L : HP48_KEY_L )
#  define UI4X_KEY_M ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_M : HP48_KEY_M )
#  define UI4X_KEY_N ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_N : HP48_KEY_N )
#  define UI4X_KEY_O ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_O : HP48_KEY_O )
#  define UI4X_KEY_P ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_P : HP48_KEY_P )
#  define UI4X_KEY_Q ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_Q : HP48_KEY_Q )
#  define UI4X_KEY_R ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_R : HP48_KEY_R )
#  define UI4X_KEY_S ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_S : HP48_KEY_S )
#  define UI4X_KEY_T ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_T : HP48_KEY_T )
#  define UI4X_KEY_U ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_U : HP48_KEY_U )
#  define UI4X_KEY_V ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_V : HP48_KEY_V )
#  define UI4X_KEY_W ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_W : HP48_KEY_W )
#  define UI4X_KEY_X ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_X : HP48_KEY_X )
#  define UI4X_KEY_Y ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_Y : HP48_KEY_Y )
#  define UI4X_KEY_Z ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_Z : HP48_KEY_Z )
#  define UI4X_KEY_UP ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_UP : HP48_KEY_K )
#  define UI4X_KEY_DOWN ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_DOWN : HP48_KEY_Q )
#  define UI4X_KEY_LEFT ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_LEFT : HP48_KEY_P )
#  define UI4X_KEY_RIGHT ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_RIGHT : HP48_KEY_R )
#  define UI4X_KEY_SPACE ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_SPACE : HP48_KEY_SPACE )
#  define UI4X_KEY_ENTER ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_ENTER : HP48_KEY_ENTER )
#  define UI4X_KEY_BACKSPACE                                                                                                               \
      ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_BACKSPACE : HP48_KEY_BACKSPACE )
#  define UI4X_KEY_DELETE ( ui4x_config.model == MODEL_50G ? -1 : ui4x_config.model == MODEL_49G ? -1 : HP48_KEY_DEL )
#  define UI4X_KEY_PERIOD ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_PERIOD : HP48_KEY_PERIOD )
#  define UI4X_KEY_PLUS ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_PLUS : HP48_KEY_PLUS )
#  define UI4X_KEY_MINUS ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_MINUS : HP48_KEY_MINUS )
#  define UI4X_KEY_MULTIPLY ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_MULTIPLY : HP48_KEY_MUL )
#  define UI4X_KEY_DIVIDE ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_Z : HP48_KEY_DIVIDE )
#  define UI4X_KEY_LSHIFT ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_LEFTSHIFT : HP48_KEY_LEFTSHIFT )
#  define UI4X_KEY_RSHIFT ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_RIGHTSHIFT : HP48_KEY_RIGHTSHIFT )
#  define UI4X_KEY_ALPHA ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_SPACE : HP48_KEY_SPACE )
#  define UI4X_KEY_ON ( ui4x_config.model == MODEL_50G || ui4x_config.model == MODEL_49G ? HP4950_KEY_ON : HP48_KEY_ON )

/* 4.1.1.1: When defined, this symbol represents the threshold of the long
   key pression.  When the mouse button is kept pressed on a calculator's key
   for more than LONG_PRESS_THR milliseconds, the key stays pressed after
   release.
*/
#  define LONG_PRESS_THR 750

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
    UI4X_COLOR_BLACK_PIXEL_OFF,
    UI4X_COLOR_ANNUNCIATOR,
    UI4X_COLOR_LABEL,
    UI4X_COLOR_ALPHA,
    UI4X_COLOR_LEFTSHIFT,
    UI4X_COLOR_RIGHTSHIFT,
    NB_COLORS,
} colors_t;

typedef enum { KEY_PRESS, KEY_RELEASE } key_event_t;

/***********/
/* typedef */
/***********/
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
    unsigned int label_graphic_w, label_graphic_h;
    unsigned char* label_graphic;

    const char* css_class;
    const char* css_id;
    const char* label;
    const char* label_sdl;
    const char* letter;
    const char* left;
    const char* left_sdl;
    const char* right;
    const char* right_sdl;
    const char* below;

    int hpkey;
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
extern button_t buttons_49g[ NB_HP4950_KEYS ];
extern button_t buttons_50g[ NB_HP4950_KEYS ];

extern char* ui_annunciators[ NB_ANNUNCIATORS ];

extern ui4x_config_t ui4x_config;

/********************************************/
/* API for UI to interact with the emulator */
/********************************************/
extern ui4x_emulator_api_t ui4x_emulator_api;

/*************/
/* functions */
/*************/
extern int SmallTextWidth( const char* string, unsigned int length );
extern int BigTextWidth( const char* string, unsigned int length );

#endif /* !(_UI4x_INNER_H) */
