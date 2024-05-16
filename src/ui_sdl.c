#include <ctype.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>

#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h> /* lineColor(); pixelColor(); rectangleColor();stringColor(); */

#include "romio.h" /* opt_gx */
#include "config.h"
#include "ui.h"
#include "ui_inner.h"

#define _KEYBOARD_HEIGHT ( buttons_gx[ LAST_HPKEY ].y + buttons_gx[ LAST_HPKEY ].h )
#define _KEYBOARD_WIDTH ( buttons_gx[ LAST_HPKEY ].x + buttons_gx[ LAST_HPKEY ].w )

#define _TOP_SKIP 65
#define _SIDE_SKIP 20
#define _BOTTOM_SKIP 25
#define _DISP_KBD_SKIP 65
#define _KBD_UPLINE 25

#define _DISPLAY_WIDTH ( 264 + 8 )
#define _DISPLAY_HEIGHT ( 128 + 16 + 8 )
#define _DISPLAY_OFFSET_X ( SIDE_SKIP + ( 286 - DISPLAY_WIDTH ) / 2 )
#define _DISPLAY_OFFSET_Y TOP_SKIP

#define _DISP_FRAME 8

#define _KEYBOARD_OFFSET_X SIDE_SKIP
#define _KEYBOARD_OFFSET_Y ( TOP_SKIP + DISPLAY_HEIGHT + DISP_KBD_SKIP )

// Control how the screen update is performed: at regular intervals (delayed)
// or immediatly Note: this is only for the LCD. The annunciators and the
// buttons are always immediately updated
// #define DELAYEDDISPUPDATE
// Interval in millisecond between screen updates
#define DISPUPDATEINTERVAL 200

/***********/
/* typedef */
/***********/

typedef struct sdl_color_t {
    const char* name;
    int r, g, b;
} sdl_color_t;

typedef struct sdl_keypad_t {
    unsigned int width;
    unsigned int height;
} sdl_keypad_t;

typedef struct sdl_button_t {
    const char* name;

    int x, y;
    unsigned int w, h;

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

    SDL_Surface* surfaceup;
    SDL_Surface* surfacedown;
} sdl_button_t;

// This mimicks the structure formerly lcd.c, except with SDL surfaces instead
// of Pixmaps.
typedef struct sdl_ann_struct_t {
    int x;
    int y;
    unsigned int width;
    unsigned int height;
    unsigned char* bits;

    SDL_Surface* surfaceon;
    SDL_Surface* surfaceoff;
} sdl_ann_struct_t;

/*************/
/* variables */
/*************/
static sdl_keypad_t keypad;
static sdl_color_t* sdl_colors;

static sdl_color_t sdl_colors_sx[] = {
    {"white",        255, 255, 255},
    {"left",         255, 166, 0  },
    {"right",        0,   210, 255},
    {"but_top",      109, 93,  93 },
    {"button",       90,  77,  77 },
    {"but_bot",      76,  65,  65 },
    {"lcd_col",      202, 221, 92 },
    {"pix_col",      0,   0,   128},
    {"pad_top",      109, 78,  78 },
    {"pad",          90,  64,  64 },
    {"pad_bot",      76,  54,  54 },
    {"disp_pad_top", 155, 118, 84 },
    {"disp_pad",     124, 94,  67 },
    {"disp_pad_bot", 100, 75,  53 },
    {"logo",         204, 169, 107},
    {"logo_back",    64,  64,  64 },
    {"label",        202, 184, 144},
    {"frame",        0,   0,   0  },
    {"underlay",     60,  42,  42 },
    {"black",        0,   0,   0  },
    /* { 0 } */
};

static sdl_color_t sdl_colors_gx[] = {
    {"white",        255, 255, 255},
    {"left",         255, 186, 255},
    {"right",        0,   255, 204},
    {"but_top",      104, 104, 104},
    {"button",       88,  88,  88 },
    {"but_bot",      74,  74,  74 },
    {"lcd_col",      202, 221, 92 },
    {"pix_col",      0,   0,   128},
    {"pad_top",      88,  88,  88 },
    {"pad",          74,  74,  74 },
    {"pad_bot",      64,  64,  64 },
    {"disp_pad_top", 128, 128, 138},
    {"disp_pad",     104, 104, 110},
    {"disp_pad_bot", 84,  84,  90 },
    {"logo",         176, 176, 184},
    {"logo_back",    104, 104, 110},
    {"label",        240, 240, 240},
    {"frame",        0,   0,   0  },
    {"underlay",     104, 104, 110},
    {"black",        0,   0,   0  },
    /* { 0 } */
};

// This will take the value of the defines, but can be run-time modified
static unsigned KEYBOARD_HEIGHT, KEYBOARD_WIDTH, TOP_SKIP, SIDE_SKIP, BOTTOM_SKIP, DISP_KBD_SKIP, DISPLAY_WIDTH, DISPLAY_HEIGHT,
    DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, DISP_FRAME, KEYBOARD_OFFSET_X, KEYBOARD_OFFSET_Y, KBD_UPLINE;

static unsigned int ARGBColors[ BLACK + 1 ];

static sdl_button_t* buttons = 0;

static sdl_button_t buttons_sx[] = {
    {"A",      0,   0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "A", 0,          0, 0,        0,      0, 0},
    {"B",      50,  0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "B", 0,          0, 0,        0,      0, 0},
    {"C",      100, 0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "C", 0,          0, 0,        0,      0, 0},
    {"D",      150, 0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "D", 0,          0, 0,        0,      0, 0},
    {"E",      200, 0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "E", 0,          0, 0,        0,      0, 0},
    {"F",      250, 0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "F", 0,          0, 0,        0,      0, 0},

    {"MTH",    0,   50,  36, 26, WHITE, "MTH",   0, 0,                0,                 0,                 "G", "PRINT",    1, 0,        0,      0, 0},
    {"PRG",    50,  50,  36, 26, WHITE, "PRG",   0, 0,                0,                 0,                 "H", "I/O",      1, 0,        0,      0, 0},
    {"CST",    100, 50,  36, 26, WHITE, "CST",   0, 0,                0,                 0,                 "I", "MODES",    1, 0,        0,      0, 0},
    {"VAR",    150, 50,  36, 26, WHITE, "VAR",   0, 0,                0,                 0,                 "J", "MEMORY",   1, 0,        0,      0, 0},
    {"UP",     200, 50,  36, 26, WHITE, 0,       0, up_width,         up_height,         up_bitmap,         "K", "LIBRARY",  1, 0,        0,      0, 0},
    {"NXT",    250, 50,  36, 26, WHITE, "NXT",   0, 0,                0,                 0,                 "L", "PREV",     0, 0,        0,      0, 0},

    {"COLON",  0,   100, 36, 26, WHITE, 0,       0, colon_width,      colon_height,      colon_bitmap,      "M", "UP",       0, "HOME",   0,      0, 0},
    {"STO",    50,  100, 36, 26, WHITE, "STO",   0, 0,                0,                 0,                 "N", "DEF",      0, "RCL",    0,      0, 0},
    {"EVAL",   100, 100, 36, 26, WHITE, "EVAL",  0, 0,                0,                 0,                 "O", "aQ",       0, "aNUM",   0,      0, 0},
    {"LEFT",   150, 100, 36, 26, WHITE, 0,       0, left_width,       left_height,       left_bitmap,       "P", "GRAPH",    0, 0,        0,      0, 0},
    {"DOWN",   200, 100, 36, 26, WHITE, 0,       0, down_width,       down_height,       down_bitmap,       "Q", "REVIEW",   0, 0,        0,      0, 0},
    {"RIGHT",  250, 100, 36, 26, WHITE, 0,       0, right_width,      right_height,      right_bitmap,      "R", "SWAP",     0, 0,        0,      0, 0},

    {"SIN",    0,   150, 36, 26, WHITE, "SIN",   0, 0,                0,                 0,                 "S", "ASIN",     0, "b",      0,      0, 0},
    {"COS",    50,  150, 36, 26, WHITE, "COS",   0, 0,                0,                 0,                 "T", "ACOS",     0, "c",      0,      0, 0},
    {"TAN",    100, 150, 36, 26, WHITE, "TAN",   0, 0,                0,                 0,                 "U", "ATAN",     0, "d",      0,      0, 0},
    {"SQRT",   150, 150, 36, 26, WHITE, 0,       0, sqrt_width,       sqrt_height,       sqrt_bitmap,       "V", "e",        0, "f",      0,      0, 0},
    {"POWER",  200, 150, 36, 26, WHITE, 0,       0, power_width,      power_height,      power_bitmap,      "W", "g",        0, "LOG",    0,      0, 0},
    {"INV",    250, 150, 36, 26, WHITE, 0,       0, inv_width,        inv_height,        inv_bitmap,        "X", "h",        0, "LN",     0,      0, 0},

    {"ENTER",  0,   200, 86, 26, WHITE, "ENTER", 2, 0,                0,                 0,                 0,   "EQUATION", 0, "MATRIX", 0,      0, 0},
    {"NEG",    100, 200, 36, 26, WHITE, 0,       0, neg_width,        neg_height,        neg_bitmap,        "Y", "EDIT",     0, "VISIT",  0,      0, 0},
    {"EEX",    150, 200, 36, 26, WHITE, "EEX",   0, 0,                0,                 0,                 "Z", "2D",       0, "3D",     0,      0, 0},
    {"DEL",    200, 200, 36, 26, WHITE, "DEL",   0, 0,                0,                 0,                 0,   "PURGE",    0, 0,        0,      0, 0},
    {"BS",     250, 200, 36, 26, WHITE, 0,       0, bs_width,         bs_height,         bs_bitmap,         0,   "DROP",     0, "CLR",    0,      0, 0},

    {"ALPHA",  0,   250, 36, 26, WHITE, 0,       0, alpha_width,      alpha_height,      alpha_bitmap,      0,   "USR",      0, "ENTRY",  0,      0, 0},
    {"7",      60,  250, 46, 26, WHITE, "7",     1, 0,                0,                 0,                 0,   "SOLVE",    1, 0,        0,      0, 0},
    {"8",      120, 250, 46, 26, WHITE, "8",     1, 0,                0,                 0,                 0,   "PLOT",     1, 0,        0,      0, 0},
    {"9",      180, 250, 46, 26, WHITE, "9",     1, 0,                0,                 0,                 0,   "ALGEBRA",  1, 0,        0,      0, 0},
    {"DIV",    240, 250, 46, 26, WHITE, 0,       0, div_width,        div_height,        div_bitmap,        0,   "( )",      0, "#",      0,      0, 0},

    {"SHL",    0,   300, 36, 26, LEFT,  0,       0, shl_width,        shl_height,        shl_bitmap,        0,   0,          0, 0,        0,      0, 0},
    {"4",      60,  300, 46, 26, WHITE, "4",     1, 0,                0,                 0,                 0,   "TIME",     1, 0,        0,      0, 0},
    {"5",      120, 300, 46, 26, WHITE, "5",     1, 0,                0,                 0,                 0,   "STAT",     1, 0,        0,      0, 0},
    {"6",      180, 300, 46, 26, WHITE, "6",     1, 0,                0,                 0,                 0,   "UNITS",    1, 0,        0,      0, 0},
    {"MUL",    240, 300, 46, 26, WHITE, 0,       0, mul_width,        mul_height,        mul_bitmap,        0,   "[ ]",      0, "_",      0,      0, 0},

    {"SHR",    0,   350, 36, 26, RIGHT, 0,       0, shr_width,        shr_height,        shr_bitmap,        0,   0,          0, 0,        0,      0, 0},
    {"1",      60,  350, 46, 26, WHITE, "1",     1, 0,                0,                 0,                 0,   "RAD",      0, "POLAR",  0,      0, 0},
    {"2",      120, 350, 46, 26, WHITE, "2",     1, 0,                0,                 0,                 0,   "STACK",    0, "ARG",    0,      0, 0},
    {"3",      180, 350, 46, 26, WHITE, "3",     1, 0,                0,                 0,                 0,   "CMD",      0, "MENU",   0,      0, 0},
    {"MINUS",  240, 350, 46, 26, WHITE, 0,       0, minus_width,      minus_height,      minus_bitmap,      0,   "i",        0, "j",      0,      0, 0},

    {"ON",     0,   400, 36, 26, WHITE, "ON",    0, 0,                0,                 0,                 0,   "CONT",     0, "OFF",    "ATTN", 0, 0},
    {"0",      60,  400, 46, 26, WHITE, "0",     1, 0,                0,                 0,                 0,   "= ",       0, " a",     0,      0, 0},
    {"PERIOD", 120, 400, 46, 26, WHITE, ".",     1, 0,                0,                 0,                 0,   ", ",       0, " k",     0,      0, 0},
    {"SPC",    180, 400, 46, 26, WHITE, "SPC",   0, 0,                0,                 0,                 0,   "l ",       0, " m",     0,      0, 0},
    {"PLUS",   240, 400, 46, 26, WHITE, 0,       0, plus_width,       plus_height,       plus_bitmap,       0,   "{ }",      0, ": :",    0,      0, 0},
};

