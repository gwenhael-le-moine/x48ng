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
#include <locale.h>

#include <SDL2/SDL.h>

#include "emulator.h"
#include "romio.h"
#include "runtime_options.h"
#include "ui.h"
#include "ui_inner.h"

#define PIXEL_WIDTH 2
#define PIXEL_HEIGHT 2

#define LCD_WIDTH 131
#define LCD_HEIGHT 64

#define LCD_OFFSET_X 1
#define LCD_OFFSET_Y 2
#define LCD_BOTTOM LCD_HEIGHT + LCD_OFFSET_Y
#define LCD_RIGHT LCD_WIDTH + LCD_OFFSET_X

#define LCD_COLOR_BG 48
#define LCD_COLOR_FG 49

#define LCD_PIXEL_ON 1
#define LCD_PIXEL_OFF 2

#define _KEYBOARD_HEIGHT ( buttons_gx[ LAST_HPKEY ].y + buttons_gx[ LAST_HPKEY ].h )
#define _KEYBOARD_WIDTH ( buttons_gx[ LAST_HPKEY ].x + buttons_gx[ LAST_HPKEY ].w )

#define _TOP_SKIP 65
#define _SIDE_SKIP 20
#define _BOTTOM_SKIP 25
#define _DISP_KBD_SKIP 65
#define _KBD_UPLINE 25

#define _DISPLAY_WIDTH ( ( LCD_WIDTH * PIXEL_WIDTH ) + 8 )
#define _DISPLAY_HEIGHT ( ( LCD_HEIGHT * PIXEL_HEIGHT ) + 16 + 8 )
#define _DISPLAY_OFFSET_X ( _SIDE_SKIP + ( 286 - _DISPLAY_WIDTH ) / 2 )
#define _DISPLAY_OFFSET_Y _TOP_SKIP

#define _DISP_FRAME 8

#define _KEYBOARD_OFFSET_X SIDE_SKIP
#define _KEYBOARD_OFFSET_Y ( TOP_SKIP + DISPLAY_HEIGHT + DISP_KBD_SKIP )

#define ANN_WIDTH 15
#define ANN_HEIGHT 12

/************/
/* typedefs */
/************/
typedef struct sdl2_color_t {
    const char* name;
    SDL_Color color;
} sdl2_color_t;

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

    SDL_Texture* surfaceup;
    SDL_Texture* surfacedown;
} sdl_button_t;

/*************/
/* variables */
/*************/
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

// This will take the value of the defines, but can be run-time modified
static unsigned display_offset_x, display_offset_y;

static SDL_Texture* annunciators_textures[ NB_ANNUNCIATORS ];
static SDL_Texture* annunciators_texture_off = NULL;
static SDL_Texture* lcd_texture = NULL;
static SDL_Texture* annunciators_texture = NULL;

static SDL_Rect lcd_rectangle;

static unsigned int ARGBColors[ BLACK + 1 ];
static sdl2_color_t* sdl_colors;

static sdl2_color_t sdl_colors_sx[] = {
    {"white",         { 255, 255, 255, 255 }},
    { "left",         { 255, 166, 0, 255 }  },
    { "right",        { 0, 210, 255, 255 }  },
    { "but_top",      { 109, 93, 93, 255 }  },
    { "button",       { 90, 77, 77, 255 }   },
    { "but_bot",      { 76, 65, 65, 255 }   },
    { "lcd_col",      { 202, 221, 92, 255 } },
    { "pix_col",      { 0, 0, 128, 255 }    },
    { "pad_top",      { 109, 78, 78, 255 }  },
    { "pad",          { 90, 64, 64, 255 }   },
    { "pad_bot",      { 76, 54, 54, 255 }   },
    { "disp_pad_top", { 155, 118, 84, 255 } },
    { "disp_pad",     { 124, 94, 67, 255 }  },
    { "disp_pad_bot", { 100, 75, 53, 255 }  },
    { "logo",         { 204, 169, 107, 255 }},
    { "logo_back",    { 64, 64, 64, 255 }   },
    { "label",        { 202, 184, 144, 255 }},
    { "frame",        { 0, 0, 0, 255 }      },
    { "underlay",     { 60, 42, 42, 255 }   },
    { "black",        { 0, 0, 0, 255 }      },
};

