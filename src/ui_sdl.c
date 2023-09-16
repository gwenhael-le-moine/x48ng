#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>

#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h> /* lineColor(); pixelColor(); rectangleColor();stringColor(); */

#include "runtime_options.h"
#include "emulator.h"
#include "romio.h"
#include "ui.h"
#include "ui_bitmaps.h"

#define ANN_LEFT 0x81
#define ANN_RIGHT 0x82
#define ANN_ALPHA 0x84
#define ANN_BATTERY 0x88
#define ANN_BUSY 0x90
#define ANN_IO 0xa0

// Colors
#define WHITE 0
#define LEFT 1
#define RIGHT 2
#define BUT_TOP 3
#define BUTTON 4
#define BUT_BOT 5
#define LCD 6
#define PIXEL 7
#define PAD_TOP 8
#define PAD 9
#define PAD_BOT 10
#define DISP_PAD_TOP 11
#define DISP_PAD 12
#define DISP_PAD_BOT 13
#define LOGO 14
#define LOGO_BACK 15
#define LABEL 16
#define FRAME 17
#define UNDERLAY 18
#define BLACK 19

// Buttons
#define BUTTON_A 0
#define BUTTON_B 1
#define BUTTON_C 2
#define BUTTON_D 3
#define BUTTON_E 4
#define BUTTON_F 5

#define BUTTON_MTH 6
#define BUTTON_PRG 7
#define BUTTON_CST 8
#define BUTTON_VAR 9
#define BUTTON_UP 10
#define BUTTON_NXT 11

#define BUTTON_COLON 12
#define BUTTON_STO 13
#define BUTTON_EVAL 14
#define BUTTON_LEFT 15
#define BUTTON_DOWN 16
#define BUTTON_RIGHT 17

#define BUTTON_SIN 18
#define BUTTON_COS 19
#define BUTTON_TAN 20
#define BUTTON_SQRT 21
#define BUTTON_POWER 22
#define BUTTON_INV 23

#define BUTTON_ENTER 24
#define BUTTON_NEG 25
#define BUTTON_EEX 26
#define BUTTON_DEL 27
#define BUTTON_BS 28

#define BUTTON_ALPHA 29
#define BUTTON_7 30
#define BUTTON_8 31
#define BUTTON_9 32
#define BUTTON_DIV 33

#define BUTTON_SHL 34
#define BUTTON_4 35
#define BUTTON_5 36
#define BUTTON_6 37
#define BUTTON_MUL 38

#define BUTTON_SHR 39
#define BUTTON_1 40
#define BUTTON_2 41
#define BUTTON_3 42
#define BUTTON_MINUS 43

#define BUTTON_ON 44
#define BUTTON_0 45
#define BUTTON_PERIOD 46
#define BUTTON_SPC 47
#define BUTTON_PLUS 48

#define LAST_BUTTON 48

#define DISP_ROWS 64
#define NIBS_PER_BUFFER_ROW ( NIBBLES_PER_ROW + 2 )

#define UPDATE_MENU 1
#define UPDATE_DISP 2

#define _KEYBOARD_HEIGHT                                                       \
    ( buttons_gx[ LAST_BUTTON ].y + buttons_gx[ LAST_BUTTON ].h )
#define _KEYBOARD_WIDTH                                                        \
    ( buttons_gx[ LAST_BUTTON ].x + buttons_gx[ LAST_BUTTON ].w )

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
typedef struct letter_t {
    unsigned int w, h;
    unsigned char* bits;
} letter_t;

typedef struct color_t {
    const char* name;
    int r, g, b;
} color_t;

typedef struct keypad_t {
    unsigned int width;
    unsigned int height;
} keypad_t;

typedef struct button_t {
    const char* name;
    short pressed;
    short extra;

    int code;
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
} button_t;

// This mimicks the structure formerly lcd.c, except with SDL surfaces instead
// of Pixmaps.
typedef struct ann_struct {
    int bit;
    int x;
    int y;
    unsigned int width;
    unsigned int height;
    unsigned char* bits;

    SDL_Surface* surfaceon;
    SDL_Surface* surfaceoff;
} ann_struct_t;

typedef struct SDLWINDOW {
    SDL_Surface *oldsurf, *surf;
    int x, y;
} SDLWINDOW_t;

typedef struct sdltohpkeymap_t {
    SDLKey sdlkey;
    int hpkey;
} sdltohpkeymap_t;

/*************/
/* variables */
/*************/
disp_t disp;

keypad_t keypad;
color_t* colors;

color_t colors_sx[] = { { "white", 255, 255, 255 },
                        { "left", 255, 166, 0 },
                        { "right", 0, 210, 255 },
                        { "but_top", 109, 93, 93 },
                        { "button", 90, 77, 77 },
                        { "but_bot", 76, 65, 65 },
                        { "lcd_col", 202, 221, 92 },
                        { "pix_col", 0, 0, 128 },
                        { "pad_top", 109, 78, 78 },
                        { "pad", 90, 64, 64 },
                        { "pad_bot", 76, 54, 54 },
                        { "disp_pad_top", 155, 118, 84 },
                        { "disp_pad", 124, 94, 67 },
                        { "disp_pad_bot", 100, 75, 53 },
                        { "logo", 204, 169, 107 },
                        { "logo_back", 64, 64, 64 },
                        { "label", 202, 184, 144 },
                        { "frame", 0, 0, 0 },
                        { "underlay", 60, 42, 42 },
                        { "black", 0, 0, 0 },
                        { 0 } };

color_t colors_gx[] = { { "white", 255, 255, 255 },
                        { "left", 255, 186, 255 },
                        { "right", 0, 255, 204 },
                        { "but_top", 104, 104, 104 },
                        { "button", 88, 88, 88 },
                        { "but_bot", 74, 74, 74 },
                        { "lcd_col", 202, 221, 92 },
                        { "pix_col", 0, 0, 128 },
                        { "pad_top", 88, 88, 88 },
                        { "pad", 74, 74, 74 },
                        { "pad_bot", 64, 64, 64 },
                        { "disp_pad_top", 128, 128, 138 },
                        { "disp_pad", 104, 104, 110 },
                        { "disp_pad_bot", 84, 84, 90 },
                        { "logo", 176, 176, 184 },
                        { "logo_back", 104, 104, 110 },
                        { "label", 240, 240, 240 },
                        { "frame", 0, 0, 0 },
                        { "underlay", 104, 104, 110 },
                        { "black", 0, 0, 0 },
                        { 0 } };

letter_t small_font[] = {
    { 0, 0, 0 },
    { nl_gx_width, nl_gx_height, nl_gx_bitmap },          /* \001 == \n gx */
    { comma_gx_width, comma_gx_height, comma_gx_bitmap }, /* \002 == comma gx */
    { arrow_gx_width, arrow_gx_height, arrow_gx_bitmap }, /* \003 == \-> gx */
    { equal_gx_width, equal_gx_height, equal_gx_bitmap }, /* \004 == equal gx */
    { pi_gx_width, pi_gx_height, pi_gx_bitmap },          /* \005 == pi gx */
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 }, /* # 16 */
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { blank_width, blank_height, blank_bitmap }, /* # 32 */
    { 0, 0, 0 },
    { 0, 0, 0 },
    { hash_width, hash_height, hash_bitmap },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { lbrace_width, lbrace_height, lbrace_bitmap },
    { rbrace_width, rbrace_height, rbrace_bitmap },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { comma_width, comma_height, comma_bitmap },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { slash_width, slash_height, slash_bitmap },
    { 0, 0, 0 }, /* # 48 */
    { 0, 0, 0 },
    { two_width, two_height, two_bitmap },
    { three_width, three_height, three_bitmap },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { small_colon_width, small_colon_height, small_colon_bitmap },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { equal_width, equal_height, equal_bitmap },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 }, /* # 64 */
    { A_width, A_height, A_bitmap },
    { B_width, B_height, B_bitmap },
    { C_width, C_height, C_bitmap },
    { D_width, D_height, D_bitmap },
    { E_width, E_height, E_bitmap },
    { F_width, F_height, F_bitmap },
    { G_width, G_height, G_bitmap },
    { H_width, H_height, H_bitmap },
    { I_width, I_height, I_bitmap },
    { J_width, J_height, J_bitmap },
    { K_width, K_height, K_bitmap },
    { L_width, L_height, L_bitmap },
    { M_width, M_height, M_bitmap },
    { N_width, N_height, N_bitmap },
    { O_width, O_height, O_bitmap },
    { P_width, P_height, P_bitmap }, /* # 80 */
    { Q_width, Q_height, Q_bitmap },
    { R_width, R_height, R_bitmap },
    { S_width, S_height, S_bitmap },
    { T_width, T_height, T_bitmap },
    { U_width, U_height, U_bitmap },
    { V_width, V_height, V_bitmap },
    { W_width, W_height, W_bitmap },
    { X_width, X_height, X_bitmap },
    { Y_width, Y_height, Y_bitmap },
    { Z_width, Z_height, Z_bitmap },
    { lbracket_width, lbracket_height, lbracket_bitmap },
    { 0, 0, 0 },
    { rbracket_width, rbracket_height, rbracket_bitmap },
    { 0, 0, 0 },
    { under_width, under_height, under_bitmap },
    { 0, 0, 0 },                                 /* # 96 */
    { arrow_width, arrow_height, arrow_bitmap }, /* a == left arrow   */
    { diff_width, diff_height, diff_bitmap },    /* b == differential */
    { integral_width, integral_height, integral_bitmap },    /* c == integral */
    { sigma_width, sigma_height, sigma_bitmap },             /* d == sigma */
    { sqr_width, sqr_height, sqr_bitmap },                   /* e == sqr */
    { root_width, root_height, root_bitmap },                /* f == root */
    { pow10_width, pow10_height, pow10_bitmap },             /* g == pow10 */
    { exp_width, exp_height, exp_bitmap },                   /* h == exp */
    { prog_width, prog_height, prog_bitmap },                /* i == << >> */
    { string_width, string_height, string_bitmap },          /* j == " " */
    { nl_width, nl_height, nl_bitmap },                      /* k == New Line */
    { pi_width, pi_height, pi_bitmap },                      /* l == pi */
    { angle_width, angle_height, angle_bitmap },             /* m == angle */
    { sqr_gx_width, sqr_gx_height, sqr_gx_bitmap },          /* n == sqr gx */
    { root_gx_width, root_gx_height, root_gx_bitmap },       /* o == root gx */
    { pow10_gx_width, pow10_gx_height, pow10_gx_bitmap },    /* p == pow10 gx */
    { exp_gx_width, exp_gx_height, exp_gx_bitmap },          /* q == exp gx */
    { parens_gx_width, parens_gx_height, parens_gx_bitmap }, /* r == ( ) gx */
    { hash_gx_width, hash_gx_height, hash_gx_bitmap },       /* s == # gx */
    { bracket_gx_width, bracket_gx_height, bracket_gx_bitmap }, /* t == [] gx */
    { under_gx_width, under_gx_height, under_gx_bitmap },       /* u == _ gx */
    { prog_gx_width, prog_gx_height, prog_gx_bitmap },    /* v == << >> gx */
    { quote_gx_width, quote_gx_height, quote_gx_bitmap }, /* w == " " gx */
    { curly_gx_width, curly_gx_height, curly_gx_bitmap }, /* x == {} gx */
    { colon_gx_width, colon_gx_height, colon_gx_bitmap }, /* y == :: gx */
    { angle_gx_width, angle_gx_height, angle_gx_bitmap }, /* z == angle gx */
    { lcurly_width, lcurly_height, lcurly_bitmap },
    { 0, 0, 0 },
    { rcurly_width, rcurly_height, rcurly_bitmap },
    { 0, 0, 0 },
    { 0, 0, 0 } };

// This will take the value of the defines, but can be run-time modified
unsigned KEYBOARD_HEIGHT, KEYBOARD_WIDTH, TOP_SKIP, SIDE_SKIP, BOTTOM_SKIP,
    DISP_KBD_SKIP, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_OFFSET_X,
    DISPLAY_OFFSET_Y, DISP_FRAME, KEYBOARD_OFFSET_X, KEYBOARD_OFFSET_Y,
    KBD_UPLINE;

unsigned int ARGBColors[ BLACK + 1 ];

button_t* buttons = 0;