static sdl_button_t buttons_gx[] = {
    {"A",      0,   0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "A", 0,          0, 0,          0,        0, 0},
    {"B",      50,  0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "B", 0,          0, 0,          0,        0, 0},
    {"C",      100, 0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "C", 0,          0, 0,          0,        0, 0},
    {"D",      150, 0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "D", 0,          0, 0,          0,        0, 0},
    {"E",      200, 0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "E", 0,          0, 0,          0,        0, 0},
    {"F",      250, 0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "F", 0,          0, 0,          0,        0, 0},

    {"MTH",    0,   50,  36, 26, WHITE, "MTH",   0, 0,                0,                 0,                 "G", "RAD",      0, "POLAR",    0,        0, 0},
    {"PRG",    50,  50,  36, 26, WHITE, "PRG",   0, 0,                0,                 0,                 "H", 0,          0, "CHARS",    0,        0, 0},
    {"CST",    100, 50,  36, 26, WHITE, "CST",   0, 0,                0,                 0,                 "I", 0,          0, "MODES",    0,        0, 0},
    {"VAR",    150, 50,  36, 26, WHITE, "VAR",   0, 0,                0,                 0,                 "J", 0,          0, "MEMORY",   0,        0, 0},
    {"UP",     200, 50,  36, 26, WHITE, 0,       0, up_width,         up_height,         up_bitmap,         "K", 0,          0, "STACK",    0,        0, 0},
    {"NXT",    250, 50,  36, 26, WHITE, "NXT",   0, 0,                0,                 0,                 "L", "PREV",     0, "MENU",     0,        0, 0},

    {"COLON",  0,   100, 36, 26, WHITE, 0,       0, colon_width,      colon_height,      colon_bitmap,      "M", "UP",       0, "HOME",     0,        0, 0},
    {"STO",    50,  100, 36, 26, WHITE, "STO",   0, 0,                0,                 0,                 "N", "DEF",      0, "RCL",      0,        0, 0},
    {"EVAL",   100, 100, 36, 26, WHITE, "EVAL",  0, 0,                0,                 0,                 "O", "aNUM",     0, "UNDO",     0,        0, 0},
    {"LEFT",   150, 100, 36, 26, WHITE, 0,       0, left_width,       left_height,       left_bitmap,       "P", "PICTURE",  0, 0,          0,        0, 0},
    {"DOWN",   200, 100, 36, 26, WHITE, 0,       0, down_width,       down_height,       down_bitmap,       "Q", "VIEW",     0, 0,          0,        0, 0},
    {"RIGHT",  250, 100, 36, 26, WHITE, 0,       0, right_width,      right_height,      right_bitmap,      "R", "SWAP",     0, 0,          0,        0, 0},

    {"SIN",    0,   150, 36, 26, WHITE, "SIN",   0, 0,                0,                 0,                 "S", "ASIN",     0, "b",        0,        0, 0},
    {"COS",    50,  150, 36, 26, WHITE, "COS",   0, 0,                0,                 0,                 "T", "ACOS",     0, "c",        0,        0, 0},
    {"TAN",    100, 150, 36, 26, WHITE, "TAN",   0, 0,                0,                 0,                 "U", "ATAN",     0, "d",        0,        0, 0},
    {"SQRT",   150, 150, 36, 26, WHITE, 0,       0, sqrt_width,       sqrt_height,       sqrt_bitmap,       "V", "n",        0, "o",        0,        0, 0},
    {"POWER",  200, 150, 36, 26, WHITE, 0,       0, power_width,      power_height,      power_bitmap,      "W", "p",        0, "LOG",      0,        0, 0},
    {"INV",    250, 150, 36, 26, WHITE, 0,       0, inv_width,        inv_height,        inv_bitmap,        "X", "q",        0, "LN",       0,        0, 0},

    {"ENTER",  0,   200, 86, 26, WHITE, "ENTER", 2, 0,                0,                 0,                 0,   "EQUATION", 0, "MATRIX",   0,        0, 0},
    {"NEG",    100, 200, 36, 26, WHITE, 0,       0, neg_width,        neg_height,        neg_bitmap,        "Y", "EDIT",     0, "CMD",      0,        0, 0},
    {"EEX",    150, 200, 36, 26, WHITE, "EEX",   0, 0,                0,                 0,                 "Z", "PURG",     0, "ARG",      0,        0, 0},
    {"DEL",    200, 200, 36, 26, WHITE, "DEL",   0, 0,                0,                 0,                 0,   "CLEAR",    0, 0,          0,        0, 0},
    {"BS",     250, 200, 36, 26, WHITE, 0,       0, bs_width,         bs_height,         bs_bitmap,         0,   "DROP",     0, 0,          0,        0, 0},

    {"ALPHA",  0,   250, 36, 26, WHITE, 0,       0, alpha_width,      alpha_height,      alpha_bitmap,      0,   "USER",     0, "ENTRY",    0,        0, 0},
    {"7",      60,  250, 46, 26, WHITE, "7",     1, 0,                0,                 0,                 0,   0,          1, "SOLVE",    0,        0, 0},
    {"8",      120, 250, 46, 26, WHITE, "8",     1, 0,                0,                 0,                 0,   0,          1, "PLOT",     0,        0, 0},
    {"9",      180, 250, 46, 26, WHITE, "9",     1, 0,                0,                 0,                 0,   0,          1, "SYMBOLIC", 0,        0, 0},
    {"DIV",    240, 250, 46, 26, WHITE, 0,       0, div_width,        div_height,        div_bitmap,        0,   "r ",       0, "s",        0,        0, 0},

    {"SHL",    0,   300, 36, 26, LEFT,  0,       0, shl_width,        shl_height,        shl_bitmap,        0,   0,          0, 0,          0,        0, 0},
    {"4",      60,  300, 46, 26, WHITE, "4",     1, 0,                0,                 0,                 0,   0,          1, "TIME",     0,        0, 0},
    {"5",      120, 300, 46, 26, WHITE, "5",     1, 0,                0,                 0,                 0,   0,          1, "STAT",     0,        0, 0},
    {"6",      180, 300, 46, 26, WHITE, "6",     1, 0,                0,                 0,                 0,   0,          1, "UNITS",    0,        0, 0},
    {"MUL",    240, 300, 46, 26, WHITE, 0,       0, mul_width,        mul_height,        mul_bitmap,        0,   "t ",       0, "u",        0,        0, 0},

    {"SHR",    0,   350, 36, 26, RIGHT, 0,       0, shr_width,        shr_height,        shr_bitmap,        0,   0,          1, " ",        0,        0, 0},
    {"1",      60,  350, 46, 26, WHITE, "1",     1, 0,                0,                 0,                 0,   0,          1, "I/O",      0,        0, 0},
    {"2",      120, 350, 46, 26, WHITE, "2",     1, 0,                0,                 0,                 0,   0,          1, "LIBRARY",  0,        0, 0},
    {"3",      180, 350, 46, 26, WHITE, "3",     1, 0,                0,                 0,                 0,   0,          1, "EQ LIB",   0,        0, 0},
    {"MINUS",  240, 350, 46, 26, WHITE, 0,       0, minus_width,      minus_height,      minus_bitmap,      0,   "v ",       0, "w",        0,        0, 0},

    {"ON",     0,   400, 36, 26, WHITE, "ON",    0, 0,                0,                 0,                 0,   "CONT",     0, "OFF",      "CANCEL", 0, 0},
    {"0",      60,  400, 46, 26, WHITE, "0",     1, 0,                0,                 0,                 0,   "\004 ",    0, "\003",     0,        0, 0},
    {"PERIOD", 120, 400, 46, 26, WHITE, ".",     1, 0,                0,                 0,                 0,   "\002 ",    0, "\001",     0,        0, 0},
    {"SPC",    180, 400, 46, 26, WHITE, "SPC",   0, 0,                0,                 0,                 0,   "\005 ",    0, "z",        0,        0, 0},
    {"PLUS",   240, 400, 46, 26, WHITE, 0,       0, plus_width,       plus_height,       plus_bitmap,       0,   "x ",       0, "y",        0,        0, 0},
    /* { 0 } */
};

static sdl_ann_struct_t ann_tbl[] = {
    {16,  4, ann_left_width,    ann_left_height,    ann_left_bitmap,    0, 0},
    {61,  4, ann_right_width,   ann_right_height,   ann_right_bitmap,   0, 0},
    {106, 4, ann_alpha_width,   ann_alpha_height,   ann_alpha_bitmap,   0, 0},
    {151, 4, ann_battery_width, ann_battery_height, ann_battery_bitmap, 0, 0},
    {196, 4, ann_busy_width,    ann_busy_height,    ann_busy_bitmap,    0, 0},
    {241, 4, ann_io_width,      ann_io_height,      ann_io_bitmap,      0, 0},
};

// State to displayed zoomed last pressed key
static SDL_Surface* showkeylastsurf = 0;
static int showkeylastx, showkeylasty, showkeylastkey;

static SDL_Surface* sdlwindow;

/****************************/
/* functions implementation */
/****************************/
static inline unsigned bgra2argb( unsigned color )
{
    unsigned a = ( color >> 24 ) & 0xff, r = ( color >> 16 ) & 0xff, g = ( color >> 8 ) & 0xff, b = color & 0xff;

    color = a | ( r << 24 ) | ( g << 16 ) | ( b << 8 );
    return color;
}

/*
        Create a surface from binary bitmap data
*/
static SDL_Surface* SDLCreateSurfFromData( unsigned int w, unsigned int h, unsigned char* data, unsigned int coloron,
                                           unsigned int coloroff )
{
    unsigned int x, y;
    SDL_Surface* surf;

    surf = SDL_CreateRGBSurface( SDL_SWSURFACE, w, h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 );

    SDL_LockSurface( surf );

    unsigned char* pixels = ( unsigned char* )surf->pixels;
    unsigned int pitch = surf->pitch;
    unsigned byteperline = w / 8;
    if ( byteperline * 8 != w )
        byteperline++;
    for ( y = 0; y < h; y++ ) {
        unsigned int* lineptr = ( unsigned int* )( pixels + y * pitch );
        for ( x = 0; x < w; x++ ) {
            // Address the correct byte
            char c = data[ y * byteperline + ( x >> 3 ) ];
            // Look for the bit in that byte
            char b = c & ( 1 << ( x & 7 ) );
            if ( b )
                lineptr[ x ] = coloron;
            else
                lineptr[ x ] = coloroff;
        }
    }

    SDL_UnlockSurface( surf );

    return surf;
}

static void SDLDrawSmallString( int x, int y, const char* string, unsigned int length, unsigned int coloron, unsigned int coloroff )
{
    for ( unsigned int i = 0; i < length; i++ ) {
        if ( small_font[ ( int )string[ i ] ].h != 0 ) {
            int w = small_font[ ( int )string[ i ] ].w;
            int h = small_font[ ( int )string[ i ] ].h;

            SDL_Surface* surf = SDLCreateSurfFromData( w, h, small_font[ ( int )string[ i ] ].bits, coloron, coloroff );

            SDL_Rect srect;
            SDL_Rect drect;
            srect.x = 0;
            srect.y = 0;
            srect.w = w;
            srect.h = h;
            drect.x = x;
            drect.y = ( int )( y - small_font[ ( int )string[ i ] ].h );
            drect.w = w;
            drect.h = h;
            SDL_BlitSurface( surf, &srect, sdlwindow, &drect );
            SDL_FreeSurface( surf );
        }
        x += SmallTextWidth( &string[ i ], 1 );
    }
}