static sdl2_color_t sdl_colors_gx[] = {
    {"white",         { 255, 255, 255, 255 }},
    { "left",         { 255, 186, 255, 255 }},
    { "right",        { 0, 255, 204, 255 }  },
    { "but_top",      { 104, 104, 104, 255 }},
    { "button",       { 88, 88, 88, 255 }   },
    { "but_bot",      { 74, 74, 74, 255 }   },
    { "lcd_col",      { 202, 221, 92, 255 } },
    { "pix_col",      { 0, 0, 128, 255 }    },
    { "pad_top",      { 88, 88, 88, 255 }   },
    { "pad",          { 74, 74, 74, 255 }   },
    { "pad_bot",      { 64, 64, 64, 255 }   },
    { "disp_pad_top", { 128, 128, 138, 255 }},
    { "disp_pad",     { 104, 104, 110, 255 }},
    { "disp_pad_bot", { 84, 84, 90, 255 }   },
    { "logo",         { 176, 176, 184, 255 }},
    { "logo_back",    { 104, 104, 110, 255 }},
    { "label",        { 240, 240, 240, 255 }},
    { "frame",        { 0, 0, 0, 255 }      },
    { "underlay",     { 104, 104, 110, 255 }},
    { "black",        { 0, 0, 0, 255 }      },
};

static sdl_button_t* buttons = 0;