button_t buttons_sx[] = {
    { "A",
      0,
      0,
      0x14,
      0,
      0,
      36,
      23,
      WHITE,
      0,
      0,
      menu_label_width,
      menu_label_height,
      menu_label_bitmap,
      "A",
      0,
      0,
      0,
      0,
      0,
      0 },
    { "B",
      0,
      0,
      0x84,
      50,
      0,
      36,
      23,
      WHITE,
      0,
      0,
      menu_label_width,
      menu_label_height,
      menu_label_bitmap,
      "B",
      0,
      0,
      0,
      0,
      0,
      0 },
    { "C",
      0,
      0,
      0x83,
      100,
      0,
      36,
      23,
      WHITE,
      0,
      0,
      menu_label_width,
      menu_label_height,
      menu_label_bitmap,
      "C",
      0,
      0,
      0,
      0,
      0,
      0 },
    { "D",
      0,
      0,
      0x82,
      150,
      0,
      36,
      23,
      WHITE,
      0,
      0,
      menu_label_width,
      menu_label_height,
      menu_label_bitmap,
      "D",
      0,
      0,
      0,
      0,
      0,
      0 },
    { "E",
      0,
      0,
      0x81,
      200,
      0,
      36,
      23,
      WHITE,
      0,
      0,
      menu_label_width,
      menu_label_height,
      menu_label_bitmap,
      "E",
      0,
      0,
      0,
      0,
      0,
      0 },
    { "F",
      0,
      0,
      0x80,
      250,
      0,
      36,
      23,
      WHITE,
      0,
      0,
      menu_label_width,
      menu_label_height,
      menu_label_bitmap,
      "F",
      0,
      0,
      0,
      0,
      0,
      0 },

    { "MTH", 0, 0, 0x24, 0,       50, 36, 26, WHITE, "MTH", 0,
      0,     0, 0, "G",  "PRINT", 1,  0,  0,  0,     0 },
    { "PRG", 0, 0, 0x74, 50,    50, 36, 26, WHITE, "PRG", 0,
      0,     0, 0, "H",  "I/O", 1,  0,  0,  0,     0 },
    { "CST", 0, 0, 0x73, 100,     50, 36, 26, WHITE, "CST", 0,
      0,     0, 0, "I",  "MODES", 1,  0,  0,  0,     0 },
    { "VAR", 0, 0, 0x72, 150,      50, 36, 26, WHITE, "VAR", 0,
      0,     0, 0, "J",  "MEMORY", 1,  0,  0,  0,     0 },
    { "UP",     0,         0,         0x71, 200,       50, 36, 26, WHITE, 0, 0,
      up_width, up_height, up_bitmap, "K",  "LIBRARY", 1,  0,  0,  0,     0 },
    { "NXT", 0, 0, 0x70, 250,    50, 36, 26, WHITE, "NXT", 0,
      0,     0, 0, "L",  "PREV", 0,  0,  0,  0,     0 },

    { "COLON",
      0,
      0,
      0x04,
      0,
      100,
      36,
      26,
      WHITE,
      0,
      0,
      colon_width,
      colon_height,
      colon_bitmap,
      "M",
      "UP",
      0,
      "HOME",
      0,
      0,
      0 },
    { "STO", 0, 0, 0x64, 50,    100, 36,    26, WHITE, "STO", 0,
      0,     0, 0, "N",  "DEF", 0,   "RCL", 0,  0,     0 },
    { "EVAL", 0, 0, 0x63, 100,  100, 36,     26, WHITE, "EVAL", 0,
      0,      0, 0, "O",  "aQ", 0,   "aNUM", 0,  0,     0 },
    { "LEFT", 0,       0, 0x62, 150,        100,         36,
      26,     WHITE,   0, 0,    left_width, left_height, left_bitmap,
      "P",    "GRAPH", 0, 0,    0,          0,           0 },
    { "DOWN", 0,        0, 0x61, 200,        100,         36,
      26,     WHITE,    0, 0,    down_width, down_height, down_bitmap,
      "Q",    "REVIEW", 0, 0,    0,          0,           0 },
    { "RIGHT",
      0,
      0,
      0x60,
      250,
      100,
      36,
      26,
      WHITE,
      0,
      0,
      right_width,
      right_height,
      right_bitmap,
      "R",
      "SWAP",
      0,
      0,
      0,
      0,
      0 },

    { "SIN", 0, 0, 0x34, 0,      150, 36,  26, WHITE, "SIN", 0,
      0,     0, 0, "S",  "ASIN", 0,   "b", 0,  0,     0 },
    { "COS", 0, 0, 0x54, 50,     150, 36,  26, WHITE, "COS", 0,
      0,     0, 0, "T",  "ACOS", 0,   "c", 0,  0,     0 },
    { "TAN", 0, 0, 0x53, 100,    150, 36,  26, WHITE, "TAN", 0,
      0,     0, 0, "U",  "ATAN", 0,   "d", 0,  0,     0 },
    { "SQRT", 0,     0, 0x52, 150,        150,         36,
      26,     WHITE, 0, 0,    sqrt_width, sqrt_height, sqrt_bitmap,
      "V",    "e",   0, "f",  0,          0,           0 },
    { "POWER",
      0,
      0,
      0x51,
      200,
      150,
      36,
      26,
      WHITE,
      0,
      0,
      power_width,
      power_height,
      power_bitmap,
      "W",
      "g",
      0,
      "LOG",
      0,
      0,
      0 },
    { "INV",     0,          0,          0x50, 250, 150, 36,   26, WHITE, 0, 0,
      inv_width, inv_height, inv_bitmap, "X",  "h", 0,   "LN", 0,  0,     0 },

    { "ENTER", 0, 0, 0x44, 0,          200, 86,       26, WHITE, "ENTER", 2,
      0,       0, 0, 0,    "EQUATION", 0,   "MATRIX", 0,  0,     0 },
    { "NEG", 0,      0, 0x43,    100,       200,        36,
      26,    WHITE,  0, 0,       neg_width, neg_height, neg_bitmap,
      "Y",   "EDIT", 0, "VISIT", 0,         0,          0 },
    { "EEX", 0, 0, 0x42, 150,  200, 36,   26, WHITE, "EEX", 0,
      0,     0, 0, "Z",  "2D", 0,   "3D", 0,  0,     0 },
    { "DEL", 0, 0, 0x41, 200,     200, 36, 26, WHITE, "DEL", 0,
      0,     0, 0, 0,    "PURGE", 0,   0,  0,  0,     0 },
    { "BS",     0,         0,         0x40, 250,    200, 36,    26, WHITE, 0, 0,
      bs_width, bs_height, bs_bitmap, 0,    "DROP", 0,   "CLR", 0,  0,     0 },

    { "ALPHA",
      0,
      0,
      0x35,
      0,
      250,
      36,
      26,
      WHITE,
      0,
      0,
      alpha_width,
      alpha_height,
      alpha_bitmap,
      0,
      "USR",
      0,
      "ENTRY",
      0,
      0,
      0 },
    { "7", 0, 0, 0x33, 60,      250, 46, 26, WHITE, "7", 1,
      0,   0, 0, 0,    "SOLVE", 1,   0,  0,  0,     0 },
    { "8", 0, 0, 0x32, 120,    250, 46, 26, WHITE, "8", 1,
      0,   0, 0, 0,    "PLOT", 1,   0,  0,  0,     0 },
    { "9", 0, 0, 0x31, 180,       250, 46, 26, WHITE, "9", 1,
      0,   0, 0, 0,    "ALGEBRA", 1,   0,  0,  0,     0 },
    { "DIV",     0,          0,          0x30, 240,   250, 46,  26, WHITE, 0, 0,
      div_width, div_height, div_bitmap, 0,    "( )", 0,   "#", 0,  0,     0 },

    { "SHL",     0,          0,          0x25, 0, 300, 36, 26, LEFT, 0, 0,
      shl_width, shl_height, shl_bitmap, 0,    0, 0,   0,  0,  0,    0 },
    { "4", 0, 0, 0x23, 60,     300, 46, 26, WHITE, "4", 1,
      0,   0, 0, 0,    "TIME", 1,   0,  0,  0,     0 },
    { "5", 0, 0, 0x22, 120,    300, 46, 26, WHITE, "5", 1,
      0,   0, 0, 0,    "STAT", 1,   0,  0,  0,     0 },
    { "6", 0, 0, 0x21, 180,     300, 46, 26, WHITE, "6", 1,
      0,   0, 0, 0,    "UNITS", 1,   0,  0,  0,     0 },
    { "MUL",     0,          0,          0x20, 240,   300, 46,  26, WHITE, 0, 0,
      mul_width, mul_height, mul_bitmap, 0,    "[ ]", 0,   "_", 0,  0,     0 },

    { "SHR",     0,          0,          0x15, 0, 350, 36, 26, RIGHT, 0, 0,
      shr_width, shr_height, shr_bitmap, 0,    0, 0,   0,  0,  0,     0 },
    { "1", 0, 0, 0x13, 60,    350, 46,      26, WHITE, "1", 1,
      0,   0, 0, 0,    "RAD", 0,   "POLAR", 0,  0,     0 },
    { "2", 0, 0, 0x12, 120,     350, 46,    26, WHITE, "2", 1,
      0,   0, 0, 0,    "STACK", 0,   "ARG", 0,  0,     0 },
    { "3", 0, 0, 0x11, 180,   350, 46,     26, WHITE, "3", 1,
      0,   0, 0, 0,    "CMD", 0,   "MENU", 0,  0,     0 },
    { "MINUS",
      0,
      0,
      0x10,
      240,
      350,
      46,
      26,
      WHITE,
      0,
      0,
      minus_width,
      minus_height,
      minus_bitmap,
      0,
      "i",
      0,
      "j",
      0,
      0,
      0 },

    { "ON", 0, 0, 0x8000, 0,      400, 36,    26,     WHITE, "ON", 0,
      0,    0, 0, 0,      "CONT", 0,   "OFF", "ATTN", 0,     0 },
    { "0", 0, 0, 0x03, 60,   400, 46,   26, WHITE, "0", 1,
      0,   0, 0, 0,    "= ", 0,   " a", 0,  0,     0 },
    { "PERIOD", 0, 0, 0x02, 120,  400, 46,   26, WHITE, ".", 1,
      0,        0, 0, 0,    ", ", 0,   " k", 0,  0,     0 },
    { "SPC", 0, 0, 0x01, 180,  400, 46,   26, WHITE, "SPC", 0,
      0,     0, 0, 0,    "l ", 0,   " m", 0,  0,     0 },
    { "PLUS", 0,     0, 0x00,  240,        400,         46,
      26,     WHITE, 0, 0,     plus_width, plus_height, plus_bitmap,
      0,      "{ }", 0, ": :", 0,          0,           0 },

    { 0 } };