static void SDLInit( void )
{
    unsigned int width, height;

    // Initialize SDL
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        printf( "Couldn't initialize SDL: %s\n", SDL_GetError() );
        exit( 1 );
    }

    // On exit: clean SDL
    atexit( SDL_Quit );

    // Initialize the geometric values
    KEYBOARD_HEIGHT = _KEYBOARD_HEIGHT;
    KEYBOARD_WIDTH = _KEYBOARD_WIDTH;
    TOP_SKIP = _TOP_SKIP;
    SIDE_SKIP = _SIDE_SKIP;
    BOTTOM_SKIP = _BOTTOM_SKIP;
    DISP_KBD_SKIP = _DISP_KBD_SKIP;
    DISPLAY_WIDTH = _DISPLAY_WIDTH;
    DISPLAY_HEIGHT = _DISPLAY_HEIGHT;
    DISPLAY_OFFSET_X = _DISPLAY_OFFSET_X;
    DISPLAY_OFFSET_Y = _DISPLAY_OFFSET_Y;
    DISP_FRAME = _DISP_FRAME;
    KEYBOARD_OFFSET_X = _KEYBOARD_OFFSET_X;
    KEYBOARD_OFFSET_Y = _KEYBOARD_OFFSET_Y;
    KBD_UPLINE = _KBD_UPLINE;

    if ( config.hide_chrome ) {
        width = DISPLAY_WIDTH;
        height = DISPLAY_HEIGHT;
        DISPLAY_OFFSET_X = 0;
        DISPLAY_OFFSET_Y = 0;
    } else {
        width = ( buttons_gx[ LAST_HPKEY ].x + buttons_gx[ LAST_HPKEY ].w ) + 2 * SIDE_SKIP;
        height = DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + DISP_KBD_SKIP + buttons_gx[ LAST_HPKEY ].y + buttons_gx[ LAST_HPKEY ].h + BOTTOM_SKIP;
    }

    uint32_t sdl_window_flags = SDL_SWSURFACE | SDL_RESIZABLE;
    if ( config.show_ui_fullscreen )
        sdl_window_flags |= SDL_FULLSCREEN;

    sdlwindow = SDL_SetVideoMode( width, height, 32, sdl_window_flags );

    if ( sdlwindow == NULL ) {
        printf( "Couldn't set video mode: %s\n", SDL_GetError() );
        exit( 1 );
    }
}

static void SDLCreateColors( void )
{
    unsigned i;

    for ( i = WHITE; i < BLACK; i++ )
        ARGBColors[ i ] = 0xff000000 | ( sdl_colors[ i ].r << 16 ) | ( sdl_colors[ i ].g << 8 ) | sdl_colors[ i ].b;

    // Adjust the LCD color according to the contrast
    int contrast, r, g, b;
    contrast = display.contrast;

    if ( contrast < 0x3 )
        contrast = 0x3;
    if ( contrast > 0x13 )
        contrast = 0x13;

    r = ( 0x13 - contrast ) * ( sdl_colors[ LCD ].r / 0x10 );
    g = ( 0x13 - contrast ) * ( sdl_colors[ LCD ].g / 0x10 );
    b = 128 - ( ( 0x13 - contrast ) * ( ( 128 - sdl_colors[ LCD ].b ) / 0x10 ) );
    ARGBColors[ PIXEL ] = 0xff000000 | ( r << 16 ) | ( g << 8 ) | b;
}

// This should be called once to setup the surfaces. Calling it multiple
// times is fine, it won't do anything on subsequent calls.
static void SDLCreateAnnunc( void )
{
    for ( int i = 0; i < NB_ANNUNCIATORS; i++ ) {
        // If the SDL surface does not exist yet, we create it on the fly
        if ( ann_tbl[ i ].surfaceon ) {
            SDL_FreeSurface( ann_tbl[ i ].surfaceon );
            ann_tbl[ i ].surfaceon = 0;
        }

        ann_tbl[ i ].surfaceon =
            SDLCreateSurfFromData( ann_tbl[ i ].width, ann_tbl[ i ].height, ann_tbl[ i ].bits, ARGBColors[ PIXEL ], ARGBColors[ LCD ] );

        if ( ann_tbl[ i ].surfaceoff ) {
            SDL_FreeSurface( ann_tbl[ i ].surfaceoff );
            ann_tbl[ i ].surfaceoff = 0;
        }

        ann_tbl[ i ].surfaceoff =
            SDLCreateSurfFromData( ann_tbl[ i ].width, ann_tbl[ i ].height, ann_tbl[ i ].bits, ARGBColors[ LCD ], ARGBColors[ LCD ] );
    }
}

// Find which key is pressed, if any.
// Returns -1 is no key is pressed
static int SDLCoordinateToKey( unsigned int x, unsigned int y )
{
    /* return immediatly if the click isn't even in the keyboard area */
    if ( y < KEYBOARD_OFFSET_Y )
        return -1;

    int row = ( y - KEYBOARD_OFFSET_Y ) / ( KEYBOARD_HEIGHT / 9 );
    int column;
    switch ( row ) {
        case 0:
        case 1:
        case 2:
        case 3:
            column = ( x - KEYBOARD_OFFSET_X ) / ( KEYBOARD_WIDTH / 6 );
            return ( row * 6 ) + column;
        case 4: /* with [ENTER] key */
            column = ( ( x - KEYBOARD_OFFSET_X ) / ( KEYBOARD_WIDTH / 5 ) ) - 1;
            if ( column < 0 )
                column = 0;
            return ( 4 * 6 ) + column;
        case 5:
        case 6:
        case 7:
        case 8:
            column = ( x - KEYBOARD_OFFSET_X ) / ( KEYBOARD_WIDTH / 5 );
            return ( 4 * 6 ) + 5 + ( ( row - 5 ) * 5 ) + column;

        default:
            return -1;
    }

    return -1;
}

// Map the keyboard keys to the HP keys
// Returns -1 if there is no mapping
static int SDLKeyToKey( SDLKey k )
{
    switch ( k ) {
        case SDLK_0:
            return HPKEY_0;
            break;
        case SDLK_1:
            return HPKEY_1;
            break;
        case SDLK_2:
            return HPKEY_2;
            break;
        case SDLK_3:
            return HPKEY_3;
            break;
        case SDLK_4:
            return HPKEY_4;
            break;
        case SDLK_5:
            return HPKEY_5;
            break;
        case SDLK_6:
            return HPKEY_6;
            break;
        case SDLK_7:
            return HPKEY_7;
            break;
        case SDLK_8:
            return HPKEY_8;
            break;
        case SDLK_9:
            return HPKEY_9;
            break;
        case SDLK_KP0:
            return HPKEY_0;
            break;
        case SDLK_KP1:
            return HPKEY_1;
            break;
        case SDLK_KP2:
            return HPKEY_2;
            break;
        case SDLK_KP3:
            return HPKEY_3;
            break;
        case SDLK_KP4:
            return HPKEY_4;
            break;
        case SDLK_KP5:
            return HPKEY_5;
            break;
        case SDLK_KP6:
            return HPKEY_6;
            break;
        case SDLK_KP7:
            return HPKEY_7;
            break;
        case SDLK_KP8:
            return HPKEY_8;
            break;
        case SDLK_KP9:
            return HPKEY_9;
            break;
        case SDLK_a:
            return HPKEY_A;
            break;
        case SDLK_b:
            return HPKEY_B;
            break;
        case SDLK_c:
            return HPKEY_C;
            break;
        case SDLK_d:
            return HPKEY_D;
            break;
        case SDLK_e:
            return HPKEY_E;
            break;
        case SDLK_f:
            return HPKEY_F;
            break;
        case SDLK_g:
            return HPKEY_MTH;
            break;
        case SDLK_h:
            return HPKEY_PRG;
            break;
        case SDLK_i:
            return HPKEY_CST;
            break;
        case SDLK_j:
            return HPKEY_VAR;
            break;
        case SDLK_k:
            return HPKEY_UP;
            break;
        case SDLK_UP:
            return HPKEY_UP;
            break;
        case SDLK_l:
            return HPKEY_NXT;
            break;
        case SDLK_m:
            return HPKEY_COLON;
            break;
        case SDLK_n:
            return HPKEY_STO;
            break;
        case SDLK_o:
            return HPKEY_EVAL;
            break;
        case SDLK_p:
            return HPKEY_LEFT;
            break;
        case SDLK_LEFT:
            return HPKEY_LEFT;
            break;
        case SDLK_q:
            return HPKEY_DOWN;
            break;
        case SDLK_DOWN:
            return HPKEY_DOWN;
            break;
        case SDLK_r:
            return HPKEY_RIGHT;
            break;
        case SDLK_RIGHT:
            return HPKEY_RIGHT;
            break;
        case SDLK_s:
            return HPKEY_SIN;
            break;
        case SDLK_t:
            return HPKEY_COS;
            break;
        case SDLK_u:
            return HPKEY_TAN;
            break;
        case SDLK_v:
            return HPKEY_SQRT;
            break;
        case SDLK_w:
            return HPKEY_POWER;
            break;
        case SDLK_x:
            return HPKEY_INV;
            break;
        case SDLK_y:
            return HPKEY_NEG;
            break;
        case SDLK_z:
            return HPKEY_EEX;
            break;
        case SDLK_SPACE:
            return HPKEY_SPC;
            break;
        case SDLK_F1:
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            return HPKEY_ENTER;
            break;
        case SDLK_BACKSPACE:
            return HPKEY_BS;
            break;
        case SDLK_DELETE:
            return HPKEY_DEL;
            break;
        case SDLK_PERIOD:
            return HPKEY_PERIOD;
            break;
        case SDLK_KP_PERIOD:
            return HPKEY_PERIOD;
            break;
        case SDLK_PLUS:
            return HPKEY_PLUS;
            break;
        case SDLK_KP_PLUS:
            return HPKEY_PLUS;
            break;
        case SDLK_MINUS:
            return HPKEY_MINUS;
            break;
        case SDLK_KP_MINUS:
            return HPKEY_MINUS;
            break;
        case SDLK_ASTERISK:
            return HPKEY_MUL;
            break;
        case SDLK_KP_MULTIPLY:
            return HPKEY_MUL;
            break;
        case SDLK_SLASH:
            return HPKEY_DIV;
            break;
        case SDLK_KP_DIVIDE:
            return HPKEY_DIV;
            break;
        case SDLK_F5:
        case SDLK_ESCAPE:
            return HPKEY_ON;
            break;
        case SDLK_LSHIFT:
            if ( !config.leave_shift_keys )
                return HPKEY_SHL;
            break;
        case SDLK_RSHIFT:
            if ( !config.leave_shift_keys )
                return HPKEY_SHR;
            break;
        case SDLK_F2:
        case SDLK_RCTRL:
            return HPKEY_SHL;
            break;
        case SDLK_F3:
        case SDLK_LCTRL:
            return HPKEY_SHR;
            break;
        case SDLK_F4:
        case SDLK_LALT:
        case SDLK_RALT:
            return HPKEY_ALPHA;
            break;
        case SDLK_F7:
        case SDLK_F10:
            please_exit = true;
            break;
        default:
            return -1;
    }

    return -1;
}