static sdl_button_t buttons_sx[] = {
    {"A",       0,   0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "A", 0,          0, 0,        0,      NULL, NULL},
    { "B",      50,  0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "B", 0,          0, 0,        0,      NULL, NULL},
    { "C",      100, 0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "C", 0,          0, 0,        0,      NULL, NULL},
    { "D",      150, 0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "D", 0,          0, 0,        0,      NULL, NULL},
    { "E",      200, 0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "E", 0,          0, 0,        0,      NULL, NULL},
    { "F",      250, 0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "F", 0,          0, 0,        0,      NULL, NULL},

    { "MTH",    0,   50,  36, 26, WHITE, "MTH",   0, 0,                0,                 0,                 "G", "PRINT",    1, 0,        0,      NULL, NULL},
    { "PRG",    50,  50,  36, 26, WHITE, "PRG",   0, 0,                0,                 0,                 "H", "I/O",      1, 0,        0,      NULL, NULL},
    { "CST",    100, 50,  36, 26, WHITE, "CST",   0, 0,                0,                 0,                 "I", "MODES",    1, 0,        0,      NULL, NULL},
    { "VAR",    150, 50,  36, 26, WHITE, "VAR",   0, 0,                0,                 0,                 "J", "MEMORY",   1, 0,        0,      NULL, NULL},
    { "UP",     200, 50,  36, 26, WHITE, 0,       0, up_width,         up_height,         up_bitmap,         "K", "LIBRARY",  1, 0,        0,      NULL, NULL},
    { "NXT",    250, 50,  36, 26, WHITE, "NXT",   0, 0,                0,                 0,                 "L", "PREV",     0, 0,        0,      NULL, NULL},

    { "COLON",  0,   100, 36, 26, WHITE, 0,       0, colon_width,      colon_height,      colon_bitmap,      "M", "UP",       0, "HOME",   0,      NULL, NULL},
    { "STO",    50,  100, 36, 26, WHITE, "STO",   0, 0,                0,                 0,                 "N", "DEF",      0, "RCL",    0,      NULL, NULL},
    { "EVAL",   100, 100, 36, 26, WHITE, "EVAL",  0, 0,                0,                 0,                 "O", "aQ",       0, "aNUM",   0,      NULL, NULL},
    { "LEFT",   150, 100, 36, 26, WHITE, 0,       0, left_width,       left_height,       left_bitmap,       "P", "GRAPH",    0, 0,        0,      NULL, NULL},
    { "DOWN",   200, 100, 36, 26, WHITE, 0,       0, down_width,       down_height,       down_bitmap,       "Q", "REVIEW",   0, 0,        0,      NULL, NULL},
    { "RIGHT",  250, 100, 36, 26, WHITE, 0,       0, right_width,      right_height,      right_bitmap,      "R", "SWAP",     0, 0,        0,      NULL, NULL},

    { "SIN",    0,   150, 36, 26, WHITE, "SIN",   0, 0,                0,                 0,                 "S", "ASIN",     0, "b",      0,      NULL, NULL},
    { "COS",    50,  150, 36, 26, WHITE, "COS",   0, 0,                0,                 0,                 "T", "ACOS",     0, "c",      0,      NULL, NULL},
    { "TAN",    100, 150, 36, 26, WHITE, "TAN",   0, 0,                0,                 0,                 "U", "ATAN",     0, "d",      0,      NULL, NULL},
    { "SQRT",   150, 150, 36, 26, WHITE, 0,       0, sqrt_width,       sqrt_height,       sqrt_bitmap,       "V", "e",        0, "f",      0,      NULL, NULL},
    { "POWER",  200, 150, 36, 26, WHITE, 0,       0, power_width,      power_height,      power_bitmap,      "W", "g",        0, "LOG",    0,      NULL, NULL},
    { "INV",    250, 150, 36, 26, WHITE, 0,       0, inv_width,        inv_height,        inv_bitmap,        "X", "h",        0, "LN",     0,      NULL, NULL},

    { "ENTER",  0,   200, 86, 26, WHITE, "ENTER", 2, 0,                0,                 0,                 0,   "EQUATION", 0, "MATRIX", 0,      NULL, NULL},
    { "NEG",    100, 200, 36, 26, WHITE, 0,       0, neg_width,        neg_height,        neg_bitmap,        "Y", "EDIT",     0, "VISIT",  0,      NULL, NULL},
    { "EEX",    150, 200, 36, 26, WHITE, "EEX",   0, 0,                0,                 0,                 "Z", "2D",       0, "3D",     0,      NULL, NULL},
    { "DEL",    200, 200, 36, 26, WHITE, "DEL",   0, 0,                0,                 0,                 0,   "PURGE",    0, 0,        0,      NULL, NULL},
    { "BS",     250, 200, 36, 26, WHITE, 0,       0, bs_width,         bs_height,         bs_bitmap,         0,   "DROP",     0, "CLR",    0,      NULL, NULL},

    { "ALPHA",  0,   250, 36, 26, WHITE, 0,       0, alpha_width,      alpha_height,      alpha_bitmap,      0,   "USR",      0, "ENTRY",  0,      NULL, NULL},
    { "7",      60,  250, 46, 26, WHITE, "7",     1, 0,                0,                 0,                 0,   "SOLVE",    1, 0,        0,      NULL, NULL},
    { "8",      120, 250, 46, 26, WHITE, "8",     1, 0,                0,                 0,                 0,   "PLOT",     1, 0,        0,      NULL, NULL},
    { "9",      180, 250, 46, 26, WHITE, "9",     1, 0,                0,                 0,                 0,   "ALGEBRA",  1, 0,        0,      NULL, NULL},
    { "DIV",    240, 250, 46, 26, WHITE, 0,       0, div_width,        div_height,        div_bitmap,        0,   "( )",      0, "#",      0,      NULL, NULL},

    { "SHL",    0,   300, 36, 26, LEFT,  0,       0, shl_width,        shl_height,        shl_bitmap,        0,   0,          0, 0,        0,      NULL, NULL},
    { "4",      60,  300, 46, 26, WHITE, "4",     1, 0,                0,                 0,                 0,   "TIME",     1, 0,        0,      NULL, NULL},
    { "5",      120, 300, 46, 26, WHITE, "5",     1, 0,                0,                 0,                 0,   "STAT",     1, 0,        0,      NULL, NULL},
    { "6",      180, 300, 46, 26, WHITE, "6",     1, 0,                0,                 0,                 0,   "UNITS",    1, 0,        0,      NULL, NULL},
    { "MUL",    240, 300, 46, 26, WHITE, 0,       0, mul_width,        mul_height,        mul_bitmap,        0,   "[ ]",      0, "_",      0,      NULL, NULL},

    { "SHR",    0,   350, 36, 26, RIGHT, 0,       0, shr_width,        shr_height,        shr_bitmap,        0,   0,          0, 0,        0,      NULL, NULL},
    { "1",      60,  350, 46, 26, WHITE, "1",     1, 0,                0,                 0,                 0,   "RAD",      0, "POLAR",  0,      NULL, NULL},
    { "2",      120, 350, 46, 26, WHITE, "2",     1, 0,                0,                 0,                 0,   "STACK",    0, "ARG",    0,      NULL, NULL},
    { "3",      180, 350, 46, 26, WHITE, "3",     1, 0,                0,                 0,                 0,   "CMD",      0, "MENU",   0,      NULL, NULL},
    { "MINUS",  240, 350, 46, 26, WHITE, 0,       0, minus_width,      minus_height,      minus_bitmap,      0,   "i",        0, "j",      0,      NULL, NULL},

    { "ON",     0,   400, 36, 26, WHITE, "ON",    0, 0,                0,                 0,                 0,   "CONT",     0, "OFF",    "ATTN", NULL, NULL},
    { "0",      60,  400, 46, 26, WHITE, "0",     1, 0,                0,                 0,                 0,   "= ",       0, " a",     0,      NULL, NULL},
    { "PERIOD", 120, 400, 46, 26, WHITE, ".",     1, 0,                0,                 0,                 0,   ", ",       0, " k",     0,      NULL, NULL},
    { "SPC",    180, 400, 46, 26, WHITE, "SPC",   0, 0,                0,                 0,                 0,   "l ",       0, " m",     0,      NULL, NULL},
    { "PLUS",   240, 400, 46, 26, WHITE, 0,       0, plus_width,       plus_height,       plus_bitmap,       0,   "{ }",      0, ": :",    0,      NULL, NULL},
};

