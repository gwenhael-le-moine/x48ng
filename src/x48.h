#ifndef _X48_GUI_H
#define _X48_GUI_H 1

#include "hp48.h" /* word_20, word_4 */

#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_rotozoom.h>

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

/* #ifndef _HP_H */
/* #define _HP_H 1 */

#define hp_width 96
#define hp_height 24
static unsigned char hp_bitmap[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xf8, 0x9f, 0xfd, 0x3f, 0x60, 0xcc, 0x6f, 0x66, 0x83, 0xdf, 0xff, 0x3f,
    0xfc, 0x9f, 0xf1, 0x7f, 0x60, 0xcc, 0x60, 0x66, 0x83, 0x01, 0x06, 0x06,
    0xfc, 0xc7, 0xc0, 0x7f, 0x60, 0xcc, 0x60, 0x66, 0x83, 0x01, 0x06, 0x06,
    0xfc, 0xc3, 0x80, 0x7f, 0x60, 0xcc, 0x40, 0x26, 0x83, 0x01, 0x06, 0x06,
    0xfc, 0x61, 0x00, 0x7f, 0xe0, 0xcf, 0xcf, 0x36, 0x83, 0x1f, 0x06, 0x06,
    0xfc, 0x60, 0x00, 0x7e, 0x60, 0xcc, 0xc0, 0x36, 0x83, 0x01, 0x06, 0x06,
    0xfc, 0x30, 0x00, 0x7e, 0x60, 0xcc, 0x80, 0x19, 0x83, 0x01, 0x06, 0x06,
    0x7c, 0xb0, 0x68, 0x7c, 0x60, 0xcc, 0x80, 0x19, 0x83, 0x01, 0x06, 0x06,
    0x7c, 0xf8, 0xf9, 0x7c, 0x60, 0xcc, 0x8f, 0x19, 0xbf, 0x1f, 0x06, 0x06,
    0x7c, 0x98, 0xcd, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7c, 0xcc, 0xcc, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7c, 0xcc, 0x66, 0x7c, 0xe0, 0x87, 0x81, 0x67, 0x0c, 0xc3, 0xcf, 0x0f,
    0x7c, 0x66, 0x66, 0x7c, 0x60, 0xcc, 0xc3, 0x6c, 0x86, 0xc7, 0xd8, 0x18,
    0x7c, 0x66, 0x3f, 0x7e, 0x60, 0x4c, 0x62, 0x60, 0x83, 0xc4, 0xd8, 0x30,
    0xfc, 0x00, 0x03, 0x7e, 0x60, 0x6c, 0x66, 0xe0, 0xc1, 0xcc, 0xd8, 0x30,
    0xfc, 0x80, 0x01, 0x7f, 0xe0, 0x67, 0x66, 0xe0, 0xc1, 0xcc, 0xcf, 0x30,
    0xfc, 0x81, 0x81, 0x7f, 0x60, 0xe0, 0x67, 0x60, 0xc3, 0xcf, 0xcc, 0x30,
    0xfc, 0xc3, 0xc0, 0x7f, 0x60, 0x30, 0x6c, 0x60, 0x66, 0xd8, 0xd8, 0x30,
    0xfc, 0xcf, 0xf0, 0x7f, 0x60, 0x30, 0xcc, 0x6c, 0x6c, 0xd8, 0xd8, 0x18,
    0xf8, 0x6f, 0xfe, 0x3f, 0x60, 0x30, 0x8c, 0x67, 0x78, 0xd8, 0xd8, 0x0f,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define hp48sx_width 42
#define hp48sx_height 10
static unsigned char hp48sx_bitmap[] = {
    0xe0, 0xf1, 0xc3, 0x3f, 0x87, 0x03, 0xf0, 0xf9, 0xe7, 0x7f, 0xc7, 0x01,
    0xf8, 0x39, 0xe7, 0x70, 0xee, 0x00, 0xdc, 0x39, 0xe7, 0x00, 0x7e, 0x00,
    0xee, 0xf0, 0xe3, 0x0f, 0x3c, 0x00, 0xe7, 0xf8, 0xc1, 0x1f, 0x1c, 0x00,
    0xff, 0x9d, 0x03, 0x1c, 0x3e, 0x00, 0xff, 0x9d, 0x3b, 0x1c, 0x3f, 0x00,
    0x70, 0xfc, 0xfb, 0x9f, 0x73, 0x00, 0x70, 0xf8, 0xf1, 0xcf, 0x71, 0x00 };

#define hp48gx_width 44
#define hp48gx_height 14
static unsigned char hp48gx_bitmap[] = {
    0x00, 0xc3, 0x03, 0x7c, 0x0c, 0x0c, 0x80, 0xe3, 0x07, 0xff, 0x0c, 0x0e,
    0xc0, 0x33, 0x86, 0xc3, 0x1c, 0x06, 0xe0, 0x31, 0xc6, 0xc0, 0x18, 0x03,
    0xb0, 0x31, 0xe6, 0x00, 0xb0, 0x01, 0x98, 0x31, 0x63, 0x00, 0xf0, 0x01,
    0x8c, 0xe1, 0x61, 0x00, 0xe0, 0x00, 0xc6, 0xb8, 0x31, 0xfc, 0x70, 0x00,
    0xc7, 0x18, 0x33, 0xfc, 0xf8, 0x00, 0xff, 0x0d, 0x33, 0x60, 0xd8, 0x00,
    0xff, 0x0d, 0x73, 0x60, 0x8c, 0x01, 0x60, 0x8c, 0x63, 0x30, 0x86, 0x03,
    0x60, 0xfc, 0xe1, 0x3f, 0x07, 0x03, 0x60, 0xf8, 0x80, 0x37, 0x03, 0x03 };

#define science_width 131
#define science_height 8
static unsigned char science_bitmap[] = {
    0x38, 0x1c, 0xf2, 0x09, 0x7d, 0x79, 0xe2, 0x80, 0x2f, 0xe4, 0x41, 0x08,
    0x79, 0x20, 0x3c, 0xc2, 0x07, 0x44, 0x22, 0x12, 0x08, 0x11, 0x09, 0x12,
    0x81, 0x20, 0x22, 0x62, 0x08, 0x89, 0x30, 0x44, 0x42, 0x00, 0x02, 0x01,
    0x09, 0x94, 0x88, 0x04, 0x09, 0x40, 0x40, 0x11, 0x52, 0x94, 0x88, 0x28,
    0x42, 0x21, 0x00, 0x1c, 0x01, 0xf9, 0x94, 0x88, 0x3c, 0x09, 0xc0, 0xc7,
    0xf0, 0x51, 0x94, 0x84, 0x28, 0x3e, 0xe1, 0x03, 0xa0, 0x00, 0x09, 0x94,
    0x88, 0x04, 0x05, 0x40, 0xc0, 0x10, 0x48, 0x94, 0x44, 0x24, 0x22, 0x21,
    0x00, 0xa1, 0xa0, 0x04, 0xa2, 0x44, 0x82, 0x04, 0x21, 0xa0, 0x08, 0xfc,
    0xa2, 0x42, 0x7e, 0xa1, 0x10, 0x00, 0x91, 0x90, 0x04, 0x42, 0x44, 0x82,
    0x84, 0x20, 0x10, 0x09, 0x84, 0x42, 0x22, 0x42, 0xb1, 0x10, 0x00, 0x0e,
    0x8f, 0x7c, 0x42, 0x44, 0x82, 0x78, 0xe0, 0x0b, 0x09, 0x82, 0x42, 0x1e,
    0x41, 0x9f, 0xf7, 0x01 };

#define gx_128K_ram_x_hot 1
#define gx_128K_ram_y_hot 8
#define gx_128K_ram_width 43
#define gx_128K_ram_height 31
static unsigned char gx_128K_ram_bitmap[] = {
    0xfe, 0xdf, 0xff, 0xff, 0x03, 0x00, 0xfe, 0xdf, 0xff, 0xff, 0x03, 0x00,
    0xfe, 0xdf, 0xff, 0xff, 0x03, 0x00, 0xe2, 0xdf, 0xff, 0xff, 0x03, 0x00,
    0x9c, 0xdf, 0xff, 0xff, 0x03, 0x00, 0x7e, 0xdf, 0xff, 0xff, 0x01, 0x00,
    0x7e, 0xdf, 0xff, 0xff, 0x01, 0x00, 0xfe, 0xde, 0xff, 0xff, 0x02, 0x00,
    0xfe, 0xde, 0xff, 0xff, 0x02, 0x00, 0xfe, 0xdd, 0xff, 0x7f, 0x03, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xdd, 0xff, 0xbf, 0x03, 0x00,
    0xfe, 0xdb, 0xff, 0xdf, 0x03, 0x00, 0xfe, 0xdb, 0xff, 0xef, 0x03, 0x00,
    0xfe, 0xd7, 0xff, 0xf7, 0x03, 0x00, 0xfe, 0xcf, 0xff, 0xfb, 0x03, 0x00,
    0xfe, 0xcf, 0xff, 0xfc, 0x03, 0x00, 0xfe, 0x1f, 0x3f, 0xff, 0x03, 0x00,
    0xfe, 0xdf, 0xc0, 0xff, 0x03, 0x00, 0xfe, 0xdf, 0xff, 0xff, 0x03, 0x00,
    0xfe, 0xdf, 0xff, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc4, 0x30, 0x12, 0x1c, 0x44, 0x04,
    0x27, 0x49, 0x0a, 0x24, 0x46, 0x04, 0x84, 0x39, 0x06, 0x24, 0xc9, 0x06,
    0x62, 0x24, 0x07, 0x9e, 0xaf, 0x06, 0x12, 0x24, 0x09, 0x92, 0xa8, 0x05,
    0xf2, 0x18, 0x11, 0x52, 0x28, 0x05 };

#define gx_silver_x_hot 0
#define gx_silver_y_hot 8
#define gx_silver_width 35
#define gx_silver_height 21
static unsigned char gx_silver_bitmap[] = {
    0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x40,
    0x00, 0x00, 0x00, 0x38, 0x40, 0x00, 0x00, 0x00, 0xc4, 0x40, 0x00, 0x00,
    0x00, 0x02, 0x41, 0x00, 0x00, 0x04, 0x02, 0x41, 0x00, 0x00, 0x04, 0x02,
    0x42, 0x00, 0x00, 0x02, 0x01, 0x42, 0x00, 0x00, 0x02, 0x01, 0x44, 0x00,
    0x00, 0x01, 0xfd, 0xff, 0xff, 0xff, 0x07, 0x01, 0x44, 0x00, 0x80, 0x00,
    0x01, 0x48, 0x00, 0x40, 0x00, 0x01, 0x48, 0x00, 0x20, 0x00, 0x00, 0x50,
    0x00, 0x10, 0x00, 0x00, 0x60, 0x00, 0x08, 0x00, 0x00, 0x60, 0x00, 0x06,
    0x00, 0x00, 0xc0, 0x81, 0x01, 0x00, 0x00, 0x40, 0x7e, 0x00, 0x00, 0x00,
    0x40, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00 };

#define gx_green_x_hot 11
#define gx_green_y_hot 0
#define gx_green_width 34
#define gx_green_height 22
static unsigned char gx_green_bitmap[] = {
    0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff,
    0xff, 0xff, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
    0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0x03, 0x00, 0x00, 0x00,
    0xfc, 0x03, 0x00, 0x00, 0x00, 0xfc, 0x03, 0x00, 0x00, 0x00, 0xfc,
    0x03, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x00, 0x00, 0x00, 0xf0, 0x03,
    0x00, 0x00, 0x00, 0xf0, 0x03, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x00,
    0x00, 0x00, 0xf0, 0x03, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x00, 0x00,
    0x00, 0xf0, 0x03, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x00, 0x00, 0x00,
    0xf0, 0x03, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x00, 0x00, 0x00, 0xf0,
    0x03, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x00, 0x00, 0x00, 0xf0, 0x03 };

/* #endif /\* !_HP_H *\/ */

/* #ifndef _ANNUNC_H */
/* #define _ANNUNC_H 1 */

#define ann_alpha_width 15
#define ann_alpha_height 12
static unsigned char ann_alpha_bitmap[] = {
    0xe0, 0x03, 0x18, 0x44, 0x0c, 0x4c, 0x06, 0x2c, 0x07, 0x2c, 0x07, 0x1c,
    0x07, 0x0c, 0x07, 0x0c, 0x07, 0x0e, 0x0e, 0x4d, 0xf8, 0x38, 0x00, 0x00 };

#define ann_battery_width 15
#define ann_battery_height 12
static unsigned char ann_battery_bitmap[] = {
    0x04, 0x10, 0x02, 0x20, 0x12, 0x24, 0x09, 0x48, 0xc9, 0x49, 0xc9, 0x49,
    0xc9, 0x49, 0x09, 0x48, 0x12, 0x24, 0x02, 0x20, 0x04, 0x10, 0x00, 0x00 };

#define ann_busy_width 15
#define ann_busy_height 12
static unsigned char ann_busy_bitmap[] = {
    0xfc, 0x1f, 0x08, 0x08, 0x08, 0x08, 0xf0, 0x07, 0xe0, 0x03, 0xc0, 0x01,
    0x40, 0x01, 0x20, 0x02, 0x10, 0x04, 0xc8, 0x09, 0xe8, 0x0b, 0xfc, 0x1f };

#define ann_io_width 15
#define ann_io_height 12
static unsigned char ann_io_bitmap[] = {
    0x0c, 0x00, 0x1e, 0x00, 0x33, 0x0c, 0x61, 0x18, 0xcc, 0x30, 0xfe, 0x7f,
    0xfe, 0x7f, 0xcc, 0x30, 0x61, 0x18, 0x33, 0x0c, 0x1e, 0x00, 0x0c, 0x00 };

#define ann_left_width 15
#define ann_left_height 12
static unsigned char ann_left_bitmap[] = {
    0xfe, 0x3f, 0xff, 0x7f, 0x9f, 0x7f, 0xcf, 0x7f, 0xe7, 0x7f, 0x03, 0x78,
    0x03, 0x70, 0xe7, 0x73, 0xcf, 0x73, 0x9f, 0x73, 0xff, 0x73, 0xfe, 0x33 };

#define ann_right_width 15
#define ann_right_height 12
static unsigned char ann_right_bitmap[] = {
    0xfe, 0x3f, 0xff, 0x7f, 0xff, 0x7c, 0xff, 0x79, 0xff, 0x73, 0x0f, 0x60,
    0x07, 0x60, 0xe7, 0x73, 0xe7, 0x79, 0xe7, 0x7c, 0xe7, 0x7f, 0xe6, 0x3f };

/* #endif /\* !_ANNUNC_H *\/ */

/* #ifndef _BUTTONS_H */
/* #define _BUTTONS_H 1 */

#define menu_label_width 24
#define menu_label_height 11
static unsigned char menu_label_bitmap[] = {
    0xfe, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0x7f };

#define up_width 11
#define up_height 11
static unsigned char up_bitmap[] = {
    0x20, 0x00, 0x20, 0x00, 0x70, 0x00, 0x70, 0x00, 0xf8, 0x00, 0xf8,
    0x00, 0xfc, 0x01, 0xfc, 0x01, 0xfe, 0x03, 0xfe, 0x03, 0xff, 0x07 };

#define down_width 11
#define down_height 11
static unsigned char down_bitmap[] = {
    0xff, 0x07, 0xfe, 0x03, 0xfe, 0x03, 0xfc, 0x01, 0xfc, 0x01, 0xf8,
    0x00, 0xf8, 0x00, 0x70, 0x00, 0x70, 0x00, 0x20, 0x00, 0x20, 0x00 };

#define left_width 11
#define left_height 11
static unsigned char left_bitmap[] = {
    0x00, 0x04, 0x00, 0x07, 0xc0, 0x07, 0xf0, 0x07, 0xfc, 0x07, 0xff,
    0x07, 0xfc, 0x07, 0xf0, 0x07, 0xc0, 0x07, 0x00, 0x07, 0x00, 0x04 };

#define right_width 11
#define right_height 11
static unsigned char right_bitmap[] = {
    0x01, 0x00, 0x07, 0x00, 0x1f, 0x00, 0x7f, 0x00, 0xff, 0x01, 0xff,
    0x07, 0xff, 0x01, 0x7f, 0x00, 0x1f, 0x00, 0x07, 0x00, 0x01, 0x00 };

#define sqrt_width 20
#define sqrt_height 11
static unsigned char sqrt_bitmap[] = {
    0x00, 0xff, 0x0f, 0x00, 0x01, 0x08, 0x00, 0x01, 0x08, 0x80, 0x8c,
    0x01, 0x80, 0x58, 0x01, 0x80, 0x38, 0x00, 0x47, 0x30, 0x00, 0x4c,
    0x30, 0x00, 0x58, 0x78, 0x00, 0x30, 0x6a, 0x01, 0x20, 0xc6, 0x00 };

#define power_width 17
#define power_height 14
static unsigned char power_bitmap[] = {
    0x00, 0x8c, 0x01, 0x00, 0x58, 0x01, 0x00, 0x38, 0x00, 0xc8, 0x30,
    0x00, 0x9c, 0x30, 0x00, 0x98, 0x78, 0x00, 0x58, 0x6a, 0x01, 0x58,
    0xc6, 0x00, 0x38, 0x00, 0x00, 0x30, 0x00, 0x00, 0x10, 0x00, 0x00,
    0x08, 0x00, 0x00, 0x05, 0x00, 0x00, 0x03, 0x00, 0x00 };

#define inv_width 18
#define inv_height 13
static unsigned char inv_bitmap[] = {
    0x0c, 0x04, 0x00, 0x0f, 0x06, 0x00, 0x0c, 0x02, 0x00, 0x0c,
    0x03, 0x00, 0x0c, 0x01, 0x00, 0x8c, 0x19, 0x03, 0x8c, 0xb0,
    0x02, 0xcc, 0x70, 0x00, 0x40, 0x60, 0x00, 0x60, 0x60, 0x00,
    0x20, 0xf0, 0x00, 0x30, 0xd4, 0x02, 0x10, 0x8c, 0x01 };

#define neg_width 21
#define neg_height 11
static unsigned char neg_bitmap[] = {
    0x18, 0x00, 0x00, 0x18, 0x30, 0x00, 0x18, 0x30, 0x00, 0xff, 0x18,
    0x00, 0xff, 0x18, 0x00, 0x18, 0x0c, 0x00, 0x18, 0x0c, 0x00, 0x18,
    0xc6, 0x1f, 0x00, 0xc6, 0x1f, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00 };

#define bs_width 11
#define bs_height 11
static unsigned char bs_bitmap[] = {
    0x20, 0x00, 0x30, 0x00, 0x38, 0x00, 0xfc, 0x07, 0xfe, 0x07, 0xff,
    0x07, 0xfe, 0x07, 0xfc, 0x07, 0x38, 0x00, 0x30, 0x00, 0x20, 0x00 };

#define alpha_width 12
#define alpha_height 10
static unsigned char alpha_bitmap[] = {
    0x78, 0x00, 0x84, 0x08, 0x82, 0x09, 0x83, 0x05, 0x83, 0x05,
    0x83, 0x03, 0x83, 0x01, 0x83, 0x01, 0x46, 0x09, 0x3c, 0x06 };

#define div_width 10
#define div_height 10
static unsigned char div_bitmap[] = { 0x30, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0xff, 0x03, 0xff, 0x03, 0x00, 0x00,
                                      0x00, 0x00, 0x30, 0x00, 0x30, 0x00 };

#define shl_width 24
#define shl_height 14
static unsigned char shl_bitmap[] = {
    0xfe, 0xff, 0x7f, 0xff, 0xfc, 0xff, 0x7f, 0xfc, 0xff, 0x3f, 0xfe,
    0xff, 0x1f, 0xff, 0xff, 0x0f, 0x00, 0xfc, 0x07, 0x00, 0xf8, 0x0f,
    0x00, 0xf0, 0x1f, 0xff, 0xf1, 0x3f, 0xfe, 0xf1, 0x7f, 0xfc, 0xf1,
    0xff, 0xfc, 0xf1, 0xff, 0xff, 0xf1, 0xfe, 0xff, 0x71 };

#define mul_width 10
#define mul_height 10
static unsigned char mul_bitmap[] = { 0x03, 0x03, 0x87, 0x03, 0xce, 0x01, 0xfc,
                                      0x00, 0x78, 0x00, 0x78, 0x00, 0xfc, 0x00,
                                      0xce, 0x01, 0x87, 0x03, 0x03, 0x03 };

#define shr_width 24
#define shr_height 14
static unsigned char shr_bitmap[] = {
    0xfe, 0xff, 0x7f, 0xff, 0x3f, 0xff, 0xff, 0x3f, 0xfe, 0xff, 0x7f,
    0xfc, 0xff, 0xff, 0xf8, 0x3f, 0x00, 0xf0, 0x1f, 0x00, 0xe0, 0x0f,
    0x00, 0xf0, 0x8f, 0xff, 0xf8, 0x8f, 0x7f, 0xfc, 0x8f, 0x3f, 0xfe,
    0x8f, 0x3f, 0xff, 0x8f, 0xff, 0xff, 0x8e, 0xff, 0x7f };

#define minus_width 10
#define minus_height 10
static unsigned char minus_bitmap[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03,
    0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define plus_width 10
#define plus_height 10
static unsigned char plus_bitmap[] = { 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30,
                                       0x00, 0xff, 0x03, 0xff, 0x03, 0x30, 0x00,
                                       0x30, 0x00, 0x30, 0x00, 0x30, 0x00 };

#define colon_width 2
#define colon_height 10
static unsigned char colon_bitmap[] = { 0x03, 0x03, 0x03, 0x03, 0x00,
                                        0x00, 0x00, 0x00, 0x00, 0x00 };

/* Below used only for X11 */
#define last_width 120
#define last_height 6
static unsigned char last_bitmap[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xc6, 0x1c, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x29, 0x09,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x11, 0x49, 0x08, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x10, 0x8f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x29, 0x09, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0xc9, 0x08,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80 };

/* #endif /\* !_BUTTONS_H *\/ */

/* #ifndef _SMALL_H */
/* #define _SMALL_H 1 */

#define small_ascent 8
#define small_descent 4

#define blank_width 4
#define blank_height 7
static unsigned char blank_bitmap[] = { 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00 };

#define hash_width 5
#define hash_height 7
static unsigned char hash_bitmap[] = { 0x00, 0x0a, 0x1f, 0x0a,
                                       0x0a, 0x1f, 0x0a };

#define lbrace_width 3
#define lbrace_height 7
static unsigned char lbrace_bitmap[] = { 0x04, 0x02, 0x01, 0x01,
                                         0x01, 0x02, 0x04 };

#define rbrace_width 3
#define rbrace_height 7
static unsigned char rbrace_bitmap[] = { 0x01, 0x02, 0x04, 0x04,
                                         0x04, 0x02, 0x01 };

#define comma_width 3
#define comma_height 7
static unsigned char comma_bitmap[] = { 0x00, 0x00, 0x00, 0x00,
                                        0x06, 0x06, 0x03 };

#define slash_width 3
#define slash_height 7
static unsigned char slash_bitmap[] = { 0x04, 0x04, 0x02, 0x02,
                                        0x02, 0x01, 0x01 };

#define two_width 5
#define two_height 7
static unsigned char two_bitmap[] = { 0x0e, 0x11, 0x10, 0x08,
                                      0x04, 0x02, 0x1f };

#define three_width 5
#define three_height 7
static unsigned char three_bitmap[] = { 0x0e, 0x11, 0x10, 0x0c,
                                        0x10, 0x11, 0x0e };

#define small_colon_width 2
#define small_colon_height 7
static unsigned char small_colon_bitmap[] = { 0x00, 0x03, 0x03, 0x00,
                                              0x03, 0x03, 0x00 };

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
static unsigned char lbracket_bitmap[] = { 0x07, 0x01, 0x01, 0x01,
                                           0x01, 0x01, 0x07 };

#define rbracket_width 3
#define rbracket_height 7
static unsigned char rbracket_bitmap[] = { 0x07, 0x04, 0x04, 0x04,
                                           0x04, 0x04, 0x07 };

#define arrow_width 7
#define arrow_height 7
static unsigned char arrow_bitmap[] = { 0x00, 0x08, 0x18, 0x3f,
                                        0x18, 0x08, 0x00 };

#define diff_width 5
#define diff_height 7
static unsigned char diff_bitmap[] = { 0x0e, 0x10, 0x10, 0x1e,
                                       0x11, 0x11, 0x0e };

#define integral_width 5
#define integral_height 8
static unsigned char integral_bitmap[] = { 0x0c, 0x12, 0x02, 0x04,
                                           0x04, 0x08, 0x09, 0x06 };

#define sigma_width 6
#define sigma_height 9
static unsigned char sigma_bitmap[] = { 0x3f, 0x21, 0x02, 0x04, 0x08,
                                        0x04, 0x02, 0x21, 0x3f };

#define sqr_width 11
#define sqr_height 10
static unsigned char sqr_bitmap[] = { 0x00, 0x03, 0x80, 0x04, 0x00, 0x04, 0x00,
                                      0x02, 0x26, 0x01, 0x94, 0x07, 0x08, 0x00,
                                      0x14, 0x00, 0x53, 0x00, 0x21, 0x00 };

#define root_width 18
#define root_height 13
static unsigned char root_bitmap[] = {
    0x26, 0x00, 0x00, 0x14, 0x00, 0x00, 0x08, 0xfe, 0x03, 0x14,
    0x02, 0x02, 0x53, 0x02, 0x00, 0x21, 0x99, 0x00, 0x00, 0x91,
    0x00, 0x10, 0x91, 0x00, 0xa0, 0x50, 0x00, 0xc0, 0x60, 0x00,
    0x80, 0x20, 0x00, 0x00, 0x14, 0x00, 0x00, 0x0c, 0x00 };

#define pow10_width 13
#define pow10_height 9
static unsigned char pow10_bitmap[] = { 0x00, 0x12, 0x00, 0x0c, 0x32, 0x04,
                                        0x4b, 0x0a, 0x4a, 0x09, 0x4a, 0x00,
                                        0x4a, 0x00, 0x4a, 0x00, 0x32, 0x00 };

#define exp_width 11
#define exp_height 9
static unsigned char exp_bitmap[] = { 0x80, 0x04, 0x00, 0x03, 0x00, 0x01,
                                      0x8c, 0x02, 0x52, 0x02, 0x09, 0x00,
                                      0x07, 0x00, 0x21, 0x00, 0x1e, 0x00 };

#define under_width 6
#define under_height 7
static unsigned char under_bitmap[] = { 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x3f };

#define prog_width 16
#define prog_height 7
static unsigned char prog_bitmap[] = { 0x48, 0x12, 0x24, 0x24, 0x12,
                                       0x48, 0x09, 0x90, 0x12, 0x48,
                                       0x24, 0x24, 0x48, 0x12 };

#define string_width 10
#define string_height 7
static unsigned char string_bitmap[] = { 0x85, 0x02, 0x85, 0x02, 0x85,
                                         0x02, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00 };

#define equal_width 5
#define equal_height 7
static unsigned char equal_bitmap[] = { 0x00, 0x1f, 0x00, 0x00,
                                        0x1f, 0x00, 0x00 };

#define nl_width 8
#define nl_height 7
static unsigned char nl_bitmap[] = { 0x00, 0x84, 0x86, 0xff, 0x06, 0x04, 0x00 };

#define pi_width 6
#define pi_height 7
static unsigned char pi_bitmap[] = { 0x20, 0x1f, 0x12, 0x12, 0x12, 0x12, 0x12 };

#define angle_width 8
#define angle_height 7
static unsigned char angle_bitmap[] = { 0x40, 0x20, 0x10, 0x28,
                                        0x44, 0x42, 0xff };

#define lcurly_width 5
#define lcurly_height 7
static unsigned char lcurly_bitmap[] = { 0x18, 0x04, 0x04, 0x02,
                                         0x04, 0x04, 0x18 };

#define rcurly_width 5
#define rcurly_height 7
static unsigned char rcurly_bitmap[] = { 0x03, 0x04, 0x04, 0x08,
                                         0x04, 0x04, 0x03 };

#define sqr_gx_width 11
#define sqr_gx_height 13
static unsigned char sqr_gx_bitmap[] = {
    0x00, 0x03, 0x80, 0x04, 0x00, 0x04, 0x00, 0x02, 0x00,
    0x01, 0x80, 0x07, 0x00, 0x00, 0x66, 0x00, 0x14, 0x00,
    0x08, 0x00, 0x14, 0x00, 0x53, 0x00, 0x21, 0x00 };

#define root_gx_width 18
#define root_gx_height 15
static unsigned char root_gx_bitmap[] = {
    0x66, 0x00, 0x00, 0x14, 0x00, 0x00, 0x08, 0x00, 0x00, 0x14, 0x00, 0x00,
    0x53, 0xfe, 0x03, 0x21, 0x02, 0x02, 0x00, 0x02, 0x00, 0x00, 0x99, 0x00,
    0x00, 0x91, 0x00, 0x10, 0x91, 0x00, 0xa0, 0x50, 0x00, 0xc0, 0x60, 0x00,
    0x80, 0x20, 0x00, 0x00, 0x14, 0x00, 0x00, 0x0c, 0x00 };

#define pow10_gx_width 13
#define pow10_gx_height 12
static unsigned char pow10_gx_bitmap[] = {
    0x00, 0x12, 0x00, 0x0c, 0x00, 0x04, 0x00, 0x0a, 0x00, 0x09, 0x32, 0x00,
    0x4b, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x32, 0x00 };

#define exp_gx_width 13
#define exp_gx_height 12
static unsigned char exp_gx_bitmap[] = {
    0x00, 0xfb, 0x00, 0xf6, 0x00, 0xe6, 0x00, 0xf6, 0x80, 0xed, 0x18, 0xe0,
    0x36, 0xe0, 0x36, 0xe0, 0x1f, 0xe0, 0x03, 0xe0, 0x13, 0xe0, 0x0e, 0xe0 };
#define parens_gx_width 20
#define parens_gx_height 12
static unsigned char parens_gx_bitmap[] = {
    0x0c, 0x00, 0x03, 0x06, 0x00, 0x06, 0x06, 0x00, 0x06, 0x03, 0x00, 0x0c,
    0x03, 0x00, 0x0c, 0x03, 0x00, 0x0c, 0x03, 0x00, 0x0c, 0x03, 0x00, 0x0c,
    0x03, 0x00, 0x0c, 0x06, 0x00, 0x06, 0x06, 0x00, 0x06, 0x0c, 0x00, 0x03 };

#define hash_gx_width 8
#define hash_gx_height 12
static unsigned char hash_gx_bitmap[] = { 0x00, 0x00, 0x48, 0x48, 0xfe, 0x24,
                                          0x24, 0x7f, 0x12, 0x12, 0x00, 0x00 };

#define bracket_gx_width 12
#define bracket_gx_height 12
static unsigned char bracket_gx_bitmap[] = {
    0x0f, 0x0f, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c,
    0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x0f, 0x0f };

#define under_gx_width 10
#define under_gx_height 12
static unsigned char under_gx_bitmap[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03, 0xff, 0x03 };

#define prog_gx_width 24
#define prog_gx_height 12
static unsigned char prog_gx_bitmap[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0xc3, 0x18,
    0x8c, 0x81, 0x31, 0xc6, 0x00, 0x63, 0x63, 0x00, 0xc6, 0xc6, 0x00, 0x63,
    0x8c, 0x81, 0x31, 0x18, 0xc3, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define quote_gx_width 12
#define quote_gx_height 12
static unsigned char quote_gx_bitmap[] = {
    0x05, 0x0a, 0x05, 0x0a, 0x05, 0x0a, 0x05, 0x0a, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define curly_gx_width 14
#define curly_gx_height 12
static unsigned char curly_gx_bitmap[] = {
    0x0c, 0x0c, 0x06, 0x18, 0x06, 0x18, 0x06, 0x18, 0x06, 0x18, 0x03, 0x30,
    0x06, 0x18, 0x06, 0x18, 0x06, 0x18, 0x06, 0x18, 0x06, 0x18, 0x0c, 0x0c };

#define colon_gx_width 8
#define colon_gx_height 12
static unsigned char colon_gx_bitmap[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0xc3,
                                           0xc3, 0x00, 0x00, 0xc3, 0xc3, 0x00 };

#define angle_gx_width 12
#define angle_gx_height 12
static unsigned char angle_gx_bitmap[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01, 0xc0, 0x00, 0xe0, 0x01,
    0xb0, 0x03, 0x18, 0x03, 0x0c, 0x03, 0x06, 0x03, 0xff, 0x0f, 0xff, 0x0f };

#define pi_gx_width 10
#define pi_gx_height 12
static unsigned char pi_gx_bitmap[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xfe, 0x03, 0xff, 0x01,
    0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00 };

#define nl_gx_width 18
#define nl_gx_height 12
static unsigned char nl_gx_bitmap[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xf0, 0x00, 0x03,
    0xfc, 0x00, 0x03, 0xff, 0xff, 0x03, 0xff, 0xff, 0x03, 0xfc, 0x00, 0x00,
    0xf0, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define comma_gx_width 3
#define comma_gx_height 12
static unsigned char comma_gx_bitmap[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                           0x07, 0x07, 0x07, 0x04, 0x04, 0x02 };

#define arrow_gx_width 18
#define arrow_gx_height 12
static unsigned char arrow_gx_bitmap[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x3c, 0x00,
    0x00, 0xfc, 0x00, 0xff, 0xff, 0x03, 0xff, 0xff, 0x03, 0x00, 0xfc, 0x00,
    0x00, 0x3c, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define equal_gx_width 8
#define equal_gx_height 12
static unsigned char equal_gx_bitmap[] = { 0x00, 0x00, 0x00, 0xff, 0xff, 0x00,
                                           0x00, 0xff, 0xff, 0x00, 0x00, 0x00 };

typedef struct letter_t {
    unsigned int w, h;
    unsigned char* bits;
} letter_t;

static letter_t small_font[] = {
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

/* #endif /\* !_SMALL_H *\/ */

typedef struct color_t {
    const char* name;
    int r, g, b;
} color_t;

typedef struct keypad_t {
    unsigned int width;
    unsigned int height;
} keypad_t;

typedef struct disp_t {
    unsigned int w, h;
    short mapped;
    int offset;
    int lines;
} disp_t;

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

extern color_t* colors;
extern disp_t disp;
extern ann_struct_t ann_tbl[];

extern unsigned int ARGBColors[ BLACK + 1 ];
extern SDL_Surface* sdlwindow;
extern SDL_Surface* sdlsurface;

/*************************/
/* Functions' prototypes */
/*************************/

extern int get_ui_event( void );
extern void adjust_contrast();

extern void ShowConnections();

/* #ifndef _DEVICE_H */
/* #define _DEVICE_H 1 */
extern void init_display( void );                         /* x48_lcd.c */
extern void update_display( void );                       /* x48_lcd.c */
extern void redraw_display( void );                       /* x48_lcd.c */
extern void disp_draw_nibble( word_20 addr, word_4 val ); /* x48_lcd.c */
extern void menu_draw_nibble( word_20 addr, word_4 val ); /* x48_lcd.c */
extern void draw_annunc( void );                          /* x48_lcd.c */
extern void redraw_annunc( void );                        /* x48_lcd.c */
/* #endif /\* !_DEVICE_H *\/ */

extern void SDLCreateHP( void );
extern void SDLInit( void );
extern void SDLDrawAnnunc( char* annunc );

extern void SDLCreateAnnunc( void );
extern void SDLDrawNibble( int nx, int ny, int val );
extern void SDLDrawKeypad( void );
extern void SDLDrawButtons( void );
extern SDL_Surface* SDLCreateSurfFromData( unsigned int w, unsigned int h,
                                           unsigned char* data,
                                           unsigned int coloron,
                                           unsigned int coloroff );
extern SDL_Surface* SDLCreateARGBSurfFromData( unsigned int w, unsigned int h,
                                               unsigned char* data,
                                               unsigned int xpcolor );
extern void SDLDrawSmallString( int x, int y, const char* string,
                                unsigned int length, unsigned int coloron,
                                unsigned int coloroff );
extern void SDLCreateColors( void );
extern void SDLDrawKeyLetter( void );
extern unsigned SDLBGRA2ARGB( unsigned color );
extern void SDLDrawBezel();
extern void SDLDrawMore( unsigned int cut, unsigned int offset_y,
                         int keypad_width, int keypad_height );
extern void SDLDrawLogo();
extern void SDLDrawBackground( int width, int height, int w_top, int h_top );
extern void SDLUIShowKey( int hpkey );
extern void SDLUIHideKey( void );
extern void SDLUIFeedback( void );
extern SDLWINDOW_t SDLCreateWindow( int x, int y, int w, int h, unsigned color,
                                    int framewidth, int inverted );
extern void SDLShowWindow( SDLWINDOW_t* win );
extern void SDLSHideWindow( SDLWINDOW_t* win );
extern void SDLARGBTo( unsigned color, unsigned* a, unsigned* r, unsigned* g,
                       unsigned* b );
extern unsigned SDLToARGB( unsigned a, unsigned r, unsigned g, unsigned b );
extern void SDLMessageBox( int w, int h, const char* title, const char* text[],
                           unsigned color, unsigned colortext, int center );
extern void SDLEventWaitClickOrKey( void );
extern void SDLShowInformation( void );

#endif /* !_X48_GUI_H */