static void SDLDrawMore( unsigned int cut, unsigned int offset_y, int keypad_width, int keypad_height )
{
    // bottom lines
    lineColor( sdlwindow, 1, keypad_height - 1, keypad_width - 1, keypad_height - 1, bgra2argb( ARGBColors[ PAD_TOP ] ) );
    lineColor( sdlwindow, 2, keypad_height - 2, keypad_width - 2, keypad_height - 2, bgra2argb( ARGBColors[ PAD_TOP ] ) );

    // right lines
    lineColor( sdlwindow, keypad_width - 1, keypad_height - 1, keypad_width - 1, cut, bgra2argb( ARGBColors[ PAD_TOP ] ) );
    lineColor( sdlwindow, keypad_width - 2, keypad_height - 2, keypad_width - 2, cut, bgra2argb( ARGBColors[ PAD_TOP ] ) );

    // right lines
    lineColor( sdlwindow, keypad_width - 1, cut - 1, keypad_width - 1, 1, bgra2argb( ARGBColors[ DISP_PAD_TOP ] ) );
    lineColor( sdlwindow, keypad_width - 2, cut - 1, keypad_width - 2, 2, bgra2argb( ARGBColors[ DISP_PAD_TOP ] ) );

    // top lines
    lineColor( sdlwindow, 0, 0, keypad_width - 2, 0, bgra2argb( ARGBColors[ DISP_PAD_BOT ] ) );
    lineColor( sdlwindow, 1, 1, keypad_width - 3, 1, bgra2argb( ARGBColors[ DISP_PAD_BOT ] ) );

    // left lines
    lineColor( sdlwindow, 0, cut - 1, 0, 0, bgra2argb( ARGBColors[ DISP_PAD_BOT ] ) );
    lineColor( sdlwindow, 1, cut - 1, 1, 1, bgra2argb( ARGBColors[ DISP_PAD_BOT ] ) );

    // left lines
    lineColor( sdlwindow, 0, keypad_height - 2, 0, cut, bgra2argb( ARGBColors[ PAD_BOT ] ) );
    lineColor( sdlwindow, 1, keypad_height - 3, 1, cut, bgra2argb( ARGBColors[ PAD_BOT ] ) );

    // lower the menu buttons

    // bottom lines
    lineColor( sdlwindow, 3, keypad_height - 3, keypad_width - 3, keypad_height - 3, bgra2argb( ARGBColors[ PAD_TOP ] ) );
    lineColor( sdlwindow, 4, keypad_height - 4, keypad_width - 4, keypad_height - 4, bgra2argb( ARGBColors[ PAD_TOP ] ) );

    // right lines
    lineColor( sdlwindow, keypad_width - 3, keypad_height - 3, keypad_width - 3, cut, bgra2argb( ARGBColors[ PAD_TOP ] ) );
    lineColor( sdlwindow, keypad_width - 4, keypad_height - 4, keypad_width - 4, cut, bgra2argb( ARGBColors[ PAD_TOP ] ) );

    // right lines
    lineColor( sdlwindow, keypad_width - 3, cut - 1, keypad_width - 3, offset_y - ( KBD_UPLINE - 1 ),
               bgra2argb( ARGBColors[ DISP_PAD_TOP ] ) );
    lineColor( sdlwindow, keypad_width - 4, cut - 1, keypad_width - 4, offset_y - ( KBD_UPLINE - 2 ),
               bgra2argb( ARGBColors[ DISP_PAD_TOP ] ) );

    // top lines
    lineColor( sdlwindow, 2, offset_y - ( KBD_UPLINE - 0 ), keypad_width - 4, offset_y - ( KBD_UPLINE - 0 ),
               bgra2argb( ARGBColors[ DISP_PAD_BOT ] ) );
    lineColor( sdlwindow, 3, offset_y - ( KBD_UPLINE - 1 ), keypad_width - 5, offset_y - ( KBD_UPLINE - 1 ),
               bgra2argb( ARGBColors[ DISP_PAD_BOT ] ) );

    // left lines
    lineColor( sdlwindow, 2, cut - 1, 2, offset_y - ( KBD_UPLINE - 1 ), bgra2argb( ARGBColors[ DISP_PAD_BOT ] ) );
    lineColor( sdlwindow, 3, cut - 1, 3, offset_y - ( KBD_UPLINE - 2 ), bgra2argb( ARGBColors[ DISP_PAD_BOT ] ) );

    // left lines
    lineColor( sdlwindow, 2, keypad_height - 4, 2, cut, bgra2argb( ARGBColors[ PAD_BOT ] ) );
    lineColor( sdlwindow, 3, keypad_height - 5, 3, cut, bgra2argb( ARGBColors[ PAD_BOT ] ) );

    // lower the keyboard

    // bottom lines
    lineColor( sdlwindow, 5, keypad_height - 5, keypad_width - 3, keypad_height - 5, bgra2argb( ARGBColors[ PAD_TOP ] ) );
    lineColor( sdlwindow, 6, keypad_height - 6, keypad_width - 4, keypad_height - 6, bgra2argb( ARGBColors[ PAD_TOP ] ) );

    // right lines
    lineColor( sdlwindow, keypad_width - 5, keypad_height - 5, keypad_width - 5, cut + 1, bgra2argb( ARGBColors[ PAD_TOP ] ) );
    lineColor( sdlwindow, keypad_width - 6, keypad_height - 6, keypad_width - 6, cut + 2, bgra2argb( ARGBColors[ PAD_TOP ] ) );

    // top lines
    lineColor( sdlwindow, 4, cut, keypad_width - 6, cut, bgra2argb( ARGBColors[ DISP_PAD_BOT ] ) );
    lineColor( sdlwindow, 5, cut + 1, keypad_width - 7, cut + 1, bgra2argb( ARGBColors[ DISP_PAD_BOT ] ) );

    // left lines
    lineColor( sdlwindow, 4, keypad_height - 6, 4, cut + 1, bgra2argb( ARGBColors[ PAD_BOT ] ) );
    lineColor( sdlwindow, 5, keypad_height - 7, 5, cut + 2, bgra2argb( ARGBColors[ PAD_BOT ] ) );

    // round off the bottom edge

    lineColor( sdlwindow, keypad_width - 7, keypad_height - 7, keypad_width - 7, keypad_height - 14, bgra2argb( ARGBColors[ PAD_TOP ] ) );
    lineColor( sdlwindow, keypad_width - 8, keypad_height - 8, keypad_width - 8, keypad_height - 11, bgra2argb( ARGBColors[ PAD_TOP ] ) );
    lineColor( sdlwindow, keypad_width - 7, keypad_height - 7, keypad_width - 14, keypad_height - 7, bgra2argb( ARGBColors[ PAD_TOP ] ) );
    lineColor( sdlwindow, keypad_width - 7, keypad_height - 8, keypad_width - 11, keypad_height - 8, bgra2argb( ARGBColors[ PAD_TOP ] ) );
    pixelColor( sdlwindow, keypad_width - 9, keypad_height - 9, bgra2argb( ARGBColors[ PAD_TOP ] ) );

    lineColor( sdlwindow, 7, keypad_height - 7, 13, keypad_height - 7, bgra2argb( ARGBColors[ PAD_TOP ] ) );
    lineColor( sdlwindow, 8, keypad_height - 8, 10, keypad_height - 8, bgra2argb( ARGBColors[ PAD_TOP ] ) );

    lineColor( sdlwindow, 6, keypad_height - 8, 6, keypad_height - 14, bgra2argb( ARGBColors[ PAD_BOT ] ) );
    lineColor( sdlwindow, 7, keypad_height - 9, 7, keypad_height - 11, bgra2argb( ARGBColors[ PAD_BOT ] ) );
}

static void SDLDrawLogo()
{
    int x, y;
    SDL_Surface* surf;

    int display_width = DISPLAY_WIDTH;

    // insert the HP Logo
    surf = SDLCreateSurfFromData( hp_width, hp_height, hp_bitmap, ARGBColors[ LOGO ], ARGBColors[ LOGO_BACK ] );
    if ( opt_gx )
        x = DISPLAY_OFFSET_X - 6;
    else
        x = DISPLAY_OFFSET_X;

    SDL_Rect srect;
    SDL_Rect drect;
    srect.x = 0;
    srect.y = 0;
    srect.w = hp_width;
    srect.h = hp_height;
    drect.x = x;
    drect.y = 10;
    drect.w = hp_width;
    drect.h = hp_height;
    SDL_BlitSurface( surf, &srect, sdlwindow, &drect );
    SDL_FreeSurface( surf );

    if ( !opt_gx ) {
        lineColor( sdlwindow, DISPLAY_OFFSET_X, 9, DISPLAY_OFFSET_X + hp_width - 1, 9, bgra2argb( ARGBColors[ FRAME ] ) );
        lineColor( sdlwindow, DISPLAY_OFFSET_X - 1, 10, DISPLAY_OFFSET_X - 1, 10 + hp_height - 1, bgra2argb( ARGBColors[ FRAME ] ) );
        lineColor( sdlwindow, DISPLAY_OFFSET_X, 10 + hp_height, DISPLAY_OFFSET_X + hp_width - 1, 10 + hp_height,
                   bgra2argb( ARGBColors[ FRAME ] ) );
        lineColor( sdlwindow, DISPLAY_OFFSET_X + hp_width, 10, DISPLAY_OFFSET_X + hp_width, 10 + hp_height - 1,
                   bgra2argb( ARGBColors[ FRAME ] ) );
    }

    // write the name of it

    if ( opt_gx ) {
        x = DISPLAY_OFFSET_X + display_width - gx_128K_ram_width + gx_128K_ram_x_hot + 2;
        y = 10 + gx_128K_ram_y_hot;

        surf =
            SDLCreateSurfFromData( gx_128K_ram_width, gx_128K_ram_height, gx_128K_ram_bitmap, ARGBColors[ LABEL ], ARGBColors[ DISP_PAD ] );
        srect.x = 0;
        srect.y = 0;
        srect.w = gx_128K_ram_width;
        srect.h = gx_128K_ram_height;
        drect.x = x;
        drect.y = y;
        drect.w = gx_128K_ram_width;
        drect.h = gx_128K_ram_height;
        SDL_BlitSurface( surf, &srect, sdlwindow, &drect );
        SDL_FreeSurface( surf );

        x = DISPLAY_OFFSET_X + hp_width;
        y = hp_height + 8 - hp48gx_height;
        surf = SDLCreateSurfFromData( hp48gx_width, hp48gx_height, hp48gx_bitmap, ARGBColors[ LOGO ], ARGBColors[ DISP_PAD ] );
        srect.x = 0;
        srect.y = 0;
        srect.w = hp48gx_width;
        srect.h = hp48gx_height;
        drect.x = x;
        drect.y = y;
        drect.w = hp48gx_width;
        drect.h = hp48gx_height;
        SDL_BlitSurface( surf, &srect, sdlwindow, &drect );
        SDL_FreeSurface( surf );

        x = DISPLAY_OFFSET_X + DISPLAY_WIDTH - gx_128K_ram_width + gx_silver_x_hot + 2;
        y = 10 + gx_silver_y_hot;
        surf = SDLCreateSurfFromData( gx_silver_width, gx_silver_height, gx_silver_bitmap, ARGBColors[ LOGO ],
                                      0 ); // Background transparent: draw only silver line
        srect.x = 0;
        srect.y = 0;
        srect.w = gx_silver_width;
        srect.h = gx_silver_height;
        drect.x = x;
        drect.y = y;
        drect.w = gx_silver_width;
        drect.h = gx_silver_height;
        SDL_BlitSurface( surf, &srect, sdlwindow, &drect );
        SDL_FreeSurface( surf );

        x = DISPLAY_OFFSET_X + display_width - gx_128K_ram_width + gx_green_x_hot + 2;
        y = 10 + gx_green_y_hot;
        surf = SDLCreateSurfFromData( gx_green_width, gx_green_height, gx_green_bitmap, ARGBColors[ RIGHT ],
                                      0 ); // Background transparent: draw only green menu
        srect.x = 0;
        srect.y = 0;
        srect.w = gx_green_width;
        srect.h = gx_green_height;
        drect.x = x;
        drect.y = y;
        drect.w = gx_green_width;
        drect.h = gx_green_height;
        SDL_BlitSurface( surf, &srect, sdlwindow, &drect );
        SDL_FreeSurface( surf );
    } else {
        x = DISPLAY_OFFSET_X;
        y = TOP_SKIP - DISP_FRAME - hp48sx_height - 3;
        surf = SDLCreateSurfFromData( hp48sx_width, hp48sx_height, hp48sx_bitmap, ARGBColors[ RIGHT ],
                                      0 ); // Background transparent: draw only green menu
        srect.x = 0;
        srect.y = 0;
        srect.w = hp48sx_width;
        srect.h = hp48sx_height;
        drect.x = x;
        drect.y = y;
        drect.w = hp48sx_width;
        drect.h = hp48sx_height;
        SDL_BlitSurface( surf, &srect, sdlwindow, &drect );
        SDL_FreeSurface( surf );

        x = DISPLAY_OFFSET_X + display_width - 1 - science_width;
        y = TOP_SKIP - DISP_FRAME - science_height - 4;
        surf = SDLCreateSurfFromData( science_width, science_height, science_bitmap, ARGBColors[ RIGHT ],
                                      0 ); // Background transparent: draw only green menu
        srect.x = 0;
        srect.y = 0;
        srect.w = science_width;
        srect.h = science_height;
        drect.x = x;
        drect.y = y;
        drect.w = science_width;
        drect.h = science_height;
        SDL_BlitSurface( surf, &srect, sdlwindow, &drect );
        SDL_FreeSurface( surf );
    }
}

