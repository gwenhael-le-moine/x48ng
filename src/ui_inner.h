#ifndef _UI_INNER_H
#define _UI_INNER_H 1

#include "emulator.h"

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
    int r, g, b, a;
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

extern color_t colors_sx[ NB_COLORS ];
extern color_t colors_gx[ NB_COLORS ];
#define COLORS ( opt_gx ? colors_gx : colors_sx )

extern button_t buttons_sx[ NB_KEYS ];
extern button_t buttons_gx[ NB_KEYS ];
#define BUTTONS ( opt_gx ? buttons_gx : buttons_sx )

extern ann_struct_t ann_tbl[ NB_ANNUNCIATORS ];

/***********/
/* bitmaps */
/***********/
#define hp_width 96
#define hp_height 24
static unsigned char hp_bitmap[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xf8, 0x9f, 0xfd, 0x3f, 0x60, 0xcc, 0x6f, 0x66, 0x83, 0xdf, 0xff, 0x3f, 0xfc, 0x9f, 0xf1, 0x7f, 0x60, 0xcc,
    0x60, 0x66, 0x83, 0x01, 0x06, 0x06, 0xfc, 0xc7, 0xc0, 0x7f, 0x60, 0xcc, 0x60, 0x66, 0x83, 0x01, 0x06, 0x06, 0xfc, 0xc3, 0x80,
    0x7f, 0x60, 0xcc, 0x40, 0x26, 0x83, 0x01, 0x06, 0x06, 0xfc, 0x61, 0x00, 0x7f, 0xe0, 0xcf, 0xcf, 0x36, 0x83, 0x1f, 0x06, 0x06,
    0xfc, 0x60, 0x00, 0x7e, 0x60, 0xcc, 0xc0, 0x36, 0x83, 0x01, 0x06, 0x06, 0xfc, 0x30, 0x00, 0x7e, 0x60, 0xcc, 0x80, 0x19, 0x83,
    0x01, 0x06, 0x06, 0x7c, 0xb0, 0x68, 0x7c, 0x60, 0xcc, 0x80, 0x19, 0x83, 0x01, 0x06, 0x06, 0x7c, 0xf8, 0xf9, 0x7c, 0x60, 0xcc,
    0x8f, 0x19, 0xbf, 0x1f, 0x06, 0x06, 0x7c, 0x98, 0xcd, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0xcc, 0xcc,
    0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0xcc, 0x66, 0x7c, 0xe0, 0x87, 0x81, 0x67, 0x0c, 0xc3, 0xcf, 0x0f,
    0x7c, 0x66, 0x66, 0x7c, 0x60, 0xcc, 0xc3, 0x6c, 0x86, 0xc7, 0xd8, 0x18, 0x7c, 0x66, 0x3f, 0x7e, 0x60, 0x4c, 0x62, 0x60, 0x83,
    0xc4, 0xd8, 0x30, 0xfc, 0x00, 0x03, 0x7e, 0x60, 0x6c, 0x66, 0xe0, 0xc1, 0xcc, 0xd8, 0x30, 0xfc, 0x80, 0x01, 0x7f, 0xe0, 0x67,
    0x66, 0xe0, 0xc1, 0xcc, 0xcf, 0x30, 0xfc, 0x81, 0x81, 0x7f, 0x60, 0xe0, 0x67, 0x60, 0xc3, 0xcf, 0xcc, 0x30, 0xfc, 0xc3, 0xc0,
    0x7f, 0x60, 0x30, 0x6c, 0x60, 0x66, 0xd8, 0xd8, 0x30, 0xfc, 0xcf, 0xf0, 0x7f, 0x60, 0x30, 0xcc, 0x6c, 0x6c, 0xd8, 0xd8, 0x18,
    0xf8, 0x6f, 0xfe, 0x3f, 0x60, 0x30, 0x8c, 0x67, 0x78, 0xd8, 0xd8, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define hp48sx_width 42
#define hp48sx_height 10
static unsigned char hp48sx_bitmap[] = { 0xe0, 0xf1, 0xc3, 0x3f, 0x87, 0x03, 0xf0, 0xf9, 0xe7, 0x7f, 0xc7, 0x01, 0xf8, 0x39, 0xe7,
                                         0x70, 0xee, 0x00, 0xdc, 0x39, 0xe7, 0x00, 0x7e, 0x00, 0xee, 0xf0, 0xe3, 0x0f, 0x3c, 0x00,
                                         0xe7, 0xf8, 0xc1, 0x1f, 0x1c, 0x00, 0xff, 0x9d, 0x03, 0x1c, 0x3e, 0x00, 0xff, 0x9d, 0x3b,
                                         0x1c, 0x3f, 0x00, 0x70, 0xfc, 0xfb, 0x9f, 0x73, 0x00, 0x70, 0xf8, 0xf1, 0xcf, 0x71, 0x00 };

#define hp48gx_width 44
#define hp48gx_height 14
static unsigned char hp48gx_bitmap[] = {
    0x00, 0xc3, 0x03, 0x7c, 0x0c, 0x0c, 0x80, 0xe3, 0x07, 0xff, 0x0c, 0x0e, 0xc0, 0x33, 0x86, 0xc3, 0x1c, 0x06, 0xe0, 0x31, 0xc6,
    0xc0, 0x18, 0x03, 0xb0, 0x31, 0xe6, 0x00, 0xb0, 0x01, 0x98, 0x31, 0x63, 0x00, 0xf0, 0x01, 0x8c, 0xe1, 0x61, 0x00, 0xe0, 0x00,
    0xc6, 0xb8, 0x31, 0xfc, 0x70, 0x00, 0xc7, 0x18, 0x33, 0xfc, 0xf8, 0x00, 0xff, 0x0d, 0x33, 0x60, 0xd8, 0x00, 0xff, 0x0d, 0x73,
    0x60, 0x8c, 0x01, 0x60, 0x8c, 0x63, 0x30, 0x86, 0x03, 0x60, 0xfc, 0xe1, 0x3f, 0x07, 0x03, 0x60, 0xf8, 0x80, 0x37, 0x03, 0x03 };

#define science_width 131
#define science_height 8
static unsigned char science_bitmap[] = {
    0x38, 0x1c, 0xf2, 0x09, 0x7d, 0x79, 0xe2, 0x80, 0x2f, 0xe4, 0x41, 0x08, 0x79, 0x20, 0x3c, 0xc2, 0x07, 0x44, 0x22, 0x12,
    0x08, 0x11, 0x09, 0x12, 0x81, 0x20, 0x22, 0x62, 0x08, 0x89, 0x30, 0x44, 0x42, 0x00, 0x02, 0x01, 0x09, 0x94, 0x88, 0x04,
    0x09, 0x40, 0x40, 0x11, 0x52, 0x94, 0x88, 0x28, 0x42, 0x21, 0x00, 0x1c, 0x01, 0xf9, 0x94, 0x88, 0x3c, 0x09, 0xc0, 0xc7,
    0xf0, 0x51, 0x94, 0x84, 0x28, 0x3e, 0xe1, 0x03, 0xa0, 0x00, 0x09, 0x94, 0x88, 0x04, 0x05, 0x40, 0xc0, 0x10, 0x48, 0x94,
    0x44, 0x24, 0x22, 0x21, 0x00, 0xa1, 0xa0, 0x04, 0xa2, 0x44, 0x82, 0x04, 0x21, 0xa0, 0x08, 0xfc, 0xa2, 0x42, 0x7e, 0xa1,
    0x10, 0x00, 0x91, 0x90, 0x04, 0x42, 0x44, 0x82, 0x84, 0x20, 0x10, 0x09, 0x84, 0x42, 0x22, 0x42, 0xb1, 0x10, 0x00, 0x0e,
    0x8f, 0x7c, 0x42, 0x44, 0x82, 0x78, 0xe0, 0x0b, 0x09, 0x82, 0x42, 0x1e, 0x41, 0x9f, 0xf7, 0x01 };

#define gx_128K_ram_x_hot 1
#define gx_128K_ram_y_hot 8
#define gx_128K_ram_width 43
#define gx_128K_ram_height 31
static unsigned char gx_128K_ram_bitmap[] = {
    0xfe, 0xdf, 0xff, 0xff, 0x03, 0x00, 0xfe, 0xdf, 0xff, 0xff, 0x03, 0x00, 0xfe, 0xdf, 0xff, 0xff, 0x03, 0x00, 0xe2, 0xdf, 0xff,
    0xff, 0x03, 0x00, 0x9c, 0xdf, 0xff, 0xff, 0x03, 0x00, 0x7e, 0xdf, 0xff, 0xff, 0x01, 0x00, 0x7e, 0xdf, 0xff, 0xff, 0x01, 0x00,
    0xfe, 0xde, 0xff, 0xff, 0x02, 0x00, 0xfe, 0xde, 0xff, 0xff, 0x02, 0x00, 0xfe, 0xdd, 0xff, 0x7f, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xfe, 0xdd, 0xff, 0xbf, 0x03, 0x00, 0xfe, 0xdb, 0xff, 0xdf, 0x03, 0x00, 0xfe, 0xdb, 0xff, 0xef, 0x03, 0x00,
    0xfe, 0xd7, 0xff, 0xf7, 0x03, 0x00, 0xfe, 0xcf, 0xff, 0xfb, 0x03, 0x00, 0xfe, 0xcf, 0xff, 0xfc, 0x03, 0x00, 0xfe, 0x1f, 0x3f,
    0xff, 0x03, 0x00, 0xfe, 0xdf, 0xc0, 0xff, 0x03, 0x00, 0xfe, 0xdf, 0xff, 0xff, 0x03, 0x00, 0xfe, 0xdf, 0xff, 0xff, 0x03, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xc4, 0x30, 0x12, 0x1c, 0x44, 0x04, 0x27, 0x49, 0x0a, 0x24, 0x46, 0x04, 0x84, 0x39, 0x06, 0x24, 0xc9, 0x06,
    0x62, 0x24, 0x07, 0x9e, 0xaf, 0x06, 0x12, 0x24, 0x09, 0x92, 0xa8, 0x05, 0xf2, 0x18, 0x11, 0x52, 0x28, 0x05 };

#define gx_silver_x_hot 0
#define gx_silver_y_hot 8
#define gx_silver_width 35
#define gx_silver_height 21
static unsigned char gx_silver_bitmap[] = {
    0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x38, 0x40, 0x00, 0x00, 0x00, 0xc4,
    0x40, 0x00, 0x00, 0x00, 0x02, 0x41, 0x00, 0x00, 0x04, 0x02, 0x41, 0x00, 0x00, 0x04, 0x02, 0x42, 0x00, 0x00, 0x02, 0x01, 0x42,
    0x00, 0x00, 0x02, 0x01, 0x44, 0x00, 0x00, 0x01, 0xfd, 0xff, 0xff, 0xff, 0x07, 0x01, 0x44, 0x00, 0x80, 0x00, 0x01, 0x48, 0x00,
    0x40, 0x00, 0x01, 0x48, 0x00, 0x20, 0x00, 0x00, 0x50, 0x00, 0x10, 0x00, 0x00, 0x60, 0x00, 0x08, 0x00, 0x00, 0x60, 0x00, 0x06,
    0x00, 0x00, 0xc0, 0x81, 0x01, 0x00, 0x00, 0x40, 0x7e, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00 };

#define gx_green_x_hot 11
#define gx_green_y_hot 0
#define gx_green_width 34
#define gx_green_height 22
static unsigned char gx_green_bitmap[] = {
    0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
    0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0x03, 0x00, 0x00, 0x00, 0xfc, 0x03, 0x00, 0x00, 0x00, 0xfc, 0x03, 0x00, 0x00, 0x00, 0xfc,
    0x03, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x00,
    0x00, 0x00, 0xf0, 0x03, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x00, 0x00, 0x00,
    0xf0, 0x03, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x00, 0x00, 0x00, 0xf0, 0x03 };

#define ann_alpha_width 15
#define ann_alpha_height 12
static unsigned char ann_alpha_bitmap[] = { 0xe0, 0x03, 0x18, 0x44, 0x0c, 0x4c, 0x06, 0x2c, 0x07, 0x2c, 0x07, 0x1c,
                                            0x07, 0x0c, 0x07, 0x0c, 0x07, 0x0e, 0x0e, 0x4d, 0xf8, 0x38, 0x00, 0x00 };

#define ann_battery_width 15
#define ann_battery_height 12
static unsigned char ann_battery_bitmap[] = { 0x04, 0x10, 0x02, 0x20, 0x12, 0x24, 0x09, 0x48, 0xc9, 0x49, 0xc9, 0x49,
                                              0xc9, 0x49, 0x09, 0x48, 0x12, 0x24, 0x02, 0x20, 0x04, 0x10, 0x00, 0x00 };

#define ann_busy_width 15
#define ann_busy_height 12
static unsigned char ann_busy_bitmap[] = { 0xfc, 0x1f, 0x08, 0x08, 0x08, 0x08, 0xf0, 0x07, 0xe0, 0x03, 0xc0, 0x01,
                                           0x40, 0x01, 0x20, 0x02, 0x10, 0x04, 0xc8, 0x09, 0xe8, 0x0b, 0xfc, 0x1f };

#define ann_io_width 15
#define ann_io_height 12
static unsigned char ann_io_bitmap[] = { 0x0c, 0x00, 0x1e, 0x00, 0x33, 0x0c, 0x61, 0x18, 0xcc, 0x30, 0xfe, 0x7f,
                                         0xfe, 0x7f, 0xcc, 0x30, 0x61, 0x18, 0x33, 0x0c, 0x1e, 0x00, 0x0c, 0x00 };

#define ann_left_width 15
#define ann_left_height 12
static unsigned char ann_left_bitmap[] = { 0xfe, 0x3f, 0xff, 0x7f, 0x9f, 0x7f, 0xcf, 0x7f, 0xe7, 0x7f, 0x03, 0x78,
                                           0x03, 0x70, 0xe7, 0x73, 0xcf, 0x73, 0x9f, 0x73, 0xff, 0x73, 0xfe, 0x33 };

#define ann_right_width 15
#define ann_right_height 12
static unsigned char ann_right_bitmap[] = { 0xfe, 0x3f, 0xff, 0x7f, 0xff, 0x7c, 0xff, 0x79, 0xff, 0x73, 0x0f, 0x60,
                                            0x07, 0x60, 0xe7, 0x73, 0xe7, 0x79, 0xe7, 0x7c, 0xe7, 0x7f, 0xe6, 0x3f };

/* #endif /\* !_ANNUNC_H *\/ */

/* #ifndef _BUTTONS_H */
/* #define _BUTTONS_H 1 */

#define menu_label_width 24
#define menu_label_height 11
static unsigned char menu_label_bitmap[] = { 0xfe, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                             0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                             0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0x7f };

#define up_width 11
#define up_height 11
static unsigned char up_bitmap[] = { 0x20, 0x00, 0x20, 0x00, 0x70, 0x00, 0x70, 0x00, 0xf8, 0x00, 0xf8,
                                     0x00, 0xfc, 0x01, 0xfc, 0x01, 0xfe, 0x03, 0xfe, 0x03, 0xff, 0x07 };

#define down_width 11
#define down_height 11
static unsigned char down_bitmap[] = { 0xff, 0x07, 0xfe, 0x03, 0xfe, 0x03, 0xfc, 0x01, 0xfc, 0x01, 0xf8,
                                       0x00, 0xf8, 0x00, 0x70, 0x00, 0x70, 0x00, 0x20, 0x00, 0x20, 0x00 };

#define left_width 11
#define left_height 11
static unsigned char left_bitmap[] = { 0x00, 0x04, 0x00, 0x07, 0xc0, 0x07, 0xf0, 0x07, 0xfc, 0x07, 0xff,
                                       0x07, 0xfc, 0x07, 0xf0, 0x07, 0xc0, 0x07, 0x00, 0x07, 0x00, 0x04 };

#define right_width 11
#define right_height 11
static unsigned char right_bitmap[] = { 0x01, 0x00, 0x07, 0x00, 0x1f, 0x00, 0x7f, 0x00, 0xff, 0x01, 0xff,
                                        0x07, 0xff, 0x01, 0x7f, 0x00, 0x1f, 0x00, 0x07, 0x00, 0x01, 0x00 };

#define sqrt_width 20
#define sqrt_height 11
static unsigned char sqrt_bitmap[] = { 0x00, 0xff, 0x0f, 0x00, 0x01, 0x08, 0x00, 0x01, 0x08, 0x80, 0x8c, 0x01, 0x80, 0x58, 0x01, 0x80, 0x38,
                                       0x00, 0x47, 0x30, 0x00, 0x4c, 0x30, 0x00, 0x58, 0x78, 0x00, 0x30, 0x6a, 0x01, 0x20, 0xc6, 0x00 };

#define power_width 17
#define power_height 14
static unsigned char power_bitmap[] = { 0x00, 0x8c, 0x01, 0x00, 0x58, 0x01, 0x00, 0x38, 0x00, 0xc8, 0x30, 0x00, 0x9c, 0x30,
                                        0x00, 0x98, 0x78, 0x00, 0x58, 0x6a, 0x01, 0x58, 0xc6, 0x00, 0x38, 0x00, 0x00, 0x30,
                                        0x00, 0x00, 0x10, 0x00, 0x00, 0x08, 0x00, 0x00, 0x05, 0x00, 0x00, 0x03, 0x00, 0x00 };

#define inv_width 18
#define inv_height 13
static unsigned char inv_bitmap[] = { 0x0c, 0x04, 0x00, 0x0f, 0x06, 0x00, 0x0c, 0x02, 0x00, 0x0c, 0x03, 0x00, 0x0c,
                                      0x01, 0x00, 0x8c, 0x19, 0x03, 0x8c, 0xb0, 0x02, 0xcc, 0x70, 0x00, 0x40, 0x60,
                                      0x00, 0x60, 0x60, 0x00, 0x20, 0xf0, 0x00, 0x30, 0xd4, 0x02, 0x10, 0x8c, 0x01 };

#define neg_width 21
#define neg_height 11
static unsigned char neg_bitmap[] = { 0x18, 0x00, 0x00, 0x18, 0x30, 0x00, 0x18, 0x30, 0x00, 0xff, 0x18, 0x00, 0xff, 0x18, 0x00, 0x18, 0x0c,
                                      0x00, 0x18, 0x0c, 0x00, 0x18, 0xc6, 0x1f, 0x00, 0xc6, 0x1f, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00 };

#define bs_width 11
#define bs_height 11
static unsigned char bs_bitmap[] = { 0x20, 0x00, 0x30, 0x00, 0x38, 0x00, 0xfc, 0x07, 0xfe, 0x07, 0xff,
                                     0x07, 0xfe, 0x07, 0xfc, 0x07, 0x38, 0x00, 0x30, 0x00, 0x20, 0x00 };

#define alpha_width 12
#define alpha_height 10
static unsigned char alpha_bitmap[] = { 0x78, 0x00, 0x84, 0x08, 0x82, 0x09, 0x83, 0x05, 0x83, 0x05,
                                        0x83, 0x03, 0x83, 0x01, 0x83, 0x01, 0x46, 0x09, 0x3c, 0x06 };

#define div_width 10
#define div_height 10
static unsigned char div_bitmap[] = { 0x30, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03,
                                      0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x30, 0x00 };

#define shl_width 24
#define shl_height 14
static unsigned char shl_bitmap[] = { 0xfe, 0xff, 0x7f, 0xff, 0xfc, 0xff, 0x7f, 0xfc, 0xff, 0x3f, 0xfe, 0xff, 0x1f, 0xff,
                                      0xff, 0x0f, 0x00, 0xfc, 0x07, 0x00, 0xf8, 0x0f, 0x00, 0xf0, 0x1f, 0xff, 0xf1, 0x3f,
                                      0xfe, 0xf1, 0x7f, 0xfc, 0xf1, 0xff, 0xfc, 0xf1, 0xff, 0xff, 0xf1, 0xfe, 0xff, 0x71 };

#define mul_width 10
#define mul_height 10
static unsigned char mul_bitmap[] = { 0x03, 0x03, 0x87, 0x03, 0xce, 0x01, 0xfc, 0x00, 0x78, 0x00,
                                      0x78, 0x00, 0xfc, 0x00, 0xce, 0x01, 0x87, 0x03, 0x03, 0x03 };

#define shr_width 24
#define shr_height 14
static unsigned char shr_bitmap[] = { 0xfe, 0xff, 0x7f, 0xff, 0x3f, 0xff, 0xff, 0x3f, 0xfe, 0xff, 0x7f, 0xfc, 0xff, 0xff,
                                      0xf8, 0x3f, 0x00, 0xf0, 0x1f, 0x00, 0xe0, 0x0f, 0x00, 0xf0, 0x8f, 0xff, 0xf8, 0x8f,
                                      0x7f, 0xfc, 0x8f, 0x3f, 0xfe, 0x8f, 0x3f, 0xff, 0x8f, 0xff, 0xff, 0x8e, 0xff, 0x7f };

#define minus_width 10
#define minus_height 10
static unsigned char minus_bitmap[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03,
                                        0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define plus_width 10
#define plus_height 10
static unsigned char plus_bitmap[] = { 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0xff, 0x03,
                                       0xff, 0x03, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00 };

#define colon_width 2
#define colon_height 10
static unsigned char colon_bitmap[] = { 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/* Below used only for X11 */
#define last_width 120
#define last_height 6
static unsigned char last_bitmap[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xc6, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x29, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x11, 0x49, 0x08, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                       0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x8f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
                                       0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x29, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
                                       0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0xc9, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80 };

#define small_ascent 8
#define small_descent 4

#define blank_width 4
#define blank_height 7
static unsigned char blank_bitmap[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define hash_width 5
#define hash_height 7
static unsigned char hash_bitmap[] = { 0x00, 0x0a, 0x1f, 0x0a, 0x0a, 0x1f, 0x0a };

#define lbrace_width 3
#define lbrace_height 7
static unsigned char lbrace_bitmap[] = { 0x04, 0x02, 0x01, 0x01, 0x01, 0x02, 0x04 };

#define rbrace_width 3
#define rbrace_height 7
static unsigned char rbrace_bitmap[] = { 0x01, 0x02, 0x04, 0x04, 0x04, 0x02, 0x01 };

#define comma_width 3
#define comma_height 7
static unsigned char comma_bitmap[] = { 0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x03 };

#define slash_width 3
#define slash_height 7
static unsigned char slash_bitmap[] = { 0x04, 0x04, 0x02, 0x02, 0x02, 0x01, 0x01 };

#define two_width 5
#define two_height 7
static unsigned char two_bitmap[] = { 0x0e, 0x11, 0x10, 0x08, 0x04, 0x02, 0x1f };

#define three_width 5
#define three_height 7
static unsigned char three_bitmap[] = { 0x0e, 0x11, 0x10, 0x0c, 0x10, 0x11, 0x0e };

#define small_colon_width 2
#define small_colon_height 7
static unsigned char small_colon_bitmap[] = { 0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x00 };

#define A_width 5
#define A_height 7
static unsigned char A_bitmap[] = { 0x0e, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11 };

#define B_width 5
#define B_height 7
static unsigned char B_bitmap[] = { 0x0f, 0x11, 0x11, 0x0f, 0x11, 0x11, 0x0f };

#define C_width 5
#define C_height 7
static unsigned char C_bitmap[] = { 0x0e, 0x11, 0x01, 0x01, 0x01, 0x11, 0x0e };

#define D_width 5
#define D_height 7
static unsigned char D_bitmap[] = { 0x0f, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0f };

#define E_width 5
#define E_height 7
static unsigned char E_bitmap[] = { 0x1f, 0x01, 0x01, 0x0f, 0x01, 0x01, 0x1f };

#define F_width 5
#define F_height 7
static unsigned char F_bitmap[] = { 0x1f, 0x01, 0x01, 0x0f, 0x01, 0x01, 0x01 };

#define G_width 5
#define G_height 7
static unsigned char G_bitmap[] = { 0x0e, 0x11, 0x01, 0x01, 0x19, 0x11, 0x0e };

#define H_width 5
#define H_height 7
static unsigned char H_bitmap[] = { 0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11 };

#define I_width 1
#define I_height 7
static unsigned char I_bitmap[] = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 };

#define J_width 4
#define J_height 7
static unsigned char J_bitmap[] = { 0x08, 0x08, 0x08, 0x08, 0x08, 0x09, 0x06 };

#define K_width 5
#define K_height 7
static unsigned char K_bitmap[] = { 0x11, 0x09, 0x05, 0x03, 0x05, 0x09, 0x11 };

#define L_width 4
#define L_height 7
static unsigned char L_bitmap[] = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x0f };

#define M_width 5
#define M_height 7
static unsigned char M_bitmap[] = { 0x11, 0x1b, 0x15, 0x11, 0x11, 0x11, 0x11 };

#define N_width 5
#define N_height 7
static unsigned char N_bitmap[] = { 0x11, 0x11, 0x13, 0x15, 0x19, 0x11, 0x11 };

#define O_width 5
#define O_height 7
static unsigned char O_bitmap[] = { 0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e };

#define P_width 5
#define P_height 7
static unsigned char P_bitmap[] = { 0x0f, 0x11, 0x11, 0x0f, 0x01, 0x01, 0x01 };

#define Q_width 5
#define Q_height 7
static unsigned char Q_bitmap[] = { 0x0e, 0x11, 0x11, 0x11, 0x15, 0x09, 0x16 };

#define R_width 5
#define R_height 7
static unsigned char R_bitmap[] = { 0x0f, 0x11, 0x11, 0x0f, 0x05, 0x09, 0x11 };

#define S_width 5
#define S_height 7
static unsigned char S_bitmap[] = { 0x0e, 0x11, 0x01, 0x0e, 0x10, 0x11, 0x0e };

#define T_width 5
#define T_height 7
static unsigned char T_bitmap[] = { 0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 };

#define U_width 5
#define U_height 7
static unsigned char U_bitmap[] = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e };

#define V_width 5
#define V_height 7
static unsigned char V_bitmap[] = { 0x11, 0x11, 0x11, 0x11, 0x0a, 0x0a, 0x04 };

#define W_width 5
#define W_height 7
static unsigned char W_bitmap[] = { 0x11, 0x11, 0x11, 0x11, 0x15, 0x1b, 0x11 };

#define X_width 5
#define X_height 7
static unsigned char X_bitmap[] = { 0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11 };

#define Y_width 5
#define Y_height 7
static unsigned char Y_bitmap[] = { 0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04 };

#define Z_width 5
#define Z_height 7
static unsigned char Z_bitmap[] = { 0x1f, 0x10, 0x08, 0x04, 0x02, 0x01, 0x1f };

#define lbracket_width 3
#define lbracket_height 7
static unsigned char lbracket_bitmap[] = { 0x07, 0x01, 0x01, 0x01, 0x01, 0x01, 0x07 };

#define rbracket_width 3
#define rbracket_height 7
static unsigned char rbracket_bitmap[] = { 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07 };

#define arrow_width 7
#define arrow_height 7
static unsigned char arrow_bitmap[] = { 0x00, 0x08, 0x18, 0x3f, 0x18, 0x08, 0x00 };

#define diff_width 5
#define diff_height 7
static unsigned char diff_bitmap[] = { 0x0e, 0x10, 0x10, 0x1e, 0x11, 0x11, 0x0e };

#define integral_width 5
#define integral_height 8
static unsigned char integral_bitmap[] = { 0x0c, 0x12, 0x02, 0x04, 0x04, 0x08, 0x09, 0x06 };

#define sigma_width 6
#define sigma_height 9
static unsigned char sigma_bitmap[] = { 0x3f, 0x21, 0x02, 0x04, 0x08, 0x04, 0x02, 0x21, 0x3f };

#define sqr_width 11
#define sqr_height 10
static unsigned char sqr_bitmap[] = { 0x00, 0x03, 0x80, 0x04, 0x00, 0x04, 0x00, 0x02, 0x26, 0x01,
                                      0x94, 0x07, 0x08, 0x00, 0x14, 0x00, 0x53, 0x00, 0x21, 0x00 };

#define root_width 18
#define root_height 13
static unsigned char root_bitmap[] = { 0x26, 0x00, 0x00, 0x14, 0x00, 0x00, 0x08, 0xfe, 0x03, 0x14, 0x02, 0x02, 0x53,
                                       0x02, 0x00, 0x21, 0x99, 0x00, 0x00, 0x91, 0x00, 0x10, 0x91, 0x00, 0xa0, 0x50,
                                       0x00, 0xc0, 0x60, 0x00, 0x80, 0x20, 0x00, 0x00, 0x14, 0x00, 0x00, 0x0c, 0x00 };

#define pow10_width 13
#define pow10_height 9
static unsigned char pow10_bitmap[] = { 0x00, 0x12, 0x00, 0x0c, 0x32, 0x04, 0x4b, 0x0a, 0x4a,
                                        0x09, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x32, 0x00 };

#define exp_width 11
#define exp_height 9
static unsigned char exp_bitmap[] = { 0x80, 0x04, 0x00, 0x03, 0x00, 0x01, 0x8c, 0x02, 0x52,
                                      0x02, 0x09, 0x00, 0x07, 0x00, 0x21, 0x00, 0x1e, 0x00 };

#define under_width 6
#define under_height 7
static unsigned char under_bitmap[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f };

#define prog_width 16
#define prog_height 7
static unsigned char prog_bitmap[] = { 0x48, 0x12, 0x24, 0x24, 0x12, 0x48, 0x09, 0x90, 0x12, 0x48, 0x24, 0x24, 0x48, 0x12 };

#define string_width 10
#define string_height 7
static unsigned char string_bitmap[] = { 0x85, 0x02, 0x85, 0x02, 0x85, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define equal_width 5
#define equal_height 7
static unsigned char equal_bitmap[] = { 0x00, 0x1f, 0x00, 0x00, 0x1f, 0x00, 0x00 };

#define nl_width 8
#define nl_height 7
static unsigned char nl_bitmap[] = { 0x00, 0x84, 0x86, 0xff, 0x06, 0x04, 0x00 };

#define pi_width 6
#define pi_height 7
static unsigned char pi_bitmap[] = { 0x20, 0x1f, 0x12, 0x12, 0x12, 0x12, 0x12 };

#define angle_width 8
#define angle_height 7
static unsigned char angle_bitmap[] = { 0x40, 0x20, 0x10, 0x28, 0x44, 0x42, 0xff };

#define lcurly_width 5
#define lcurly_height 7
static unsigned char lcurly_bitmap[] = { 0x18, 0x04, 0x04, 0x02, 0x04, 0x04, 0x18 };

#define rcurly_width 5
#define rcurly_height 7
static unsigned char rcurly_bitmap[] = { 0x03, 0x04, 0x04, 0x08, 0x04, 0x04, 0x03 };

#define sqr_gx_width 11
#define sqr_gx_height 13
static unsigned char sqr_gx_bitmap[] = { 0x00, 0x03, 0x80, 0x04, 0x00, 0x04, 0x00, 0x02, 0x00, 0x01, 0x80, 0x07, 0x00,
                                         0x00, 0x66, 0x00, 0x14, 0x00, 0x08, 0x00, 0x14, 0x00, 0x53, 0x00, 0x21, 0x00 };

#define root_gx_width 18
#define root_gx_height 15
static unsigned char root_gx_bitmap[] = { 0x66, 0x00, 0x00, 0x14, 0x00, 0x00, 0x08, 0x00, 0x00, 0x14, 0x00, 0x00, 0x53, 0xfe, 0x03,
                                          0x21, 0x02, 0x02, 0x00, 0x02, 0x00, 0x00, 0x99, 0x00, 0x00, 0x91, 0x00, 0x10, 0x91, 0x00,
                                          0xa0, 0x50, 0x00, 0xc0, 0x60, 0x00, 0x80, 0x20, 0x00, 0x00, 0x14, 0x00, 0x00, 0x0c, 0x00 };

#define pow10_gx_width 13
#define pow10_gx_height 12
static unsigned char pow10_gx_bitmap[] = { 0x00, 0x12, 0x00, 0x0c, 0x00, 0x04, 0x00, 0x0a, 0x00, 0x09, 0x32, 0x00,
                                           0x4b, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x32, 0x00 };

#define exp_gx_width 13
#define exp_gx_height 12
static unsigned char exp_gx_bitmap[] = { 0x00, 0xfb, 0x00, 0xf6, 0x00, 0xe6, 0x00, 0xf6, 0x80, 0xed, 0x18, 0xe0,
                                         0x36, 0xe0, 0x36, 0xe0, 0x1f, 0xe0, 0x03, 0xe0, 0x13, 0xe0, 0x0e, 0xe0 };
#define parens_gx_width 20
#define parens_gx_height 12
static unsigned char parens_gx_bitmap[] = { 0x0c, 0x00, 0x03, 0x06, 0x00, 0x06, 0x06, 0x00, 0x06, 0x03, 0x00, 0x0c,
                                            0x03, 0x00, 0x0c, 0x03, 0x00, 0x0c, 0x03, 0x00, 0x0c, 0x03, 0x00, 0x0c,
                                            0x03, 0x00, 0x0c, 0x06, 0x00, 0x06, 0x06, 0x00, 0x06, 0x0c, 0x00, 0x03 };

#define hash_gx_width 8
#define hash_gx_height 12
static unsigned char hash_gx_bitmap[] = { 0x00, 0x00, 0x48, 0x48, 0xfe, 0x24, 0x24, 0x7f, 0x12, 0x12, 0x00, 0x00 };

#define bracket_gx_width 12
#define bracket_gx_height 12
static unsigned char bracket_gx_bitmap[] = { 0x0f, 0x0f, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c,
                                             0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x0f, 0x0f };

#define under_gx_width 10
#define under_gx_height 12
static unsigned char under_gx_bitmap[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03, 0xff, 0x03 };

#define prog_gx_width 24
#define prog_gx_height 12
static unsigned char prog_gx_bitmap[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0xc3, 0x18,
                                          0x8c, 0x81, 0x31, 0xc6, 0x00, 0x63, 0x63, 0x00, 0xc6, 0xc6, 0x00, 0x63,
                                          0x8c, 0x81, 0x31, 0x18, 0xc3, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define quote_gx_width 12
#define quote_gx_height 12
static unsigned char quote_gx_bitmap[] = { 0x05, 0x0a, 0x05, 0x0a, 0x05, 0x0a, 0x05, 0x0a, 0x00, 0x00, 0x00, 0x00,
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define curly_gx_width 14
#define curly_gx_height 12
static unsigned char curly_gx_bitmap[] = { 0x0c, 0x0c, 0x06, 0x18, 0x06, 0x18, 0x06, 0x18, 0x06, 0x18, 0x03, 0x30,
                                           0x06, 0x18, 0x06, 0x18, 0x06, 0x18, 0x06, 0x18, 0x06, 0x18, 0x0c, 0x0c };

#define colon_gx_width 8
#define colon_gx_height 12
static unsigned char colon_gx_bitmap[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0xc3, 0xc3, 0x00, 0x00, 0xc3, 0xc3, 0x00 };

#define angle_gx_width 12
#define angle_gx_height 12
static unsigned char angle_gx_bitmap[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01, 0xc0, 0x00, 0xe0, 0x01,
                                           0xb0, 0x03, 0x18, 0x03, 0x0c, 0x03, 0x06, 0x03, 0xff, 0x0f, 0xff, 0x0f };

#define pi_gx_width 10
#define pi_gx_height 12
static unsigned char pi_gx_bitmap[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xfe, 0x03, 0xff, 0x01,
                                        0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00 };

#define nl_gx_width 18
#define nl_gx_height 12
static unsigned char nl_gx_bitmap[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xf0, 0x00, 0x03,
                                        0xfc, 0x00, 0x03, 0xff, 0xff, 0x03, 0xff, 0xff, 0x03, 0xfc, 0x00, 0x00,
                                        0xf0, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define comma_gx_width 3
#define comma_gx_height 12
static unsigned char comma_gx_bitmap[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0x07, 0x04, 0x04, 0x02 };

#define arrow_gx_width 18
#define arrow_gx_height 12
static unsigned char arrow_gx_bitmap[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x3c, 0x00,
                                           0x00, 0xfc, 0x00, 0xff, 0xff, 0x03, 0xff, 0xff, 0x03, 0x00, 0xfc, 0x00,
                                           0x00, 0x3c, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define equal_gx_width 8
#define equal_gx_height 12
static unsigned char equal_gx_bitmap[] = { 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00 };

/*************/
/* functions */
/*************/
extern int SmallTextWidth( const char* string, unsigned int length );
extern void ui_init_LCD( void );

#endif /* _UI_INNER_H */