static sdl_button_t buttons_gx[] = {
    {"A",       0,   0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "A", 0,          0, 0,          0,        NULL, NULL},
    { "B",      50,  0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "B", 0,          0, 0,          0,        NULL, NULL},
    { "C",      100, 0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "C", 0,          0, 0,          0,        NULL, NULL},
    { "D",      150, 0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "D", 0,          0, 0,          0,        NULL, NULL},
    { "E",      200, 0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "E", 0,          0, 0,          0,        NULL, NULL},
    { "F",      250, 0,   36, 23, WHITE, 0,       0, menu_label_width, menu_label_height, menu_label_bitmap, "F", 0,          0, 0,          0,        NULL, NULL},

    { "MTH",    0,   50,  36, 26, WHITE, "MTH",   0, 0,                0,                 0,                 "G", "RAD",      0, "POLAR",    0,        NULL, NULL},
    { "PRG",    50,  50,  36, 26, WHITE, "PRG",   0, 0,                0,                 0,                 "H", 0,          0, "CHARS",    0,        NULL, NULL},
    { "CST",    100, 50,  36, 26, WHITE, "CST",   0, 0,                0,                 0,                 "I", 0,          0, "MODES",    0,        NULL, NULL},
    { "VAR",    150, 50,  36, 26, WHITE, "VAR",   0, 0,                0,                 0,                 "J", 0,          0, "MEMORY",   0,        NULL, NULL},
    { "UP",     200, 50,  36, 26, WHITE, 0,       0, up_width,         up_height,         up_bitmap,         "K", 0,          0, "STACK",    0,        NULL, NULL},
    { "NXT",    250, 50,  36, 26, WHITE, "NXT",   0, 0,                0,                 0,                 "L", "PREV",     0, "MENU",     0,        NULL, NULL},

    { "COLON",  0,   100, 36, 26, WHITE, 0,       0, colon_width,      colon_height,      colon_bitmap,      "M", "UP",       0, "HOME",     0,        NULL, NULL},
    { "STO",    50,  100, 36, 26, WHITE, "STO",   0, 0,                0,                 0,                 "N", "DEF",      0, "RCL",      0,        NULL, NULL},
    { "EVAL",   100, 100, 36, 26, WHITE, "EVAL",  0, 0,                0,                 0,                 "O", "aNUM",     0, "UNDO",     0,        NULL, NULL},
    { "LEFT",   150, 100, 36, 26, WHITE, 0,       0, left_width,       left_height,       left_bitmap,       "P", "PICTURE",  0, 0,          0,        NULL, NULL},
    { "DOWN",   200, 100, 36, 26, WHITE, 0,       0, down_width,       down_height,       down_bitmap,       "Q", "VIEW",     0, 0,          0,        NULL, NULL},
    { "RIGHT",  250, 100, 36, 26, WHITE, 0,       0, right_width,      right_height,      right_bitmap,      "R", "SWAP",     0, 0,          0,        NULL, NULL},

    { "SIN",    0,   150, 36, 26, WHITE, "SIN",   0, 0,                0,                 0,                 "S", "ASIN",     0, "b",        0,        NULL, NULL},
    { "COS",    50,  150, 36, 26, WHITE, "COS",   0, 0,                0,                 0,                 "T", "ACOS",     0, "c",        0,        NULL, NULL},
    { "TAN",    100, 150, 36, 26, WHITE, "TAN",   0, 0,                0,                 0,                 "U", "ATAN",     0, "d",        0,        NULL, NULL},
    { "SQRT",   150, 150, 36, 26, WHITE, 0,       0, sqrt_width,       sqrt_height,       sqrt_bitmap,       "V", "n",        0, "o",        0,        NULL, NULL},
    { "POWER",  200, 150, 36, 26, WHITE, 0,       0, power_width,      power_height,      power_bitmap,      "W", "p",        0, "LOG",      0,        NULL, NULL},
    { "INV",    250, 150, 36, 26, WHITE, 0,       0, inv_width,        inv_height,        inv_bitmap,        "X", "q",        0, "LN",       0,        NULL, NULL},

    { "ENTER",  0,   200, 86, 26, WHITE, "ENTER", 2, 0,                0,                 0,                 0,   "EQUATION", 0, "MATRIX",   0,        NULL, NULL},
    { "NEG",    100, 200, 36, 26, WHITE, 0,       0, neg_width,        neg_height,        neg_bitmap,        "Y", "EDIT",     0, "CMD",      0,        NULL, NULL},
    { "EEX",    150, 200, 36, 26, WHITE, "EEX",   0, 0,                0,                 0,                 "Z", "PURG",     0, "ARG",      0,        NULL, NULL},
    { "DEL",    200, 200, 36, 26, WHITE, "DEL",   0, 0,                0,                 0,                 0,   "CLEAR",    0, 0,          0,        NULL, NULL},
    { "BS",     250, 200, 36, 26, WHITE, 0,       0, bs_width,         bs_height,         bs_bitmap,         0,   "DROP",     0, 0,          0,        NULL, NULL},

    { "ALPHA",  0,   250, 36, 26, WHITE, 0,       0, alpha_width,      alpha_height,      alpha_bitmap,      0,   "USER",     0, "ENTRY",    0,        NULL, NULL},
    { "7",      60,  250, 46, 26, WHITE, "7",     1, 0,                0,                 0,                 0,   0,          1, "SOLVE",    0,        NULL, NULL},
    { "8",      120, 250, 46, 26, WHITE, "8",     1, 0,                0,                 0,                 0,   0,          1, "PLOT",     0,        NULL, NULL},
    { "9",      180, 250, 46, 26, WHITE, "9",     1, 0,                0,                 0,                 0,   0,          1, "SYMBOLIC", 0,        NULL, NULL},
    { "DIV",    240, 250, 46, 26, WHITE, 0,       0, div_width,        div_height,        div_bitmap,        0,   "r ",       0, "s",        0,        NULL, NULL},

    { "SHL",    0,   300, 36, 26, LEFT,  0,       0, shl_width,        shl_height,        shl_bitmap,        0,   0,          0, 0,          0,        NULL, NULL},
    { "4",      60,  300, 46, 26, WHITE, "4",     1, 0,                0,                 0,                 0,   0,          1, "TIME",     0,        NULL, NULL},
    { "5",      120, 300, 46, 26, WHITE, "5",     1, 0,                0,                 0,                 0,   0,          1, "STAT",     0,        NULL, NULL},
    { "6",      180, 300, 46, 26, WHITE, "6",     1, 0,                0,                 0,                 0,   0,          1, "UNITS",    0,        NULL, NULL},
    { "MUL",    240, 300, 46, 26, WHITE, 0,       0, mul_width,        mul_height,        mul_bitmap,        0,   "t ",       0, "u",        0,        NULL, NULL},

    { "SHR",    0,   350, 36, 26, RIGHT, 0,       0, shr_width,        shr_height,        shr_bitmap,        0,   0,          1, " ",        0,        NULL, NULL},
    { "1",      60,  350, 46, 26, WHITE, "1",     1, 0,                0,                 0,                 0,   0,          1, "I/O",      0,        NULL, NULL},
    { "2",      120, 350, 46, 26, WHITE, "2",     1, 0,                0,                 0,                 0,   0,          1, "LIBRARY",  0,        NULL, NULL},
    { "3",      180, 350, 46, 26, WHITE, "3",     1, 0,                0,                 0,                 0,   0,          1, "EQ LIB",   0,        NULL, NULL},
    { "MINUS",  240, 350, 46, 26, WHITE, 0,       0, minus_width,      minus_height,      minus_bitmap,      0,   "v ",       0, "w",        0,        NULL, NULL},

    { "ON",     0,   400, 36, 26, WHITE, "ON",    0, 0,                0,                 0,                 0,   "CONT",     0, "OFF",      "CANCEL", NULL, NULL},
    { "0",      60,  400, 46, 26, WHITE, "0",     1, 0,                0,                 0,                 0,   "\004 ",    0, "\003",     0,        NULL, NULL},
    { "PERIOD", 120, 400, 46, 26, WHITE, ".",     1, 0,                0,                 0,                 0,   "\002 ",    0, "\001",     0,        NULL, NULL},
    { "SPC",    180, 400, 46, 26, WHITE, "SPC",   0, 0,                0,                 0,                 0,   "\005 ",    0, "z",        0,        NULL, NULL},
    { "PLUS",   240, 400, 46, 26, WHITE, 0,       0, plus_width,       plus_height,       plus_bitmap,       0,   "x ",       0, "y",        0,        NULL, NULL},
};