static void SDLCreateKeys( void )
{
    unsigned i, x, y;
    unsigned pixel;

    for ( i = FIRST_HPKEY; i <= LAST_HPKEY; i++ ) {
        // Create surfaces for each button
        if ( !buttons[ i ].surfaceup )
            buttons[ i ].surfaceup =
                SDL_CreateRGBSurface( SDL_SWSURFACE, buttons[ i ].w, buttons[ i ].h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 );

        if ( !buttons[ i ].surfacedown )
            buttons[ i ].surfacedown =
                SDL_CreateRGBSurface( SDL_SWSURFACE, buttons[ i ].w, buttons[ i ].h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 );

        // Use alpha channel
        pixel = 0x00000000;
        // pixel = 0xffff0000;

        // Fill the button and outline
        SDL_FillRect( buttons[ i ].surfaceup, 0, pixel );
        SDL_FillRect( buttons[ i ].surfacedown, 0, pixel );

        SDL_Rect rect;
        rect.x = 1;
        rect.y = 1;
        rect.w = buttons[ i ].w - 2;
        rect.h = buttons[ i ].h - 2;
        SDL_FillRect( buttons[ i ].surfaceup, &rect, ARGBColors[ BUTTON ] );
        SDL_FillRect( buttons[ i ].surfacedown, &rect, ARGBColors[ BUTTON ] );

        // draw the released button
        // draw edge of button
        lineColor( buttons[ i ].surfaceup, 1, buttons[ i ].h - 2, 1, 1, bgra2argb( ARGBColors[ BUT_TOP ] ) );
        lineColor( buttons[ i ].surfaceup, 2, buttons[ i ].h - 3, 2, 2, bgra2argb( ARGBColors[ BUT_TOP ] ) );
        lineColor( buttons[ i ].surfaceup, 3, buttons[ i ].h - 4, 3, 3, bgra2argb( ARGBColors[ BUT_TOP ] ) );

        lineColor( buttons[ i ].surfaceup, 1, 1, buttons[ i ].w - 2, 1, bgra2argb( ARGBColors[ BUT_TOP ] ) );
        lineColor( buttons[ i ].surfaceup, 2, 2, buttons[ i ].w - 3, 2, bgra2argb( ARGBColors[ BUT_TOP ] ) );
        lineColor( buttons[ i ].surfaceup, 3, 3, buttons[ i ].w - 4, 3, bgra2argb( ARGBColors[ BUT_TOP ] ) );
        lineColor( buttons[ i ].surfaceup, 4, 4, buttons[ i ].w - 5, 4, bgra2argb( ARGBColors[ BUT_TOP ] ) );

        pixelColor( buttons[ i ].surfaceup, 4, 5, bgra2argb( ARGBColors[ BUT_TOP ] ) );

        lineColor( buttons[ i ].surfaceup, 3, buttons[ i ].h - 2, buttons[ i ].w - 2, buttons[ i ].h - 2,
                   bgra2argb( ARGBColors[ BUT_BOT ] ) );
        lineColor( buttons[ i ].surfaceup, 4, buttons[ i ].h - 3, buttons[ i ].w - 3, buttons[ i ].h - 3,
                   bgra2argb( ARGBColors[ BUT_BOT ] ) );

        lineColor( buttons[ i ].surfaceup, buttons[ i ].w - 2, buttons[ i ].h - 2, buttons[ i ].w - 2, 3,
                   bgra2argb( ARGBColors[ BUT_BOT ] ) );
        lineColor( buttons[ i ].surfaceup, buttons[ i ].w - 3, buttons[ i ].h - 3, buttons[ i ].w - 3, 4,
                   bgra2argb( ARGBColors[ BUT_BOT ] ) );
        lineColor( buttons[ i ].surfaceup, buttons[ i ].w - 4, buttons[ i ].h - 4, buttons[ i ].w - 4, 5,
                   bgra2argb( ARGBColors[ BUT_BOT ] ) );
        pixelColor( buttons[ i ].surfaceup, buttons[ i ].w - 5, buttons[ i ].h - 4, bgra2argb( ARGBColors[ BUT_BOT ] ) );

        // draw frame around button

        lineColor( buttons[ i ].surfaceup, 0, buttons[ i ].h - 3, 0, 2, bgra2argb( ARGBColors[ FRAME ] ) );
        lineColor( buttons[ i ].surfaceup, 2, 0, buttons[ i ].w - 3, 0, bgra2argb( ARGBColors[ FRAME ] ) );
        lineColor( buttons[ i ].surfaceup, 2, buttons[ i ].h - 1, buttons[ i ].w - 3, buttons[ i ].h - 1,
                   bgra2argb( ARGBColors[ FRAME ] ) );
        lineColor( buttons[ i ].surfaceup, buttons[ i ].w - 1, buttons[ i ].h - 3, buttons[ i ].w - 1, 2,
                   bgra2argb( ARGBColors[ FRAME ] ) );
        if ( i == HPKEY_ON ) {
            lineColor( buttons[ i ].surfaceup, 1, 1, buttons[ 1 ].w - 2, 1, bgra2argb( ARGBColors[ FRAME ] ) );
            pixelColor( buttons[ i ].surfaceup, 1, 2, bgra2argb( ARGBColors[ FRAME ] ) );
            pixelColor( buttons[ i ].surfaceup, buttons[ i ].w - 2, 2, bgra2argb( ARGBColors[ FRAME ] ) );
        } else {
            pixelColor( buttons[ i ].surfaceup, 1, 1, bgra2argb( ARGBColors[ FRAME ] ) );
            pixelColor( buttons[ i ].surfaceup, buttons[ i ].w - 2, 1, bgra2argb( ARGBColors[ FRAME ] ) );
        }
        pixelColor( buttons[ i ].surfaceup, 1, buttons[ i ].h - 2, bgra2argb( ARGBColors[ FRAME ] ) );
        pixelColor( buttons[ i ].surfaceup, buttons[ i ].w - 2, buttons[ i ].h - 2, bgra2argb( ARGBColors[ FRAME ] ) );

        // draw the depressed button

        // draw edge of button
        lineColor( buttons[ i ].surfacedown, 2, buttons[ i ].h - 4, 2, 2, bgra2argb( ARGBColors[ BUT_TOP ] ) );
        lineColor( buttons[ i ].surfacedown, 3, buttons[ i ].h - 5, 3, 3, bgra2argb( ARGBColors[ BUT_TOP ] ) );
        lineColor( buttons[ i ].surfacedown, 2, 2, buttons[ i ].w - 4, 2, bgra2argb( ARGBColors[ BUT_TOP ] ) );
        lineColor( buttons[ i ].surfacedown, 3, 3, buttons[ i ].w - 5, 3, bgra2argb( ARGBColors[ BUT_TOP ] ) );
        pixelColor( buttons[ i ].surfacedown, 4, 4, bgra2argb( ARGBColors[ BUT_TOP ] ) );

        lineColor( buttons[ i ].surfacedown, 3, buttons[ i ].h - 3, buttons[ i ].w - 3, buttons[ i ].h - 3,
                   bgra2argb( ARGBColors[ BUT_BOT ] ) );
        lineColor( buttons[ i ].surfacedown, 4, buttons[ i ].h - 4, buttons[ i ].w - 4, buttons[ i ].h - 4,
                   bgra2argb( ARGBColors[ BUT_BOT ] ) );
        lineColor( buttons[ i ].surfacedown, buttons[ i ].w - 3, buttons[ i ].h - 3, buttons[ i ].w - 3, 3,
                   bgra2argb( ARGBColors[ BUT_BOT ] ) );
        lineColor( buttons[ i ].surfacedown, buttons[ i ].w - 4, buttons[ i ].h - 4, buttons[ i ].w - 4, 4,
                   bgra2argb( ARGBColors[ BUT_BOT ] ) );
        pixelColor( buttons[ i ].surfacedown, buttons[ i ].w - 5, buttons[ i ].h - 5, bgra2argb( ARGBColors[ BUT_BOT ] ) );

        // draw frame around button
        lineColor( buttons[ i ].surfacedown, 0, buttons[ i ].h - 3, 0, 2, bgra2argb( ARGBColors[ FRAME ] ) );
        lineColor( buttons[ i ].surfacedown, 2, 0, buttons[ i ].w - 3, 0, bgra2argb( ARGBColors[ FRAME ] ) );
        lineColor( buttons[ i ].surfacedown, 2, buttons[ i ].h - 1, buttons[ i ].w - 3, buttons[ i ].h - 1,
                   bgra2argb( ARGBColors[ FRAME ] ) );
        lineColor( buttons[ i ].surfacedown, buttons[ i ].w - 1, buttons[ i ].h - 3, buttons[ i ].w - 1, 2,
                   bgra2argb( ARGBColors[ FRAME ] ) );

        if ( i == HPKEY_ON ) {
            lineColor( buttons[ i ].surfacedown, 1, 1, buttons[ i ].w - 2, 1, bgra2argb( ARGBColors[ FRAME ] ) );
            pixelColor( buttons[ i ].surfacedown, 1, 2, bgra2argb( ARGBColors[ FRAME ] ) );
            pixelColor( buttons[ i ].surfacedown, buttons[ i ].w - 2, 2, bgra2argb( ARGBColors[ FRAME ] ) );
        } else {
            pixelColor( buttons[ i ].surfacedown, 1, 1, bgra2argb( ARGBColors[ FRAME ] ) );
            pixelColor( buttons[ i ].surfacedown, buttons[ i ].w - 2, 1, bgra2argb( ARGBColors[ FRAME ] ) );
        }
        pixelColor( buttons[ i ].surfacedown, 1, buttons[ i ].h - 2, bgra2argb( ARGBColors[ FRAME ] ) );
        pixelColor( buttons[ i ].surfacedown, buttons[ i ].w - 2, buttons[ i ].h - 2, bgra2argb( ARGBColors[ FRAME ] ) );
        if ( i == HPKEY_ON ) {
            rectangleColor( buttons[ i ].surfacedown, 1, 2, 1 + buttons[ i ].w - 3, 2 + buttons[ i ].h - 4,
                            bgra2argb( ARGBColors[ FRAME ] ) );
            pixelColor( buttons[ i ].surfacedown, 2, 3, bgra2argb( ARGBColors[ FRAME ] ) );
            pixelColor( buttons[ i ].surfacedown, buttons[ i ].w - 3, 3, bgra2argb( ARGBColors[ FRAME ] ) );
        } else {
            rectangleColor( buttons[ i ].surfacedown, 1, 1, 1 + buttons[ i ].w - 3, 1 + buttons[ i ].h - 3,
                            bgra2argb( ARGBColors[ FRAME ] ) );
            pixelColor( buttons[ i ].surfacedown, 2, 2, bgra2argb( ARGBColors[ FRAME ] ) );
            pixelColor( buttons[ i ].surfacedown, buttons[ i ].w - 3, 2, bgra2argb( ARGBColors[ FRAME ] ) );
        }
        pixelColor( buttons[ i ].surfacedown, 2, buttons[ i ].h - 3, bgra2argb( ARGBColors[ FRAME ] ) );
        pixelColor( buttons[ i ].surfacedown, buttons[ i ].w - 3, buttons[ i ].h - 3, bgra2argb( ARGBColors[ FRAME ] ) );

        if ( buttons[ i ].label != ( char* )0 ) {
            // Todo: use SDL_ttf to print "nice" fonts

            // for the time being use SDL_gfxPrimitives' font
            x = ( buttons[ i ].w - strlen( buttons[ i ].label ) * 8 ) / 2;
            y = ( buttons[ i ].h + 1 ) / 2 - 4;
            stringColor( buttons[ i ].surfaceup, x, y, buttons[ i ].label, 0xffffffff );
            stringColor( buttons[ i ].surfacedown, x, y, buttons[ i ].label, 0xffffffff );
        }
        // Pixmap centered in button
        if ( buttons[ i ].lw != 0 ) {
            // If there's a bitmap, try to plot this
            unsigned colorbg = ARGBColors[ BUTTON ];
            unsigned colorfg = ARGBColors[ buttons[ i ].lc ];

            // Blit the label surface to the button
            SDL_Surface* surf;
            surf = SDLCreateSurfFromData( buttons[ i ].lw, buttons[ i ].lh, buttons[ i ].lb, colorfg, colorbg );
            // Draw the surface on the center of the button
            x = ( 1 + buttons[ i ].w - buttons[ i ].lw ) / 2;
            y = ( 1 + buttons[ i ].h - buttons[ i ].lh ) / 2 + 1;
            SDL_Rect srect;
            SDL_Rect drect;
            srect.x = 0;
            srect.y = 0;
            srect.w = buttons[ i ].lw;
            srect.h = buttons[ i ].lh;
            drect.x = x;
            drect.y = y;
            drect.w = buttons[ i ].lw;
            drect.h = buttons[ i ].lh;
            SDL_BlitSurface( surf, &srect, buttons[ i ].surfacedown, &drect );
            SDL_BlitSurface( surf, &srect, buttons[ i ].surfaceup, &drect );
            SDL_FreeSurface( surf );
        }
    }
}