button_t buttons_gx[] = {
    { "A",
      0,
      0,
      0x14,
      0,
      0,
      36,
      23,
      WHITE,
      0,
      0,
      menu_label_width,
      menu_label_height,
      menu_label_bitmap,
      "A",
      0,
      0,
      0,
      0,
      0,
      0 },
    { "B",
      0,
      0,
      0x84,
      50,
      0,
      36,
      23,
      WHITE,
      0,
      0,
      menu_label_width,
      menu_label_height,
      menu_label_bitmap,
      "B",
      0,
      0,
      0,
      0,
      0,
      0 },
    { "C",
      0,
      0,
      0x83,
      100,
      0,
      36,
      23,
      WHITE,
      0,
      0,
      menu_label_width,
      menu_label_height,
      menu_label_bitmap,
      "C",
      0,
      0,
      0,
      0,
      0,
      0 },
    { "D",
      0,
      0,
      0x82,
      150,
      0,
      36,
      23,
      WHITE,
      0,
      0,
      menu_label_width,
      menu_label_height,
      menu_label_bitmap,
      "D",
      0,
      0,
      0,
      0,
      0,
      0 },
    { "E",
      0,
      0,
      0x81,
      200,
      0,
      36,
      23,
      WHITE,
      0,
      0,
      menu_label_width,
      menu_label_height,
      menu_label_bitmap,
      "E",
      0,
      0,
      0,
      0,
      0,
      0 },
    { "F",
      0,
      0,
      0x80,
      250,
      0,
      36,
      23,
      WHITE,
      0,
      0,
      menu_label_width,
      menu_label_height,
      menu_label_bitmap,
      "F",
      0,
      0,
      0,
      0,
      0,
      0 },

    { "MTH", 0, 0, 0x24, 0,     50, 36,      26, WHITE, "MTH", 0,
      0,     0, 0, "G",  "RAD", 0,  "POLAR", 0,  0,     0 },
    { "PRG", 0, 0, 0x74, 50, 50, 36,      26, WHITE, "PRG", 0,
      0,     0, 0, "H",  0,  0,  "CHARS", 0,  0,     0 },
    { "CST", 0, 0, 0x73, 100, 50, 36,      26, WHITE, "CST", 0,
      0,     0, 0, "I",  0,   0,  "MODES", 0,  0,     0 },
    { "VAR", 0, 0, 0x72, 150, 50, 36,       26, WHITE, "VAR", 0,
      0,     0, 0, "J",  0,   0,  "MEMORY", 0,  0,     0 },
    { "UP",     0,         0,         0x71, 200, 50, 36,      26, WHITE, 0, 0,
      up_width, up_height, up_bitmap, "K",  0,   0,  "STACK", 0,  0,     0 },
    { "NXT", 0, 0, 0x70, 250,    50, 36,     26, WHITE, "NXT", 0,
      0,     0, 0, "L",  "PREV", 0,  "MENU", 0,  0,     0 },

    { "COLON",
      0,
      0,
      0x04,
      0,
      100,
      36,
      26,
      WHITE,
      0,
      0,
      colon_width,
      colon_height,
      colon_bitmap,
      "M",
      "UP",
      0,
      "HOME",
      0,
      0,
      0 },
    { "STO", 0, 0, 0x64, 50,    100, 36,    26, WHITE, "STO", 0,
      0,     0, 0, "N",  "DEF", 0,   "RCL", 0,  0,     0 },
    { "EVAL", 0, 0, 0x63, 100,    100, 36,     26, WHITE, "EVAL", 0,
      0,      0, 0, "O",  "aNUM", 0,   "UNDO", 0,  0,     0 },
    { "LEFT", 0,         0, 0x62, 150,        100,         36,
      26,     WHITE,     0, 0,    left_width, left_height, left_bitmap,
      "P",    "PICTURE", 0, 0,    0,          0,           0 },
    { "DOWN", 0,      0, 0x61, 200,        100,         36,
      26,     WHITE,  0, 0,    down_width, down_height, down_bitmap,
      "Q",    "VIEW", 0, 0,    0,          0,           0 },
    { "RIGHT",
      0,
      0,
      0x60,
      250,
      100,
      36,
      26,
      WHITE,
      0,
      0,
      right_width,
      right_height,
      right_bitmap,
      "R",
      "SWAP",
      0,
      0,
      0,
      0,
      0 },

    { "SIN", 0, 0, 0x34, 0,      150, 36,  26, WHITE, "SIN", 0,
      0,     0, 0, "S",  "ASIN", 0,   "b", 0,  0,     0 },
    { "COS", 0, 0, 0x54, 50,     150, 36,  26, WHITE, "COS", 0,
      0,     0, 0, "T",  "ACOS", 0,   "c", 0,  0,     0 },
    { "TAN", 0, 0, 0x53, 100,    150, 36,  26, WHITE, "TAN", 0,
      0,     0, 0, "U",  "ATAN", 0,   "d", 0,  0,     0 },
    { "SQRT", 0,     0, 0x52, 150,        150,         36,
      26,     WHITE, 0, 0,    sqrt_width, sqrt_height, sqrt_bitmap,
      "V",    "n",   0, "o",  0,          0,           0 },
    { "POWER",
      0,
      0,
      0x51,
      200,
      150,
      36,
      26,
      WHITE,
      0,
      0,
      power_width,
      power_height,
      power_bitmap,
      "W",
      "p",
      0,
      "LOG",
      0,
      0,
      0 },
    { "INV",     0,          0,          0x50, 250, 150, 36,   26, WHITE, 0, 0,
      inv_width, inv_height, inv_bitmap, "X",  "q", 0,   "LN", 0,  0,     0 },

    { "ENTER", 0, 0, 0x44, 0,          200, 86,       26, WHITE, "ENTER", 2,
      0,       0, 0, 0,    "EQUATION", 0,   "MATRIX", 0,  0,     0 },
    { "NEG", 0,      0, 0x43,  100,       200,        36,
      26,    WHITE,  0, 0,     neg_width, neg_height, neg_bitmap,
      "Y",   "EDIT", 0, "CMD", 0,         0,          0 },
    { "EEX", 0, 0, 0x42, 150,    200, 36,    26, WHITE, "EEX", 0,
      0,     0, 0, "Z",  "PURG", 0,   "ARG", 0,  0,     0 },
    { "DEL", 0, 0, 0x41, 200,     200, 36, 26, WHITE, "DEL", 0,
      0,     0, 0, 0,    "CLEAR", 0,   0,  0,  0,     0 },
    { "BS",     0,         0,         0x40, 250,    200, 36, 26, WHITE, 0, 0,
      bs_width, bs_height, bs_bitmap, 0,    "DROP", 0,   0,  0,  0,     0 },

    { "ALPHA",
      0,
      0,
      0x35,
      0,
      250,
      36,
      26,
      WHITE,
      0,
      0,
      alpha_width,
      alpha_height,
      alpha_bitmap,
      0,
      "USER",
      0,
      "ENTRY",
      0,
      0,
      0 },
    { "7", 0, 0, 0x33, 60, 250, 46,      26, WHITE, "7", 1,
      0,   0, 0, 0,    0,  1,   "SOLVE", 0,  0,     0 },
    { "8", 0, 0, 0x32, 120, 250, 46,     26, WHITE, "8", 1,
      0,   0, 0, 0,    0,   1,   "PLOT", 0,  0,     0 },
    { "9", 0, 0, 0x31, 180, 250, 46,         26, WHITE, "9", 1,
      0,   0, 0, 0,    0,   1,   "SYMBOLIC", 0,  0,     0 },
    { "DIV",     0,          0,          0x30, 240,  250, 46,  26, WHITE, 0, 0,
      div_width, div_height, div_bitmap, 0,    "r ", 0,   "s", 0,  0,     0 },

    { "SHL",     0,          0,          0x25, 0, 300, 36, 26, LEFT, 0, 0,
      shl_width, shl_height, shl_bitmap, 0,    0, 0,   0,  0,  0,    0 },
    { "4", 0, 0, 0x23, 60, 300, 46,     26, WHITE, "4", 1,
      0,   0, 0, 0,    0,  1,   "TIME", 0,  0,     0 },
    { "5", 0, 0, 0x22, 120, 300, 46,     26, WHITE, "5", 1,
      0,   0, 0, 0,    0,   1,   "STAT", 0,  0,     0 },
    { "6", 0, 0, 0x21, 180, 300, 46,      26, WHITE, "6", 1,
      0,   0, 0, 0,    0,   1,   "UNITS", 0,  0,     0 },
    { "MUL",     0,          0,          0x20, 240,  300, 46,  26, WHITE, 0, 0,
      mul_width, mul_height, mul_bitmap, 0,    "t ", 0,   "u", 0,  0,     0 },

    { "SHR",     0,          0,          0x15, 0, 350, 36,  26, RIGHT, 0, 0,
      shr_width, shr_height, shr_bitmap, 0,    0, 1,   " ", 0,  0,     0 },
    { "1", 0, 0, 0x13, 60, 350, 46,    26, WHITE, "1", 1,
      0,   0, 0, 0,    0,  1,   "I/O", 0,  0,     0 },
    { "2", 0, 0, 0x12, 120, 350, 46,        26, WHITE, "2", 1,
      0,   0, 0, 0,    0,   1,   "LIBRARY", 0,  0,     0 },
    { "3", 0, 0, 0x11, 180, 350, 46,       26, WHITE, "3", 1,
      0,   0, 0, 0,    0,   1,   "EQ LIB", 0,  0,     0 },
    { "MINUS",
      0,
      0,
      0x10,
      240,
      350,
      46,
      26,
      WHITE,
      0,
      0,
      minus_width,
      minus_height,
      minus_bitmap,
      0,
      "v ",
      0,
      "w",
      0,
      0,
      0 },

    { "ON", 0, 0, 0x8000, 0,      400, 36,    26,       WHITE, "ON", 0,
      0,    0, 0, 0,      "CONT", 0,   "OFF", "CANCEL", 0,     0 },
    { "0", 0, 0, 0x03, 60,      400, 46,     26, WHITE, "0", 1,
      0,   0, 0, 0,    "\004 ", 0,   "\003", 0,  0,     0 },
    { "PERIOD", 0, 0, 0x02, 120,     400, 46,     26, WHITE, ".", 1,
      0,        0, 0, 0,    "\002 ", 0,   "\001", 0,  0,     0 },
    { "SPC", 0, 0, 0x01, 180,     400, 46,  26, WHITE, "SPC", 0,
      0,     0, 0, 0,    "\005 ", 0,   "z", 0,  0,     0 },
    { "PLUS", 0,     0, 0x00, 240,        400,         46,
      26,     WHITE, 0, 0,    plus_width, plus_height, plus_bitmap,
      0,      "x ",  0, "y",  0,          0,           0 },

    { 0 } };

/* x48_lcd.c */
static int last_annunc_state = -1;

display_t display;

unsigned char disp_buf[ DISP_ROWS ][ NIBS_PER_BUFFER_ROW ];
unsigned char lcd_buffer[ DISP_ROWS ][ NIBS_PER_BUFFER_ROW ];

ann_struct_t ann_tbl[] = {
    { ANN_LEFT, 16, 4, ann_left_width, ann_left_height, ann_left_bitmap, 0, 0 },
    { ANN_RIGHT, 61, 4, ann_right_width, ann_right_height, ann_right_bitmap, 0,
      0 },
    { ANN_ALPHA, 106, 4, ann_alpha_width, ann_alpha_height, ann_alpha_bitmap, 0,
      0 },
    { ANN_BATTERY, 151, 4, ann_battery_width, ann_battery_height,
      ann_battery_bitmap, 0, 0 },
    { ANN_BUSY, 196, 4, ann_busy_width, ann_busy_height, ann_busy_bitmap, 0,
      0 },
    { ANN_IO, 241, 4, ann_io_width, ann_io_height, ann_io_bitmap, 0, 0 },
    { 0 } };
/* \ x48_lcd.c */

sdltohpkeymap_t sdltohpkeymap[] = {
    // Numbers
    { SDLK_0, BUTTON_0 },
    { SDLK_1, BUTTON_1 },
    { SDLK_2, BUTTON_2 },
    { SDLK_3, BUTTON_3 },
    { SDLK_4, BUTTON_4 },
    { SDLK_5, BUTTON_5 },
    { SDLK_6, BUTTON_6 },
    { SDLK_7, BUTTON_7 },
    { SDLK_8, BUTTON_8 },
    { SDLK_9, BUTTON_9 },
    { SDLK_KP0, BUTTON_0 },
    { SDLK_KP1, BUTTON_1 },
    { SDLK_KP2, BUTTON_2 },
    { SDLK_KP3, BUTTON_3 },
    { SDLK_KP4, BUTTON_4 },
    { SDLK_KP5, BUTTON_5 },
    { SDLK_KP6, BUTTON_6 },
    { SDLK_KP7, BUTTON_7 },
    { SDLK_KP8, BUTTON_8 },
    { SDLK_KP9, BUTTON_9 },
    // Letters, space, return, backspace, delete, period, arrows
    { SDLK_a, BUTTON_A },
    { SDLK_b, BUTTON_B },
    { SDLK_c, BUTTON_C },
    { SDLK_d, BUTTON_D },
    { SDLK_e, BUTTON_E },
    { SDLK_f, BUTTON_F },
    { SDLK_g, BUTTON_MTH },
    { SDLK_h, BUTTON_PRG },
    { SDLK_i, BUTTON_CST },
    { SDLK_j, BUTTON_VAR },
    { SDLK_k, BUTTON_UP },
    { SDLK_UP, BUTTON_UP },
    { SDLK_l, BUTTON_NXT },
    { SDLK_m, BUTTON_COLON },
    { SDLK_n, BUTTON_STO },
    { SDLK_o, BUTTON_EVAL },
    { SDLK_p, BUTTON_LEFT },
    { SDLK_LEFT, BUTTON_LEFT },
    { SDLK_q, BUTTON_DOWN },
    { SDLK_DOWN, BUTTON_DOWN },
    { SDLK_r, BUTTON_RIGHT },
    { SDLK_RIGHT, BUTTON_RIGHT },
    { SDLK_s, BUTTON_SIN },
    { SDLK_t, BUTTON_COS },
    { SDLK_u, BUTTON_TAN },
    { SDLK_v, BUTTON_SQRT },
    { SDLK_w, BUTTON_POWER },
    { SDLK_x, BUTTON_INV },
    { SDLK_y, BUTTON_NEG },
    { SDLK_z, BUTTON_EEX },
    { SDLK_SPACE, BUTTON_SPC },
    { SDLK_RETURN, BUTTON_ENTER },
    { SDLK_KP_ENTER, BUTTON_ENTER },
    { SDLK_BACKSPACE, BUTTON_BS },
    { SDLK_DELETE, BUTTON_DEL },
    { SDLK_PERIOD, BUTTON_PERIOD },
    { SDLK_KP_PERIOD, BUTTON_PERIOD },
    // Math operators
    { SDLK_PLUS, BUTTON_PLUS },
    { SDLK_KP_PLUS, BUTTON_PLUS },
    { SDLK_MINUS, BUTTON_MINUS },
    { SDLK_KP_MINUS, BUTTON_MINUS },
    { SDLK_ASTERISK, BUTTON_MUL },
    { SDLK_KP_MULTIPLY, BUTTON_MUL },
    { SDLK_SLASH, BUTTON_DIV },
    { SDLK_KP_DIVIDE, BUTTON_DIV },
    // on, shift, alpha
    { SDLK_ESCAPE, BUTTON_ON },
    { SDLK_LSHIFT, BUTTON_SHL },
    { SDLK_RSHIFT, BUTTON_SHL },
    { SDLK_LCTRL, BUTTON_SHR },
    { SDLK_RCTRL, BUTTON_SHR },
    { SDLK_LALT, BUTTON_ALPHA },
    { SDLK_RALT, BUTTON_ALPHA },

    // end marker
    { ( SDLKey )0, ( SDLKey )0 } };