/****************************/
/* functions implementation */
/****************************/
/* Create a surface from binary bitmap data */
static SDL_Texture* SDLCreateTextureFromData( unsigned int w, unsigned int h, unsigned char* data, unsigned int coloron,
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

    return SDL_CreateTextureFromSurface( renderer, surf );
}

static inline void sdl2_draw_lcd( void )
{
    short bit;
    int nibble;
    int bit_stop;
    int init_x;

    SDL_Color rgba;
    SDL_Rect rect;
    rect.w = PIXEL_WIDTH;
    rect.h = PIXEL_HEIGHT;
    int xoffset = display_offset_x + 5;
    int yoffset = display_offset_y + 20;

    /* select lcd_texture to draw upon */
    SDL_SetRenderTarget( renderer, lcd_texture );

    SDL_SetRenderDrawColor( renderer, 240, 0, 0, 0xFF ); // bleu foncé
    /* SDL_SetRenderDrawColor( renderer, 48, 68, 90, 0xFF ); // bleu foncé */
    SDL_RenderClear( renderer );

    for ( int nibble_x = 0; nibble_x < NIBBLES_PER_ROW; ++nibble_x ) {
        for ( int y = 0; y < LCD_HEIGHT; ++y ) {
            nibble = lcd_nibbles_buffer[ y ][ nibble_x ];
            nibble &= 0x0f;

            init_x = nibble_x * NIBBLES_NB_BITS;
            bit_stop = ( ( init_x + NIBBLES_NB_BITS >= LCD_WIDTH ) ? LCD_WIDTH - init_x : 4 );

            for ( int bit_x = 0; bit_x < bit_stop; bit_x++ ) {
                bit = nibble & ( 1 << ( bit_x & 3 ) );

                rgba = bit ? sdl_colors[ PIXEL ].color : sdl_colors[ LCD ].color;

                SDL_SetRenderDrawColor( renderer, rgba.r, rgba.g, rgba.b, rgba.a );

                rect.x = xoffset + PIXEL_WIDTH * ( nibble_x + bit_x );
                rect.y = yoffset + PIXEL_HEIGHT * y;
                SDL_RenderFillRect( renderer, &rect );
            }
        }
    }

    /* back to drawing on renderer */
    SDL_SetRenderTarget( renderer, NULL );

    SDL_RenderCopy( renderer, lcd_texture, NULL, &lcd_rectangle );

    SDL_RenderPresent( renderer );
}