// Draw the left labels (violet on GX)
static void SDLDrawKeysLabelsLeft( void )
{
    int i, x, y;
    unsigned int pw /* , ph */;
    int wl, wr, ws;
    int offset_y = KEYBOARD_OFFSET_Y;
    int offset_x = KEYBOARD_OFFSET_X;

    unsigned colorbg, colorfg;

    // Draw the left labels
    for ( i = FIRST_HPKEY; i <= LAST_HPKEY; i++ ) {
        // No label -> skip
        if ( buttons[ i ].left == ( char* )0 )
            continue;

        if ( buttons[ i ].is_menu ) {
            // draw the dark shade under the label

            if ( opt_gx ) {
                pw = 58;
            } else {
                pw = 46;
            }

            colorbg = ARGBColors[ UNDERLAY ];
            colorfg = ARGBColors[ LEFT ];

            x = ( pw + 1 - SmallTextWidth( buttons[ i ].left, strlen( buttons[ i ].left ) ) ) / 2;
            if ( opt_gx )
                y = 14;
            else
                y = 9;

            // Set the coordinates to absolute
            if ( opt_gx ) {
                x += offset_x + buttons[ i ].x - 6;
                y += offset_y + buttons[ i ].y - small_ascent - small_descent - 6;
            } else {
                x += offset_x + buttons[ i ].x + ( buttons[ i ].w - pw ) / 2;
                y += offset_y + buttons[ i ].y - small_ascent - small_descent;
            }

            SDLDrawSmallString( x, y, buttons[ i ].left, strlen( buttons[ i ].left ), colorfg, colorbg );
        } else // is_menu
        {
            colorbg = ARGBColors[ BLACK ];
            colorfg = ARGBColors[ LEFT ];

            if ( buttons[ i ].right == ( char* )0 ) {
                // centered label
                x = offset_x + buttons[ i ].x +
                    ( 1 + buttons[ i ].w - SmallTextWidth( buttons[ i ].left, strlen( buttons[ i ].left ) ) ) / 2;
            } else {
                // label to the left
                wl = SmallTextWidth( buttons[ i ].left, strlen( buttons[ i ].left ) );
                wr = SmallTextWidth( buttons[ i ].right, strlen( buttons[ i ].right ) );
                ws = SmallTextWidth( " ", 1 );

                x = offset_x + buttons[ i ].x + ( 1 + buttons[ i ].w - ( wl + wr + ws ) ) / 2;
            }

            y = offset_y + buttons[ i ].y - small_descent;

            SDLDrawSmallString( x, y, buttons[ i ].left, strlen( buttons[ i ].left ), colorfg, colorbg );
        } // is_menu

    } // for
}

// Draw the right labels (green on GX)
static void SDLDrawKeysLabelsRight( void )
{
    int i, x, y;
    unsigned int pw /* , ph */;
    int wl, wr, ws;
    int offset_y = KEYBOARD_OFFSET_Y;
    int offset_x = KEYBOARD_OFFSET_X;
    unsigned colorbg, colorfg;

    // draw the right labels
    for ( i = FIRST_HPKEY; i <= LAST_HPKEY; i++ ) {
        if ( buttons[ i ].right == ( char* )0 )
            continue;

        if ( buttons[ i ].is_menu ) {
            // draw the dark shade under the label
            if ( opt_gx ) {
                pw = 58;
            } else {
                pw = 44;
            }

            colorbg = ARGBColors[ UNDERLAY ];
            colorfg = ARGBColors[ RIGHT ];

            x = ( pw + 1 - SmallTextWidth( buttons[ i ].right, strlen( buttons[ i ].right ) ) ) / 2;
            if ( opt_gx )
                y = 14;
            else
                y = 8;

            // Set the coordinates to absolute
            if ( opt_gx ) {
                x += offset_x + buttons[ i ].x - 6;
                y += offset_y + buttons[ i ].y - small_ascent - small_descent - 6;
            } else {
                x += offset_x + buttons[ i ].x + ( buttons[ i ].w - pw ) / 2;
                y += offset_y + buttons[ i ].y - small_ascent - small_descent;
            }

            SDLDrawSmallString( x, y, buttons[ i ].right, strlen( buttons[ i ].right ), colorfg, colorbg );
        } // buttons[i].is_menu
        else {
            colorbg = ARGBColors[ BLACK ];
            colorfg = ARGBColors[ RIGHT ];

            if ( buttons[ i ].left == ( char* )0 ) {
                // centered label
                x = offset_x + buttons[ i ].x +
                    ( 1 + buttons[ i ].w - SmallTextWidth( buttons[ i ].right, strlen( buttons[ i ].right ) ) ) / 2;
            } else {
                // label to the right
                wl = SmallTextWidth( buttons[ i ].left, strlen( buttons[ i ].left ) );
                wr = SmallTextWidth( buttons[ i ].right, strlen( buttons[ i ].right ) );
                ws = SmallTextWidth( " ", 1 );

                x = offset_x + buttons[ i ].x + ( 1 + buttons[ i ].w - ( wl + wr + ws ) ) / 2 + wl + ws;
            }

            y = offset_y + buttons[ i ].y - small_descent;

            SDLDrawSmallString( x, y, buttons[ i ].right, strlen( buttons[ i ].right ), colorfg, colorbg );
        }

    } // for
}

// Draw the letter bottom right of the keys
static void SDLDrawKeysLetters( void )
{
    int i, x, y;
    int offset_y = KEYBOARD_OFFSET_Y;
    int offset_x = KEYBOARD_OFFSET_X;
    unsigned colorbg;

    for ( i = FIRST_HPKEY; i <= LAST_HPKEY; i++ ) {

        if ( i < HPKEY_MTH )
            colorbg = ARGBColors[ DISP_PAD ];
        else
            colorbg = ARGBColors[ PAD ];

        // Letter ( small character bottom right of key)
        if ( buttons[ i ].letter != ( char* )0 ) {
            if ( opt_gx ) {
                x = offset_x + buttons[ i ].x + buttons[ i ].w + 3;
                y = offset_y + buttons[ i ].y + buttons[ i ].h + 1;
            } else {
                x = offset_x + buttons[ i ].x + buttons[ i ].w - SmallTextWidth( buttons[ i ].letter, 1 ) / 2 + 5;
                y = offset_y + buttons[ i ].y + buttons[ i ].h - 2;
            }

            SDLDrawSmallString( x, y, buttons[ i ].letter, 1, 0xffffffff, colorbg );
        }
    }
}

// Bottom label: the only one is the cancel button
static void SDLDrawKeysLabelsBottom( void )
{
    int i, x, y;
    int offset_y = KEYBOARD_OFFSET_Y;
    int offset_x = KEYBOARD_OFFSET_X;
    unsigned colorbg, colorfg;

    // Bottom label: the only one is the cancel button
    for ( i = FIRST_HPKEY; i <= LAST_HPKEY; i++ ) {
        if ( buttons[ i ].sub == ( char* )0 )
            continue;

        if ( i < HPKEY_MTH )
            colorbg = ARGBColors[ DISP_PAD ];
        else
            colorbg = ARGBColors[ PAD ];

        colorfg = ARGBColors[ WHITE ];

        x = offset_x + buttons[ i ].x + ( 1 + buttons[ i ].w - SmallTextWidth( buttons[ i ].sub, strlen( buttons[ i ].sub ) ) ) / 2;
        y = offset_y + buttons[ i ].y + buttons[ i ].h + small_ascent + 2;
        SDLDrawSmallString( x, y, buttons[ i ].sub, strlen( buttons[ i ].sub ), colorfg, colorbg );
    }
}

// Draws the greyish area around keys that trigger menus
static void SDLDrawKeyMenu( void )
{
    int i, x, y;
    int offset_y = KEYBOARD_OFFSET_Y;
    int offset_x = KEYBOARD_OFFSET_X;
    SDL_Rect rect;
    unsigned color;
    unsigned pw, ph;

    for ( i = FIRST_HPKEY; i <= LAST_HPKEY; i++ ) {
        if ( !buttons[ i ].is_menu )
            continue;

        // draw the dark shade under the label
        if ( opt_gx ) {
            pw = 58;
            ph = 48;
        } else {
            pw = 44;
            ph = 9;
        }
        color = ARGBColors[ UNDERLAY ];

        // Set the coordinates to absolute
        if ( opt_gx ) {
            x = offset_x + buttons[ i ].x - 6;
            y = offset_y + buttons[ i ].y - small_ascent - small_descent - 6;
        } else {
            x = offset_x + buttons[ i ].x + ( buttons[ i ].w - pw ) / 2;
            y = offset_y + buttons[ i ].y - small_ascent - small_descent;
        }

        rect.x = x;
        rect.y = y;
        rect.w = pw;
        rect.h = ph;
        SDL_FillRect( sdlwindow, &rect, color );
    }
}

static void SDLDrawButtons( void )
{
    SDL_Rect srect, drect;

    for ( int i = FIRST_HPKEY; i <= LAST_HPKEY; i++ ) {
        // Blit the button surface to the screen
        srect.x = 0;
        srect.y = 0;
        srect.w = buttons[ i ].w;
        srect.h = buttons[ i ].h;
        drect.x = KEYBOARD_OFFSET_X + buttons[ i ].x;
        drect.y = KEYBOARD_OFFSET_Y + buttons[ i ].y;
        drect.w = buttons[ i ].w;
        drect.h = buttons[ i ].h;
        if ( keyboard[ i ].pressed )
            SDL_BlitSurface( buttons[ i ].surfacedown, &srect, sdlwindow, &drect );
        else
            SDL_BlitSurface( buttons[ i ].surfaceup, &srect, sdlwindow, &drect );
    }

    // Always update immediately buttons
    SDL_UpdateRect( sdlwindow, KEYBOARD_OFFSET_X + buttons[ 0 ].x, KEYBOARD_OFFSET_Y + buttons[ 0 ].y,
                    buttons[ LAST_HPKEY ].x + buttons[ LAST_HPKEY ].w - buttons[ 0 ].x,
                    buttons[ LAST_HPKEY ].y + buttons[ LAST_HPKEY ].h - buttons[ 0 ].y );
}

static void SDLDrawKeypad( void )
{
    SDLDrawKeyMenu();
    SDLDrawKeysLetters();
    SDLDrawKeysLabelsBottom();
    SDLDrawKeysLabelsLeft();
    SDLDrawKeysLabelsRight();
    SDLCreateKeys();
    SDLDrawButtons();
}