// State to displayed zoomed last pressed key
SDL_Surface* showkeylastsurf = 0;
int showkeylastx, showkeylasty, showkeylastkey;

SDL_Surface* sdlwindow;

/****************************/
/* functions implementation */
/****************************/
unsigned SDLBGRA2ARGB( unsigned color ) {
    unsigned a = ( color >> 24 ) & 0xff, r = ( color >> 16 ) & 0xff,
             g = ( color >> 8 ) & 0xff, b = color & 0xff;

    color = a | ( r << 24 ) | ( g << 16 ) | ( b << 8 );
    return color;
}

/*
        Create a surface from binary bitmap data
*/
SDL_Surface* SDLCreateSurfFromData( unsigned int w, unsigned int h,
                                    unsigned char* data, unsigned int coloron,
                                    unsigned int coloroff ) {
    unsigned int x, y;
    SDL_Surface* surf;

    surf = SDL_CreateRGBSurface( SDL_SWSURFACE, w, h, 32, 0x00ff0000,
                                 0x0000ff00, 0x000000ff, 0xff000000 );

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

int SmallTextWidth( const char* string, unsigned int length ) {
    unsigned int i;
    int w;

    w = 0;
    for ( i = 0; i < length; i++ ) {
        if ( small_font[ ( int )string[ i ] ].h != 0 ) {
            w += small_font[ ( int )string[ i ] ].w + 1;
        } else {
            if ( verbose )
                fprintf( stderr, "Unknown small letter 0x00%x\n",
                         ( int )string[ i ] );
            w += 5;
        }
    }

    return w;
}

void SDLDrawSmallString( int x, int y, const char* string, unsigned int length,
                         unsigned int coloron, unsigned int coloroff ) {
    unsigned int i;

    for ( i = 0; i < length; i++ ) {
        if ( small_font[ ( int )string[ i ] ].h != 0 ) {
            int w = small_font[ ( int )string[ i ] ].w;
            int h = small_font[ ( int )string[ i ] ].h;

            SDL_Surface* surf = SDLCreateSurfFromData(
                w, h, small_font[ ( int )string[ i ] ].bits, coloron,
                coloroff );

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

void SDLInit( void ) {
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

    if ( show_ui_chrome ) {
        width = ( buttons_gx[ LAST_BUTTON ].x + buttons_gx[ LAST_BUTTON ].w ) +
                2 * SIDE_SKIP;
        height = DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + DISP_KBD_SKIP +
                 buttons_gx[ LAST_BUTTON ].y + buttons_gx[ LAST_BUTTON ].h +
                 BOTTOM_SKIP;
    } else {
        width = DISPLAY_WIDTH;
        height = DISPLAY_HEIGHT;
        DISPLAY_OFFSET_X = 0;
        DISPLAY_OFFSET_Y = 0;
    }

    uint32_t sdl_window_flags = SDL_SWSURFACE | SDL_RESIZABLE;
    if ( show_ui_fullscreen )
        sdl_window_flags |= SDL_FULLSCREEN;

    sdlwindow = SDL_SetVideoMode( width, height, 32, sdl_window_flags );

    if ( sdlwindow == NULL ) {
        printf( "Couldn't set video mode: %s\n", SDL_GetError() );
        exit( 1 );
    }
}

int button_pressed( int b ) {
    int code;
    int i, r, c;

    if ( buttons[ b ].pressed == 1 ) // Check not already pressed (may be
                                     // important: avoids a useless do_kbd_int)
        return 0;

    buttons[ b ].pressed = 1;

    code = buttons[ b ].code;

    if ( code == 0x8000 ) {
        for ( i = 0; i < 9; i++ )
            saturn.keybuf.rows[ i ] |= 0x8000;
        do_kbd_int();
    } else {
        r = code >> 4;
        c = 1 << ( code & 0xf );
        if ( ( saturn.keybuf.rows[ r ] & c ) == 0 ) {
            if ( saturn.kbd_ien )
                do_kbd_int();
            if ( ( saturn.keybuf.rows[ r ] & c ) )
                fprintf( stderr, "bug\n" );

            saturn.keybuf.rows[ r ] |= c;
        }
    }

    return 0;
}

int button_released( int b ) {
    int code;

    // Check not already released (not critical)
    if ( buttons[ b ].pressed == 0 )
        return 0;

    buttons[ b ].pressed = 0;

    code = buttons[ b ].code;
    if ( code == 0x8000 ) {
        for ( int i = 0; i < 9; i++ )
            saturn.keybuf.rows[ i ] &= ~0x8000;
    } else {
        int r, c;
        r = code >> 4;
        c = 1 << ( code & 0xf );
        saturn.keybuf.rows[ r ] &= ~c;
    }

    return 0;
}

void SDLCreateColors( void ) {
    unsigned i;

    for ( i = WHITE; i < BLACK; i++ )
        ARGBColors[ i ] = 0xff000000 | ( colors[ i ].r << 16 ) |
                          ( colors[ i ].g << 8 ) | colors[ i ].b;

    // Adjust the LCD color according to the contrast
    int contrast, r, g, b;
    contrast = display.contrast;

    if ( contrast < 0x3 )
        contrast = 0x3;
    if ( contrast > 0x13 )
        contrast = 0x13;

    r = ( 0x13 - contrast ) * ( colors[ LCD ].r / 0x10 );
    g = ( 0x13 - contrast ) * ( colors[ LCD ].g / 0x10 );
    b = 128 - ( ( 0x13 - contrast ) * ( ( 128 - colors[ LCD ].b ) / 0x10 ) );
    ARGBColors[ PIXEL ] = 0xff000000 | ( r << 16 ) | ( g << 8 ) | b;
}

// This should be called once to setup the surfaces. Calling it multiple
// times is fine, it won't do anything on subsequent calls.
void SDLCreateAnnunc( void ) {
    for ( int i = 0; i < 6; i++ ) {
        // If the SDL surface does not exist yet, we create it on the fly
        if ( ann_tbl[ i ].surfaceon ) {
            SDL_FreeSurface( ann_tbl[ i ].surfaceon );
            ann_tbl[ i ].surfaceon = 0;
        }

        ann_tbl[ i ].surfaceon = SDLCreateSurfFromData(
            ann_tbl[ i ].width, ann_tbl[ i ].height, ann_tbl[ i ].bits,
            ARGBColors[ PIXEL ], ARGBColors[ LCD ] );

        if ( ann_tbl[ i ].surfaceoff ) {
            SDL_FreeSurface( ann_tbl[ i ].surfaceoff );
            ann_tbl[ i ].surfaceoff = 0;
        }

        ann_tbl[ i ].surfaceoff = SDLCreateSurfFromData(
            ann_tbl[ i ].width, ann_tbl[ i ].height, ann_tbl[ i ].bits,
            ARGBColors[ LCD ], ARGBColors[ LCD ] );
    }
}

void ui__adjust_contrast() {
    SDLCreateColors();
    SDLCreateAnnunc();

    // redraw_LCD();
    memset( disp_buf, 0, sizeof( disp_buf ) );
    memset( lcd_buffer, 0, sizeof( lcd_buffer ) );

    ui__update_LCD();

    // redraw_annunc();
    last_annunc_state = -1;

    ui__draw_annunc();
}

// Find which key is pressed, if any.
// Returns -1 is no key is pressed
int SDLCoordinateToKey( unsigned int x, unsigned int y ) {
    int i;
    for ( i = BUTTON_A; i <= LAST_BUTTON; i++ ) {
        if ( x >= KEYBOARD_OFFSET_X + buttons[ i ].x &&
             x <= KEYBOARD_OFFSET_X + buttons[ i ].x + buttons[ i ].w &&
             y >= KEYBOARD_OFFSET_Y + buttons[ i ].y &&
             y <= KEYBOARD_OFFSET_Y + buttons[ i ].y + buttons[ i ].h ) {
            return i;
        }
    }
    return -1;
}
// Map the keyboard keys to the HP keys
// Returns -1 if there is no mapping
int SDLKeyToKey( SDLKey k ) {
    int i = 0;

    while ( sdltohpkeymap[ i ].sdlkey ) {
        if ( sdltohpkeymap[ i ].sdlkey == k )
            return sdltohpkeymap[ i ].hpkey;
        i++;
    }
    return -1;
}

void SDLDrawMore( unsigned int cut, unsigned int offset_y, int keypad_width,
                  int keypad_height ) {
    // bottom lines
    lineColor( sdlwindow, 1, keypad_height - 1, keypad_width - 1,
               keypad_height - 1, SDLBGRA2ARGB( ARGBColors[ PAD_TOP ] ) );
    lineColor( sdlwindow, 2, keypad_height - 2, keypad_width - 2,
               keypad_height - 2, SDLBGRA2ARGB( ARGBColors[ PAD_TOP ] ) );

    // right lines
    lineColor( sdlwindow, keypad_width - 1, keypad_height - 1, keypad_width - 1,
               cut, SDLBGRA2ARGB( ARGBColors[ PAD_TOP ] ) );
    lineColor( sdlwindow, keypad_width - 2, keypad_height - 2, keypad_width - 2,
               cut, SDLBGRA2ARGB( ARGBColors[ PAD_TOP ] ) );

    // right lines
    lineColor( sdlwindow, keypad_width - 1, cut - 1, keypad_width - 1, 1,
               SDLBGRA2ARGB( ARGBColors[ DISP_PAD_TOP ] ) );
    lineColor( sdlwindow, keypad_width - 2, cut - 1, keypad_width - 2, 2,
               SDLBGRA2ARGB( ARGBColors[ DISP_PAD_TOP ] ) );

    // top lines
    lineColor( sdlwindow, 0, 0, keypad_width - 2, 0,
               SDLBGRA2ARGB( ARGBColors[ DISP_PAD_BOT ] ) );
    lineColor( sdlwindow, 1, 1, keypad_width - 3, 1,
               SDLBGRA2ARGB( ARGBColors[ DISP_PAD_BOT ] ) );

    // left lines
    lineColor( sdlwindow, 0, cut - 1, 0, 0,
               SDLBGRA2ARGB( ARGBColors[ DISP_PAD_BOT ] ) );
    lineColor( sdlwindow, 1, cut - 1, 1, 1,
               SDLBGRA2ARGB( ARGBColors[ DISP_PAD_BOT ] ) );

    // left lines
    lineColor( sdlwindow, 0, keypad_height - 2, 0, cut,
               SDLBGRA2ARGB( ARGBColors[ PAD_BOT ] ) );
    lineColor( sdlwindow, 1, keypad_height - 3, 1, cut,
               SDLBGRA2ARGB( ARGBColors[ PAD_BOT ] ) );

    // lower the menu buttons

    // bottom lines
    lineColor( sdlwindow, 3, keypad_height - 3, keypad_width - 3,
               keypad_height - 3, SDLBGRA2ARGB( ARGBColors[ PAD_TOP ] ) );
    lineColor( sdlwindow, 4, keypad_height - 4, keypad_width - 4,
               keypad_height - 4, SDLBGRA2ARGB( ARGBColors[ PAD_TOP ] ) );

    // right lines
    lineColor( sdlwindow, keypad_width - 3, keypad_height - 3, keypad_width - 3,
               cut, SDLBGRA2ARGB( ARGBColors[ PAD_TOP ] ) );
    lineColor( sdlwindow, keypad_width - 4, keypad_height - 4, keypad_width - 4,
               cut, SDLBGRA2ARGB( ARGBColors[ PAD_TOP ] ) );

    // right lines
    lineColor( sdlwindow, keypad_width - 3, cut - 1, keypad_width - 3,
               offset_y - ( KBD_UPLINE - 1 ),
               SDLBGRA2ARGB( ARGBColors[ DISP_PAD_TOP ] ) );
    lineColor( sdlwindow, keypad_width - 4, cut - 1, keypad_width - 4,
               offset_y - ( KBD_UPLINE - 2 ),
               SDLBGRA2ARGB( ARGBColors[ DISP_PAD_TOP ] ) );

    // top lines
    lineColor( sdlwindow, 2, offset_y - ( KBD_UPLINE - 0 ), keypad_width - 4,
               offset_y - ( KBD_UPLINE - 0 ),
               SDLBGRA2ARGB( ARGBColors[ DISP_PAD_BOT ] ) );
    lineColor( sdlwindow, 3, offset_y - ( KBD_UPLINE - 1 ), keypad_width - 5,
               offset_y - ( KBD_UPLINE - 1 ),
               SDLBGRA2ARGB( ARGBColors[ DISP_PAD_BOT ] ) );

    // left lines
    lineColor( sdlwindow, 2, cut - 1, 2, offset_y - ( KBD_UPLINE - 1 ),
               SDLBGRA2ARGB( ARGBColors[ DISP_PAD_BOT ] ) );
    lineColor( sdlwindow, 3, cut - 1, 3, offset_y - ( KBD_UPLINE - 2 ),
               SDLBGRA2ARGB( ARGBColors[ DISP_PAD_BOT ] ) );

    // left lines
    lineColor( sdlwindow, 2, keypad_height - 4, 2, cut,
               SDLBGRA2ARGB( ARGBColors[ PAD_BOT ] ) );
    lineColor( sdlwindow, 3, keypad_height - 5, 3, cut,
               SDLBGRA2ARGB( ARGBColors[ PAD_BOT ] ) );

    // lower the keyboard

    // bottom lines
    lineColor( sdlwindow, 5, keypad_height - 5, keypad_width - 3,
               keypad_height - 5, SDLBGRA2ARGB( ARGBColors[ PAD_TOP ] ) );
    lineColor( sdlwindow, 6, keypad_height - 6, keypad_width - 4,
               keypad_height - 6, SDLBGRA2ARGB( ARGBColors[ PAD_TOP ] ) );

    // right lines
    lineColor( sdlwindow, keypad_width - 5, keypad_height - 5, keypad_width - 5,
               cut + 1, SDLBGRA2ARGB( ARGBColors[ PAD_TOP ] ) );
    lineColor( sdlwindow, keypad_width - 6, keypad_height - 6, keypad_width - 6,
               cut + 2, SDLBGRA2ARGB( ARGBColors[ PAD_TOP ] ) );

    // top lines
    lineColor( sdlwindow, 4, cut, keypad_width - 6, cut,
               SDLBGRA2ARGB( ARGBColors[ DISP_PAD_BOT ] ) );
    lineColor( sdlwindow, 5, cut + 1, keypad_width - 7, cut + 1,
               SDLBGRA2ARGB( ARGBColors[ DISP_PAD_BOT ] ) );

    // left lines
    lineColor( sdlwindow, 4, keypad_height - 6, 4, cut + 1,
               SDLBGRA2ARGB( ARGBColors[ PAD_BOT ] ) );
    lineColor( sdlwindow, 5, keypad_height - 7, 5, cut + 2,
               SDLBGRA2ARGB( ARGBColors[ PAD_BOT ] ) );

    // round off the bottom edge

    lineColor( sdlwindow, keypad_width - 7, keypad_height - 7, keypad_width - 7,
               keypad_height - 14, SDLBGRA2ARGB( ARGBColors[ PAD_TOP ] ) );
    lineColor( sdlwindow, keypad_width - 8, keypad_height - 8, keypad_width - 8,
               keypad_height - 11, SDLBGRA2ARGB( ARGBColors[ PAD_TOP ] ) );
    lineColor( sdlwindow, keypad_width - 7, keypad_height - 7,
               keypad_width - 14, keypad_height - 7,
               SDLBGRA2ARGB( ARGBColors[ PAD_TOP ] ) );
    lineColor( sdlwindow, keypad_width - 7, keypad_height - 8,
               keypad_width - 11, keypad_height - 8,
               SDLBGRA2ARGB( ARGBColors[ PAD_TOP ] ) );
    pixelColor( sdlwindow, keypad_width - 9, keypad_height - 9,
                SDLBGRA2ARGB( ARGBColors[ PAD_TOP ] ) );

    lineColor( sdlwindow, 7, keypad_height - 7, 13, keypad_height - 7,
               SDLBGRA2ARGB( ARGBColors[ PAD_TOP ] ) );
    lineColor( sdlwindow, 8, keypad_height - 8, 10, keypad_height - 8,
               SDLBGRA2ARGB( ARGBColors[ PAD_TOP ] ) );

    lineColor( sdlwindow, 6, keypad_height - 8, 6, keypad_height - 14,
               SDLBGRA2ARGB( ARGBColors[ PAD_BOT ] ) );
    lineColor( sdlwindow, 7, keypad_height - 9, 7, keypad_height - 11,
               SDLBGRA2ARGB( ARGBColors[ PAD_BOT ] ) );
}

void SDLDrawLogo() {
    int x, y;
    SDL_Surface* surf;

    int display_width = DISPLAY_WIDTH;

    // insert the HP Logo
    surf = SDLCreateSurfFromData( hp_width, hp_height, hp_bitmap,
                                  ARGBColors[ LOGO ], ARGBColors[ LOGO_BACK ] );
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
        lineColor( sdlwindow, DISPLAY_OFFSET_X, 9,
                   DISPLAY_OFFSET_X + hp_width - 1, 9,
                   SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
        lineColor( sdlwindow, DISPLAY_OFFSET_X - 1, 10, DISPLAY_OFFSET_X - 1,
                   10 + hp_height - 1, SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
        lineColor( sdlwindow, DISPLAY_OFFSET_X, 10 + hp_height,
                   DISPLAY_OFFSET_X + hp_width - 1, 10 + hp_height,
                   SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
        lineColor( sdlwindow, DISPLAY_OFFSET_X + hp_width, 10,
                   DISPLAY_OFFSET_X + hp_width, 10 + hp_height - 1,
                   SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
    }

    // write the name of it

    if ( opt_gx ) {
        x = DISPLAY_OFFSET_X + display_width - gx_128K_ram_width +
            gx_128K_ram_x_hot + 2;
        y = 10 + gx_128K_ram_y_hot;

        surf = SDLCreateSurfFromData( gx_128K_ram_width, gx_128K_ram_height,
                                      gx_128K_ram_bitmap, ARGBColors[ LABEL ],
                                      ARGBColors[ DISP_PAD ] );
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
        surf =
            SDLCreateSurfFromData( hp48gx_width, hp48gx_height, hp48gx_bitmap,
                                   ARGBColors[ LOGO ], ARGBColors[ DISP_PAD ] );
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

        x = DISPLAY_OFFSET_X + DISPLAY_WIDTH - gx_128K_ram_width +
            gx_silver_x_hot + 2;
        y = 10 + gx_silver_y_hot;
        surf = SDLCreateSurfFromData(
            gx_silver_width, gx_silver_height, gx_silver_bitmap,
            ARGBColors[ LOGO ],
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

        x = DISPLAY_OFFSET_X + display_width - gx_128K_ram_width +
            gx_green_x_hot + 2;
        y = 10 + gx_green_y_hot;
        surf = SDLCreateSurfFromData(
            gx_green_width, gx_green_height, gx_green_bitmap,
            ARGBColors[ RIGHT ],
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
        surf = SDLCreateSurfFromData(
            hp48sx_width, hp48sx_height, hp48sx_bitmap, ARGBColors[ RIGHT ],
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
        surf = SDLCreateSurfFromData(
            science_width, science_height, science_bitmap, ARGBColors[ RIGHT ],
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

void SDLCreateKeys( void ) {
    unsigned i, x, y;
    unsigned pixel;

    for ( i = BUTTON_A; i <= LAST_BUTTON; i++ ) {
        // Create surfaces for each button
        if ( !buttons[ i ].surfaceup ) {
            buttons[ i ].surfaceup = SDL_CreateRGBSurface(
                SDL_SWSURFACE, buttons[ i ].w, buttons[ i ].h, 32, 0x00ff0000,
                0x0000ff00, 0x000000ff, 0xff000000 );
        }
        if ( !buttons[ i ].surfacedown ) {
            buttons[ i ].surfacedown = SDL_CreateRGBSurface(
                SDL_SWSURFACE, buttons[ i ].w, buttons[ i ].h, 32, 0x00ff0000,
                0x0000ff00, 0x000000ff, 0xff000000 );
        }
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
        lineColor( buttons[ i ].surfaceup, 1, buttons[ i ].h - 2, 1, 1,
                   SDLBGRA2ARGB( ARGBColors[ BUT_TOP ] ) );
        lineColor( buttons[ i ].surfaceup, 2, buttons[ i ].h - 3, 2, 2,
                   SDLBGRA2ARGB( ARGBColors[ BUT_TOP ] ) );
        lineColor( buttons[ i ].surfaceup, 3, buttons[ i ].h - 4, 3, 3,
                   SDLBGRA2ARGB( ARGBColors[ BUT_TOP ] ) );

        lineColor( buttons[ i ].surfaceup, 1, 1, buttons[ i ].w - 2, 1,
                   SDLBGRA2ARGB( ARGBColors[ BUT_TOP ] ) );
        lineColor( buttons[ i ].surfaceup, 2, 2, buttons[ i ].w - 3, 2,
                   SDLBGRA2ARGB( ARGBColors[ BUT_TOP ] ) );
        lineColor( buttons[ i ].surfaceup, 3, 3, buttons[ i ].w - 4, 3,
                   SDLBGRA2ARGB( ARGBColors[ BUT_TOP ] ) );
        lineColor( buttons[ i ].surfaceup, 4, 4, buttons[ i ].w - 5, 4,
                   SDLBGRA2ARGB( ARGBColors[ BUT_TOP ] ) );

        pixelColor( buttons[ i ].surfaceup, 4, 5,
                    SDLBGRA2ARGB( ARGBColors[ BUT_TOP ] ) );

        lineColor( buttons[ i ].surfaceup, 3, buttons[ i ].h - 2,
                   buttons[ i ].w - 2, buttons[ i ].h - 2,
                   SDLBGRA2ARGB( ARGBColors[ BUT_BOT ] ) );
        lineColor( buttons[ i ].surfaceup, 4, buttons[ i ].h - 3,
                   buttons[ i ].w - 3, buttons[ i ].h - 3,
                   SDLBGRA2ARGB( ARGBColors[ BUT_BOT ] ) );

        lineColor( buttons[ i ].surfaceup, buttons[ i ].w - 2,
                   buttons[ i ].h - 2, buttons[ i ].w - 2, 3,
                   SDLBGRA2ARGB( ARGBColors[ BUT_BOT ] ) );
        lineColor( buttons[ i ].surfaceup, buttons[ i ].w - 3,
                   buttons[ i ].h - 3, buttons[ i ].w - 3, 4,
                   SDLBGRA2ARGB( ARGBColors[ BUT_BOT ] ) );
        lineColor( buttons[ i ].surfaceup, buttons[ i ].w - 4,
                   buttons[ i ].h - 4, buttons[ i ].w - 4, 5,
                   SDLBGRA2ARGB( ARGBColors[ BUT_BOT ] ) );
        pixelColor( buttons[ i ].surfaceup, buttons[ i ].w - 5,
                    buttons[ i ].h - 4, SDLBGRA2ARGB( ARGBColors[ BUT_BOT ] ) );

        // draw frame around button

        lineColor( buttons[ i ].surfaceup, 0, buttons[ i ].h - 3, 0, 2,
                   SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
        lineColor( buttons[ i ].surfaceup, 2, 0, buttons[ i ].w - 3, 0,
                   SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
        lineColor( buttons[ i ].surfaceup, 2, buttons[ i ].h - 1,
                   buttons[ i ].w - 3, buttons[ i ].h - 1,
                   SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
        lineColor( buttons[ i ].surfaceup, buttons[ i ].w - 1,
                   buttons[ i ].h - 3, buttons[ i ].w - 1, 2,
                   SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
        if ( i == BUTTON_ON ) {
            lineColor( buttons[ i ].surfaceup, 1, 1, buttons[ 1 ].w - 2, 1,
                       SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
            pixelColor( buttons[ i ].surfaceup, 1, 2,
                        SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
            pixelColor( buttons[ i ].surfaceup, buttons[ i ].w - 2, 2,
                        SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
        } else {
            pixelColor( buttons[ i ].surfaceup, 1, 1,
                        SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
            pixelColor( buttons[ i ].surfaceup, buttons[ i ].w - 2, 1,
                        SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
        }
        pixelColor( buttons[ i ].surfaceup, 1, buttons[ i ].h - 2,
                    SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
        pixelColor( buttons[ i ].surfaceup, buttons[ i ].w - 2,
                    buttons[ i ].h - 2, SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );

        // draw the depressed button

        // draw edge of button
        lineColor( buttons[ i ].surfacedown, 2, buttons[ i ].h - 4, 2, 2,
                   SDLBGRA2ARGB( ARGBColors[ BUT_TOP ] ) );
        lineColor( buttons[ i ].surfacedown, 3, buttons[ i ].h - 5, 3, 3,
                   SDLBGRA2ARGB( ARGBColors[ BUT_TOP ] ) );
        lineColor( buttons[ i ].surfacedown, 2, 2, buttons[ i ].w - 4, 2,
                   SDLBGRA2ARGB( ARGBColors[ BUT_TOP ] ) );
        lineColor( buttons[ i ].surfacedown, 3, 3, buttons[ i ].w - 5, 3,
                   SDLBGRA2ARGB( ARGBColors[ BUT_TOP ] ) );
        pixelColor( buttons[ i ].surfacedown, 4, 4,
                    SDLBGRA2ARGB( ARGBColors[ BUT_TOP ] ) );

        lineColor( buttons[ i ].surfacedown, 3, buttons[ i ].h - 3,
                   buttons[ i ].w - 3, buttons[ i ].h - 3,
                   SDLBGRA2ARGB( ARGBColors[ BUT_BOT ] ) );
        lineColor( buttons[ i ].surfacedown, 4, buttons[ i ].h - 4,
                   buttons[ i ].w - 4, buttons[ i ].h - 4,
                   SDLBGRA2ARGB( ARGBColors[ BUT_BOT ] ) );
        lineColor( buttons[ i ].surfacedown, buttons[ i ].w - 3,
                   buttons[ i ].h - 3, buttons[ i ].w - 3, 3,
                   SDLBGRA2ARGB( ARGBColors[ BUT_BOT ] ) );
        lineColor( buttons[ i ].surfacedown, buttons[ i ].w - 4,
                   buttons[ i ].h - 4, buttons[ i ].w - 4, 4,
                   SDLBGRA2ARGB( ARGBColors[ BUT_BOT ] ) );
        pixelColor( buttons[ i ].surfacedown, buttons[ i ].w - 5,
                    buttons[ i ].h - 5, SDLBGRA2ARGB( ARGBColors[ BUT_BOT ] ) );

        // draw frame around button
        lineColor( buttons[ i ].surfacedown, 0, buttons[ i ].h - 3, 0, 2,
                   SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
        lineColor( buttons[ i ].surfacedown, 2, 0, buttons[ i ].w - 3, 0,
                   SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
        lineColor( buttons[ i ].surfacedown, 2, buttons[ i ].h - 1,
                   buttons[ i ].w - 3, buttons[ i ].h - 1,
                   SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
        lineColor( buttons[ i ].surfacedown, buttons[ i ].w - 1,
                   buttons[ i ].h - 3, buttons[ i ].w - 1, 2,
                   SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );

        if ( i == BUTTON_ON ) {
            lineColor( buttons[ i ].surfacedown, 1, 1, buttons[ i ].w - 2, 1,
                       SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
            pixelColor( buttons[ i ].surfacedown, 1, 2,
                        SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
            pixelColor( buttons[ i ].surfacedown, buttons[ i ].w - 2, 2,
                        SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
        } else {
            pixelColor( buttons[ i ].surfacedown, 1, 1,
                        SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
            pixelColor( buttons[ i ].surfacedown, buttons[ i ].w - 2, 1,
                        SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
        }
        pixelColor( buttons[ i ].surfacedown, 1, buttons[ i ].h - 2,
                    SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
        pixelColor( buttons[ i ].surfacedown, buttons[ i ].w - 2,
                    buttons[ i ].h - 2, SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
        if ( i == BUTTON_ON ) {
            rectangleColor( buttons[ i ].surfacedown, 1, 2,
                            1 + buttons[ i ].w - 3, 2 + buttons[ i ].h - 4,
                            SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
            pixelColor( buttons[ i ].surfacedown, 2, 3,
                        SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
            pixelColor( buttons[ i ].surfacedown, buttons[ i ].w - 3, 3,
                        SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
        } else {
            rectangleColor( buttons[ i ].surfacedown, 1, 1,
                            1 + buttons[ i ].w - 3, 1 + buttons[ i ].h - 3,
                            SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
            pixelColor( buttons[ i ].surfacedown, 2, 2,
                        SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
            pixelColor( buttons[ i ].surfacedown, buttons[ i ].w - 3, 2,
                        SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
        }
        pixelColor( buttons[ i ].surfacedown, 2, buttons[ i ].h - 3,
                    SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );
        pixelColor( buttons[ i ].surfacedown, buttons[ i ].w - 3,
                    buttons[ i ].h - 3, SDLBGRA2ARGB( ARGBColors[ FRAME ] ) );

        if ( buttons[ i ].label != ( char* )0 ) {
            // Todo: use SDL_ttf to print "nice" fonts

            // for the time being use SDL_gfxPrimitives' font
            x = ( buttons[ i ].w - strlen( buttons[ i ].label ) * 8 ) / 2;
            y = ( buttons[ i ].h + 1 ) / 2 - 4;
            stringColor( buttons[ i ].surfaceup, x, y, buttons[ i ].label,
                         0xffffffff );
            stringColor( buttons[ i ].surfacedown, x, y, buttons[ i ].label,
                         0xffffffff );
        }
        // Pixmap centered in button
        if ( buttons[ i ].lw != 0 ) {
            // If there's a bitmap, try to plot this
            unsigned colorbg = ARGBColors[ BUTTON ];
            unsigned colorfg = ARGBColors[ buttons[ i ].lc ];

            // Blit the label surface to the button
            SDL_Surface* surf;
            surf = SDLCreateSurfFromData( buttons[ i ].lw, buttons[ i ].lh,
                                          buttons[ i ].lb, colorfg, colorbg );
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
void SDLDrawKeysLabelsLeft( void ) {
    int i, x, y;
    unsigned int pw /* , ph */;
    int wl, wr, ws;
    int offset_y = KEYBOARD_OFFSET_Y;
    int offset_x = KEYBOARD_OFFSET_X;

    unsigned colorbg, colorfg;

    // Draw the left labels
    for ( i = BUTTON_A; i <= LAST_BUTTON; i++ ) {
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

            x = ( pw + 1 -
                  SmallTextWidth( buttons[ i ].left,
                                  strlen( buttons[ i ].left ) ) ) /
                2;
            if ( opt_gx )
                y = 14;
            else
                y = 9;

            // Set the coordinates to absolute
            if ( opt_gx ) {
                x += offset_x + buttons[ i ].x - 6;
                y += offset_y + buttons[ i ].y - small_ascent - small_descent -
                     6;
            } else {
                x += offset_x + buttons[ i ].x + ( buttons[ i ].w - pw ) / 2;
                y += offset_y + buttons[ i ].y - small_ascent - small_descent;
            }

            SDLDrawSmallString( x, y, buttons[ i ].left,
                                strlen( buttons[ i ].left ), colorfg, colorbg );
        } else // is_menu
        {
            colorbg = ARGBColors[ BLACK ];
            colorfg = ARGBColors[ LEFT ];

            if ( buttons[ i ].right == ( char* )0 ) {
                // centered label
                x = offset_x + buttons[ i ].x +
                    ( 1 + buttons[ i ].w -
                      SmallTextWidth( buttons[ i ].left,
                                      strlen( buttons[ i ].left ) ) ) /
                        2;
            } else {
                // label to the left
                wl = SmallTextWidth( buttons[ i ].left,
                                     strlen( buttons[ i ].left ) );
                wr = SmallTextWidth( buttons[ i ].right,
                                     strlen( buttons[ i ].right ) );
                ws = SmallTextWidth( " ", 1 );

                x = offset_x + buttons[ i ].x +
                    ( 1 + buttons[ i ].w - ( wl + wr + ws ) ) / 2;
            }

            y = offset_y + buttons[ i ].y - small_descent;

            SDLDrawSmallString( x, y, buttons[ i ].left,
                                strlen( buttons[ i ].left ), colorfg, colorbg );
        } // is_menu

    } // for
}

// Draw the right labels (green on GX)
void SDLDrawKeysLabelsRight( void ) {
    int i, x, y;
    unsigned int pw /* , ph */;
    int wl, wr, ws;
    int offset_y = KEYBOARD_OFFSET_Y;
    int offset_x = KEYBOARD_OFFSET_X;
    unsigned colorbg, colorfg;

    // draw the right labels
    for ( i = BUTTON_A; i <= LAST_BUTTON; i++ ) {
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

            x = ( pw + 1 -
                  SmallTextWidth( buttons[ i ].right,
                                  strlen( buttons[ i ].right ) ) ) /
                2;
            if ( opt_gx )
                y = 14;
            else
                y = 8;

            // Set the coordinates to absolute
            if ( opt_gx ) {
                x += offset_x + buttons[ i ].x - 6;
                y += offset_y + buttons[ i ].y - small_ascent - small_descent -
                     6;
            } else {
                x += offset_x + buttons[ i ].x + ( buttons[ i ].w - pw ) / 2;
                y += offset_y + buttons[ i ].y - small_ascent - small_descent;
            }

            SDLDrawSmallString( x, y, buttons[ i ].right,
                                strlen( buttons[ i ].right ), colorfg,
                                colorbg );
        } // buttons[i].is_menu
        else {
            colorbg = ARGBColors[ BLACK ];
            colorfg = ARGBColors[ RIGHT ];

            if ( buttons[ i ].left == ( char* )0 ) {
                // centered label
                x = offset_x + buttons[ i ].x +
                    ( 1 + buttons[ i ].w -
                      SmallTextWidth( buttons[ i ].right,
                                      strlen( buttons[ i ].right ) ) ) /
                        2;
            } else {
                // label to the right
                wl = SmallTextWidth( buttons[ i ].left,
                                     strlen( buttons[ i ].left ) );
                wr = SmallTextWidth( buttons[ i ].right,
                                     strlen( buttons[ i ].right ) );
                ws = SmallTextWidth( " ", 1 );

                x = offset_x + buttons[ i ].x +
                    ( 1 + buttons[ i ].w - ( wl + wr + ws ) ) / 2 + wl + ws;
            }

            y = offset_y + buttons[ i ].y - small_descent;

            SDLDrawSmallString( x, y, buttons[ i ].right,
                                strlen( buttons[ i ].right ), colorfg,
                                colorbg );
        }

    } // for
}

// Draw the letter bottom right of the keys
void SDLDrawKeysLetters( void ) {
    int i, x, y;
    int offset_y = KEYBOARD_OFFSET_Y;
    int offset_x = KEYBOARD_OFFSET_X;
    unsigned colorbg;

    for ( i = BUTTON_A; i <= LAST_BUTTON; i++ ) {

        if ( i < BUTTON_MTH )
            colorbg = ARGBColors[ DISP_PAD ];
        else
            colorbg = ARGBColors[ PAD ];

        // Letter ( small character bottom right of key)
        if ( buttons[ i ].letter != ( char* )0 ) {
            if ( opt_gx ) {
                x = offset_x + buttons[ i ].x + buttons[ i ].w + 3;
                y = offset_y + buttons[ i ].y + buttons[ i ].h + 1;
            } else {
                x = offset_x + buttons[ i ].x + buttons[ i ].w -
                    SmallTextWidth( buttons[ i ].letter, 1 ) / 2 + 5;
                y = offset_y + buttons[ i ].y + buttons[ i ].h - 2;
            }

            SDLDrawSmallString( x, y, buttons[ i ].letter, 1, 0xffffffff,
                                colorbg );
        }
    }
}

// Bottom label: the only one is the cancel button
void SDLDrawKeysLabelsBottom( void ) {
    int i, x, y;
    int offset_y = KEYBOARD_OFFSET_Y;
    int offset_x = KEYBOARD_OFFSET_X;
    unsigned colorbg, colorfg;

    // Bottom label: the only one is the cancel button
    for ( i = BUTTON_A; i <= LAST_BUTTON; i++ ) {
        if ( buttons[ i ].sub == ( char* )0 )
            continue;

        if ( i < BUTTON_MTH )
            colorbg = ARGBColors[ DISP_PAD ];
        else
            colorbg = ARGBColors[ PAD ];

        colorfg = ARGBColors[ WHITE ];

        x = offset_x + buttons[ i ].x +
            ( 1 + buttons[ i ].w -
              SmallTextWidth( buttons[ i ].sub, strlen( buttons[ i ].sub ) ) ) /
                2;
        y = offset_y + buttons[ i ].y + buttons[ i ].h + small_ascent + 2;
        SDLDrawSmallString( x, y, buttons[ i ].sub, strlen( buttons[ i ].sub ),
                            colorfg, colorbg );
    }
}

// Draws the greyish area around keys that trigger menus
void SDLDrawKeyMenu( void ) {
    int i, x, y;
    int offset_y = KEYBOARD_OFFSET_Y;
    int offset_x = KEYBOARD_OFFSET_X;
    SDL_Rect rect;
    unsigned color;
    unsigned pw, ph;

    for ( i = BUTTON_A; i <= LAST_BUTTON; i++ ) {
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

void SDLDrawButtons( void ) {
    int i;

    for ( i = BUTTON_A; i <= LAST_BUTTON; i++ ) {
        SDL_Rect /* rect, */ srect, drect;

        // Blit the button surface to the screen
        srect.x = 0;
        srect.y = 0;
        srect.w = buttons[ i ].w;
        srect.h = buttons[ i ].h;
        drect.x = KEYBOARD_OFFSET_X + buttons[ i ].x;
        drect.y = KEYBOARD_OFFSET_Y + buttons[ i ].y;
        drect.w = buttons[ i ].w;
        drect.h = buttons[ i ].h;
        if ( buttons[ i ].pressed ) {
            SDL_BlitSurface( buttons[ i ].surfacedown, &srect, sdlwindow,
                             &drect );
        } else {
            SDL_BlitSurface( buttons[ i ].surfaceup, &srect, sdlwindow,
                             &drect );
        }
    }

    // Always update immediately buttons
    SDL_UpdateRect(
        sdlwindow, KEYBOARD_OFFSET_X + buttons[ 0 ].x,
        KEYBOARD_OFFSET_Y + buttons[ 0 ].y,
        buttons[ LAST_BUTTON ].x + buttons[ LAST_BUTTON ].w - buttons[ 0 ].x,
        buttons[ LAST_BUTTON ].y + buttons[ LAST_BUTTON ].h - buttons[ 0 ].y );
}

void SDLDrawKeypad( void ) {
    SDLDrawKeyMenu();
    SDLDrawKeysLetters();
    SDLDrawKeysLabelsBottom();
    SDLDrawKeysLabelsLeft();
    SDLDrawKeysLabelsRight();
    SDLCreateKeys();
    SDLDrawButtons();
}

void SDLDrawBezel() {
    unsigned int i;
    int display_height = DISPLAY_HEIGHT;
    int display_width = DISPLAY_WIDTH;

    // draw the frame around the display
    for ( i = 0; i < DISP_FRAME; i++ ) {
        lineColor( sdlwindow, DISPLAY_OFFSET_X - i,
                   DISPLAY_OFFSET_Y + display_height + 2 * i,
                   DISPLAY_OFFSET_X + display_width + i,
                   DISPLAY_OFFSET_Y + display_height + 2 * i,
                   SDLBGRA2ARGB( ARGBColors[ DISP_PAD_TOP ] ) );
        lineColor( sdlwindow, DISPLAY_OFFSET_X - i,
                   DISPLAY_OFFSET_Y + display_height + 2 * i + 1,
                   DISPLAY_OFFSET_X + display_width + i,
                   DISPLAY_OFFSET_Y + display_height + 2 * i + 1,
                   SDLBGRA2ARGB( ARGBColors[ DISP_PAD_TOP ] ) );
        lineColor( sdlwindow, DISPLAY_OFFSET_X + display_width + i,
                   DISPLAY_OFFSET_Y - i, DISPLAY_OFFSET_X + display_width + i,
                   DISPLAY_OFFSET_Y + display_height + 2 * i,
                   SDLBGRA2ARGB( ARGBColors[ DISP_PAD_TOP ] ) );
    }

    for ( i = 0; i < DISP_FRAME; i++ ) {
        lineColor(
            sdlwindow, DISPLAY_OFFSET_X - i - 1, DISPLAY_OFFSET_Y - i - 1,
            DISPLAY_OFFSET_X + display_width + i - 1, DISPLAY_OFFSET_Y - i - 1,
            SDLBGRA2ARGB( ARGBColors[ DISP_PAD_BOT ] ) );
        lineColor( sdlwindow, DISPLAY_OFFSET_X - i - 1,
                   DISPLAY_OFFSET_Y - i - 1, DISPLAY_OFFSET_X - i - 1,
                   DISPLAY_OFFSET_Y + display_height + 2 * i - 1,
                   SDLBGRA2ARGB( ARGBColors[ DISP_PAD_BOT ] ) );
    }

    // round off corners
    lineColor( sdlwindow, DISPLAY_OFFSET_X - DISP_FRAME,
               DISPLAY_OFFSET_Y - DISP_FRAME, DISPLAY_OFFSET_X - DISP_FRAME + 3,
               DISPLAY_OFFSET_Y - DISP_FRAME,
               SDLBGRA2ARGB( ARGBColors[ DISP_PAD ] ) );
    lineColor( sdlwindow, DISPLAY_OFFSET_X - DISP_FRAME,
               DISPLAY_OFFSET_Y - DISP_FRAME, DISPLAY_OFFSET_X - DISP_FRAME,
               DISPLAY_OFFSET_Y - DISP_FRAME + 3,
               SDLBGRA2ARGB( ARGBColors[ DISP_PAD ] ) );
    pixelColor( sdlwindow, DISPLAY_OFFSET_X - DISP_FRAME + 1,
                DISPLAY_OFFSET_Y - DISP_FRAME + 1,
                SDLBGRA2ARGB( ARGBColors[ DISP_PAD ] ) );

    lineColor( sdlwindow, DISPLAY_OFFSET_X + display_width + DISP_FRAME - 4,
               DISPLAY_OFFSET_Y - DISP_FRAME,
               DISPLAY_OFFSET_X + display_width + DISP_FRAME - 1,
               DISPLAY_OFFSET_Y - DISP_FRAME,
               SDLBGRA2ARGB( ARGBColors[ DISP_PAD ] ) );
    lineColor( sdlwindow, DISPLAY_OFFSET_X + display_width + DISP_FRAME - 1,
               DISPLAY_OFFSET_Y - DISP_FRAME,
               DISPLAY_OFFSET_X + display_width + DISP_FRAME - 1,
               DISPLAY_OFFSET_Y - DISP_FRAME + 3,
               SDLBGRA2ARGB( ARGBColors[ DISP_PAD ] ) );
    pixelColor( sdlwindow, DISPLAY_OFFSET_X + display_width + DISP_FRAME - 2,
                DISPLAY_OFFSET_Y - DISP_FRAME + 1,
                SDLBGRA2ARGB( ARGBColors[ DISP_PAD ] ) );

    lineColor( sdlwindow, DISPLAY_OFFSET_X - DISP_FRAME,
               DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 4,
               DISPLAY_OFFSET_X - DISP_FRAME,
               DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 1,
               SDLBGRA2ARGB( ARGBColors[ DISP_PAD ] ) );
    lineColor( sdlwindow, DISPLAY_OFFSET_X - DISP_FRAME,
               DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 1,
               DISPLAY_OFFSET_X - DISP_FRAME + 3,
               DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 1,
               SDLBGRA2ARGB( ARGBColors[ DISP_PAD ] ) );
    pixelColor( sdlwindow, DISPLAY_OFFSET_X - DISP_FRAME + 1,
                DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 2,
                SDLBGRA2ARGB( ARGBColors[ DISP_PAD ] ) );

    lineColor( sdlwindow, DISPLAY_OFFSET_X + display_width + DISP_FRAME - 1,
               DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 4,
               DISPLAY_OFFSET_X + display_width + DISP_FRAME - 1,
               DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 1,
               SDLBGRA2ARGB( ARGBColors[ DISP_PAD ] ) );
    lineColor( sdlwindow, DISPLAY_OFFSET_X + display_width + DISP_FRAME - 4,
               DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 1,
               DISPLAY_OFFSET_X + display_width + DISP_FRAME - 1,
               DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 1,
               SDLBGRA2ARGB( ARGBColors[ DISP_PAD ] ) );
    pixelColor( sdlwindow, DISPLAY_OFFSET_X + display_width + DISP_FRAME - 2,
                DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 2,
                SDLBGRA2ARGB( ARGBColors[ DISP_PAD ] ) );

    // simulate rounded lcd corners

    lineColor( sdlwindow, DISPLAY_OFFSET_X - 1, DISPLAY_OFFSET_Y + 1,
               DISPLAY_OFFSET_X - 1, DISPLAY_OFFSET_Y + display_height - 2,
               SDLBGRA2ARGB( ARGBColors[ LCD ] ) );
    lineColor( sdlwindow, DISPLAY_OFFSET_X + 1, DISPLAY_OFFSET_Y - 1,
               DISPLAY_OFFSET_X + display_width - 2, DISPLAY_OFFSET_Y - 1,
               SDLBGRA2ARGB( ARGBColors[ LCD ] ) );
    lineColor(
        sdlwindow, DISPLAY_OFFSET_X + 1, DISPLAY_OFFSET_Y + display_height,
        DISPLAY_OFFSET_X + display_width - 2, DISPLAY_OFFSET_Y + display_height,
        SDLBGRA2ARGB( ARGBColors[ LCD ] ) );
    lineColor( sdlwindow, DISPLAY_OFFSET_X + display_width,
               DISPLAY_OFFSET_Y + 1, DISPLAY_OFFSET_X + display_width,
               DISPLAY_OFFSET_Y + display_height - 2,
               SDLBGRA2ARGB( ARGBColors[ LCD ] ) );
}

void SDLDrawBackground( int width, int height, int w_top, int h_top ) {
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

void SDLDrawBackgroundLCD() {
    SDL_Rect rect;

    rect.x = DISPLAY_OFFSET_X;
    rect.y = DISPLAY_OFFSET_Y;
    rect.w = DISPLAY_WIDTH;
    rect.h = DISPLAY_HEIGHT;
    SDL_FillRect( sdlwindow, &rect, ARGBColors[ LCD ] );
}

void SDLDrawAnnunc( char* annunc ) {
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
            SDL_BlitSurface( ann_tbl[ i ].surfaceon, &srect, sdlwindow,
                             &drect );
        else
            SDL_BlitSurface( ann_tbl[ i ].surfaceoff, &srect, sdlwindow,
                             &drect );
    }

    // Always immediately update annunciators
    SDL_UpdateRect( sdlwindow, DISPLAY_OFFSET_X + ann_tbl[ 0 ].x,
                    DISPLAY_OFFSET_Y + ann_tbl[ 0 ].y,
                    ann_tbl[ 5 ].x + ann_tbl[ 5 ].width - ann_tbl[ 0 ].x,
                    ann_tbl[ 5 ].y + ann_tbl[ 5 ].height - ann_tbl[ 0 ].y );
}

void SDLDrawNibble( int nx, int ny, int val ) {
    int x, y;
    int xoffset = DISPLAY_OFFSET_X + 5;
    int yoffset = DISPLAY_OFFSET_Y + 20;

    SDL_LockSurface( sdlwindow );
    unsigned char* buffer = ( unsigned char* )sdlwindow->pixels;
    unsigned int pitch = sdlwindow->pitch;

    for ( y = 0; y < 2; y++ ) {
        unsigned int* lineptr;
        lineptr =
            ( unsigned int* )( buffer + pitch * ( yoffset + 2 * ny + y ) );

        for ( x = 0; x < 4; x++ ) {
            // Check if bit is on
            // char c = lcd_buffer[y/2][x>>2];		// The 4 lower bits
            // in a byte are used (1 nibble per byte)
            if ( nx + x >= 131 ) // Clip at 131 pixels (some nibble writes may
                                 // go beyond the range, but are not visible)
                break;
            char c = val;
            char b = c & ( 1 << ( x & 3 ) );
            if ( b ) {
                lineptr[ xoffset + 2 * ( nx + x ) ] = ARGBColors[ PIXEL ];
                lineptr[ xoffset + 2 * ( nx + x ) + 1 ] = ARGBColors[ PIXEL ];
            } else {
                lineptr[ xoffset + 2 * ( nx + x ) ] = ARGBColors[ LCD ];
                lineptr[ xoffset + 2 * ( nx + x ) + 1 ] = ARGBColors[ LCD ];
            }
        }
    }
    SDL_UnlockSurface( sdlwindow );

#ifndef DELAYEDDISPUPDATE
    // Either update immediately or with a delay the display
    SDL_UpdateRect( sdlwindow, xoffset + 2 * nx, yoffset + 2 * ny, 8, 2 );
#endif
}

void SDLUIHideKey( void ) {
    SDL_Rect drect;

    if ( showkeylastsurf == 0 )
        return;

    drect.x = showkeylastx;
    drect.y = showkeylasty;
    SDL_BlitSurface( showkeylastsurf, 0, sdlwindow, &drect );

    // Update
    SDL_UpdateRect( sdlwindow, showkeylastx, showkeylasty, showkeylastsurf->w,
                    showkeylastsurf->h );

    // Free
    SDL_FreeSurface( showkeylastsurf );
    showkeylastsurf = 0;
}

// Show the hp key which is being pressed
void SDLUIShowKey( int hpkey ) {
    SDL_Rect /* rect,  */ srect, drect;
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
    if ( buttons[ hpkey ].pressed )
        ssurf = buttons[ hpkey ].surfacedown;
    else
        ssurf = buttons[ hpkey ].surfaceup;

    // Background backup
    showkeylastsurf =
        SDL_CreateRGBSurface( SDL_SWSURFACE, ssurf->w, ssurf->h, 32, 0x00ff0000,
                              0x0000ff00, 0x000000ff, 0xff000000 );

    // Where to
    x = KEYBOARD_OFFSET_X + buttons[ hpkey ].x -
        ( ssurf->w - ssurf->w + 1 ) / 2;
    y = KEYBOARD_OFFSET_Y + buttons[ hpkey ].y -
        ( ssurf->h - ssurf->h + 1 ) / 2;
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

void SDLUIFeedback( void ) {}

static int button_release_all( void ) {
    for ( int b = BUTTON_A; b <= LAST_BUTTON; b++ )
        if ( buttons[ b ].pressed ) {
            button_released( b );
        }

    return 0;
}

void SDLDrawSerialDevices() {
    char text[ 1024 ];

    if ( verbose ) {
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

int ui__get_event( void ) {
    SDL_Event event;
    int hpkey;
    int rv;
    static int lasthpkey = -1; // last key that was pressed or -1 for none
    static int lastticks =
        -1; // time at which a key was pressed or -1 if timer expired
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

        if ( state & SDL_BUTTON( 1 ) &&
             SDLCoordinateToKey( x, y ) == lasthpkey ) {
            lastislongpress = 1;
            SDLUIFeedback();
        }
    }

    // Iterate as long as there are events
    // while( SDL_PollEvent( &event ) )
    if ( SDL_PollEvent( &event ) ) {
        switch ( event.type ) {
            case SDL_QUIT:
                exit_emulator();
                exit( 0 );
                break;

            // Mouse move: react to state changes in the buttons that are
            // pressed
            case SDL_MOUSEMOTION:
                hpkey = SDLCoordinateToKey( event.motion.x, event.motion.y );
                if ( event.motion.state & SDL_BUTTON( 1 ) ) {
                    // Mouse moves on a key different from the last key
                    // (state change):
                    // - release last (if last was pressed)
                    // - press new (if new is pressed)
                    if ( hpkey != lasthpkey ) {
                        keyispressed = hpkey;

                        if ( lasthpkey != -1 ) {
                            if ( !lastislongpress ) {
                                button_release_all();
                                rv = 1;
                                SDLUIFeedback();
                            }
                            // Stop timer, clear long key press
                            lastticks = -1;
                            lastislongpress = 0;
                        }
                        if ( hpkey != -1 ) {
                            if ( !buttons[ hpkey ]
                                      .pressed ) // If a key is down, it
                                                 // can't be down another
                                                 // time
                            {
                                button_pressed( hpkey );
                                rv = 1;
                                // Start timer
                                lastticks = SDL_GetTicks();
                                SDLUIFeedback();
                            }
                        }
                    }
                    lasthpkey = hpkey;
                }
                if ( hpkey == -1 ) // Needed to avoid pressing and moving
                                   // outside of a button releases
                    lasthpkey = -1;

                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                hpkey = SDLCoordinateToKey( event.button.x, event.button.y );

                if ( hpkey !=
                     -1 ) // React to mouse up/down when click over a button
                {
                    if ( event.type == SDL_MOUSEBUTTONDOWN ) {
                        keyispressed = hpkey;

                        if ( !buttons[ hpkey ].pressed ) // Key can't be pressed
                                                         // when down
                        {
                            button_pressed( hpkey );
                            rv = 1;
                            lasthpkey = hpkey;
                            // Start timer
                            lastticks = SDL_GetTicks();
                            SDLUIFeedback();
                        }
                    } else {
                        keyispressed = -1;

                        if ( !lastislongpress ) {
                            button_release_all();
                            rv = 1;
                            lasthpkey = -1; // No key is pressed anymore
                            SDLUIFeedback();
                        }

                        // Stop timer, clear long key press
                        lastticks = -1;
                        lastislongpress = 0;
                    }
                }
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                hpkey = SDLKeyToKey( event.key.keysym.sym );

                if ( hpkey != -1 ) {
                    if ( event.type == SDL_KEYDOWN ) {
                        keyispressed = hpkey;

                        // Avoid pressing if it is already pressed
                        if ( !buttons[ hpkey ].pressed ) {
                            button_pressed( hpkey );
                            rv = 1;
                            SDLUIFeedback();
                        }
                    } else {
                        keyispressed = -1;

                        button_released( hpkey );
                        rv = 1;
                        SDLUIFeedback();
                    }
                }

                break;
        }
    }

    // Display button being pressed, if any
    if ( show_ui_chrome )
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

/* x48_lcd.c */

static inline void draw_nibble( int c, int r, int val ) {
    int x, y;

    x = ( c * 4 ); // x: start in pixels

    if ( r <= display.lines )
        x -= disp.offset;
    y = r; // y: start in pixels

    val &= 0x0f;
    if ( val != lcd_buffer[ r ][ c ] ) {
        lcd_buffer[ r ][ c ] = val;
    }
    if ( val != lcd_buffer[ r ][ c ] )
        lcd_buffer[ r ][ c ] = val;

    SDLDrawNibble( x, y, val );
}

/* void ui__draw_nibble( int c, int r, int val ) { draw_nibble( c, r, val ); }
 */

static inline void draw_row( long addr, int row ) {
    int i, v;
    int line_length;

    line_length = NIBBLES_PER_ROW;
    if ( ( display.offset > 3 ) && ( row <= display.lines ) )
        line_length += 2;
    for ( i = 0; i < line_length; i++ ) {
        v = read_nibble( addr + i );
        if ( v != disp_buf[ row ][ i ] ) {
            disp_buf[ row ][ i ] = v;
            draw_nibble( i, row, v );
        }
    }
}

void ui__update_LCD( void ) {
    int i, j;
    long addr;
    static int old_offset = -1;
    static int old_lines = -1;

    if ( !disp.mapped )
        return;

    if ( display.on ) {
        addr = display.disp_start;
        if ( display.offset != old_offset ) {
            memset( disp_buf, 0xf0,
                    ( size_t )( ( display.lines + 1 ) * NIBS_PER_BUFFER_ROW ) );
            memset( lcd_buffer, 0xf0,
                    ( size_t )( ( display.lines + 1 ) * NIBS_PER_BUFFER_ROW ) );
            old_offset = display.offset;
        }
        if ( display.lines != old_lines ) {
            memset( &disp_buf[ 56 ][ 0 ], 0xf0,
                    ( size_t )( 8 * NIBS_PER_BUFFER_ROW ) );
            memset( &lcd_buffer[ 56 ][ 0 ], 0xf0,
                    ( size_t )( 8 * NIBS_PER_BUFFER_ROW ) );
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
    } else {
        memset( disp_buf, 0xf0, sizeof( disp_buf ) );
        for ( i = 0; i < 64; i++ ) {
            for ( j = 0; j < NIBBLES_PER_ROW; j++ ) {
                draw_nibble( j, i, 0x00 );
            }
        }
    }
}

void ui__disp_draw_nibble( word_20 addr, word_4 val ) {
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
        if ( val != disp_buf[ y ][ x ] ) {
            disp_buf[ y ][ x ] = val;
            draw_nibble( x, y, val );
        }
    } else {
        for ( y = 0; y < display.lines; y++ ) {
            if ( val != disp_buf[ y ][ x ] ) {
                disp_buf[ y ][ x ] = val;
                draw_nibble( x, y, val );
            }
        }
    }
}

void ui__menu_draw_nibble( word_20 addr, word_4 val ) {
    long offset;
    int x, y;

    offset = ( addr - display.menu_start );
    x = offset % NIBBLES_PER_ROW;
    y = display.lines + ( offset / NIBBLES_PER_ROW ) + 1;
    if ( val != disp_buf[ y ][ x ] ) {
        disp_buf[ y ][ x ] = val;
        draw_nibble( x, y, val );
    }
}

void ui__draw_annunc( void ) {
    int val;

    val = display.annunc;

    if ( val == last_annunc_state )
        return;

    last_annunc_state = val;

    char sdl_annuncstate[ 6 ];
    for ( int i = 0; ann_tbl[ i ].bit; i++ )
        sdl_annuncstate[ i ] =
            ( ( ann_tbl[ i ].bit & val ) == ann_tbl[ i ].bit ) ? 1 : 0;

    SDLDrawAnnunc( sdl_annuncstate );
}

void ui__init_LCD( void ) {
    display.on = ( int )( saturn.disp_io & 0x8 ) >> 3;

    display.disp_start = ( saturn.disp_addr & 0xffffe );
    display.offset = ( saturn.disp_io & 0x7 );
    disp.offset = 2 * display.offset;

    display.lines = ( saturn.line_count & 0x3f );
    if ( display.lines == 0 )
        display.lines = 63;
    disp.lines = 2 * display.lines;
    if ( disp.lines < 110 )
        disp.lines = 110;

    if ( display.offset > 3 )
        display.nibs_per_line =
            ( NIBBLES_PER_ROW + saturn.line_offset + 2 ) & 0xfff;
    else
        display.nibs_per_line =
            ( NIBBLES_PER_ROW + saturn.line_offset ) & 0xfff;

    display.disp_end =
        display.disp_start + ( display.nibs_per_line * ( display.lines + 1 ) );

    display.menu_start = saturn.menu_addr;
    display.menu_end = saturn.menu_addr + 0x110;

    display.contrast = saturn.contrast_ctrl;
    display.contrast |= ( ( saturn.disp_test & 0x1 ) << 4 );

    display.annunc = saturn.annunc;

    memset( disp_buf, 0xf0, sizeof( disp_buf ) );
    memset( lcd_buffer, 0xf0, sizeof( lcd_buffer ) );
}
/* \ x48_lcd.c */

void SDLCreateHP( void ) {
    unsigned int width, height;

    if ( show_ui_chrome ) {
        width = KEYBOARD_WIDTH + 2 * SIDE_SKIP;
        height = DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + DISP_KBD_SKIP +
                 KEYBOARD_HEIGHT + BOTTOM_SKIP;
    } else {
        width = KEYBOARD_WIDTH;
        height = DISPLAY_HEIGHT;
    }

    disp.mapped = 1;
    disp.w = DISPLAY_WIDTH;
    disp.h = DISPLAY_HEIGHT;

    keypad.width = width;
    keypad.height = height;

    colors = opt_gx ? colors_gx : colors_sx;

    // we allocate memory for the buttons because we need to modify
    // their coordinates, and we don't want to change the original buttons_gx or
    // buttons_sx
    if ( buttons ) {
        free( buttons );
        buttons = 0;
    }
    buttons = ( button_t* )malloc( sizeof( buttons_gx ) );

    if ( opt_gx )
        memcpy( buttons, buttons_gx, sizeof( buttons_gx ) );
    else
        memcpy( buttons, buttons_sx, sizeof( buttons_sx ) );

    SDLCreateColors();

    if ( show_ui_chrome ) {
        int cut = buttons[ BUTTON_MTH ].y + KEYBOARD_OFFSET_Y - 19;

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

void ui__init( void ) {
    // SDL Initialization
    SDLInit();

    /*
     *  Create the HP-48 window
     */
    SDLCreateHP();

    ui__init_LCD();
}