static int SDLKeyToHPKey( SDL_Keycode k )
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
        case SDLK_KP_0:
            return HPKEY_0;
            break;
        case SDLK_KP_1:
            return HPKEY_1;
            break;
        case SDLK_KP_2:
            return HPKEY_2;
            break;
        case SDLK_KP_3:
            return HPKEY_3;
            break;
        case SDLK_KP_4:
            return HPKEY_4;
            break;
        case SDLK_KP_5:
            return HPKEY_5;
            break;
        case SDLK_KP_6:
            return HPKEY_6;
            break;
        case SDLK_KP_7:
            return HPKEY_7;
            break;
        case SDLK_KP_8:
            return HPKEY_8;
            break;
        case SDLK_KP_9:
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
        case SDLK_RETURN:
            return HPKEY_ENTER;
            break;
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
        case SDLK_ESCAPE:
            return HPKEY_ON;
            break;
        case SDLK_LSHIFT:
            return HPKEY_SHL;
            break;
        case SDLK_RSHIFT:
            return HPKEY_SHL;
            break;
        case SDLK_LCTRL:
            return HPKEY_SHR;
            break;
        case SDLK_RCTRL:
            return HPKEY_SHR;
            break;
        case SDLK_LALT:
            return HPKEY_ALPHA;
            break;
        case SDLK_RALT:
            return HPKEY_ALPHA;
            break;
        default:
            return -1;
    }

    return -1;
}

static inline void draw_row( long addr, int row )
{
    int nibble;
    int line_length = NIBBLES_PER_ROW;

    if ( ( display.offset > 3 ) && ( row <= display.lines ) )
        line_length += 2;

    for ( int i = 0; i < line_length; i++ ) {
        nibble = read_nibble( addr + i );
        if ( nibble == lcd_nibbles_buffer[ row ][ i ] )
            continue;

        lcd_nibbles_buffer[ row ][ i ] = nibble;
    }
}

static void exit_ui( void )
{
    if ( NULL != lcd_texture )
        SDL_DestroyTexture( lcd_texture );
    if ( NULL != annunciators_texture )
        SDL_DestroyTexture( annunciators_texture );
    if ( NULL != renderer )
        SDL_DestroyRenderer( renderer );
    if ( NULL != window )
        SDL_DestroyWindow( window );
    SDL_Quit();
}