static void SDLDrawBezel()
{
    unsigned int i;
    int display_height = DISPLAY_HEIGHT;
    int display_width = DISPLAY_WIDTH;

    // draw the frame around the display
    for ( i = 0; i < DISP_FRAME; i++ ) {
        lineColor( sdlwindow, DISPLAY_OFFSET_X - i, DISPLAY_OFFSET_Y + display_height + 2 * i, DISPLAY_OFFSET_X + display_width + i,
                   DISPLAY_OFFSET_Y + display_height + 2 * i, bgra2argb( ARGBColors[ DISP_PAD_TOP ] ) );
        lineColor( sdlwindow, DISPLAY_OFFSET_X - i, DISPLAY_OFFSET_Y + display_height + 2 * i + 1, DISPLAY_OFFSET_X + display_width + i,
                   DISPLAY_OFFSET_Y + display_height + 2 * i + 1, bgra2argb( ARGBColors[ DISP_PAD_TOP ] ) );
        lineColor( sdlwindow, DISPLAY_OFFSET_X + display_width + i, DISPLAY_OFFSET_Y - i, DISPLAY_OFFSET_X + display_width + i,
                   DISPLAY_OFFSET_Y + display_height + 2 * i, bgra2argb( ARGBColors[ DISP_PAD_TOP ] ) );
    }

    for ( i = 0; i < DISP_FRAME; i++ ) {
        lineColor( sdlwindow, DISPLAY_OFFSET_X - i - 1, DISPLAY_OFFSET_Y - i - 1, DISPLAY_OFFSET_X + display_width + i - 1,
                   DISPLAY_OFFSET_Y - i - 1, bgra2argb( ARGBColors[ DISP_PAD_BOT ] ) );
        lineColor( sdlwindow, DISPLAY_OFFSET_X - i - 1, DISPLAY_OFFSET_Y - i - 1, DISPLAY_OFFSET_X - i - 1,
                   DISPLAY_OFFSET_Y + display_height + 2 * i - 1, bgra2argb( ARGBColors[ DISP_PAD_BOT ] ) );
    }

    // round off corners
    lineColor( sdlwindow, DISPLAY_OFFSET_X - DISP_FRAME, DISPLAY_OFFSET_Y - DISP_FRAME, DISPLAY_OFFSET_X - DISP_FRAME + 3,
               DISPLAY_OFFSET_Y - DISP_FRAME, bgra2argb( ARGBColors[ DISP_PAD ] ) );
    lineColor( sdlwindow, DISPLAY_OFFSET_X - DISP_FRAME, DISPLAY_OFFSET_Y - DISP_FRAME, DISPLAY_OFFSET_X - DISP_FRAME,
               DISPLAY_OFFSET_Y - DISP_FRAME + 3, bgra2argb( ARGBColors[ DISP_PAD ] ) );
    pixelColor( sdlwindow, DISPLAY_OFFSET_X - DISP_FRAME + 1, DISPLAY_OFFSET_Y - DISP_FRAME + 1, bgra2argb( ARGBColors[ DISP_PAD ] ) );

    lineColor( sdlwindow, DISPLAY_OFFSET_X + display_width + DISP_FRAME - 4, DISPLAY_OFFSET_Y - DISP_FRAME,
               DISPLAY_OFFSET_X + display_width + DISP_FRAME - 1, DISPLAY_OFFSET_Y - DISP_FRAME, bgra2argb( ARGBColors[ DISP_PAD ] ) );
    lineColor( sdlwindow, DISPLAY_OFFSET_X + display_width + DISP_FRAME - 1, DISPLAY_OFFSET_Y - DISP_FRAME,
               DISPLAY_OFFSET_X + display_width + DISP_FRAME - 1, DISPLAY_OFFSET_Y - DISP_FRAME + 3, bgra2argb( ARGBColors[ DISP_PAD ] ) );
    pixelColor( sdlwindow, DISPLAY_OFFSET_X + display_width + DISP_FRAME - 2, DISPLAY_OFFSET_Y - DISP_FRAME + 1,
                bgra2argb( ARGBColors[ DISP_PAD ] ) );

    lineColor( sdlwindow, DISPLAY_OFFSET_X - DISP_FRAME, DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 4,
               DISPLAY_OFFSET_X - DISP_FRAME, DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 1, bgra2argb( ARGBColors[ DISP_PAD ] ) );
    lineColor( sdlwindow, DISPLAY_OFFSET_X - DISP_FRAME, DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 1,
               DISPLAY_OFFSET_X - DISP_FRAME + 3, DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 1,
               bgra2argb( ARGBColors[ DISP_PAD ] ) );
    pixelColor( sdlwindow, DISPLAY_OFFSET_X - DISP_FRAME + 1, DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 2,
                bgra2argb( ARGBColors[ DISP_PAD ] ) );

    lineColor( sdlwindow, DISPLAY_OFFSET_X + display_width + DISP_FRAME - 1, DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 4,
               DISPLAY_OFFSET_X + display_width + DISP_FRAME - 1, DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 1,
               bgra2argb( ARGBColors[ DISP_PAD ] ) );
    lineColor( sdlwindow, DISPLAY_OFFSET_X + display_width + DISP_FRAME - 4, DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 1,
               DISPLAY_OFFSET_X + display_width + DISP_FRAME - 1, DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 1,
               bgra2argb( ARGBColors[ DISP_PAD ] ) );
    pixelColor( sdlwindow, DISPLAY_OFFSET_X + display_width + DISP_FRAME - 2, DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 2,
                bgra2argb( ARGBColors[ DISP_PAD ] ) );

    // simulate rounded lcd corners

    lineColor( sdlwindow, DISPLAY_OFFSET_X - 1, DISPLAY_OFFSET_Y + 1, DISPLAY_OFFSET_X - 1, DISPLAY_OFFSET_Y + display_height - 2,
               bgra2argb( ARGBColors[ LCD ] ) );
    lineColor( sdlwindow, DISPLAY_OFFSET_X + 1, DISPLAY_OFFSET_Y - 1, DISPLAY_OFFSET_X + display_width - 2, DISPLAY_OFFSET_Y - 1,
               bgra2argb( ARGBColors[ LCD ] ) );
    lineColor( sdlwindow, DISPLAY_OFFSET_X + 1, DISPLAY_OFFSET_Y + display_height, DISPLAY_OFFSET_X + display_width - 2,
               DISPLAY_OFFSET_Y + display_height, bgra2argb( ARGBColors[ LCD ] ) );
    lineColor( sdlwindow, DISPLAY_OFFSET_X + display_width, DISPLAY_OFFSET_Y + 1, DISPLAY_OFFSET_X + display_width,
               DISPLAY_OFFSET_Y + display_height - 2, bgra2argb( ARGBColors[ LCD ] ) );
}

static void SDLDrawBackground( int width, int height, int w_top, int h_top )
{
    SDL_Rect rect;

    rect.x = 0;
    rect.y = 0;
    rect.w = w_top;
    rect.h = h_top;
    SDL_FillRect( sdlwindow, &rect, ARGBColors[ PAD ] );

    rect.w = width;
    rect.h = height;
    SDL_FillRect( sdlwindow, &rect, ARGBColors[ DISP_PAD ] );
}

static void SDLDrawBackgroundLCD()
{
    SDL_Rect rect;

    rect.x = DISPLAY_OFFSET_X;
    rect.y = DISPLAY_OFFSET_Y;
    rect.w = DISPLAY_WIDTH;
    rect.h = DISPLAY_HEIGHT;
    SDL_FillRect( sdlwindow, &rect, ARGBColors[ LCD ] );
}

static void SDLDrawAnnunc( char* annunc )
{
    SDLCreateAnnunc();

    // Print the annunciator
    for ( int i = 0; i < 6; i++ ) {
        SDL_Rect srect;
        SDL_Rect drect;
        srect.x = 0;
        srect.y = 0;
        srect.w = ann_tbl[ i ].width;
        srect.h = ann_tbl[ i ].height;
        drect.x = DISPLAY_OFFSET_X + ann_tbl[ i ].x;
        drect.y = DISPLAY_OFFSET_Y + ann_tbl[ i ].y;
        drect.w = ann_tbl[ i ].width;
        drect.h = ann_tbl[ i ].height;
        if ( annunc[ i ] )
            SDL_BlitSurface( ann_tbl[ i ].surfaceon, &srect, sdlwindow, &drect );
        else
            SDL_BlitSurface( ann_tbl[ i ].surfaceoff, &srect, sdlwindow, &drect );
    }

    // Always immediately update annunciators
    SDL_UpdateRect( sdlwindow, DISPLAY_OFFSET_X + ann_tbl[ 0 ].x, DISPLAY_OFFSET_Y + ann_tbl[ 0 ].y,
                    ann_tbl[ 5 ].x + ann_tbl[ 5 ].width - ann_tbl[ 0 ].x, ann_tbl[ 5 ].y + ann_tbl[ 5 ].height - ann_tbl[ 0 ].y );
}

static void SDLUIHideKey( void )
{
    SDL_Rect drect;

    if ( showkeylastsurf == 0 )
        return;

    drect.x = showkeylastx;
    drect.y = showkeylasty;
    SDL_BlitSurface( showkeylastsurf, 0, sdlwindow, &drect );

    // Update
    SDL_UpdateRect( sdlwindow, showkeylastx, showkeylasty, showkeylastsurf->w, showkeylastsurf->h );

    // Free
    SDL_FreeSurface( showkeylastsurf );
    showkeylastsurf = 0;
}

// Show the hp key which is being pressed
static void SDLUIShowKey( int hpkey )
{
    SDL_Rect srect, drect;
    SDL_Surface* ssurf;
    int x;
    int y;

    // If we're called with the same key as before, do nothing
    if ( showkeylastkey == hpkey )
        return;

    showkeylastkey = hpkey;

    // Starts by hiding last
    SDLUIHideKey();

    if ( hpkey == -1 )
        return;

    // Which surface to show
    ssurf = ( keyboard[ hpkey ].pressed ) ? buttons[ hpkey ].surfacedown : buttons[ hpkey ].surfaceup;

    // Background backup
    showkeylastsurf = SDL_CreateRGBSurface( SDL_SWSURFACE, ssurf->w, ssurf->h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 );

    // Where to
    x = KEYBOARD_OFFSET_X + buttons[ hpkey ].x - ( ssurf->w - ssurf->w + 1 ) / 2;
    y = KEYBOARD_OFFSET_Y + buttons[ hpkey ].y - ( ssurf->h - ssurf->h + 1 ) / 2;
    // blitting does not clip to screen, so if we are out of the screen we
    // shift the button to fit
    if ( x < 0 )
        x = 0;
    if ( y < 0 )
        y = 0;
    if ( x + ssurf->w > sdlwindow->w )
        x = sdlwindow->w - ssurf->w;
    if ( y + ssurf->h > sdlwindow->h )
        y = sdlwindow->h - ssurf->h;

    // Backup where to
    showkeylastx = x;
    showkeylasty = y;

    // Backup old surface
    srect.x = x;
    srect.y = y;
    srect.w = ssurf->w;
    srect.h = ssurf->h;
    drect.x = 0;
    drect.y = 0;
    SDL_BlitSurface( sdlwindow, &srect, showkeylastsurf, &drect );

    // Blit the button
    drect.x = x;
    drect.y = y;
    SDL_BlitSurface( ssurf, 0, sdlwindow, &drect );

    // Update
    SDL_UpdateRect( sdlwindow, x, y, ssurf->w, ssurf->h );
}

static inline void SDLUIFeedback( void ) {}

static void SDLDrawSerialDevices()
{
    char text[ 1024 ] = "";

    if ( config.verbose ) {
        fprintf( stderr, "wire_name: %s\n", wire_name );
        fprintf( stderr, "ir_name: %s\n", ir_name );
    }

    if ( wire_name ) {
        strcat( text, "wire: " );
        strcat( text, wire_name );
    }
    if ( ir_name ) {
        if ( strlen( text ) > 0 )
            strcat( text, " | " );

        strcat( text, "ir: " );
        strcat( text, ir_name );
    }

    if ( strlen( text ) > 0 )
        stringColor( sdlwindow, 10, 240, text, 0xffffffff );
}