static void SDLCreateColors( void )
{
    for ( int i = WHITE; i < BLACK; i++ )
        ARGBColors[ i ] = 0xff000000 | ( sdl_colors[ i ].color.r << 16 ) | ( sdl_colors[ i ].color.g << 8 ) | sdl_colors[ i ].color.b;

    // Adjust the LCD color according to the contrast
    int contrast, r, g, b;
    contrast = display.contrast;

    if ( contrast < 0x3 )
        contrast = 0x3;
    if ( contrast > 0x13 )
        contrast = 0x13;

    r = ( 0x13 - contrast ) * ( sdl_colors[ LCD ].color.r / 0x10 );
    g = ( 0x13 - contrast ) * ( sdl_colors[ LCD ].color.g / 0x10 );
    b = 128 - ( ( 0x13 - contrast ) * ( ( 128 - sdl_colors[ LCD ].color.b ) / 0x10 ) );
    ARGBColors[ PIXEL ] = 0xff000000 | ( r << 16 ) | ( g << 8 ) | b;
}

static inline void sdl2_init_ui( void )
{
    /* create annunciators textures */
    annunciators_texture =
        SDL_CreateTexture( renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, LCD_WIDTH * PIXEL_WIDTH, ANN_HEIGHT + 4 );
    annunciators_texture_off = SDLCreateTextureFromData( ANN_WIDTH, ANN_HEIGHT, ann_busy_bitmap, ARGBColors[ LCD ], ARGBColors[ LCD ] );
    unsigned char* ann_bitmaps[ NB_ANNUNCIATORS ] = { ann_left_bitmap,    ann_right_bitmap, ann_alpha_bitmap,
                                                      ann_battery_bitmap, ann_busy_bitmap,  ann_io_bitmap };

    for ( int i = 0; i < NB_ANNUNCIATORS; i++ )
        annunciators_textures[ i ] =
            SDLCreateTextureFromData( ANN_WIDTH, ANN_HEIGHT, ann_bitmaps[ i ], ARGBColors[ PIXEL ], ARGBColors[ LCD ] );

    unsigned int width, height;

    // Initialize SDL
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        printf( "Couldn't initialize SDL: %s\n", SDL_GetError() );
        exit( 1 );
    }

    // On exit: clean SDL
    atexit( exit_ui );

    // Initialize the geometric values
    if ( hide_chrome ) {
        width = _DISPLAY_WIDTH;
        height = _DISPLAY_HEIGHT;
        display_offset_x = 0;
        display_offset_y = 0;
    } else {
        width = ( buttons_gx[ LAST_HPKEY ].x + buttons_gx[ LAST_HPKEY ].w ) + 2 * _SIDE_SKIP;
        height =
            display_offset_y + _DISPLAY_HEIGHT + _DISP_KBD_SKIP + buttons_gx[ LAST_HPKEY ].y + buttons_gx[ LAST_HPKEY ].h + _BOTTOM_SKIP;
        display_offset_x = _DISPLAY_OFFSET_X;
        display_offset_y = _DISPLAY_OFFSET_Y;
    }

    uint32_t sdl_window_flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;
    if ( show_ui_fullscreen )
        sdl_window_flags |= SDL_WINDOW_FULLSCREEN;

    window = SDL_CreateWindow( progname, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, sdl_window_flags );
    if ( window == NULL ) {
        printf( "Couldn't create window: %s\n", SDL_GetError() );
        exit( 1 );
    }

    renderer = SDL_CreateRenderer( window, -1, 0 );

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

    /* Select model-correct colors */
    sdl_colors = opt_gx ? sdl_colors_gx : sdl_colors_sx;

    SDLCreateColors();

    /* LCD */
    lcd_rectangle.x = display_offset_x;
    lcd_rectangle.y = display_offset_y;
    lcd_rectangle.w = _DISPLAY_WIDTH;
    lcd_rectangle.h = _DISPLAY_HEIGHT;

    lcd_texture = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, LCD_WIDTH * PIXEL_WIDTH,
                                     LCD_HEIGHT * PIXEL_HEIGHT );

    /* LCD background */
    SDL_Rect rect;

    rect.x = display_offset_x;
    rect.y = display_offset_y;
    rect.w = _DISPLAY_WIDTH;
    rect.h = _DISPLAY_HEIGHT;

    SDL_SetRenderDrawColor( renderer, sdl_colors[ LCD ].color.r, sdl_colors[ LCD ].color.g, sdl_colors[ LCD ].color.b,
                            sdl_colors[ LCD ].color.a );
    SDL_RenderFillRect( renderer, &rect );

    SDL_RenderPresent( renderer );
}

/**********/
/* public */
/**********/

void init_lcd_nibbles_buffer( void )
{
    init_display();

    memset( lcd_nibbles_buffer, 0xf0, sizeof( lcd_nibbles_buffer ) );
}

void update_lcd_nibbles_buffer( void )
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
        memset( lcd_nibbles_buffer, 0xf0, sizeof( lcd_nibbles_buffer ) );

    sdl2_draw_lcd();
}

void disp_draw_nibble( word_20 addr, word_4 val )
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

        if ( val == lcd_nibbles_buffer[ y ][ x ] )
            return;

        lcd_nibbles_buffer[ y ][ x ] = val;
    } else {
        for ( y = 0; y < display.lines; y++ ) {
            if ( val == lcd_nibbles_buffer[ y ][ x ] )
                break;

            lcd_nibbles_buffer[ y ][ x ] = val;
        }
    }
}

void menu_draw_nibble( word_20 addr, word_4 val )
{
    long offset;
    int x, y;

    offset = ( addr - display.menu_start );
    x = offset % NIBBLES_PER_ROW;
    y = display.lines + ( offset / NIBBLES_PER_ROW ) + 1;

    if ( val == lcd_nibbles_buffer[ y ][ x ] )
        return;

    lcd_nibbles_buffer[ y ][ x ] = val;
}

int sdl2_get_event( void )
{
    SDL_Event event;
    int hpkey;

    release_all_keys();

    // Iterate as long as there are events
    // while( SDL_PollEvent( &event ) )
    if ( SDL_PollEvent( &event ) ) {
        switch ( event.type ) {
            case SDL_QUIT:
                exit_emulator();
                exit( 0 );
                break;

            case SDL_KEYDOWN:
                /* case SDL_KEYUP: */
                hpkey = SDLKeyToHPKey( event.key.keysym.sym );

                if ( !keyboard[ hpkey ].pressed )
                    press_key( hpkey );
                break;
        }
    }

    return 1;
}

void sdl2_refresh_LCD( void ) {}

void sdl2_draw_annunc( void )
{
    int val = display.annunc;

    if ( val == last_annunc_state )
        return;

    last_annunc_state = val;

    SDL_Rect srect;
    SDL_Rect drect;
    srect.x = 0;
    srect.y = 0;
    srect.w = ANN_WIDTH;
    srect.h = ANN_HEIGHT;

    drect.y = display_offset_y + 4;
    drect.w = ANN_WIDTH;
    drect.h = ANN_HEIGHT;

    SDL_SetRenderTarget( renderer, annunciators_texture );

    // Print the annunciator
    for ( int i = 0; i < 6; i++ ) {
        drect.x = display_offset_x + ( 16 + ( 45 * i ) );

        if ( ( ( annunciators_bits[ i ] & val ) == annunciators_bits[ i ] ) ? 1 : 0 )
            SDL_RenderCopy( renderer, annunciators_textures[ i ], &srect, &drect );
        else
            SDL_RenderCopy( renderer, annunciators_texture_off, &srect, &drect );
    }

    SDL_SetRenderTarget( renderer, NULL );

    srect.w = _DISPLAY_WIDTH;
    srect.h = ANN_HEIGHT + 4;
    SDL_RenderCopy( renderer, annunciators_texture, NULL, &srect );

    // Always immediately update annunciators
    SDL_RenderPresent( renderer );
}

void sdl2_adjust_contrast()
{
    SDLCreateColors();

    // redraw LCD
    memset( lcd_nibbles_buffer, 0xf0, sizeof( lcd_nibbles_buffer ) );

    // redraw annunc
    last_annunc_state = -1;

    sdl2_draw_annunc();

    sdl2_draw_lcd();
}

void init_sdl_ui( int argc, char** argv )
{
    /* Set public API to this UIs functions */
    ui_disp_draw_nibble = disp_draw_nibble;
    ui_menu_draw_nibble = menu_draw_nibble;
    ui_get_event = sdl2_get_event;
    ui_update_LCD = update_lcd_nibbles_buffer;
    ui_refresh_LCD = sdl2_refresh_LCD;
    ui_adjust_contrast = sdl2_adjust_contrast;
    ui_draw_annunc = sdl2_draw_annunc;
    ui_init_LCD = init_lcd_nibbles_buffer;

    init_lcd_nibbles_buffer();

    sdl2_init_ui();
}