static void SDLDrawNibble( int nx, int ny, int val )
{
    int x, y;
    int xoffset = DISPLAY_OFFSET_X + 5;
    int yoffset = DISPLAY_OFFSET_Y + 20;

    SDL_LockSurface( sdlwindow );
    unsigned char* buffer = ( unsigned char* )sdlwindow->pixels;
    unsigned int pitch = sdlwindow->pitch;

    for ( y = 0; y < 2; y++ ) {
        unsigned int* lineptr;
        lineptr = ( unsigned int* )( buffer + pitch * ( yoffset + 2 * ny + y ) );

        for ( x = 0; x < 4; x++ ) {
            // Check if bit is on
            // bits in a byte are used (1 nibble per byte)
            if ( nx + x >= 131 ) // Clip at 131 pixels
                break;

            char c = val;
            char b = c & ( 1 << ( x & 3 ) );

            lineptr[ xoffset + 2 * ( nx + x ) ] = ARGBColors[ b ? PIXEL : LCD ];
            lineptr[ xoffset + 2 * ( nx + x ) + 1 ] = ARGBColors[ b ? PIXEL : LCD ];
        }
    }
    SDL_UnlockSurface( sdlwindow );

#ifndef DELAYEDDISPUPDATE
    // Either update immediately or with a delay the display
    SDL_UpdateRect( sdlwindow, xoffset + 2 * nx, yoffset + 2 * ny, 8, 2 );
#endif
}

static inline void draw_nibble( int col, int row, int val )
{
    val &= 0x0f;
    if ( val == lcd_nibbles_buffer[ row ][ col ] )
        return;

    lcd_nibbles_buffer[ row ][ col ] = val;

    int y = row;
    int x = col * 4;
    if ( row <= display.lines )
        x -= ( 2 * display.offset );

    SDLDrawNibble( x, y, val );
}

/* Same as in ui_x11.c */
static inline void draw_row( long addr, int row )
{
    int nibble;
    int line_length = NIBBLES_PER_ROW;

    if ( ( display.offset > 3 ) && ( row <= display.lines ) )
        line_length += 2;

    for ( int i = 0; i < line_length; i++ ) {
        nibble = read_nibble( addr + i );

        draw_nibble( i, row, nibble );
    }
}

static void SDLCreateHP( void )
{
    unsigned int width, height;

    if ( config.hide_chrome ) {
        width = KEYBOARD_WIDTH;
        height = DISPLAY_HEIGHT;
    } else {
        width = KEYBOARD_WIDTH + 2 * SIDE_SKIP;
        height = DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + DISP_KBD_SKIP + KEYBOARD_HEIGHT + BOTTOM_SKIP;
    }

    keypad.width = width;
    keypad.height = height;

    sdl_colors = opt_gx ? sdl_colors_gx : sdl_colors_sx;

    // we allocate memory for the buttons because we need to modify
    // their coordinates, and we don't want to change the original buttons_gx or
    // buttons_sx
    if ( buttons ) {
        free( buttons );
        buttons = 0;
    }
    buttons = ( sdl_button_t* )malloc( sizeof( buttons_gx ) );

    if ( opt_gx )
        memcpy( buttons, buttons_gx, sizeof( buttons_gx ) );
    else
        memcpy( buttons, buttons_sx, sizeof( buttons_sx ) );

    SDLCreateColors();

    if ( !config.hide_chrome ) {
        int cut = buttons[ HPKEY_MTH ].y + KEYBOARD_OFFSET_Y - 19;

        SDLDrawBackground( width, cut, width, height );
        SDLDrawMore( cut, KEYBOARD_OFFSET_Y, keypad.width, keypad.height );
        SDLDrawLogo();
        SDLDrawBezel();
        SDLDrawKeypad();

        SDLDrawSerialDevices();
    }

    SDLDrawBackgroundLCD();

    SDL_UpdateRect( sdlwindow, 0, 0, 0, 0 );
}

/**********/
/* public */
/**********/
int sdl_get_event( void )
{
    SDL_Event event;
    int hpkey;
    int rv;
    static int lasthpkey = -1;      // last key that was pressed or -1 for none
    static int lastticks = -1;      // time at which a key was pressed or -1 if timer expired
    static int lastislongpress = 0; // last key press was a long press
    static int keyispressed = -1;   // Indicate if a key is being held down by
                                    // a finger (not set for long presses)
    static int keyneedshow = 0;     // Indicates if the buttons need to be shown

    rv = 0; // nothing to do

    // Check whether long pres on key
    if ( lastticks > 0 && ( SDL_GetTicks() - lastticks > 750 ) ) {
        // time elapsed
        lastticks = -1;

        // Check that the mouse is still on the same last key
        int x, y, state;
        state = SDL_GetMouseState( &x, &y );

        if ( state & SDL_BUTTON( 1 ) && SDLCoordinateToKey( x, y ) == lasthpkey ) {
            lastislongpress = 1;
            SDLUIFeedback();
        }
    }

    // Iterate as long as there are events
    // while( SDL_PollEvent( &event ) )
    if ( SDL_PollEvent( &event ) ) {
        switch ( event.type ) {
            case SDL_QUIT:
                please_exit = true;
                break;

                /* // Mouse move: react to state changes in the buttons that are
                 */
                /* // pressed */
                /* case SDL_MOUSEMOTION: */
                /*     hpkey = SDLCoordinateToKey( event.motion.x,
                 * event.motion.y ); */
                /*     if ( event.motion.state & SDL_BUTTON( 1 ) ) { */
                /*         // Mouse moves on a key different from the last key
                 */
                /*         // (state change): */
                /*         // - release last (if last was pressed) */
                /*         // - press new (if new is pressed) */
                /*         if ( hpkey != lasthpkey ) { */
                /*             keyispressed = hpkey; */

                /*             if ( lasthpkey != -1 ) { */
                /*                 if ( !lastislongpress ) { */
                /*                     release_all_keys(); */
                /*                     rv = 1; */
                /*                     SDLUIFeedback(); */
                /*                 } */
                /*                 // Stop timer, clear long key press */
                /*                 lastticks = -1; */
                /*                 lastislongpress = 0; */
                /*             } */
                /*             if ( hpkey != -1 ) { */
                /*                 if ( !buttons[ hpkey ] */
                /*                           .pressed ) // If a key is down, it
                 */
                /*                                      // can't be down another
                 */
                /*                                      // time */
                /*                 { */
                /*                     press_key( hpkey ); */
                /*                     rv = 1; */
                /*                     // Start timer */
                /*                     lastticks = SDL_GetTicks(); */
                /*                     SDLUIFeedback(); */
                /*                 } */
                /*             } */
                /*         } */
                /*         lasthpkey = hpkey; */
                /*     } */
                /*     if ( hpkey == -1 ) // Needed to avoid pressing and moving
                 */
                /*                        // outside of a button releases */
                /*         lasthpkey = -1; */

                /*     break; */

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                hpkey = SDLCoordinateToKey( event.button.x, event.button.y );

                // React to mouse up/down when click over a button
                if ( hpkey == -1 )
                    break;
                if ( event.type == SDL_MOUSEBUTTONDOWN ) {
                    keyispressed = hpkey;

                    if ( !keyboard[ hpkey ].pressed ) // Key can't be pressed
                                                      // when down
                    {
                        press_key( hpkey );
                        rv = 1;
                        lasthpkey = hpkey;
                        // Start timer
                        lastticks = SDL_GetTicks();
                        SDLUIFeedback();
                    }
                } else {
                    keyispressed = -1;

                    if ( !lastislongpress ) {
                        release_all_keys();
                        rv = 1;
                        lasthpkey = -1; // No key is pressed anymore
                        SDLUIFeedback();
                    }

                    // Stop timer, clear long key press
                    lastticks = -1;
                    lastislongpress = 0;
                }
                break;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
                hpkey = SDLKeyToKey( event.key.keysym.sym );

                if ( hpkey == -1 )
                    break;
                if ( event.type == SDL_KEYDOWN ) {
                    keyispressed = hpkey;

                    // Avoid pressing if it is already pressed
                    if ( !keyboard[ hpkey ].pressed ) {
                        press_key( hpkey );
                        rv = 1;
                        SDLUIFeedback();
                    }
                } else {
                    keyispressed = -1;

                    release_key( hpkey );
                    rv = 1;
                    SDLUIFeedback();
                }
                break;
        }
    }

    // Display button being pressed, if any
    if ( !config.hide_chrome )
        SDLUIShowKey( keyispressed );

    // If we press long, then the button releases makes SDLUIShowKey restore
    // the old key, but rv does not indicate that we need to update the
    // buttons. Therefore we save it here
    if ( rv )
        keyneedshow = 1;

    // Redraw the keyboard only if there is a button state change and no
    // button is pressed (otherwise it overwrites the zoomed button)
    if ( keyneedshow && keyispressed == -1 ) {
        keyneedshow = 0;
        SDLDrawButtons();
    }

#ifdef DELAYEDDISPUPDATE
    dispupdate_t2 = SDL_GetTicks();
    if ( dispupdate_t2 - dispupdate_t1 > DISPUPDATEINTERVAL ) {
        int xoffset = DISPLAY_OFFSET_X + 5;
        int yoffset = DISPLAY_OFFSET_Y + 20;

        // LCD
        SDL_UpdateRect( sdlwindow, xoffset, yoffset, 131 * 2, 64 * 2 );
        dispupdate_t1 = dispupdate_t2;
    }
#endif

    return 1;
}

void sdl_update_LCD( void )
{
    if ( display.on ) {
        int i;
        long addr;
        static int old_offset = -1;
        static int old_lines = -1;

        addr = display.disp_start;
        if ( display.offset != old_offset ) {
            memset( lcd_nibbles_buffer, 0xf0, ( size_t )( ( display.lines + 1 ) * NIBS_PER_BUFFER_ROW ) );
            old_offset = display.offset;
        }
        if ( display.lines != old_lines ) {
            memset( &lcd_nibbles_buffer[ 56 ][ 0 ], 0xf0, ( size_t )( 8 * NIBS_PER_BUFFER_ROW ) );
            old_lines = display.lines;
        }
        for ( i = 0; i <= display.lines; i++ ) {
            draw_row( addr, i );
            addr += display.nibs_per_line;
        }
        if ( i < DISP_ROWS ) {
            addr = display.menu_start;
            for ( ; i < DISP_ROWS; i++ ) {
                draw_row( addr, i );
                addr += NIBBLES_PER_ROW;
            }
        }
    } else
        ui_init_LCD();
}

void sdl_refresh_LCD( void ) {}

void sdl_disp_draw_nibble( word_20 addr, word_4 val )
{
    long offset;
    int x, y;

    offset = ( addr - display.disp_start );
    x = offset % display.nibs_per_line;
    if ( x < 0 || x > 35 )
        return;

    if ( display.nibs_per_line != 0 ) {
        y = offset / display.nibs_per_line;
        if ( y < 0 || y > 63 )
            return;

        draw_nibble( x, y, val );
    } else
        for ( y = 0; y < display.lines; y++ )
            draw_nibble( x, y, val );
}

void sdl_menu_draw_nibble( word_20 addr, word_4 val )
{
    long offset;
    int x, y;

    offset = ( addr - display.menu_start );
    x = offset % NIBBLES_PER_ROW;
    y = display.lines + ( offset / NIBBLES_PER_ROW ) + 1;

    draw_nibble( x, y, val );
}

void sdl_draw_annunc( void )
{
    int val = saturn.annunc;
    if ( val == last_annunc_state )
        return;

    last_annunc_state = val;

    char sdl_annuncstate[ 6 ];
    for ( int i = 0; i < NB_ANNUNCIATORS; i++ )
        sdl_annuncstate[ i ] = ( ( annunciators_bits[ i ] & val ) == annunciators_bits[ i ] ) ? 1 : 0;

    SDLDrawAnnunc( sdl_annuncstate );
}

void sdl_adjust_contrast()
{
    SDLCreateColors();
    SDLCreateAnnunc();

    // redraw LCD
    ui_init_LCD();
    sdl_update_LCD();

    // redraw annunc
    last_annunc_state = -1;

    sdl_draw_annunc();
}

void sdl_ui_stop() {}

void init_sdl_ui( int argc, char** argv )
{
    /* Set public API to this UI's functions */
    ui_disp_draw_nibble = sdl_disp_draw_nibble;
    ui_menu_draw_nibble = sdl_menu_draw_nibble;
    ui_get_event = sdl_get_event;
    ui_update_LCD = sdl_update_LCD;
    ui_refresh_LCD = sdl_refresh_LCD;
    ui_adjust_contrast = sdl_adjust_contrast;
    ui_draw_annunc = sdl_draw_annunc;

    SDLInit();
    SDLCreateHP();
}
