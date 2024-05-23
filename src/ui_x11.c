#include <ctype.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/extensions/XShm.h>
#include <X11/keysym.h>

#include "romio.h" /* opt_gx */
#include "config.h"
#include "ui.h"
#include "ui_inner.h"

#define UPDATE_MENU 1
#define UPDATE_DISP 2

#define KEYBOARD_HEIGHT ( buttons[ LAST_HPKEY ].y + buttons[ LAST_HPKEY ].h )
#define KEYBOARD_WIDTH ( buttons[ LAST_HPKEY ].x + buttons[ LAST_HPKEY ].w )

#define TOP_SKIP 65
#define SIDE_SKIP 20
#define BOTTOM_SKIP 25
#define DISP_KBD_SKIP 65

#define DISPLAY_WIDTH ( 264 + 8 )
#define DISPLAY_HEIGHT ( 128 + 16 + 8 )
#define DISPLAY_OFFSET_X ( SIDE_SKIP + ( 286 - DISPLAY_WIDTH ) / 2 )
#define DISPLAY_OFFSET_Y TOP_SKIP

#define DISP_FRAME 8

#define KEYBOARD_OFFSET_X SIDE_SKIP
#define KEYBOARD_OFFSET_Y ( TOP_SKIP + DISPLAY_HEIGHT + DISP_KBD_SKIP )

#define COLOR_MODE_MONO 1
#define COLOR_MODE_GRAY 2
#define COLOR_MODE_COLOR 3

#define COLOR( c ) ( colors[ ( c ) ].xcolor.pixel )

/***********/
/* bitmaps */
/***********/

#define hp48_icon_width 32
#define hp48_icon_height 64
static unsigned char hp48_icon_bitmap[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0xe0, 0xff, 0xff, 0x07, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0x07, 0xff, 0x01, 0xe0, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
    0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
    0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x63, 0x8c,
    0x31, 0xc6, 0x63, 0x8c, 0x31, 0xc6, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x63, 0x8c, 0x31, 0xc6, 0x63, 0x8c, 0x31, 0xc6,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x63, 0x8c, 0x31, 0xc6, 0x63, 0x8c, 0x31, 0xc6, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0x63, 0x8c, 0x31, 0xc6, 0x63, 0x8c, 0x31, 0xc6, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0x8c, 0x31, 0xc6,
    0x03, 0x8c, 0x31, 0xc6, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe3, 0x30, 0x0c, 0xc3, 0xe3, 0x30, 0x0c, 0xc3, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe3, 0x30, 0x0c, 0xc3, 0xe3, 0x30, 0x0c, 0xc3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xe3, 0x30, 0x0c, 0xc3, 0xe3, 0x30, 0x0c, 0xc3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe3, 0x30, 0x0c, 0xc3, 0xe3, 0x30,
    0x0c, 0xc3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

#define hp48_top_width 32
#define hp48_top_height 30
static unsigned char hp48_top_bitmap[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0xe0, 0xff, 0xff, 0x07, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0x07, 0xff, 0x01, 0xe0, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
    0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
    0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
    0x07, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x63, 0x8c, 0x31, 0xc6, 0x63, 0x8c, 0x31, 0xc6, 0xff, 0xff, 0xff, 0xff };

#define hp48_bottom_width 32
#define hp48_bottom_height 64
static unsigned char hp48_bottom_bitmap[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x63, 0x8c, 0x31, 0xc6, 0x63, 0x8c, 0x31, 0xc6,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x63, 0x8c, 0x31, 0xc6, 0x63, 0x8c, 0x31, 0xc6, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0x63, 0x8c, 0x31, 0xc6, 0x63, 0x8c, 0x31, 0xc6, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0x8c, 0x31, 0xc6,
    0x03, 0x8c, 0x31, 0xc6, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe3, 0x30, 0x0c, 0xc3, 0xe3, 0x30, 0x0c, 0xc3, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe3, 0x30, 0x0c, 0xc3, 0xe3, 0x30, 0x0c, 0xc3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xe3, 0x30, 0x0c, 0xc3, 0xe3, 0x30, 0x0c, 0xc3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe3, 0x30, 0x0c, 0xc3, 0xe3, 0x30,
    0x0c, 0xc3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

#define hp48_logo_width 13
#define hp48_logo_height 4
static unsigned char hp48_logo_bitmap[] = { 0x00, 0x00, 0x00, 0x00, 0xf8, 0x1f, 0xf8, 0x1f };

#define hp48_text_width 29
#define hp48_text_height 7
static unsigned char hp48_text_bitmap[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0xfe, 0x1f };

#define hp48_disp_width 29
#define hp48_disp_height 21
static unsigned char hp48_disp_bitmap[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0x1f, 0xf8, 0xff, 0xff, 0x1f, 0xf8, 0xff,
    0xff, 0x1f, 0xf8, 0xff, 0xff, 0x1f, 0xf8, 0xff, 0xff, 0x1f, 0xf8, 0xff, 0xff, 0x1f, 0xf8, 0xff, 0xff, 0x1f, 0xf8, 0xff, 0xff,
    0x1f, 0xf8, 0xff, 0xff, 0x1f, 0xf8, 0xff, 0xff, 0x1f, 0xf8, 0xff, 0xff, 0x1f, 0xf8, 0xff, 0xff, 0x1f, 0xf8, 0xff, 0xff, 0x1f };

#define hp48_keys_width 30
#define hp48_keys_height 61
static unsigned char hp48_keys_bitmap[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x9c, 0x73, 0xce, 0x39, 0x9c, 0x73, 0xce, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9c, 0x73,
    0xce, 0x39, 0x9c, 0x73, 0xce, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9c, 0x73, 0xce, 0x39, 0x9c, 0x73, 0xce,
    0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9c, 0x73, 0xce, 0x39, 0x9c, 0x73, 0xce, 0x39, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xfc, 0x73, 0xce, 0x39, 0xfc, 0x73, 0xce, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c,
    0xcf, 0xf3, 0x3c, 0x1c, 0xcf, 0xf3, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcf, 0xf3, 0x3c, 0x00, 0xcf,
    0xf3, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcf, 0xf3, 0x3c, 0x00, 0xcf, 0xf3, 0x3c, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0xcf, 0xf3, 0x3c, 0x1c, 0xcf, 0xf3, 0x3c };

#define hp48_orange_width 5
#define hp48_orange_height 53
static unsigned char hp48_orange_bitmap[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x1c };

#define hp48_blue_width 5
#define hp48_blue_height 57
static unsigned char hp48_blue_bitmap[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x1c };

#define hp48_on_width 25
#define hp48_on_height 19
static unsigned char hp48_on_bitmap[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe2, 0x00, 0x00, 0x00, 0x13, 0x01,
                                          0x00, 0x80, 0x12, 0x01, 0x80, 0x48, 0x12, 0x01, 0x80, 0xc8, 0xe7, 0x00, 0x00, 0x05, 0x12, 0x01,
                                          0x00, 0x02, 0x12, 0x01, 0x00, 0x05, 0x12, 0x01, 0x80, 0x08, 0xe2, 0x00 };

#define hp48_top_gx_width 32
#define hp48_top_gx_height 30
static unsigned char hp48_top_gx_bitmap[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0x08, 0xff, 0xe1, 0x07, 0x08, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xf0,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
    0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
    0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
    0x07, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x63, 0x8c, 0x31, 0xc6, 0x63, 0x8c, 0x31, 0xc6, 0xff, 0xff, 0xff, 0xff };

#define hp48_logo_gx_width 16
#define hp48_logo_gx_height 4
static unsigned char hp48_logo_gx_bitmap[] = { 0x00, 0x00, 0x00, 0x00, 0xf8, 0xf7, 0xf8, 0xf7 };

#define hp48_text_gx_width 29
#define hp48_text_gx_height 7
static unsigned char hp48_text_gx_bitmap[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                               0x00, 0x0f, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f };

#define hp48_green_gx_width 29
#define hp48_green_gx_height 57
static unsigned char hp48_green_gx_bitmap[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00 };

typedef struct x11_color_t {
    const char* name;
    int r, g, b;

    int mono_rgb;
    int gray_rgb;
    XColor xcolor;
} x11_color_t;

typedef struct x11_keypad_t {
    unsigned int width;
    unsigned int height;

    Pixmap pixmap;
} x11_keypad_t;

typedef struct x11_button_t {
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

    Pixmap map;
    Pixmap down;
    Window xwin;
} x11_button_t;

typedef struct x11_ann_struct_t {
    int x;
    int y;
    unsigned int width;
    unsigned int height;
    unsigned char* bits;

    Pixmap pixmap;
} x11_ann_struct_t;

typedef struct x11_lcd_t {
    Window win;
    GC gc;

    int display_update;
    XShmSegmentInfo disp_info;
    XImage* disp_image;
    XShmSegmentInfo menu_info;
    XImage* menu_image;
} x11_lcd_t;

typedef struct icon_map_t {
    unsigned int w;
    unsigned int h;
    int c;
    unsigned char* bits;
} icon_map_t;

static bool mapped;

static x11_keypad_t keypad;
static x11_color_t* colors;

static int CompletionType = -1;

static x11_lcd_t lcd;

static bool shm_flag;

static Display* dpy;
static int screen;

static unsigned int depth;
static Colormap cmap;
static GC gc;
static Window mainW;
static Window iconW = 0;

static Atom wm_delete_window, wm_save_yourself, wm_protocols;
static Atom ol_decor_del, ol_decor_icon_name;
static Atom atom_type;
static Visual* visual;
static Pixmap icon_pix;
static Pixmap icon_text_pix;
static Pixmap icon_disp_pix;
static int last_icon_state = -1;
static bool xerror_flag;

static bool dynamic_color;
static bool direct_color;
static int color_mode;
static int icon_color_mode;

static char* res_name;
static char* res_class;

static x11_color_t colors_sx[] = {
    {.name = "white",
     .r = 255,
     .g = 255,
     .b = 255,
     .mono_rgb = 255,
     .gray_rgb = 255,
     .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }                                                                                        },
    {.name = "left",         .r = 255, .g = 166, .b = 0,   .mono_rgb = 255, .gray_rgb = 230, .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }},
    {.name = "right",
     .r = 0,
     .g = 210,
     .b = 255,
     .mono_rgb = 255,
     .gray_rgb = 169,
     .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }                                                                                        },
    {.name = "but_top",      .r = 109, .g = 93,  .b = 93,  .mono_rgb = 0,   .gray_rgb = 91,  .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }},
    {.name = "button",       .r = 90,  .g = 77,  .b = 77,  .mono_rgb = 0,   .gray_rgb = 81,  .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }},
    {.name = "but_bot",      .r = 76,  .g = 65,  .b = 65,  .mono_rgb = 0,   .gray_rgb = 69,  .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }},
    {.name = "lcd_col",
     .r = 202,
     .g = 221,
     .b = 92,
     .mono_rgb = 255,
     .gray_rgb = 205,
     .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }                                                                                        },
    {.name = "pix_col",      .r = 0,   .g = 0,   .b = 128, .mono_rgb = 0,   .gray_rgb = 20,  .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }},
    {.name = "pad_top",      .r = 109, .g = 78,  .b = 78,  .mono_rgb = 0,   .gray_rgb = 88,  .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }},
    {.name = "pad",          .r = 90,  .g = 64,  .b = 64,  .mono_rgb = 0,   .gray_rgb = 73,  .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }},
    {.name = "pad_bot",      .r = 76,  .g = 54,  .b = 54,  .mono_rgb = 0,   .gray_rgb = 60,  .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }},
    {.name = "disp_pad_top",
     .r = 155,
     .g = 118,
     .b = 84,
     .mono_rgb = 0,
     .gray_rgb = 124,
     .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }                                                                                        },
    {.name = "disp_pad",
     .r = 124,
     .g = 94,
     .b = 67,
     .mono_rgb = 0,
     .gray_rgb = 99,
     .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }                                                                                        },
    {.name = "disp_pad_bot",
     .r = 100,
     .g = 75,
     .b = 53,
     .mono_rgb = 0,
     .gray_rgb = 79,
     .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }                                                                                        },
    {.name = "logo",
     .r = 204,
     .g = 169,
     .b = 107,
     .mono_rgb = 255,
     .gray_rgb = 172,
     .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }                                                                                        },
    {.name = "logo_back",
     .r = 64,
     .g = 64,
     .b = 64,
     .mono_rgb = 0,
     .gray_rgb = 65,
     .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }                                                                                        },
    {.name = "label",
     .r = 202,
     .g = 184,
     .b = 144,
     .mono_rgb = 255,
     .gray_rgb = 185,
     .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }                                                                                        },
    {.name = "frame",        .r = 0,   .g = 0,   .b = 0,   .mono_rgb = 255, .gray_rgb = 0,   .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }},
    {.name = "underlay",     .r = 60,  .g = 42,  .b = 42,  .mono_rgb = 0,   .gray_rgb = 48,  .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }},
    {.name = "black",        .r = 0,   .g = 0,   .b = 0,   .mono_rgb = 0,   .gray_rgb = 0,   .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }},
};

static x11_color_t colors_gx[] = {
    {.name = "white",
     .r = 255,
     .g = 255,
     .b = 255,
     .mono_rgb = 255,
     .gray_rgb = 255,
     .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }                                                                                     },
    {.name = "left",
     .r = 255,
     .g = 186,
     .b = 255,
     .mono_rgb = 255,
     .gray_rgb = 220,
     .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }                                                                                     },
    {.name = "right",
     .r = 0,
     .g = 255,
     .b = 204,
     .mono_rgb = 255,
     .gray_rgb = 169,
     .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }                                                                                     },
    {.name = "but_top",
     .r = 104,
     .g = 104,
     .b = 104,
     .mono_rgb = 0,
     .gray_rgb = 104,
     .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }                                                                                     },
    {.name = "button",       .r = 88, .g = 88, .b = 88,  .mono_rgb = 0,   .gray_rgb = 88, .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }},
    {.name = "but_bot",      .r = 74, .g = 74, .b = 74,  .mono_rgb = 0,   .gray_rgb = 74, .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }},
    {.name = "lcd_col",
     .r = 202,
     .g = 221,
     .b = 92,
     .mono_rgb = 255,
     .gray_rgb = 205,
     .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }                                                                                     },
    {.name = "pix_col",      .r = 0,  .g = 0,  .b = 128, .mono_rgb = 0,   .gray_rgb = 20, .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }},
    {.name = "pad_top",      .r = 88, .g = 88, .b = 88,  .mono_rgb = 0,   .gray_rgb = 88, .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }},
    {.name = "pad",          .r = 74, .g = 74, .b = 74,  .mono_rgb = 0,   .gray_rgb = 74, .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }},
    {.name = "pad_bot",      .r = 64, .g = 64, .b = 64,  .mono_rgb = 0,   .gray_rgb = 64, .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }},
    {.name = "disp_pad_top",
     .r = 128,
     .g = 128,
     .b = 138,
     .mono_rgb = 0,
     .gray_rgb = 128,
     .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }                                                                                     },
    {.name = "disp_pad",
     .r = 104,
     .g = 104,
     .b = 110,
     .mono_rgb = 0,
     .gray_rgb = 104,
     .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }                                                                                     },
    {.name = "disp_pad_bot",
     .r = 84,
     .g = 84,
     .b = 90,
     .mono_rgb = 0,
     .gray_rgb = 84,
     .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }                                                                                     },
    {.name = "logo",
     .r = 176,
     .g = 176,
     .b = 184,
     .mono_rgb = 255,
     .gray_rgb = 176,
     .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }                                                                                     },
    {.name = "logo_back",
     .r = 104,
     .g = 104,
     .b = 110,
     .mono_rgb = 0,
     .gray_rgb = 104,
     .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }                                                                                     },
    {.name = "label",
     .r = 240,
     .g = 240,
     .b = 240,
     .mono_rgb = 255,
     .gray_rgb = 240,
     .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }                                                                                     },
    {.name = "frame",        .r = 0,  .g = 0,  .b = 0,   .mono_rgb = 255, .gray_rgb = 0,  .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }},
    {.name = "underlay",
     .r = 104,
     .g = 104,
     .b = 110,
     .mono_rgb = 0,
     .gray_rgb = 104,
     .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }                                                                                     },
    {.name = "black",        .r = 0,  .g = 0,  .b = 0,   .mono_rgb = 0,   .gray_rgb = 0,  .xcolor = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }},
};

static x11_button_t* buttons = 0;

static x11_button_t buttons_sx[] = {
    {.name = "A",
     .x = 0,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = menu_label_width,
     .lh = menu_label_height,
     .lb = menu_label_bitmap,
     .letter = "A",
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "B",
     .x = 50,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = menu_label_width,
     .lh = menu_label_height,
     .lb = menu_label_bitmap,
     .letter = "B",
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "C",
     .x = 100,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = menu_label_width,
     .lh = menu_label_height,
     .lb = menu_label_bitmap,
     .letter = "C",
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "D",
     .x = 150,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = menu_label_width,
     .lh = menu_label_height,
     .lb = menu_label_bitmap,
     .letter = "D",
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "E",
     .x = 200,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = menu_label_width,
     .lh = menu_label_height,
     .lb = menu_label_bitmap,
     .letter = "E",
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "F",
     .x = 250,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = menu_label_width,
     .lh = menu_label_height,
     .lb = menu_label_bitmap,
     .letter = "F",
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },

    {.name = "MTH",
     .x = 0,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "MTH",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "G",
     .left = "PRINT",
     .is_menu = 1,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "PRG",
     .x = 50,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "PRG",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "H",
     .left = "I/O",
     .is_menu = 1,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "CST",
     .x = 100,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "CST",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "I",
     .left = "MODES",
     .is_menu = 1,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "VAR",
     .x = 150,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "VAR",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "J",
     .left = "MEMORY",
     .is_menu = 1,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "UP",
     .x = 200,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = up_width,
     .lh = up_height,
     .lb = up_bitmap,
     .letter = "K",
     .left = "LIBRARY",
     .is_menu = 1,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "NXT",
     .x = 250,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "NXT",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "L",
     .left = "PREV",
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },

    {.name = "COLON",
     .x = 0,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = colon_width,
     .lh = colon_height,
     .lb = colon_bitmap,
     .letter = "M",
     .left = "UP",
     .is_menu = 0,
     .right = "HOME",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "STO",
     .x = 50,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "STO",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "N",
     .left = "DEF",
     .is_menu = 0,
     .right = "RCL",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "EVAL",
     .x = 100,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "EVAL",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "O",
     .left = "aQ",
     .is_menu = 0,
     .right = "aNUM",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "LEFT",
     .x = 150,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = left_width,
     .lh = left_height,
     .lb = left_bitmap,
     .letter = "P",
     .left = "GRAPH",
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "DOWN",
     .x = 200,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = down_width,
     .lh = down_height,
     .lb = down_bitmap,
     .letter = "Q",
     .left = "REVIEW",
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "RIGHT",
     .x = 250,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = right_width,
     .lh = right_height,
     .lb = right_bitmap,
     .letter = "R",
     .left = "SWAP",
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },

    {.name = "SIN",
     .x = 0,
     .y = 150,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "SIN",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "S",
     .left = "ASIN",
     .is_menu = 0,
     .right = "b",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "COS",
     .x = 50,
     .y = 150,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "COS",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "T",
     .left = "ACOS",
     .is_menu = 0,
     .right = "c",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "TAN",
     .x = 100,
     .y = 150,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "TAN",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "U",
     .left = "ATAN",
     .is_menu = 0,
     .right = "d",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "SQRT",
     .x = 150,
     .y = 150,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = sqrt_width,
     .lh = sqrt_height,
     .lb = sqrt_bitmap,
     .letter = "V",
     .left = "e",
     .is_menu = 0,
     .right = "f",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "POWER",
     .x = 200,
     .y = 150,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = power_width,
     .lh = power_height,
     .lb = power_bitmap,
     .letter = "W",
     .left = "g",
     .is_menu = 0,
     .right = "LOG",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "INV",
     .x = 250,
     .y = 150,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = inv_width,
     .lh = inv_height,
     .lb = inv_bitmap,
     .letter = "X",
     .left = "h",
     .is_menu = 0,
     .right = "LN",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },

    {.name = "ENTER",
     .x = 0,
     .y = 200,
     .w = 86,
     .h = 26,
     .lc = WHITE,
     .label = "ENTER",
     .font_size = 2,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "EQUATION",
     .is_menu = 0,
     .right = "MATRIX",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "NEG",
     .x = 100,
     .y = 200,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = neg_width,
     .lh = neg_height,
     .lb = neg_bitmap,
     .letter = "Y",
     .left = "EDIT",
     .is_menu = 0,
     .right = "VISIT",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "EEX",
     .x = 150,
     .y = 200,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "EEX",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "Z",
     .left = "2D",
     .is_menu = 0,
     .right = "3D",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "DEL",
     .x = 200,
     .y = 200,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "DEL",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "PURGE",
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "BS",
     .x = 250,
     .y = 200,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = bs_width,
     .lh = bs_height,
     .lb = bs_bitmap,
     .letter = 0,
     .left = "DROP",
     .is_menu = 0,
     .right = "CLR",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },

    {.name = "ALPHA",
     .x = 0,
     .y = 250,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = alpha_width,
     .lh = alpha_height,
     .lb = alpha_bitmap,
     .letter = 0,
     .left = "USR",
     .is_menu = 0,
     .right = "ENTRY",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {"7",              60, 250, 46, 26, WHITE, "7", 1, 0, 0, 0, 0, "SOLVE", 1, 0, 0, 0, 0, 0},
    {.name = "8",
     .x = 120,
     .y = 250,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "8",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "PLOT",
     .is_menu = 1,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "9",
     .x = 180,
     .y = 250,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "9",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "ALGEBRA",
     .is_menu = 1,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "DIV",
     .x = 240,
     .y = 250,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = div_width,
     .lh = div_height,
     .lb = div_bitmap,
     .letter = 0,
     .left = "( )",
     .is_menu = 0,
     .right = "#",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },

    {.name = "SHL",
     .x = 0,
     .y = 300,
     .w = 36,
     .h = 26,
     .lc = LEFT,
     .label = 0,
     .font_size = 0,
     .lw = shl_width,
     .lh = shl_height,
     .lb = shl_bitmap,
     .letter = 0,
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "4",
     .x = 60,
     .y = 300,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "4",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "TIME",
     .is_menu = 1,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "5",
     .x = 120,
     .y = 300,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "5",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "STAT",
     .is_menu = 1,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "6",
     .x = 180,
     .y = 300,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "6",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "UNITS",
     .is_menu = 1,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "MUL",
     .x = 240,
     .y = 300,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = mul_width,
     .lh = mul_height,
     .lb = mul_bitmap,
     .letter = 0,
     .left = "[ ]",
     .is_menu = 0,
     .right = "_",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },

    {.name = "SHR",
     .x = 0,
     .y = 350,
     .w = 36,
     .h = 26,
     .lc = RIGHT,
     .label = 0,
     .font_size = 0,
     .lw = shr_width,
     .lh = shr_height,
     .lb = shr_bitmap,
     .letter = 0,
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "1",
     .x = 60,
     .y = 350,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "1",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "RAD",
     .is_menu = 0,
     .right = "POLAR",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "2",
     .x = 120,
     .y = 350,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "2",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "STACK",
     .is_menu = 0,
     .right = "ARG",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "3",
     .x = 180,
     .y = 350,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "3",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "CMD",
     .is_menu = 0,
     .right = "MENU",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "MINUS",
     .x = 240,
     .y = 350,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = minus_width,
     .lh = minus_height,
     .lb = minus_bitmap,
     .letter = 0,
     .left = "i",
     .is_menu = 0,
     .right = "j",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },

    {.name = "ON",
     .x = 0,
     .y = 400,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "ON",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "CONT",
     .is_menu = 0,
     .right = "OFF",
     .sub = "ATTN",
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "0",
     .x = 60,
     .y = 400,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "0",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "= ",
     .is_menu = 0,
     .right = " a",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "PERIOD",
     .x = 120,
     .y = 400,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = ".",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = ".",
     .is_menu = 0,
     .right = " k",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "SPC",
     .x = 180,
     .y = 400,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "SPC",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "l ",
     .is_menu = 0,
     .right = " m",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
    {.name = "PLUS",
     .x = 240,
     .y = 400,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = plus_width,
     .lh = plus_height,
     .lb = plus_bitmap,
     .letter = 0,
     .left = "{ }",
     .is_menu = 0,
     .right = ": :",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                              },
};

static x11_button_t buttons_gx[] = {
    {.name = "A",
     .x = 0,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = menu_label_width,
     .lh = menu_label_height,
     .lb = menu_label_bitmap,
     .letter = "A",
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "B",
     .x = 50,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = menu_label_width,
     .lh = menu_label_height,
     .lb = menu_label_bitmap,
     .letter = "B",
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "C",
     .x = 100,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = menu_label_width,
     .lh = menu_label_height,
     .lb = menu_label_bitmap,
     .letter = "C",
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "D",
     .x = 150,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = menu_label_width,
     .lh = menu_label_height,
     .lb = menu_label_bitmap,
     .letter = "D",
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "E",
     .x = 200,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = menu_label_width,
     .lh = menu_label_height,
     .lb = menu_label_bitmap,
     .letter = "E",
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "F",
     .x = 250,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = menu_label_width,
     .lh = menu_label_height,
     .lb = menu_label_bitmap,
     .letter = "F",
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },

    {.name = "MTH",
     .x = 0,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "MTH",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "G",
     .left = "RAD",
     .is_menu = 0,
     .right = "POLAR",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "PRG",
     .x = 50,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "PRG",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "H",
     .left = 0,
     .is_menu = 0,
     .right = "CHARS",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "CST",
     .x = 100,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "CST",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "I",
     .left = 0,
     .is_menu = 0,
     .right = "MODES",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "VAR",
     .x = 150,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "VAR",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "J",
     .left = 0,
     .is_menu = 0,
     .right = "MEMORY",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "UP",
     .x = 200,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = up_width,
     .lh = up_height,
     .lb = up_bitmap,
     .letter = "K",
     .left = 0,
     .is_menu = 0,
     .right = "STACK",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "NXT",
     .x = 250,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "NXT",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "L",
     .left = "PREV",
     .is_menu = 0,
     .right = "MENU",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },

    {.name = "COLON",
     .x = 0,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = colon_width,
     .lh = colon_height,
     .lb = colon_bitmap,
     .letter = "M",
     .left = "UP",
     .is_menu = 0,
     .right = "HOME",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "STO",
     .x = 50,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "STO",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "N",
     .left = "DEF",
     .is_menu = 0,
     .right = "RCL",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "EVAL",
     .x = 100,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "EVAL",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "O",
     .left = "aNUM",
     .is_menu = 0,
     .right = "UNDO",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "LEFT",
     .x = 150,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = left_width,
     .lh = left_height,
     .lb = left_bitmap,
     .letter = "P",
     .left = "PICTURE",
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "DOWN",
     .x = 200,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = down_width,
     .lh = down_height,
     .lb = down_bitmap,
     .letter = "Q",
     .left = "VIEW",
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "RIGHT",
     .x = 250,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = right_width,
     .lh = right_height,
     .lb = right_bitmap,
     .letter = "R",
     .left = "SWAP",
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },

    {.name = "SIN",
     .x = 0,
     .y = 150,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "SIN",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "S",
     .left = "ASIN",
     .is_menu = 0,
     .right = "b",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "COS",
     .x = 50,
     .y = 150,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "COS",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "T",
     .left = "ACOS",
     .is_menu = 0,
     .right = "c",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "TAN",
     .x = 100,
     .y = 150,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "TAN",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "U",
     .left = "ATAN",
     .is_menu = 0,
     .right = "d",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "SQRT",
     .x = 150,
     .y = 150,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = sqrt_width,
     .lh = sqrt_height,
     .lb = sqrt_bitmap,
     .letter = "V",
     .left = "n",
     .is_menu = 0,
     .right = "o",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "POWER",
     .x = 200,
     .y = 150,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = power_width,
     .lh = power_height,
     .lb = power_bitmap,
     .letter = "W",
     .left = "p",
     .is_menu = 0,
     .right = "LOG",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "INV",
     .x = 250,
     .y = 150,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = inv_width,
     .lh = inv_height,
     .lb = inv_bitmap,
     .letter = "X",
     .left = "q",
     .is_menu = 0,
     .right = "LN",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },

    {.name = "ENTER",
     .x = 0,
     .y = 200,
     .w = 86,
     .h = 26,
     .lc = WHITE,
     .label = "ENTER",
     .font_size = 2,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "EQUATION",
     .is_menu = 0,
     .right = "MATRIX",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "NEG",
     .x = 100,
     .y = 200,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = neg_width,
     .lh = neg_height,
     .lb = neg_bitmap,
     .letter = "Y",
     .left = "EDIT",
     .is_menu = 0,
     .right = "CMD",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "EEX",
     .x = 150,
     .y = 200,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "EEX",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "Z",
     .left = "PURG",
     .is_menu = 0,
     .right = "ARG",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "DEL",
     .x = 200,
     .y = 200,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "DEL",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "CLEAR",
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "BS",
     .x = 250,
     .y = 200,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = bs_width,
     .lh = bs_height,
     .lb = bs_bitmap,
     .letter = 0,
     .left = "DROP",
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },

    {.name = "ALPHA",
     .x = 0,
     .y = 250,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = alpha_width,
     .lh = alpha_height,
     .lb = alpha_bitmap,
     .letter = 0,
     .left = "USER",
     .is_menu = 0,
     .right = "ENTRY",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "7",
     .x = 60,
     .y = 250,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "7",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = 0,
     .is_menu = 1,
     .right = "SOLVE",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "8",
     .x = 120,
     .y = 250,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "8",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = 0,
     .is_menu = 1,
     .right = "PLOT",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "9",
     .x = 180,
     .y = 250,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "9",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = 0,
     .is_menu = 1,
     .right = "SYMBOLIC",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "DIV",
     .x = 240,
     .y = 250,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = div_width,
     .lh = div_height,
     .lb = div_bitmap,
     .letter = 0,
     .left = "r ",
     .is_menu = 0,
     .right = "s",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },

    {.name = "SHL",
     .x = 0,
     .y = 300,
     .w = 36,
     .h = 26,
     .lc = LEFT,
     .label = 0,
     .font_size = 0,
     .lw = shl_width,
     .lh = shl_height,
     .lb = shl_bitmap,
     .letter = 0,
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "4",
     .x = 60,
     .y = 300,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "4",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = 0,
     .is_menu = 1,
     .right = "TIME",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "5",
     .x = 120,
     .y = 300,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "5",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = 0,
     .is_menu = 1,
     .right = "STAT",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "6",
     .x = 180,
     .y = 300,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "6",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = 0,
     .is_menu = 1,
     .right = "UNITS",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "MUL",
     .x = 240,
     .y = 300,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = mul_width,
     .lh = mul_height,
     .lb = mul_bitmap,
     .letter = 0,
     .left = "t ",
     .is_menu = 0,
     .right = "u",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },

    {.name = "SHR",
     .x = 0,
     .y = 350,
     .w = 36,
     .h = 26,
     .lc = RIGHT,
     .label = 0,
     .font_size = 0,
     .lw = shr_width,
     .lh = shr_height,
     .lb = shr_bitmap,
     .letter = 0,
     .left = 0,
     .is_menu = 1,
     .right = " ",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "1",
     .x = 60,
     .y = 350,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "1",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = 0,
     .is_menu = 1,
     .right = "I/O",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "2",
     .x = 120,
     .y = 350,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "2",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = 0,
     .is_menu = 1,
     .right = "LIBRARY",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "3",
     .x = 180,
     .y = 350,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "3",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = 0,
     .is_menu = 1,
     .right = "EQ LIB",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "MINUS",
     .x = 240,
     .y = 350,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = minus_width,
     .lh = minus_height,
     .lb = minus_bitmap,
     .letter = 0,
     .left = "v ",
     .is_menu = 0,
     .right = "w",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },

    {.name = "ON",
     .x = 0,
     .y = 400,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "ON",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "CONT",
     .is_menu = 0,
     .right = "OFF",
     .sub = "CANCEL",
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "0",
     .x = 60,
     .y = 400,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "0",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "\004 ",
     .is_menu = 0,
     .right = "\003",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {.name = "PERIOD",
     .x = 120,
     .y = 400,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = ".",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = ".",
     .is_menu = 0,
     .right = " k",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
    {"SPC",            180, 400, 46, 26, WHITE, "SPC", 0, 0, 0, 0, 0, "\005 ", 0, "z", 0, 0, 0, 0},
    {.name = "PLUS",
     .x = 240,
     .y = 400,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = plus_width,
     .lh = plus_height,
     .lb = plus_bitmap,
     .letter = 0,
     .left = "x ",
     .is_menu = 0,
     .right = "y",
     .sub = 0,
     .map = 0,
     .down = 0,
     .xwin = 0                                                                                   },
};

#define MAX_PASTE 128
static int paste[ MAX_PASTE * 3 ];
static int paste_count = 0;
static int paste_size = 0;
static int paste_last_key = 0;

static int first_key = 0;

static int last_button = -1;

#define ICON_MAP 0
#define ON_MAP 1
#define DISP_MAP 2
#define FIRST_MAP 3
#define LAST_MAP 9

static icon_map_t* icon_maps;

static Pixmap nibble_maps[ 16 ];

static unsigned char nibbles[ 16 ][ 2 ] = {
    {0x00, 0x00}, /* ---- */
    {0x03, 0x03}, /* *--- */
    {0x0c, 0x0c}, /* -*-- */
    {0x0f, 0x0f}, /* **-- */
    {0x30, 0x30}, /* --*- */
    {0x33, 0x33}, /* *-*- */
    {0x3c, 0x3c}, /* -**- */
    {0x3f, 0x3f}, /* ***- */
    {0xc0, 0xc0}, /* ---* */
    {0xc3, 0xc3}, /* *--* */
    {0xcc, 0xcc}, /* -*-* */
    {0xcf, 0xcf}, /* **-* */
    {0xf0, 0xf0}, /* --** */
    {0xf3, 0xf3}, /* *-** */
    {0xfc, 0xfc}, /* -*** */
    {0xff, 0xff}  /* **** */
};

static unsigned char nibble_bitmap[ 16 ];

static x11_ann_struct_t ann_tbl[] = {
    {.x = 16,  .y = 4, .width = ann_left_width,    .height = ann_left_height,    .bits = ann_left_bitmap,    .pixmap = 0},
    {.x = 61,  .y = 4, .width = ann_right_width,   .height = ann_right_height,   .bits = ann_right_bitmap,   .pixmap = 0},
    {.x = 106, .y = 4, .width = ann_alpha_width,   .height = ann_alpha_height,   .bits = ann_alpha_bitmap,   .pixmap = 0},
    {.x = 151, .y = 4, .width = ann_battery_width, .height = ann_battery_height, .bits = ann_battery_bitmap, .pixmap = 0},
    {.x = 196, .y = 4, .width = ann_busy_width,    .height = ann_busy_height,    .bits = ann_busy_bitmap,    .pixmap = 0},
    {.x = 241, .y = 4, .width = ann_io_width,      .height = ann_io_height,      .bits = ann_io_bitmap,      .pixmap = 0},
};

static icon_map_t icon_maps_sx[] = {
    {.w = hp48_icon_width,   .h = hp48_icon_height,   .c = BLACK,    .bits = hp48_icon_bitmap  },
    {.w = hp48_on_width,     .h = hp48_on_height,     .c = PIXEL,    .bits = hp48_on_bitmap    },
    {.w = hp48_disp_width,   .h = hp48_disp_height,   .c = LCD,      .bits = hp48_disp_bitmap  },
    {.w = hp48_top_width,    .h = hp48_top_height,    .c = DISP_PAD, .bits = hp48_top_bitmap   },
    {.w = hp48_bottom_width, .h = hp48_bottom_height, .c = PAD,      .bits = hp48_bottom_bitmap},
    {.w = hp48_logo_width,   .h = hp48_logo_height,   .c = LOGO,     .bits = hp48_logo_bitmap  },
    {.w = hp48_text_width,   .h = hp48_text_height,   .c = LABEL,    .bits = hp48_text_bitmap  },
    {.w = hp48_keys_width,   .h = hp48_keys_height,   .c = BLACK,    .bits = hp48_keys_bitmap  },
    {.w = hp48_orange_width, .h = hp48_orange_height, .c = LEFT,     .bits = hp48_orange_bitmap},
    {.w = hp48_blue_width,   .h = hp48_blue_height,   .c = RIGHT,    .bits = hp48_blue_bitmap  }
};

static icon_map_t icon_maps_gx[] = {
    {.w = hp48_icon_width,     .h = hp48_icon_height,     .c = BLACK,    .bits = hp48_icon_bitmap    },
    {.w = hp48_on_width,       .h = hp48_on_height,       .c = PIXEL,    .bits = hp48_on_bitmap      },
    {.w = hp48_disp_width,     .h = hp48_disp_height,     .c = LCD,      .bits = hp48_disp_bitmap    },
    {.w = hp48_top_gx_width,   .h = hp48_top_gx_height,   .c = DISP_PAD, .bits = hp48_top_gx_bitmap  },
    {.w = hp48_bottom_width,   .h = hp48_bottom_height,   .c = PAD,      .bits = hp48_bottom_bitmap  },
    {.w = hp48_logo_gx_width,  .h = hp48_logo_gx_height,  .c = LOGO,     .bits = hp48_logo_gx_bitmap },
    {.w = hp48_text_gx_width,  .h = hp48_text_gx_height,  .c = LABEL,    .bits = hp48_text_gx_bitmap },
    {.w = hp48_keys_width,     .h = hp48_keys_height,     .c = BLACK,    .bits = hp48_keys_bitmap    },
    {.w = hp48_orange_width,   .h = hp48_orange_height,   .c = LEFT,     .bits = hp48_orange_bitmap  },
    {.w = hp48_green_gx_width, .h = hp48_green_gx_height, .c = RIGHT,    .bits = hp48_green_gx_bitmap}
};

static int saved_argc;
static char** saved_argv;

/************************/
/* functions prototypes */
/************************/
void x11_draw_annunc( void );
void x11_update_LCD( void );

/*************/
/* functions */
/*************/
static inline void fatal_exit( const char* error, const char* advice )
{
    if ( error[ 0 ] == '\0' ) {
        fprintf( stderr, "FATAL ERROR, exit.\n" );
        exit( 1 );
    }

    fprintf( stderr, "FATAL ERROR, exit.\n  - %s\n", error );

    if ( advice[ 0 ] != '\0' )
        fprintf( stderr, "  - %s\n", advice );

    exit( 1 );
}

inline Visual* pick_visual_of_class( Display* local_dpy, int visual_class, unsigned int* local_depth )
{
    XVisualInfo vi_in, *vi_out;
    int out_count;

    vi_in.class = visual_class;
    vi_in.screen = DefaultScreen( local_dpy );
    vi_out = XGetVisualInfo( local_dpy, VisualClassMask | VisualScreenMask, &vi_in, &out_count );
    if ( vi_out ) { /* choose the 'best' one, if multiple */
        int i, best;
        Visual* local_visual;
        for ( i = 0, best = 0; i < out_count; i++ )
            if ( vi_out[ i ].depth > vi_out[ best ].depth )
                best = i;
        local_visual = vi_out[ best ].visual;
        *local_depth = vi_out[ best ].depth;
        XFree( ( char* )vi_out );
        return local_visual;
    } else {
        *local_depth = DefaultDepth( local_dpy, DefaultScreen( local_dpy ) );
        return DefaultVisual( local_dpy, DefaultScreen( local_dpy ) );
    }
}

inline Visual* id_to_visual( Display* local_dpy, int id, unsigned int* local_depth )
{
    XVisualInfo vi_in, *vi_out;
    int out_count;

    vi_in.screen = DefaultScreen( local_dpy );
    vi_in.visualid = id;
    vi_out = XGetVisualInfo( local_dpy, VisualScreenMask | VisualIDMask, &vi_in, &out_count );
    if ( vi_out ) {
        Visual* v = vi_out[ 0 ].visual;
        *local_depth = vi_out[ 0 ].depth;
        XFree( ( char* )vi_out );
        return v;
    }
    return 0;
}

Visual* get_visual_resource( Display* local_dpy, unsigned int* local_depth )
{
    char c;
    int vclass;
    int id;

    if ( !config.x11_visual || !strcmp( config.x11_visual, "default" ) )
        vclass = -1;
    else if ( !strcmp( config.x11_visual, "staticgray" ) )
        vclass = StaticGray;
    else if ( !strcmp( config.x11_visual, "staticcolor" ) )
        vclass = StaticColor;
    else if ( !strcmp( config.x11_visual, "truecolor" ) )
        vclass = TrueColor;
    else if ( !strcmp( config.x11_visual, "grayscale" ) )
        vclass = GrayScale;
    else if ( !strcmp( config.x11_visual, "pseudocolor" ) )
        vclass = PseudoColor;
    else if ( !strcmp( config.x11_visual, "directcolor" ) )
        vclass = DirectColor;
    else if ( 1 == sscanf( config.x11_visual, " %d %c", &id, &c ) )
        vclass = -2;
    else if ( 1 == sscanf( config.x11_visual, " 0x%d %c", &id, &c ) )
        vclass = -2;
    else {
        fprintf( stderr, "unrecognized visual \"%s\".\n", config.x11_visual );
        vclass = -1;
    }

    if ( vclass == -1 ) {
        *local_depth = DefaultDepth( local_dpy, DefaultScreen( local_dpy ) );

        return DefaultVisual( local_dpy, DefaultScreen( local_dpy ) );
    } else if ( vclass == -2 ) {
        Visual* v = id_to_visual( local_dpy, id, local_depth );
        if ( v )
            return v;

        fprintf( stderr, "no visual with id 0x%x.\n", id );

        *local_depth = DefaultDepth( local_dpy, DefaultScreen( local_dpy ) );

        return DefaultVisual( local_dpy, DefaultScreen( local_dpy ) );
    } else
        return pick_visual_of_class( local_dpy, vclass, local_depth );
}

static inline XFontStruct* load_x11_font( Display* local_dpy, char* fontname )
{
    XFontStruct* f = ( XFontStruct* )0;

    f = XLoadQueryFont( local_dpy, fontname );

    if ( f == ( XFontStruct* )0 ) {
        char errbuf[ 1024 ];
        char fixbuf[ 1024 ];
        sprintf( errbuf, "can\'t load font \'%s\'", fontname );
        sprintf( fixbuf, "Please change resource \'%s\'", config.name );
        fatal_exit( errbuf, fixbuf );
    }

    return f;
}

static inline void AllocColors( void )
{
    int c, error, dyn;
    int r_shift = 0, g_shift = 0, b_shift = 0;
    XSetWindowAttributes xswa;

    error = -1;
    dyn = dynamic_color;

    if ( direct_color ) {
        while ( !( visual->red_mask & ( 1 << r_shift ) ) )
            r_shift++;
        while ( visual->red_mask & ( 1 << r_shift ) )
            r_shift++;
        r_shift = 16 - r_shift;
        while ( !( visual->green_mask & ( 1 << g_shift ) ) )
            g_shift++;
        while ( ( visual->green_mask & ( 1 << g_shift ) ) )
            g_shift++;
        g_shift = 16 - g_shift;
        while ( !( visual->blue_mask & ( 1 << b_shift ) ) )
            b_shift++;
        while ( ( visual->blue_mask & ( 1 << b_shift ) ) )
            b_shift++;
        b_shift = 16 - b_shift;
    }

    for ( c = WHITE; c <= BLACK; c++ ) {
        switch ( color_mode ) {
            case COLOR_MODE_MONO:
                colors[ c ].xcolor.red = colors[ c ].mono_rgb << 8;
                colors[ c ].xcolor.green = colors[ c ].mono_rgb << 8;
                colors[ c ].xcolor.blue = colors[ c ].mono_rgb << 8;
                break;
            case COLOR_MODE_GRAY:
                colors[ c ].xcolor.red = colors[ c ].gray_rgb << 8;
                colors[ c ].xcolor.green = colors[ c ].gray_rgb << 8;
                colors[ c ].xcolor.blue = colors[ c ].gray_rgb << 8;
                break;
            default:
                colors[ c ].xcolor.red = colors[ c ].r << 8;
                colors[ c ].xcolor.green = colors[ c ].g << 8;
                colors[ c ].xcolor.blue = colors[ c ].b << 8;
                break;
        }
        if ( direct_color ) {
            colors[ c ].xcolor.pixel = ( ( colors[ c ].xcolor.red >> r_shift ) & visual->red_mask ) |
                                       ( ( colors[ c ].xcolor.green >> g_shift ) & visual->green_mask ) |
                                       ( ( colors[ c ].xcolor.blue >> b_shift ) & visual->blue_mask );
            XStoreColor( dpy, cmap, &colors[ c ].xcolor );
        } else {
            if ( dynamic_color && c == PIXEL ) {
                if ( XAllocColorCells( dpy, cmap, True, ( unsigned long* )0, 0, &colors[ c ].xcolor.pixel, 1 ) == 0 ) {
                    dyn = 0;
                    if ( XAllocColor( dpy, cmap, &colors[ c ].xcolor ) == 0 ) {
                        if ( config.verbose )
                            fprintf( stderr, "XAllocColor failed.\n" );
                        error = c;
                        break;
                    }
                } else if ( colors[ c ].xcolor.pixel >= ( unsigned long )( visual->map_entries ) ) {
                    dyn = 0;
                    if ( XAllocColor( dpy, cmap, &colors[ c ].xcolor ) == 0 ) {
                        if ( config.verbose )
                            fprintf( stderr, "XAllocColor failed.\n" );
                        error = c;
                        break;
                    }
                } else {
                    XStoreColor( dpy, cmap, &colors[ c ].xcolor );
                }
            } else {
                if ( XAllocColor( dpy, cmap, &colors[ c ].xcolor ) == 0 ) {
                    if ( config.verbose )
                        fprintf( stderr, "XAllocColor failed.\n" );
                    error = c;
                    break;
                }
            }
        }
    }

    /*
     * Can't be reached when visual->class == DirectColor
     */
    if ( error != -1 ) {
        if ( config.verbose )
            fprintf( stderr, "Using own Colormap.\n" );
        /*
         * free colors so far allocated
         */
        for ( c = WHITE; c < error; c++ )
            XFreeColors( dpy, cmap, &colors[ c ].xcolor.pixel, 1, 0 );

        /*
         * Create my own Colormap
         */
        cmap = XCreateColormap( dpy, mainW, visual, AllocNone );
        if ( cmap == ( Colormap )0 )
            fatal_exit( "can\'t alloc Colormap.\n", "" );

        xswa.colormap = cmap;
        XChangeWindowAttributes( dpy, mainW, CWColormap, &xswa );
        if ( iconW )
            XChangeWindowAttributes( dpy, iconW, CWColormap, &xswa );

        /*
         * Try to allocate colors again
         */
        dyn = dynamic_color;
        for ( c = WHITE; c <= BLACK; c++ ) {
            switch ( color_mode ) {
                case COLOR_MODE_MONO:
                    colors[ c ].xcolor.red = colors[ c ].mono_rgb << 8;
                    colors[ c ].xcolor.green = colors[ c ].mono_rgb << 8;
                    colors[ c ].xcolor.blue = colors[ c ].mono_rgb << 8;
                    break;
                case COLOR_MODE_GRAY:
                    colors[ c ].xcolor.red = colors[ c ].gray_rgb << 8;
                    colors[ c ].xcolor.green = colors[ c ].gray_rgb << 8;
                    colors[ c ].xcolor.blue = colors[ c ].gray_rgb << 8;
                    break;
                default:
                    colors[ c ].xcolor.red = colors[ c ].r << 8;
                    colors[ c ].xcolor.green = colors[ c ].g << 8;
                    colors[ c ].xcolor.blue = colors[ c ].b << 8;
                    break;
            }
            if ( dynamic_color && c == PIXEL ) {
                if ( XAllocColorCells( dpy, cmap, True, ( unsigned long* )0, 0, &colors[ c ].xcolor.pixel, 1 ) == 0 ) {
                    dyn = 0;
                    if ( XAllocColor( dpy, cmap, &colors[ c ].xcolor ) == 0 )
                        fatal_exit( "can\'t alloc Color.\n", "" );

                } else if ( colors[ c ].xcolor.pixel >= ( unsigned long )( visual->map_entries ) ) {
                    dyn = 0;
                    if ( XAllocColor( dpy, cmap, &colors[ c ].xcolor ) == 0 )
                        fatal_exit( "can\'t alloc Color.\n", "" );

                } else {
                    XStoreColor( dpy, cmap, &colors[ c ].xcolor );
                }
            } else {
                if ( XAllocColor( dpy, cmap, &colors[ c ].xcolor ) == 0 )
                    fatal_exit( "can\'t alloc Color.\n", "" );
            }
        }
    }

    dynamic_color = dyn;
}

static inline void __WriteButtonText( Display* the_dpy, Drawable d, GC the_gc, int x, int y, const char* string, unsigned int length )
{
    Pixmap pix;

    for ( unsigned int i = 0; i < length; i++ ) {
        if ( small_font[ ( int )string[ i ] ].h != 0 ) {
            pix = XCreateBitmapFromData( the_dpy, d, ( char* )small_font[ ( int )string[ i ] ].bits, small_font[ ( int )string[ i ] ].w,
                                         small_font[ ( int )string[ i ] ].h );
            XCopyPlane( the_dpy, pix, d, the_gc, 0, 0, small_font[ ( int )string[ i ] ].w, small_font[ ( int )string[ i ] ].h, x,
                        ( int )( y - small_font[ ( int )string[ i ] ].h ), 1 );
            XFreePixmap( the_dpy, pix );
        }
        x += SmallTextWidth( &string[ i ], 1 );
    }
}

static inline void __CreateButton( int i, int off_x, int off_y, XFontStruct* f_small, XFontStruct* f_med, XFontStruct* f_big )
{
    int x, y;
    XSetWindowAttributes xswa;
    XFontStruct* finfo;
    XGCValues val;
    unsigned long gc_mask;
    Pixmap pix;
    XCharStruct xchar;
    int dir, fa, fd;
    unsigned long pixel;

    if ( i < HPKEY_MTH )
        pixel = COLOR( DISP_PAD );
    else {
        if ( opt_gx && buttons[ i ].is_menu )
            pixel = COLOR( UNDERLAY );
        else
            pixel = COLOR( PAD );
    }

    /*
     * create the buttons subwindows
     */
    buttons[ i ].xwin = XCreateSimpleWindow( dpy, mainW, off_x + buttons[ i ].x, off_y + buttons[ i ].y, buttons[ i ].w, buttons[ i ].h, 0,
                                             COLOR( BLACK ), pixel );

    XDefineCursor( dpy, buttons[ i ].xwin, XCreateFontCursor( dpy, XC_hand1 ) );

    xswa.event_mask = LeaveWindowMask | ExposureMask | StructureNotifyMask;
    xswa.backing_store = Always;

    XChangeWindowAttributes( dpy, buttons[ i ].xwin, CWEventMask | CWBackingStore, &xswa );

    /*
     * draw the released button
     */
    buttons[ i ].map = XCreatePixmap( dpy, buttons[ i ].xwin, buttons[ i ].w, buttons[ i ].h, depth );

    XSetForeground( dpy, gc, pixel );
    XFillRectangle( dpy, buttons[ i ].map, gc, 0, 0, buttons[ i ].w, buttons[ i ].h );

    XSetForeground( dpy, gc, COLOR( BUTTON ) );
    XFillRectangle( dpy, buttons[ i ].map, gc, 1, 1, buttons[ i ].w - 2, buttons[ i ].h - 2 );

    if ( buttons[ i ].label != ( char* )0 ) {
        /*
         * set font size in gc
         */
        switch ( buttons[ i ].font_size ) {
            case 0:
                finfo = f_small;
                break;
            case 1:
                finfo = f_big;
                break;
            case 2:
                finfo = f_med;
                break;
            default:
                finfo = f_small;
                break;
        }
        val.font = finfo->fid;
        gc_mask = GCFont;
        XChangeGC( dpy, gc, gc_mask, &val );

        /*
         * draw string centered in button
         */
        XSetBackground( dpy, gc, COLOR( BUTTON ) );
        XSetForeground( dpy, gc, COLOR( buttons[ i ].lc ) );

        XTextExtents( finfo, buttons[ i ].label, ( int )strlen( buttons[ i ].label ), &dir, &fa, &fd, &xchar );
        x = ( buttons[ i ].w - xchar.width ) / 2;
        y = ( 1 + buttons[ i ].h - ( xchar.ascent + xchar.descent ) ) / 2 + xchar.ascent + 1;
        XDrawImageString( dpy, buttons[ i ].map, gc, x, y, buttons[ i ].label, ( int )strlen( buttons[ i ].label ) );

        XSetBackground( dpy, gc, COLOR( BLACK ) );

    } else if ( buttons[ i ].lw != 0 ) {
        /*
         * draw pixmap centered in button
         */
        XSetBackground( dpy, gc, COLOR( BUTTON ) );
        XSetForeground( dpy, gc, COLOR( buttons[ i ].lc ) );

        pix = XCreateBitmapFromData( dpy, buttons[ i ].xwin, ( char* )buttons[ i ].lb, buttons[ i ].lw, buttons[ i ].lh );

        x = ( 1 + buttons[ i ].w - buttons[ i ].lw ) / 2;
        y = ( 1 + buttons[ i ].h - buttons[ i ].lh ) / 2 + 1;

        XCopyPlane( dpy, pix, buttons[ i ].map, gc, 0, 0, buttons[ i ].lw, buttons[ i ].lh, x, y, 1 );

        XFreePixmap( dpy, pix );

        XSetBackground( dpy, gc, COLOR( BLACK ) );
    }

    /*
     * draw edge of button
     */
    XSetForeground( dpy, gc, COLOR( BUT_TOP ) );

    XDrawLine( dpy, buttons[ i ].map, gc, 1, ( int )( buttons[ i ].h - 2 ), 1, 1 );
    XDrawLine( dpy, buttons[ i ].map, gc, 2, ( int )( buttons[ i ].h - 3 ), 2, 2 );
    XDrawLine( dpy, buttons[ i ].map, gc, 3, ( int )( buttons[ i ].h - 4 ), 3, 3 );

    XDrawLine( dpy, buttons[ i ].map, gc, 1, 1, ( int )( buttons[ i ].w - 2 ), 1 );
    XDrawLine( dpy, buttons[ i ].map, gc, 2, 2, ( int )( buttons[ i ].w - 3 ), 2 );
    XDrawLine( dpy, buttons[ i ].map, gc, 3, 3, ( int )( buttons[ i ].w - 4 ), 3 );
    XDrawLine( dpy, buttons[ i ].map, gc, 4, 4, ( int )( buttons[ i ].w - 5 ), 4 );

    XDrawPoint( dpy, buttons[ i ].map, gc, 4, 5 );

    XSetForeground( dpy, gc, COLOR( BUT_BOT ) );

    XDrawLine( dpy, buttons[ i ].map, gc, 3, ( int )( buttons[ i ].h - 2 ), ( int )( buttons[ i ].w - 2 ), ( int )( buttons[ i ].h - 2 ) );
    XDrawLine( dpy, buttons[ i ].map, gc, 4, ( int )( buttons[ i ].h - 3 ), ( int )( buttons[ i ].w - 3 ), ( int )( buttons[ i ].h - 3 ) );

    XDrawLine( dpy, buttons[ i ].map, gc, ( int )( buttons[ i ].w - 2 ), ( int )( buttons[ i ].h - 2 ), ( int )( buttons[ i ].w - 2 ), 3 );
    XDrawLine( dpy, buttons[ i ].map, gc, ( int )( buttons[ i ].w - 3 ), ( int )( buttons[ i ].h - 3 ), ( int )( buttons[ i ].w - 3 ), 4 );
    XDrawLine( dpy, buttons[ i ].map, gc, ( int )( buttons[ i ].w - 4 ), ( int )( buttons[ i ].h - 4 ), ( int )( buttons[ i ].w - 4 ), 5 );

    XDrawPoint( dpy, buttons[ i ].map, gc, ( int )( buttons[ i ].w - 5 ), ( int )( buttons[ i ].h - 4 ) );

    /*
     * draw frame around button
     */
    XSetForeground( dpy, gc, COLOR( FRAME ) );

    XDrawLine( dpy, buttons[ i ].map, gc, 0, ( int )( buttons[ i ].h - 3 ), 0, 2 );
    XDrawLine( dpy, buttons[ i ].map, gc, 2, 0, ( int )( buttons[ i ].w - 3 ), 0 );
    XDrawLine( dpy, buttons[ i ].map, gc, 2, ( int )( buttons[ i ].h - 1 ), ( int )( buttons[ i ].w - 3 ), ( int )( buttons[ i ].h - 1 ) );
    XDrawLine( dpy, buttons[ i ].map, gc, ( int )( buttons[ i ].w - 1 ), ( int )( buttons[ i ].h - 3 ), ( int )( buttons[ i ].w - 1 ), 2 );

    if ( i == HPKEY_ON ) {
        XDrawLine( dpy, buttons[ i ].map, gc, 1, 1, ( int )( buttons[ i ].w - 2 ), 1 );
        XDrawPoint( dpy, buttons[ i ].map, gc, 1, 2 );
        XDrawPoint( dpy, buttons[ i ].map, gc, ( int )( buttons[ i ].w - 2 ), 2 );
    } else {
        XDrawPoint( dpy, buttons[ i ].map, gc, 1, 1 );
        XDrawPoint( dpy, buttons[ i ].map, gc, ( int )( buttons[ i ].w - 2 ), 1 );
    }
    XDrawPoint( dpy, buttons[ i ].map, gc, 1, ( int )( buttons[ i ].h - 2 ) );
    XDrawPoint( dpy, buttons[ i ].map, gc, ( int )( buttons[ i ].w - 2 ), ( int )( buttons[ i ].h - 2 ) );

    /*
     * draw the depressed button
     */
    buttons[ i ].down = XCreatePixmap( dpy, buttons[ i ].xwin, buttons[ i ].w, buttons[ i ].h, depth );

    XSetForeground( dpy, gc, pixel );
    XFillRectangle( dpy, buttons[ i ].down, gc, 0, 0, buttons[ i ].w, buttons[ i ].h );

    XSetForeground( dpy, gc, COLOR( BUTTON ) );
    XFillRectangle( dpy, buttons[ i ].down, gc, 1, 1, buttons[ i ].w - 2, buttons[ i ].h - 2 );

    if ( buttons[ i ].label != ( char* )0 ) {
        /*
         * set small or big font in gc
         */
        switch ( buttons[ i ].font_size ) {
            case 0:
                finfo = f_small;
                break;
            case 1:
                finfo = f_big;
                break;
            case 2:
                finfo = f_med;
                break;
            default:
                finfo = f_small;
                break;
        }
        val.font = finfo->fid;
        gc_mask = GCFont;
        XChangeGC( dpy, gc, gc_mask, &val );

        /*
         * draw string centered in button
         */
        XSetBackground( dpy, gc, COLOR( BUTTON ) );
        XSetForeground( dpy, gc, COLOR( buttons[ i ].lc ) );

        XTextExtents( finfo, buttons[ i ].label, ( int )strlen( buttons[ i ].label ), &dir, &fa, &fd, &xchar );
        x = ( buttons[ i ].w - xchar.width ) / 2;
        y = ( 1 + buttons[ i ].h - ( xchar.ascent + xchar.descent ) ) / 2 + xchar.ascent;
        XDrawImageString( dpy, buttons[ i ].down, gc, x, y, buttons[ i ].label, ( int )strlen( buttons[ i ].label ) );

        XSetBackground( dpy, gc, COLOR( BLACK ) );
    } else {
        /*
         * draw pixmap centered in button
         */
        XSetBackground( dpy, gc, COLOR( BUTTON ) );
        XSetForeground( dpy, gc, COLOR( buttons[ i ].lc ) );

        pix = XCreateBitmapFromData( dpy, buttons[ i ].xwin, ( char* )buttons[ i ].lb, buttons[ i ].lw, buttons[ i ].lh );

        x = ( 1 + buttons[ i ].w - buttons[ i ].lw ) / 2;
        y = ( 1 + buttons[ i ].h - buttons[ i ].lh ) / 2;

        XCopyPlane( dpy, pix, buttons[ i ].down, gc, 0, 0, buttons[ i ].lw, buttons[ i ].lh, x, y, 1 );

        XFreePixmap( dpy, pix );

        XSetBackground( dpy, gc, COLOR( BLACK ) );
    }

    /*
     * draw edge of button
     */
    XSetForeground( dpy, gc, COLOR( BUT_TOP ) );

    XDrawLine( dpy, buttons[ i ].down, gc, 2, ( int )( buttons[ i ].h - 4 ), 2, 2 );
    XDrawLine( dpy, buttons[ i ].down, gc, 3, ( int )( buttons[ i ].h - 5 ), 3, 3 );

    XDrawLine( dpy, buttons[ i ].down, gc, 2, 2, ( int )( buttons[ i ].w - 4 ), 2 );
    XDrawLine( dpy, buttons[ i ].down, gc, 3, 3, ( int )( buttons[ i ].w - 5 ), 3 );

    XDrawPoint( dpy, buttons[ i ].down, gc, 4, 4 );

    XSetForeground( dpy, gc, COLOR( BUT_BOT ) );

    XDrawLine( dpy, buttons[ i ].down, gc, 3, ( int )( buttons[ i ].h - 3 ), ( int )( buttons[ i ].w - 3 ), ( int )( buttons[ i ].h - 3 ) );
    XDrawLine( dpy, buttons[ i ].down, gc, 4, ( int )( buttons[ i ].h - 4 ), ( int )( buttons[ i ].w - 4 ), ( int )( buttons[ i ].h - 4 ) );

    XDrawLine( dpy, buttons[ i ].down, gc, ( int )( buttons[ i ].w - 3 ), ( int )( buttons[ i ].h - 3 ), ( int )( buttons[ i ].w - 3 ), 3 );
    XDrawLine( dpy, buttons[ i ].down, gc, ( int )( buttons[ i ].w - 4 ), ( int )( buttons[ i ].h - 4 ), ( int )( buttons[ i ].w - 4 ), 4 );

    XDrawPoint( dpy, buttons[ i ].down, gc, ( int )( buttons[ i ].w - 5 ), ( int )( buttons[ i ].h - 5 ) );

    /*
     * draw frame around button
     */
    XSetForeground( dpy, gc, COLOR( FRAME ) );

    XDrawLine( dpy, buttons[ i ].down, gc, 0, ( int )( buttons[ i ].h - 3 ), 0, 2 );
    XDrawLine( dpy, buttons[ i ].down, gc, 2, 0, ( int )( buttons[ i ].w - 3 ), 0 );
    XDrawLine( dpy, buttons[ i ].down, gc, 2, ( int )( buttons[ i ].h - 1 ), ( int )( buttons[ i ].w - 3 ), ( int )( buttons[ i ].h - 1 ) );
    XDrawLine( dpy, buttons[ i ].down, gc, ( int )( buttons[ i ].w - 1 ), ( int )( buttons[ i ].h - 3 ), ( int )( buttons[ i ].w - 1 ), 2 );

    if ( i == HPKEY_ON ) {
        XDrawLine( dpy, buttons[ i ].down, gc, 1, 1, ( int )( buttons[ i ].w - 2 ), 1 );
        XDrawPoint( dpy, buttons[ i ].down, gc, 1, 2 );
        XDrawPoint( dpy, buttons[ i ].down, gc, ( int )( buttons[ i ].w - 2 ), 2 );
    } else {
        XDrawPoint( dpy, buttons[ i ].down, gc, 1, 1 );
        XDrawPoint( dpy, buttons[ i ].down, gc, ( int )( buttons[ i ].w - 2 ), 1 );
    }
    XDrawPoint( dpy, buttons[ i ].down, gc, 1, ( int )( buttons[ i ].h - 2 ) );
    XDrawPoint( dpy, buttons[ i ].down, gc, ( int )( buttons[ i ].w - 2 ), ( int )( buttons[ i ].h - 2 ) );

    if ( i == HPKEY_ON ) {
        XDrawRectangle( dpy, buttons[ i ].down, gc, 1, 2, buttons[ i ].w - 3, buttons[ i ].h - 4 );
        XDrawPoint( dpy, buttons[ i ].down, gc, 2, 3 );
        XDrawPoint( dpy, buttons[ i ].down, gc, ( int )( buttons[ i ].w - 3 ), 3 );
    } else {
        XDrawRectangle( dpy, buttons[ i ].down, gc, 1, 1, buttons[ i ].w - 3, buttons[ i ].h - 3 );
        XDrawPoint( dpy, buttons[ i ].down, gc, 2, 2 );
        XDrawPoint( dpy, buttons[ i ].down, gc, ( int )( buttons[ i ].w - 3 ), 2 );
    }
    XDrawPoint( dpy, buttons[ i ].down, gc, 2, ( int )( buttons[ i ].h - 3 ) );
    XDrawPoint( dpy, buttons[ i ].down, gc, ( int )( buttons[ i ].w - 3 ), ( int )( buttons[ i ].h - 3 ) );
}

static inline void _CreateKeypad( unsigned int offset_y, unsigned int offset_x, x11_keypad_t* local_keypad )
{
    int i, x, y;
    int wl, wr, ws;
    Pixmap pix;
    unsigned long pixel;
    unsigned int pw, ph;
    XFontStruct *f_small, *f_med, *f_big;

    f_small = load_x11_font( dpy, config.smallFont );
    f_med = load_x11_font( dpy, config.mediumFont );
    f_big = load_x11_font( dpy, config.largeFont );

    /*
     * draw the character labels
     */
    for ( i = FIRST_HPKEY; i <= LAST_HPKEY; i++ ) {
        __CreateButton( i, offset_x, offset_y, f_small, f_med, f_big );

        pixel = ( i < HPKEY_MTH ) ? COLOR( DISP_PAD ) : COLOR( PAD );

        if ( buttons[ i ].letter != ( char* )0 ) {
            XSetBackground( dpy, gc, pixel );
            XSetForeground( dpy, gc, COLOR( WHITE ) );

            if ( opt_gx ) {
                x = offset_x + buttons[ i ].x + buttons[ i ].w + 3;
                y = offset_y + buttons[ i ].y + buttons[ i ].h + 1;
            } else {
                x = offset_x + buttons[ i ].x + buttons[ i ].w - SmallTextWidth( buttons[ i ].letter, 1 ) / 2 + 5;
                y = offset_y + buttons[ i ].y + buttons[ i ].h - 2;
            }

            __WriteButtonText( dpy, local_keypad->pixmap, gc, x, y, buttons[ i ].letter, 1 );
        }

        /*
         * draw the bottom labels
         */
        if ( buttons[ i ].sub != ( char* )0 ) {
            XSetBackground( dpy, gc, pixel );
            XSetForeground( dpy, gc, COLOR( WHITE ) );

            x = offset_x + buttons[ i ].x + ( 1 + buttons[ i ].w - SmallTextWidth( buttons[ i ].sub, strlen( buttons[ i ].sub ) ) ) / 2;
            y = offset_y + buttons[ i ].y + buttons[ i ].h + small_ascent + 2;

            __WriteButtonText( dpy, local_keypad->pixmap, gc, x, y, buttons[ i ].sub, strlen( buttons[ i ].sub ) );
        }

        /*
         * draw the left labels
         */
        if ( buttons[ i ].left != ( char* )0 ) {
            if ( buttons[ i ].is_menu ) {
                /*
                 * draw the dark shade under the label
                 */
                if ( opt_gx ) {
                    pw = 58;
                    ph = 48;
                } else {
                    pw = 46;
                    ph = 11;
                }

                pix = XCreatePixmap( dpy, local_keypad->pixmap, pw, ph, depth );

                XSetForeground( dpy, gc, COLOR( UNDERLAY ) );

                XFillRectangle( dpy, pix, gc, 0, 0, pw, ph );

                XSetBackground( dpy, gc, COLOR( UNDERLAY ) );
                XSetForeground( dpy, gc, COLOR( LEFT ) );

                x = ( pw + 1 - SmallTextWidth( buttons[ i ].left, strlen( buttons[ i ].left ) ) ) / 2;
                y = ( opt_gx ) ? 14 : 9;

                __WriteButtonText( dpy, pix, gc, x, y, buttons[ i ].left, strlen( buttons[ i ].left ) );

                XSetForeground( dpy, gc, pixel );

                if ( !opt_gx ) {
                    XDrawPoint( dpy, pix, gc, 0, 0 );
                    XDrawPoint( dpy, pix, gc, 0, ph - 1 );
                    XDrawPoint( dpy, pix, gc, pw - 1, 0 );
                    XDrawPoint( dpy, pix, gc, pw - 1, ph - 1 );
                }

                if ( opt_gx ) {
                    x = offset_x + buttons[ i ].x - 6;
                    y = offset_y + buttons[ i ].y - small_ascent - small_descent - 6;
                } else {
                    x = offset_x + buttons[ i ].x + ( buttons[ i ].w - pw ) / 2;
                    y = offset_y + buttons[ i ].y - small_ascent - small_descent;
                }

                XCopyArea( dpy, pix, local_keypad->pixmap, gc, 0, 0, pw, ph, x, y );

                XFreePixmap( dpy, pix );
            } else {
                XSetBackground( dpy, gc, pixel );
                XSetForeground( dpy, gc, COLOR( LEFT ) );

                if ( buttons[ i ].right == ( char* )0 ) /* centered label */
                    x = offset_x + buttons[ i ].x +
                        ( 1 + buttons[ i ].w - SmallTextWidth( buttons[ i ].left, strlen( buttons[ i ].left ) ) ) / 2;
                else { /* label to the left */
                    wl = SmallTextWidth( buttons[ i ].left, strlen( buttons[ i ].left ) );
                    wr = SmallTextWidth( buttons[ i ].right, strlen( buttons[ i ].right ) );
                    ws = SmallTextWidth( " ", 1 );

                    x = offset_x + buttons[ i ].x + ( 1 + buttons[ i ].w - ( wl + wr + ws ) ) / 2;
                }

                y = offset_y + buttons[ i ].y - small_descent;

                __WriteButtonText( dpy, local_keypad->pixmap, gc, x, y, buttons[ i ].left, strlen( buttons[ i ].left ) );
            }
        }

        /*
         * draw the right labels
         */
        if ( buttons[ i ].right != ( char* )0 ) {
            if ( buttons[ i ].is_menu ) {
                /*
                 * draw the dark shade under the label
                 */
                if ( opt_gx ) {
                    pw = 58;
                    ph = 48;
                } else {
                    pw = 44;
                    ph = 9;
                }

                pix = XCreatePixmap( dpy, local_keypad->pixmap, pw, ph, depth );

                XSetForeground( dpy, gc, COLOR( UNDERLAY ) );

                XFillRectangle( dpy, pix, gc, 0, 0, pw, ph );

                XSetBackground( dpy, gc, COLOR( UNDERLAY ) );
                XSetForeground( dpy, gc, COLOR( RIGHT ) );

                x = ( pw + 1 - SmallTextWidth( buttons[ i ].right, strlen( buttons[ i ].right ) ) ) / 2;
                y = ( opt_gx ) ? 14 : 8;

                __WriteButtonText( dpy, pix, gc, x, y, buttons[ i ].right, strlen( buttons[ i ].right ) );

                XSetForeground( dpy, gc, pixel );

                if ( !opt_gx ) {
                    XDrawPoint( dpy, pix, gc, 0, 0 );
                    XDrawPoint( dpy, pix, gc, 0, ph - 1 );
                    XDrawPoint( dpy, pix, gc, pw - 1, 0 );
                    XDrawPoint( dpy, pix, gc, pw - 1, ph - 1 );
                }

                if ( opt_gx ) {
                    x = offset_x + buttons[ i ].x - 6;
                    y = offset_y + buttons[ i ].y - small_ascent - small_descent - 6;
                } else {
                    x = offset_x + buttons[ i ].x + ( buttons[ i ].w - pw ) / 2;
                    y = offset_y + buttons[ i ].y - small_ascent - small_descent;
                }

                XCopyArea( dpy, pix, local_keypad->pixmap, gc, 0, 0, pw, ph, x, y );

                XFreePixmap( dpy, pix );
            } else {
                XSetBackground( dpy, gc, pixel );
                XSetForeground( dpy, gc, COLOR( RIGHT ) );

                if ( buttons[ i ].left == ( char* )0 ) /* centered label */
                    x = offset_x + buttons[ i ].x +
                        ( 1 + buttons[ i ].w - SmallTextWidth( buttons[ i ].right, strlen( buttons[ i ].right ) ) ) / 2;
                else { /* label to the right */
                    wl = SmallTextWidth( buttons[ i ].left, strlen( buttons[ i ].left ) );
                    wr = SmallTextWidth( buttons[ i ].right, strlen( buttons[ i ].right ) );
                    ws = SmallTextWidth( " ", 1 );

                    x = offset_x + buttons[ i ].x + ( 1 + buttons[ i ].w - ( wl + wr + ws ) ) / 2 + wl + ws;
                }

                y = offset_y + buttons[ i ].y - small_descent;

                __WriteButtonText( dpy, local_keypad->pixmap, gc, x, y, buttons[ i ].right, strlen( buttons[ i ].right ) );
            }
        }
    }

    XFreeFont( dpy, f_big );
    XFreeFont( dpy, f_med );
    XFreeFont( dpy, f_small );

    /*
     * at last draw the v--- LAST ---v thing
     */

    if ( !opt_gx ) {
        XSetBackground( dpy, gc, COLOR( PAD ) );
        XSetForeground( dpy, gc, COLOR( WHITE ) );

        pix = XCreateBitmapFromData( dpy, local_keypad->pixmap, ( char* )last_bitmap, last_width, last_height );

        x = offset_x + buttons[ HPKEY_1 ].x + buttons[ HPKEY_1 ].w +
            ( buttons[ HPKEY_2 ].x - buttons[ HPKEY_1 ].x - buttons[ HPKEY_1 ].w ) / 2;
        y = offset_y + buttons[ HPKEY_5 ].y + buttons[ HPKEY_5 ].h + 2;

        XCopyPlane( dpy, pix, local_keypad->pixmap, gc, 0, 0, last_width, last_height, x, y, 1 );

        XFreePixmap( dpy, pix );
    }
}

static inline void _CreateBezel( x11_keypad_t* local_keypad )
{
    int i;

    /*
     * draw the frame around the display
     */
    XSetForeground( dpy, gc, COLOR( DISP_PAD_TOP ) );

    for ( i = 0; i < DISP_FRAME; i++ ) {
        XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X - i ), ( int )( DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + 2 * i ),
                   ( int )( DISPLAY_OFFSET_X + DISPLAY_WIDTH + i ), ( int )( DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + 2 * i ) );
        XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X - i ), ( int )( DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + 2 * i + 1 ),
                   ( int )( DISPLAY_OFFSET_X + DISPLAY_WIDTH + i ), ( int )( DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + 2 * i + 1 ) );
        XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X + DISPLAY_WIDTH + i ), ( int )( DISPLAY_OFFSET_Y - i ),
                   ( int )( DISPLAY_OFFSET_X + DISPLAY_WIDTH + i ), ( int )( DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + 2 * i ) );
    }

    XSetForeground( dpy, gc, COLOR( DISP_PAD_BOT ) );

    for ( i = 0; i < DISP_FRAME; i++ ) {
        XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X - i - 1 ), ( int )( DISPLAY_OFFSET_Y - i - 1 ),
                   ( int )( DISPLAY_OFFSET_X + DISPLAY_WIDTH + i - 1 ), ( int )( DISPLAY_OFFSET_Y - i - 1 ) );
        XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X - i - 1 ), ( int )( DISPLAY_OFFSET_Y - i - 1 ),
                   ( int )( DISPLAY_OFFSET_X - i - 1 ), ( int )( DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + 2 * i - 1 ) );
    }

    /*
     * round off corners
     */
    XSetForeground( dpy, gc, COLOR( DISP_PAD ) );

    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X - DISP_FRAME ), ( int )( DISPLAY_OFFSET_Y - DISP_FRAME ),
               ( int )( DISPLAY_OFFSET_X - DISP_FRAME + 3 ), ( int )( DISPLAY_OFFSET_Y - DISP_FRAME ) );
    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X - DISP_FRAME ), ( int )( DISPLAY_OFFSET_Y - DISP_FRAME ),
               ( int )( DISPLAY_OFFSET_X - DISP_FRAME ), ( int )( DISPLAY_OFFSET_Y - DISP_FRAME + 3 ) );
    XDrawPoint( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X - DISP_FRAME + 1 ), ( int )( DISPLAY_OFFSET_Y - DISP_FRAME + 1 ) );

    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X + DISPLAY_WIDTH + DISP_FRAME - 4 ),
               ( int )( DISPLAY_OFFSET_Y - DISP_FRAME ), ( int )( DISPLAY_OFFSET_X + DISPLAY_WIDTH + DISP_FRAME - 1 ),
               ( int )( DISPLAY_OFFSET_Y - DISP_FRAME ) );
    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X + DISPLAY_WIDTH + DISP_FRAME - 1 ),
               ( int )( DISPLAY_OFFSET_Y - DISP_FRAME ), ( int )( DISPLAY_OFFSET_X + DISPLAY_WIDTH + DISP_FRAME - 1 ),
               ( int )( DISPLAY_OFFSET_Y - DISP_FRAME + 3 ) );
    XDrawPoint( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X + DISPLAY_WIDTH + DISP_FRAME - 2 ),
                ( int )( DISPLAY_OFFSET_Y - DISP_FRAME + 1 ) );

    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X - DISP_FRAME ),
               ( int )( DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 4 ), ( int )( DISPLAY_OFFSET_X - DISP_FRAME ),
               ( int )( DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 1 ) );
    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X - DISP_FRAME ),
               ( int )( DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 1 ), ( int )( DISPLAY_OFFSET_X - DISP_FRAME + 3 ),
               ( int )( DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 1 ) );
    XDrawPoint( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X - DISP_FRAME + 1 ),
                ( int )( DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 2 ) );

    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X + DISPLAY_WIDTH + DISP_FRAME - 1 ),
               ( int )( DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 4 ),
               ( int )( DISPLAY_OFFSET_X + DISPLAY_WIDTH + DISP_FRAME - 1 ),
               ( int )( DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 1 ) );
    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X + DISPLAY_WIDTH + DISP_FRAME - 4 ),
               ( int )( DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 1 ),
               ( int )( DISPLAY_OFFSET_X + DISPLAY_WIDTH + DISP_FRAME - 1 ),
               ( int )( DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 1 ) );
    XDrawPoint( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X + DISPLAY_WIDTH + DISP_FRAME - 2 ),
                ( int )( DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + 2 * DISP_FRAME - 2 ) );

    /*
     * simulate rounded lcd corners
     */
    XSetForeground( dpy, gc, COLOR( LCD ) );

    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X - 1 ), ( int )( DISPLAY_OFFSET_Y + 1 ),
               ( int )( DISPLAY_OFFSET_X - 1 ), ( int )( DISPLAY_OFFSET_Y + DISPLAY_HEIGHT - 2 ) );
    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X + 1 ), ( int )( DISPLAY_OFFSET_Y - 1 ),
               ( int )( DISPLAY_OFFSET_X + DISPLAY_WIDTH - 2 ), ( int )( DISPLAY_OFFSET_Y - 1 ) );
    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X + 1 ), ( int )( DISPLAY_OFFSET_Y + DISPLAY_HEIGHT ),
               ( int )( DISPLAY_OFFSET_X + DISPLAY_WIDTH - 2 ), ( int )( DISPLAY_OFFSET_Y + DISPLAY_HEIGHT ) );
    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X + DISPLAY_WIDTH ), ( int )( DISPLAY_OFFSET_Y + 1 ),
               ( int )( DISPLAY_OFFSET_X + DISPLAY_WIDTH ), ( int )( DISPLAY_OFFSET_Y + DISPLAY_HEIGHT - 2 ) );
}

static inline void _DrawMore( unsigned int offset_y, x11_keypad_t* local_keypad )
{
    Pixmap pix;
    int cut = 0;
    int x, y;

    /*
     * lower the whole thing
     */
    XSetForeground( dpy, gc, COLOR( PAD_TOP ) );

    /* bottom lines */
    XDrawLine( dpy, local_keypad->pixmap, gc, 1, ( int )( local_keypad->height - 1 ), ( int )( local_keypad->width - 1 ),
               ( int )( local_keypad->height - 1 ) );
    XDrawLine( dpy, local_keypad->pixmap, gc, 2, ( int )( local_keypad->height - 2 ), ( int )( local_keypad->width - 2 ),
               ( int )( local_keypad->height - 2 ) );

    /* right lines */
    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( local_keypad->width - 1 ), ( int )( local_keypad->height - 1 ),
               ( int )( local_keypad->width - 1 ), cut );
    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( local_keypad->width - 2 ), ( int )( local_keypad->height - 2 ),
               ( int )( local_keypad->width - 2 ), cut );

    XSetForeground( dpy, gc, COLOR( DISP_PAD_TOP ) );

    /* right lines */
    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( local_keypad->width - 1 ), cut - 1, ( int )( local_keypad->width - 1 ), 1 );
    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( local_keypad->width - 2 ), cut - 1, ( int )( local_keypad->width - 2 ), 2 );

    XSetForeground( dpy, gc, COLOR( DISP_PAD_BOT ) );

    /* top lines */
    XDrawLine( dpy, local_keypad->pixmap, gc, 0, 0, ( int )( local_keypad->width - 2 ), 0 );
    XDrawLine( dpy, local_keypad->pixmap, gc, 1, 1, ( int )( local_keypad->width - 3 ), 1 );

    /* left lines */
    XDrawLine( dpy, local_keypad->pixmap, gc, 0, cut - 1, 0, 0 );
    XDrawLine( dpy, local_keypad->pixmap, gc, 1, cut - 1, 1, 1 );

    XSetForeground( dpy, gc, COLOR( PAD_BOT ) );

    /* left lines */
    XDrawLine( dpy, local_keypad->pixmap, gc, 0, ( int )( local_keypad->height - 2 ), 0, cut );
    XDrawLine( dpy, local_keypad->pixmap, gc, 1, ( int )( local_keypad->height - 3 ), 1, cut );

    /*
     * lower the menu buttons
     */
    XSetForeground( dpy, gc, COLOR( PAD_TOP ) );

    /* bottom lines */
    XDrawLine( dpy, local_keypad->pixmap, gc, 3, ( int )( local_keypad->height - 3 ), ( int )( local_keypad->width - 3 ),
               ( int )( local_keypad->height - 3 ) );
    XDrawLine( dpy, local_keypad->pixmap, gc, 4, ( int )( local_keypad->height - 4 ), ( int )( local_keypad->width - 4 ),
               ( int )( local_keypad->height - 4 ) );

    /* right lines */
    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( local_keypad->width - 3 ), ( int )( local_keypad->height - 3 ),
               ( int )( local_keypad->width - 3 ), cut );
    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( local_keypad->width - 4 ), ( int )( local_keypad->height - 4 ),
               ( int )( local_keypad->width - 4 ), cut );

    XSetForeground( dpy, gc, COLOR( DISP_PAD_TOP ) );

    /* right lines */
    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( local_keypad->width - 3 ), cut - 1, ( int )( local_keypad->width - 3 ),
               offset_y - 24 );
    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( local_keypad->width - 4 ), cut - 1, ( int )( local_keypad->width - 4 ),
               offset_y - 23 );

    XSetForeground( dpy, gc, COLOR( DISP_PAD_BOT ) );

    /* top lines */
    XDrawLine( dpy, local_keypad->pixmap, gc, 2, offset_y - 25, ( int )( local_keypad->width - 4 ), offset_y - 25 );
    XDrawLine( dpy, local_keypad->pixmap, gc, 3, offset_y - 24, ( int )( local_keypad->width - 5 ), offset_y - 24 );

    /* left lines */
    XDrawLine( dpy, local_keypad->pixmap, gc, 2, cut - 1, 2, offset_y - 24 );
    XDrawLine( dpy, local_keypad->pixmap, gc, 3, cut - 1, 3, offset_y - 23 );

    XSetForeground( dpy, gc, COLOR( PAD_BOT ) );

    /* left lines */
    XDrawLine( dpy, local_keypad->pixmap, gc, 2, ( int )( local_keypad->height - 4 ), 2, cut );
    XDrawLine( dpy, local_keypad->pixmap, gc, 3, ( int )( local_keypad->height - 5 ), 3, cut );

    /*
     * lower the keyboard
     */
    XSetForeground( dpy, gc, COLOR( PAD_TOP ) );

    /* bottom lines */
    XDrawLine( dpy, local_keypad->pixmap, gc, 5, ( int )( local_keypad->height - 5 ), ( int )( local_keypad->width - 3 ),
               ( int )( local_keypad->height - 5 ) );
    XDrawLine( dpy, local_keypad->pixmap, gc, 6, ( int )( local_keypad->height - 6 ), ( int )( local_keypad->width - 4 ),
               ( int )( local_keypad->height - 6 ) );

    /* right lines */
    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( local_keypad->width - 5 ), ( int )( local_keypad->height - 5 ),
               ( int )( local_keypad->width - 5 ), cut + 1 );
    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( local_keypad->width - 6 ), ( int )( local_keypad->height - 6 ),
               ( int )( local_keypad->width - 6 ), cut + 2 );

    XSetForeground( dpy, gc, COLOR( DISP_PAD_BOT ) );

    /* top lines */
    XDrawLine( dpy, local_keypad->pixmap, gc, 4, cut, ( int )( local_keypad->width - 6 ), cut );
    XDrawLine( dpy, local_keypad->pixmap, gc, 5, cut + 1, ( int )( local_keypad->width - 7 ), cut + 1 );

    XSetForeground( dpy, gc, COLOR( PAD_BOT ) );

    /* left lines */
    XDrawLine( dpy, local_keypad->pixmap, gc, 4, ( int )( local_keypad->height - 6 ), 4, cut + 1 );
    XDrawLine( dpy, local_keypad->pixmap, gc, 5, ( int )( local_keypad->height - 7 ), 5, cut + 2 );

    /*
     * round off the bottom edge
     */
    XSetForeground( dpy, gc, COLOR( PAD_TOP ) );

    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( local_keypad->width - 7 ), ( int )( local_keypad->height - 7 ),
               ( int )( local_keypad->width - 7 ), ( int )( local_keypad->height - 14 ) );
    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( local_keypad->width - 8 ), ( int )( local_keypad->height - 8 ),
               ( int )( local_keypad->width - 8 ), ( int )( local_keypad->height - 11 ) );
    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( local_keypad->width - 7 ), ( int )( local_keypad->height - 7 ),
               ( int )( local_keypad->width - 14 ), ( int )( local_keypad->height - 7 ) );
    XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( local_keypad->width - 7 ), ( int )( local_keypad->height - 8 ),
               ( int )( local_keypad->width - 11 ), ( int )( local_keypad->height - 8 ) );
    XDrawPoint( dpy, local_keypad->pixmap, gc, ( int )( local_keypad->width - 9 ), ( int )( local_keypad->height - 9 ) );

    XDrawLine( dpy, local_keypad->pixmap, gc, 7, ( int )( local_keypad->height - 7 ), 13, ( int )( local_keypad->height - 7 ) );
    XDrawLine( dpy, local_keypad->pixmap, gc, 8, ( int )( local_keypad->height - 8 ), 10, ( int )( local_keypad->height - 8 ) );
    XSetForeground( dpy, gc, COLOR( PAD_BOT ) );
    XDrawLine( dpy, local_keypad->pixmap, gc, 6, ( int )( local_keypad->height - 8 ), 6, ( int )( local_keypad->height - 14 ) );
    XDrawLine( dpy, local_keypad->pixmap, gc, 7, ( int )( local_keypad->height - 9 ), 7, ( int )( local_keypad->height - 11 ) );

    /*
     * insert the HP Logo
     */

    XSetBackground( dpy, gc, COLOR( LOGO_BACK ) );
    XSetForeground( dpy, gc, COLOR( LOGO ) );

    pix = XCreateBitmapFromData( dpy, local_keypad->pixmap, ( char* )hp_bitmap, hp_width, hp_height );

    x = opt_gx ? DISPLAY_OFFSET_X - 6 : DISPLAY_OFFSET_X;

    XCopyPlane( dpy, pix, local_keypad->pixmap, gc, 0, 0, hp_width, hp_height, x, 10, 1 );

    XFreePixmap( dpy, pix );

    if ( !opt_gx ) {
        XSetForeground( dpy, gc, COLOR( FRAME ) );

        XDrawLine( dpy, local_keypad->pixmap, gc, ( int )DISPLAY_OFFSET_X, 9, ( int )( DISPLAY_OFFSET_X + hp_width - 1 ), 9 );
        XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X - 1 ), 10, ( int )( DISPLAY_OFFSET_X - 1 ),
                   10 + hp_height - 1 );
        XDrawLine( dpy, local_keypad->pixmap, gc, ( int )DISPLAY_OFFSET_X, 10 + hp_height, ( int )( DISPLAY_OFFSET_X + hp_width - 1 ),
                   10 + hp_height );
        XDrawLine( dpy, local_keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X + hp_width ), 10, ( int )( DISPLAY_OFFSET_X + hp_width ),
                   10 + hp_height - 1 );
    }

    /*
     * write the name of it
     */
    XSetBackground( dpy, gc, COLOR( DISP_PAD ) );
    XSetForeground( dpy, gc, COLOR( LABEL ) );

    if ( opt_gx ) {
        x = DISPLAY_OFFSET_X + DISPLAY_WIDTH - gx_128K_ram_width + gx_128K_ram_x_hot + 2;
        y = 10 + gx_128K_ram_y_hot;
        pix = XCreateBitmapFromData( dpy, local_keypad->pixmap, ( char* )gx_128K_ram_bitmap, gx_128K_ram_width, gx_128K_ram_height );
        XCopyPlane( dpy, pix, local_keypad->pixmap, gc, 0, 0, gx_128K_ram_width, gx_128K_ram_height, x, y, 1 );
        XFreePixmap( dpy, pix );

        XSetForeground( dpy, gc, COLOR( LOGO ) );
        x = DISPLAY_OFFSET_X + hp_width;
        y = hp_height + 8 - hp48gx_height;
        pix = XCreateBitmapFromData( dpy, local_keypad->pixmap, ( char* )hp48gx_bitmap, hp48gx_width, hp48gx_height );
        XCopyPlane( dpy, pix, local_keypad->pixmap, gc, 0, 0, hp48gx_width, hp48gx_height, x, y, 1 );
        XFreePixmap( dpy, pix );

        XSetFillStyle( dpy, gc, FillStippled );
        x = DISPLAY_OFFSET_X + DISPLAY_WIDTH - gx_128K_ram_width + gx_silver_x_hot + 2;
        y = 10 + gx_silver_y_hot;
        pix = XCreateBitmapFromData( dpy, local_keypad->pixmap, ( char* )gx_silver_bitmap, gx_silver_width, gx_silver_height );
        XSetStipple( dpy, gc, pix );
        XSetTSOrigin( dpy, gc, x, y );
        XFillRectangle( dpy, local_keypad->pixmap, gc, x, y, gx_silver_width, gx_silver_height );
        XFreePixmap( dpy, pix );

        XSetForeground( dpy, gc, COLOR( RIGHT ) );
        x = DISPLAY_OFFSET_X + DISPLAY_WIDTH - gx_128K_ram_width + gx_green_x_hot + 2;
        y = 10 + gx_green_y_hot;
        pix = XCreateBitmapFromData( dpy, local_keypad->pixmap, ( char* )gx_green_bitmap, gx_green_width, gx_green_height );
        XSetStipple( dpy, gc, pix );
        XSetTSOrigin( dpy, gc, x, y );
        XFillRectangle( dpy, local_keypad->pixmap, gc, x, y, gx_green_width, gx_green_height );
        XFreePixmap( dpy, pix );

        XSetTSOrigin( dpy, gc, 0, 0 );
        XSetFillStyle( dpy, gc, FillSolid );
    } else {
        x = DISPLAY_OFFSET_X;
        y = TOP_SKIP - DISP_FRAME - hp48sx_height - 3;

        pix = XCreateBitmapFromData( dpy, local_keypad->pixmap, ( char* )hp48sx_bitmap, hp48sx_width, hp48sx_height );

        XCopyPlane( dpy, pix, local_keypad->pixmap, gc, 0, 0, hp48sx_width, hp48sx_height, x, y, 1 );

        XFreePixmap( dpy, pix );

        x = DISPLAY_OFFSET_X + DISPLAY_WIDTH - 1 - science_width;
        y = TOP_SKIP - DISP_FRAME - science_height - 4;

        pix = XCreateBitmapFromData( dpy, local_keypad->pixmap, ( char* )science_bitmap, science_width, science_height );

        XCopyPlane( dpy, pix, local_keypad->pixmap, gc, 0, 0, science_width, science_height, x, y, 1 );
    }
}

static void DrawKeypad( x11_keypad_t* local_keypad )
{
    XCopyArea( dpy, local_keypad->pixmap, mainW, gc, 0, 0, local_keypad->width, local_keypad->height, 0, 0 );
}

static inline void DrawButton( int i )
{
    if ( keyboard[ i ].pressed )
        XCopyArea( dpy, buttons[ i ].down, buttons[ i ].xwin, gc, 0, 0, buttons[ i ].w, buttons[ i ].h, 0, 0 );
    else
        XCopyArea( dpy, buttons[ i ].map, buttons[ i ].xwin, gc, 0, 0, buttons[ i ].w, buttons[ i ].h, 0, 0 );
}

static inline void _CreateIcon( void )
{
    XSetWindowAttributes xswa;
    XWindowAttributes xwa;
    Pixmap tmp_pix;
    int p;

    XGetWindowAttributes( dpy, iconW, &xwa );
    xswa.event_mask = xwa.your_event_mask | ExposureMask;
    xswa.backing_store = Always;
    XChangeWindowAttributes( dpy, iconW, CWEventMask | CWBackingStore, &xswa );

    icon_pix = XCreatePixmap( dpy, iconW, hp48_icon_width, hp48_icon_height, depth );

    /*
     * draw the icon pixmap
     */
    if ( icon_color_mode == COLOR_MODE_MONO ) {
        tmp_pix =
            XCreateBitmapFromData( dpy, iconW, ( char* )icon_maps[ ICON_MAP ].bits, icon_maps[ ICON_MAP ].w, icon_maps[ ICON_MAP ].h );
        XSetForeground( dpy, gc, COLOR( BLACK ) );
        XSetBackground( dpy, gc, COLOR( WHITE ) );
        XCopyPlane( dpy, tmp_pix, icon_pix, gc, 0, 0, icon_maps[ ICON_MAP ].w, icon_maps[ ICON_MAP ].h, 0, 0, 1 );
        XFreePixmap( dpy, tmp_pix );
    } else {
        XSetFillStyle( dpy, gc, FillStippled );
        for ( p = FIRST_MAP; p <= LAST_MAP; p++ ) {
            tmp_pix = XCreateBitmapFromData( dpy, iconW, ( char* )icon_maps[ p ].bits, icon_maps[ p ].w, icon_maps[ p ].h );
            XSetStipple( dpy, gc, tmp_pix );
            XSetForeground( dpy, gc, COLOR( icon_maps[ p ].c ) );
            XFillRectangle( dpy, icon_pix, gc, 0, 0, icon_maps[ p ].w, icon_maps[ p ].h );
            XFreePixmap( dpy, tmp_pix );
        }
        XSetFillStyle( dpy, gc, FillSolid );

        /*
         * draw frame around icon
         */
        XSetForeground( dpy, gc, COLOR( BLACK ) );
        XDrawRectangle( dpy, icon_pix, gc, 0, 0, icon_maps[ ICON_MAP ].w - 1, icon_maps[ ICON_MAP ].h - 1 );
    }

    /*
     * draw the display
     */
    XSetFillStyle( dpy, gc, FillStippled );
    icon_disp_pix =
        XCreateBitmapFromData( dpy, iconW, ( char* )icon_maps[ DISP_MAP ].bits, icon_maps[ DISP_MAP ].w, icon_maps[ DISP_MAP ].h );
    XSetStipple( dpy, gc, icon_disp_pix );
    if ( icon_color_mode == COLOR_MODE_MONO )
        XSetForeground( dpy, gc, COLOR( WHITE ) );
    else
        XSetForeground( dpy, gc, COLOR( icon_maps[ DISP_MAP ].c ) );
    XFillRectangle( dpy, icon_pix, gc, 0, 0, icon_maps[ DISP_MAP ].w, icon_maps[ DISP_MAP ].h );

    /*
     * draw the 'x48' string
     */
    icon_text_pix = XCreateBitmapFromData( dpy, iconW, ( char* )icon_maps[ ON_MAP ].bits, icon_maps[ ON_MAP ].w, icon_maps[ ON_MAP ].h );
    XSetStipple( dpy, gc, icon_text_pix );
    if ( icon_color_mode == COLOR_MODE_MONO )
        XSetForeground( dpy, gc, COLOR( BLACK ) );
    else
        XSetForeground( dpy, gc, COLOR( icon_maps[ ON_MAP ].c ) );
    XFillRectangle( dpy, icon_pix, gc, 0, 0, icon_maps[ ON_MAP ].w, icon_maps[ ON_MAP ].h );
    XSetFillStyle( dpy, gc, FillSolid );
}

static inline void DrawIcon( void ) { XCopyArea( dpy, icon_pix, iconW, gc, 0, 0, hp48_icon_width, hp48_icon_height, 0, 0 ); }

static void refresh_icon( void )
{
    int icon_state = ( ( display.on && !( ( ANN_IO & saturn.annunc ) == ANN_IO ) ) ||
                       ( display.on && !( ( ANN_ALPHA & saturn.annunc ) == ANN_ALPHA ) ) );
    if ( icon_state == last_icon_state )
        return;

    last_icon_state = icon_state;
    XSetFillStyle( dpy, gc, FillStippled );
    if ( icon_state ) {
        /*
         * draw the 'x48' string
         */
        XSetStipple( dpy, gc, icon_text_pix );
        if ( icon_color_mode == COLOR_MODE_MONO )
            XSetForeground( dpy, gc, COLOR( BLACK ) );
        else
            XSetForeground( dpy, gc, COLOR( icon_maps[ ON_MAP ].c ) );
        XFillRectangle( dpy, icon_pix, gc, 0, 0, icon_maps[ ON_MAP ].w, icon_maps[ ON_MAP ].h );
    } else {
        /*
         * clear the display
         */
        XSetFillStyle( dpy, gc, FillStippled );
        XSetStipple( dpy, gc, icon_disp_pix );
        if ( icon_color_mode == COLOR_MODE_MONO )
            XSetForeground( dpy, gc, COLOR( WHITE ) );
        else
            XSetForeground( dpy, gc, COLOR( icon_maps[ DISP_MAP ].c ) );
        XFillRectangle( dpy, icon_pix, gc, 0, 0, icon_maps[ DISP_MAP ].w, icon_maps[ DISP_MAP ].h );
    }
    XSetFillStyle( dpy, gc, FillSolid );
    if ( iconW )
      DrawIcon();
}

int handle_xerror( Display* _the_dpy, XErrorEvent* _eev )
{
    xerror_flag = true;

    return 0;
}

static inline void _CreateLCDWindow( void )
{
    XSetWindowAttributes xswa;
    XGCValues val;
    unsigned long gc_mask;
    static XRectangle rect;

    /*
     * create the display subwindow
     */
    lcd.win = XCreateSimpleWindow( dpy, mainW, ( int )DISPLAY_OFFSET_X, ( int )DISPLAY_OFFSET_Y, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0,
                                   COLOR( BLACK ), COLOR( LCD ) );

    mapped = true;

    xswa.event_mask = ExposureMask | StructureNotifyMask;
    xswa.backing_store = Always;

    XChangeWindowAttributes( dpy, lcd.win, CWEventMask | CWBackingStore, &xswa );

    /*
     * set up the gc
     */
    val.foreground = COLOR( LCD );
    val.background = COLOR( LCD );
    val.function = GXcopy;
    gc_mask = GCForeground | GCBackground | GCFunction;
    lcd.gc = XCreateGC( dpy, mainW, gc_mask, &val );

    XSetForeground( dpy, lcd.gc, COLOR( PIXEL ) );

    lcd.display_update = UPDATE_DISP | UPDATE_MENU;

    xerror_flag = false;
    XSetErrorHandler( handle_xerror );
    XFlush( dpy );

    lcd.disp_image = NULL;
    lcd.menu_image = NULL;
    if ( shm_flag ) {
        /*
         * create XShmImage for DISP
         */
        lcd.disp_image = XShmCreateImage( dpy, None, 1, XYBitmap, NULL, &lcd.disp_info, 262, 128 );
        if ( lcd.disp_image == NULL ) {
            shm_flag = false;
            if ( config.verbose )
                fprintf( stderr, "XShm error in XShmCreateImage(DISP), disabling.\n" );
            goto shm_error;
        }

        /*
         * get ID of shared memory block for DISP
         */
        lcd.disp_info.shmid = shmget( IPC_PRIVATE, ( lcd.disp_image->bytes_per_line * lcd.disp_image->height ), IPC_CREAT | 0777 );
        if ( lcd.disp_info.shmid < 0 ) {
            XDestroyImage( lcd.disp_image );
            lcd.disp_image = NULL;
            shm_flag = false;
            if ( config.verbose )
                fprintf( stderr, "XShm error in shmget(DISP), disabling.\n" );
            goto shm_error;
        }

        /*
         * get address of shared memory block for DISP
         */
        lcd.disp_info.shmaddr = ( char* )shmat( lcd.disp_info.shmid, 0, 0 );
        if ( lcd.disp_info.shmaddr == ( ( char* )-1 ) ) {
            XDestroyImage( lcd.disp_image );
            lcd.disp_image = NULL;
            shm_flag = false;
            if ( config.verbose )
                fprintf( stderr, "XShm error in shmat(DISP), disabling.\n" );
            goto shm_error;
        }
        lcd.disp_image->data = lcd.disp_info.shmaddr;
        lcd.disp_info.readOnly = False;
        XShmAttach( dpy, &lcd.disp_info );

        /*
         * create XShmImage for MENU
         */
        lcd.menu_image = XShmCreateImage( dpy, None, 1, XYBitmap, NULL, &lcd.menu_info, 262, 128 );
        if ( lcd.menu_image == NULL ) {
            XDestroyImage( lcd.disp_image );
            lcd.disp_image = NULL;
            shm_flag = false;
            if ( config.verbose )
                fprintf( stderr, "XShm error in XShmCreateImage(MENU), disabling.\n" );
            goto shm_error;
        }

        /*
         * get ID of shared memory block for MENU
         */
        lcd.menu_info.shmid = shmget( IPC_PRIVATE, ( lcd.menu_image->bytes_per_line * lcd.menu_image->height ), IPC_CREAT | 0777 );
        if ( lcd.menu_info.shmid < 0 ) {
            XDestroyImage( lcd.disp_image );
            lcd.disp_image = NULL;
            XDestroyImage( lcd.menu_image );
            lcd.menu_image = NULL;
            shm_flag = false;
            if ( config.verbose )
                fprintf( stderr, "XShm error in shmget(MENU), disabling.\n" );
            goto shm_error;
        }

        /*
         * get address of shared memory block for MENU
         */
        lcd.menu_info.shmaddr = ( char* )shmat( lcd.menu_info.shmid, 0, 0 );
        if ( lcd.menu_info.shmaddr == ( ( char* )-1 ) ) {
            XDestroyImage( lcd.disp_image );
            lcd.disp_image = NULL;
            XDestroyImage( lcd.menu_image );
            lcd.menu_image = NULL;
            shm_flag = false;
            if ( config.verbose )
                fprintf( stderr, "XShm error in shmat(MENU), disabling.\n" );
            goto shm_error;
        }
        lcd.menu_image->data = lcd.menu_info.shmaddr;
        lcd.menu_info.readOnly = False;
        XShmAttach( dpy, &lcd.menu_info );

        XFlush( dpy );
        XSync( dpy, 0 );
        sleep( 1 );

        if ( xerror_flag ) {
            XDestroyImage( lcd.disp_image );
            lcd.disp_image = NULL;
            XDestroyImage( lcd.menu_image );
            lcd.menu_image = NULL;
            shm_flag = false;
            if ( config.verbose )
                fprintf( stderr, "XShm error in shmget(MENU), disabling.\n" );
            goto shm_error;
        } else {
            shmctl( lcd.disp_info.shmid, IPC_RMID, 0 );
            shmctl( lcd.menu_info.shmid, IPC_RMID, 0 );
        }

        memset( lcd.disp_image->data, 0, ( size_t )( lcd.disp_image->bytes_per_line * lcd.disp_image->height ) );
        memset( lcd.menu_image->data, 0, ( size_t )( lcd.menu_image->bytes_per_line * lcd.menu_image->height ) );

        if ( config.verbose )
            printf( "using XShm extension.\n" );

        CompletionType = XShmGetEventBase( dpy ) + ShmCompletion;
    }

shm_error:
    XSetErrorHandler( NULL );
    XFlush( dpy );

    if ( !shm_flag ) {
        rect.x = 5;
        rect.y = 0;
        rect.width = 262;
        rect.height = DISPLAY_HEIGHT;
        XSetClipRectangles( dpy, lcd.gc, 0, 0, &rect, 1, Unsorted );
    }
}

static void _DrawSerialDevices( char* wire, char* ir )
{
    char name[ 128 ];
    int x, y, w, h;
    int conn_top = DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + 18;
    XFontStruct* finfo = load_x11_font( dpy, config.connFont );
    XGCValues val;
    unsigned long gc_mask = GCFont;
    XCharStruct xchar;

    val.font = finfo->fid;
    XChangeGC( dpy, gc, gc_mask, &val );

    int dir, fa, fd;
    XTextExtents( finfo, "TEST", ( int )strlen( "TEST" ), &dir, &fa, &fd, &xchar );
    w = DISPLAY_WIDTH;
    h = fa + fd;

    Pixmap pix = XCreatePixmap( dpy, keypad.pixmap, w, h, depth );
    XSetForeground( dpy, gc, COLOR( DISP_PAD ) );
    XFillRectangle( dpy, pix, gc, 0, 0, w, h );

    XSetBackground( dpy, gc, COLOR( DISP_PAD ) );
    XSetForeground( dpy, gc, COLOR( LABEL ) );

    sprintf( name, "wire: %s", wire ? wire : "none" );
    XTextExtents( finfo, name, ( int )strlen( name ), &dir, &fa, &fd, &xchar );
    x = 0;
    y = fa;
    XDrawImageString( dpy, pix, gc, x, y, name, ( int )strlen( name ) );

    sprintf( name, "IR: %s", ir ? ir : "none" );
    XTextExtents( finfo, name, ( int )strlen( name ), &dir, &fa, &fd, &xchar );
    x = w - xchar.width - 1;
    y = fa;
    XDrawImageString( dpy, pix, gc, x, y, name, ( int )strlen( name ) );

    x = DISPLAY_OFFSET_X;
    y = conn_top;
    XCopyArea( dpy, pix, keypad.pixmap, gc, 0, 0, w, h, x, y ); /* write pix onto keypad pixmap */

    DrawKeypad( &keypad );

    XFreePixmap( dpy, pix );
    XFreeFont( dpy, finfo );
}

static inline void _CreateBackground( int width, int height, int w_top, int h_top, x11_keypad_t* local_keypad )
{
    XSetBackground( dpy, gc, COLOR( PAD ) );
    XSetForeground( dpy, gc, COLOR( PAD ) );

    XFillRectangle( dpy, local_keypad->pixmap, gc, 0, 0, w_top, h_top );

    XSetBackground( dpy, gc, COLOR( DISP_PAD ) );
    XSetForeground( dpy, gc, COLOR( DISP_PAD ) );

    XFillRectangle( dpy, local_keypad->pixmap, gc, 0, 0, width, height );
}

static bool CreateWindows( int argc, char** argv )
{
    XSizeHints hint, ih;
    XWMHints wmh;
    XClassHint clh;
    unsigned int class;
    XGCValues val;
    unsigned long gc_mask;
    unsigned int mask;
    XSetWindowAttributes xswa;
    XTextProperty wname, iname;
    Atom protocols[ 2 ];
    char *name, *user_geom = NULL, def_geom[ 40 ];
    int info, x, y, w, h;
    unsigned int width, height;

    if ( opt_gx ) {
        buttons = buttons_gx;
        colors = colors_gx;
        icon_maps = icon_maps_gx;
    } else {
        buttons = buttons_sx;
        colors = colors_sx;
        icon_maps = icon_maps_sx;
    }

    if ( config.netbook ) {
        int i;
        for ( i = 0; i < 6; i++ ) {
            buttons[ i ].x -= 3;
            buttons[ i ].y += 300;
        }
        for ( ; i <= LAST_HPKEY; i++ ) {
            buttons[ i ].x += 317;
            buttons[ i ].y -= 3;
        }
    }

    class = InputOutput;
    visual = get_visual_resource( dpy, &depth );
    if ( visual != DefaultVisual( dpy, screen ) )
        cmap = XCreateColormap( dpy, RootWindow( dpy, screen ), visual, ( visual->class == DirectColor ) ? AllocAll : AllocNone );
    else
        cmap = DefaultColormap( dpy, screen );

    direct_color = false;
    switch ( visual->class ) {
        case DirectColor:
            direct_color = true;
            break;
        case GrayScale:
        case PseudoColor:
            dynamic_color = true;
            break;
        case StaticGray:
        case StaticColor:
        case TrueColor:
        default:
            dynamic_color = false;
            break;
    }

    if ( ( visual->class == StaticGray ) || ( visual->class == GrayScale ) )
        color_mode = COLOR_MODE_GRAY;
    else
        color_mode = COLOR_MODE_COLOR;
    if ( config.gray )
        color_mode = COLOR_MODE_GRAY;

    if ( config.mono )
        color_mode = COLOR_MODE_MONO;
    if ( depth == 1 )
        color_mode = COLOR_MODE_MONO;

    icon_color_mode = color_mode;
    if ( config.monoIcon )
        icon_color_mode = COLOR_MODE_MONO;

    clh.res_name = res_name;
    clh.res_class = res_class;
    if ( !XStringListToTextProperty( &config.progname, 1, &iname ) )
        return false;

    if ( ( name = config.title ) == ( char* )0 ) {
        name = ( char* )malloc( 128 );
        if ( name == ( char* )0 )
            fatal_exit( "malloc failed.\n", "" );

        sprintf( name, "%s-%d.%d.%d", config.progname, saturn.version[ 0 ], saturn.version[ 1 ], saturn.version[ 2 ] );
    }

    if ( !XStringListToTextProperty( &name, 1, &wname ) )
        return false;

    /*
     * Set some Window Attributes
     */
    xswa.colormap = cmap;
    mask = CWColormap;

    /*
     * create the window
     */
    width = KEYBOARD_WIDTH + 2 * SIDE_SKIP;
    if ( config.netbook )
        height = KEYBOARD_HEIGHT;
    else
        height = DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + DISP_KBD_SKIP + KEYBOARD_HEIGHT + BOTTOM_SKIP;

    mainW = XCreateWindow( dpy, RootWindow( dpy, screen ), 0, 0, width, height, 0, ( int )depth, class, visual, mask, &xswa );

    if ( mainW == 0 )
        return false;

    /*
     * allocate my colors
     */
    AllocColors();

    /*
     * parse -geometry ...
     */
    hint.x = hint.y = 0;
    hint.min_width = hint.max_width = hint.base_width = hint.width = width;
    hint.min_height = hint.max_height = hint.base_height = hint.height = height;
    hint.win_gravity = NorthWestGravity;
    hint.flags = PSize | PMinSize | PMaxSize | PBaseSize | PWinGravity;

    sprintf( def_geom, "%ux%u", width, height );

    info = XWMGeometry( dpy, screen, user_geom, def_geom, 0, &hint, &x, &y, &w, &h, &hint.win_gravity );

    if ( info & ( XValue | YValue ) ) {
        if ( info & XValue )
            hint.x = x;
        if ( info & YValue )
            hint.y = y;
        hint.flags |= USPosition;
    }

    /*
     * check if we start iconic
     */
    wmh.initial_state = ( config.iconic ) ? IconicState : NormalState;
    wmh.input = True;
    wmh.flags = StateHint | InputHint;

    /*
     * Set some more Window Attributes
     */
    xswa.background_pixel = COLOR( PAD );
    xswa.border_pixel = COLOR( BLACK );
    xswa.backing_store = Always;
    xswa.win_gravity = hint.win_gravity;
    xswa.bit_gravity = NorthWestGravity;
    xswa.event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | ExposureMask | KeymapStateMask |
                      EnterWindowMask | StructureNotifyMask | FocusChangeMask;
    mask = CWBackPixel | CWBorderPixel | CWBackingStore | CWEventMask | CWBitGravity | CWWinGravity;
    XChangeWindowAttributes( dpy, mainW, mask, &xswa );
    XMoveWindow( dpy, mainW, hint.x, hint.y );

    /*
     * create the icon
     */
    xswa.colormap = cmap;
    mask = CWColormap;
    iconW = XCreateWindow( dpy, RootWindow( dpy, screen ), 0, 0, hp48_icon_width, hp48_icon_height, 0, ( int )depth, class, visual, mask,
                           &xswa );

    if ( iconW == 0 )
        return false;

    wmh.icon_window = iconW;
    wmh.window_group = mainW;
    wmh.flags |= ( IconWindowHint | WindowGroupHint );

    /*
     * set icon position if requested
     */
    ih.x = ih.y = 0;
    ih.min_width = ih.max_width = ih.base_width = ih.width = hp48_icon_width;
    ih.min_height = ih.max_height = ih.base_height = ih.height = hp48_icon_height;
    ih.win_gravity = NorthWestGravity;
    ih.flags = PSize | PMinSize | PMaxSize | PBaseSize | PWinGravity;

    info = XWMGeometry( dpy, screen, user_geom, ( char* )0, 0, &ih, &x, &y, &w, &h, &ih.win_gravity );

    if ( ( info & XValue ) && ( info & YValue ) ) {
        wmh.icon_x = x;
        wmh.icon_y = y;
        wmh.flags |= IconPositionHint;
    }

    /*
     * set some more attributes of icon window
     */
    xswa.background_pixel = COLOR( BLACK );
    xswa.border_pixel = COLOR( BLACK );
    xswa.backing_store = NotUseful;
    xswa.win_gravity = ih.win_gravity;
    xswa.bit_gravity = NorthWestGravity;
    mask = CWBackPixel | CWBorderPixel | CWBackingStore | CWBitGravity | CWWinGravity;
    XChangeWindowAttributes( dpy, iconW, mask, &xswa );

    /*
     * tell window manager all the stuff we dug out
     */
    XSetWMProperties( dpy, mainW, &wname, &iname, argv, argc, &hint, &wmh, &clh );

    /*
     * turn on WM_DELETE_WINDOW
     */
    wm_delete_window = XInternAtom( dpy, "WM_DELETE_WINDOW", 0 );
    wm_save_yourself = XInternAtom( dpy, "WM_SAVE_YOURSELF", 0 );
    wm_protocols = XInternAtom( dpy, "WM_PROTOCOLS", 0 );
    protocols[ 0 ] = wm_delete_window;
    protocols[ 1 ] = wm_save_yourself;
    XSetWMProtocols( dpy, mainW, protocols, 2 );

    /*
     * turn off icon name for olwm, olvwm
     */
    ol_decor_icon_name = XInternAtom( dpy, "_OL_DECOR_ICON_NAME", 0 );
    ol_decor_del = XInternAtom( dpy, "_OL_DECOR_DEL", 0 );
    atom_type = XInternAtom( dpy, "ATOM", 0 );
    XChangeProperty( dpy, mainW, ol_decor_del, atom_type, 32, PropModeReplace, ( unsigned char* )&ol_decor_icon_name, 1 );

    /*
     * set up the GC's
     */
    val.foreground = COLOR( WHITE );
    val.background = COLOR( BLACK );
    val.function = GXcopy;
    gc_mask = GCForeground | GCBackground | GCFunction;
    gc = XCreateGC( dpy, mainW, gc_mask, &val );

    /*
     * create the icon pixmap for desktops
     */
    _CreateIcon();

    /*
     * create the display
     */
    _CreateLCDWindow();

    /*
     * create the keypad
     */
    /*
     * draw the nice labels around the buttons
     */
    keypad.width = width;
    keypad.height = height;

    keypad.pixmap = XCreatePixmap( dpy, mainW, width, height, depth );

    int cut;
    if ( config.netbook ) {
        cut = buttons[ HPKEY_MTH ].y - ( small_ascent + small_descent + 6 + 4 );
        _CreateBackground( width / 2, height, width, height, &keypad );
        _CreateKeypad( -cut, KEYBOARD_OFFSET_X, &keypad );
    } else {
        cut = buttons[ HPKEY_MTH ].y + KEYBOARD_OFFSET_Y - 19;
        _CreateBackground( width, cut, width, height, &keypad );
        _CreateKeypad( KEYBOARD_OFFSET_Y, KEYBOARD_OFFSET_X, &keypad );
    }
    _DrawMore( KEYBOARD_OFFSET_Y, &keypad );
    _CreateBezel( &keypad );

    /*
     * map the window
     */
    XMapWindow( dpy, mainW );
    XMapSubwindows( dpy, mainW );

    // DrawKeypad( &keypad );
    for ( int b = FIRST_HPKEY; b <= LAST_HPKEY; b++ )
        DrawButton( b );
    DrawIcon();

    _DrawSerialDevices( wire_name, ir_name );

    if ( shm_flag ) {
        XSetForeground( dpy, lcd.gc, COLOR( PIXEL ) );
        XFillRectangle( dpy, lcd.win, lcd.gc, 5, 20, 262, 128 );
    }

    return true;
}

static void key_event( int b, XEvent* xev )
{
    if ( xev->type == KeyPress )
        press_key( b );
    else
        release_key( b );

    DrawButton( b );
}

static inline void _DrawDisp( void )
{
    if ( shm_flag ) {
        XShmPutImage( dpy, lcd.win, lcd.gc, lcd.disp_image, 2 * display.offset, 0, 5, 20, 262,
                      ( unsigned int )( ( 2 * display.lines ) + 2 ), 0 );
        if ( display.lines < 63 )
            XShmPutImage( dpy, lcd.win, lcd.gc, lcd.menu_image, 0, ( 2 * display.lines ) - 110, 5, 22 + ( 2 * display.lines ), 262,
                          ( unsigned int )( 126 - ( 2 * display.lines ) ), 0 );

        lcd.display_update = 0;
    } else
        x11_update_LCD();

    last_annunc_state = -1;
    x11_draw_annunc();
}

// Used in init_x11_ui()
static void save_options( int argc, char** argv )
{
    int l;

    saved_argc = argc;
    saved_argv = ( char** )malloc( ( argc + 2 ) * sizeof( char* ) );
    if ( saved_argv == ( char** )0 ) {
        fprintf( stderr, "malloc failed in save_options(), exit\n" );
        exit( 1 );
    }
    saved_argv[ argc ] = ( char* )0;
    while ( argc-- ) {
        l = strlen( argv[ argc ] ) + 1;
        saved_argv[ argc ] = ( char* )malloc( l );
        if ( saved_argv[ argc ] == ( char* )0 ) {
            fprintf( stderr, "malloc failed in save_options(), exit\n" );
            exit( 1 );
        }
        memcpy( saved_argv[ argc ], argv[ argc ], l );
    }
}

static void save_command_line( void )
{
    int wm_argc = 0;
    char** wm_argv = ( char** )malloc( ( saved_argc + 5 ) * sizeof( char* ) );

    if ( wm_argv == ( char** )0 ) {
        if ( config.verbose )
            fprintf( stderr, "warning: malloc failed in wm_save_yourself.\n" );
        XSetCommand( dpy, mainW, saved_argv, saved_argc );
        return;
    }

    XSetCommand( dpy, mainW, wm_argv, wm_argc );
}

static void decode_key( XEvent* xev, KeySym sym, char* buf, int buflen )
{
    if ( buflen == 1 )
        switch ( buf[ 0 ] ) {
            case '0':
                sym = XK_0;
                break;
            case '1':
                sym = XK_1;
                break;
            case '2':
                sym = XK_2;
                break;
            case '3':
                sym = XK_3;
                break;
            case '4':
                sym = XK_4;
                break;
            case '5':
                sym = XK_5;
                break;
            case '6':
                sym = XK_6;
                break;
            case '7':
                sym = XK_7;
                break;
            case '8':
                sym = XK_8;
                break;
            case '9':
                sym = XK_9;
                break;
            default:
                break;
        }

    switch ( ( int )sym ) {
        case XK_KP_0:
        case XK_0:
            key_event( HPKEY_0, xev );
            break;
        case XK_KP_1:
        case XK_1:
            key_event( HPKEY_1, xev );
            break;
        case XK_KP_2:
        case XK_2:
            key_event( HPKEY_2, xev );
            break;
        case XK_KP_3:
        case XK_3:
            key_event( HPKEY_3, xev );
            break;
        case XK_KP_4:
        case XK_4:
            key_event( HPKEY_4, xev );
            break;
        case XK_KP_5:
        case XK_5:
            key_event( HPKEY_5, xev );
            break;
        case XK_KP_6:
        case XK_6:
            key_event( HPKEY_6, xev );
            break;
        case XK_KP_7:
        case XK_7:
            key_event( HPKEY_7, xev );
            break;
        case XK_KP_8:
        case XK_8:
            key_event( HPKEY_8, xev );
            break;
        case XK_KP_9:
        case XK_9:
            key_event( HPKEY_9, xev );
            break;
        case XK_KP_Add:
        case XK_plus:
        case XK_equal:
            key_event( HPKEY_PLUS, xev );
            break;
        case XK_KP_Subtract:
        case XK_minus:
            key_event( HPKEY_MINUS, xev );
            break;
#ifdef XK_F25
        case XK_F25:
#endif
        case XK_KP_Divide:
        case XK_slash:
            key_event( HPKEY_DIV, xev );
            break;
#ifdef XK_F26
        case XK_F26:
#endif
        case XK_KP_Multiply:
        case XK_asterisk:
        case XK_comma:
            key_event( HPKEY_MUL, xev );
            break;
        case XK_F1:
        case XK_KP_Enter:
        case XK_Return:
            key_event( HPKEY_ENTER, xev );
            break;
        case XK_KP_Decimal:
        case XK_KP_Separator:
        case XK_period:
            key_event( HPKEY_PERIOD, xev );
            break;
        case XK_space:
            key_event( HPKEY_SPC, xev );
            break;
        case XK_Delete:
            key_event( HPKEY_DEL, xev );
            break;
        case XK_BackSpace:
            key_event( HPKEY_BS, xev );
            break;
        case XK_F5:
        case XK_Escape:
            key_event( HPKEY_ON, xev );
            break;
        case XK_Shift_L:
            if ( !config.leave_shift_keys ) {
                key_event( HPKEY_SHL, xev );
            }
            break;
        case XK_F2:
        case XK_Control_R:
            key_event( HPKEY_SHL, xev );
            break;
        case XK_Shift_R:
            if ( !config.leave_shift_keys ) {
                key_event( HPKEY_SHR, xev );
            }
            break;
        case XK_F3:
        case XK_Control_L:
            key_event( HPKEY_SHR, xev );
            break;
        case XK_F4:
        case XK_Alt_L:
        case XK_Alt_R:
        case XK_Meta_L:
        case XK_Meta_R:
            key_event( HPKEY_ALPHA, xev );
            break;
        case XK_a:
        case XK_A:
            /* case XK_F1: */
            key_event( HPKEY_A, xev );
            break;
        case XK_b:
        case XK_B:
            /* case XK_F2: */
            key_event( HPKEY_B, xev );
            break;
        case XK_c:
        case XK_C:
            /* case XK_F3: */
            key_event( HPKEY_C, xev );
            break;
        case XK_d:
        case XK_D:
            /* case XK_F4: */
            key_event( HPKEY_D, xev );
            break;
        case XK_e:
        case XK_E:
            /* case XK_F5: */
            key_event( HPKEY_E, xev );
            break;
        case XK_f:
        case XK_F:
            /* case XK_F6: */
            key_event( HPKEY_F, xev );
            break;
        case XK_g:
        case XK_G:
            key_event( HPKEY_MTH, xev );
            break;
        case XK_h:
        case XK_H:
            key_event( HPKEY_PRG, xev );
            break;
        case XK_i:
        case XK_I:
            key_event( HPKEY_CST, xev );
            break;
        case XK_j:
        case XK_J:
            key_event( HPKEY_VAR, xev );
            break;
        case XK_k:
        case XK_K:
        case XK_Up:
            key_event( HPKEY_UP, xev );
            break;
        case XK_l:
        case XK_L:
            key_event( HPKEY_NXT, xev );
            break;
        case XK_m:
        case XK_M:
            key_event( HPKEY_COLON, xev );
            break;
        case XK_n:
        case XK_N:
            key_event( HPKEY_STO, xev );
            break;
        case XK_o:
        case XK_O:
            key_event( HPKEY_EVAL, xev );
            break;
        case XK_p:
        case XK_P:
        case XK_Left:
            key_event( HPKEY_LEFT, xev );
            break;
        case XK_q:
        case XK_Q:
        case XK_Down:
            key_event( HPKEY_DOWN, xev );
            break;
        case XK_r:
        case XK_R:
        case XK_Right:
            key_event( HPKEY_RIGHT, xev );
            break;
        case XK_s:
        case XK_S:
            key_event( HPKEY_SIN, xev );
            break;
        case XK_t:
        case XK_T:
            key_event( HPKEY_COS, xev );
            break;
        case XK_u:
        case XK_U:
            key_event( HPKEY_TAN, xev );
            break;
        case XK_v:
        case XK_V:
            key_event( HPKEY_SQRT, xev );
            break;
        case XK_w:
        case XK_W:
            key_event( HPKEY_POWER, xev );
            break;
        case XK_x:
        case XK_X:
            key_event( HPKEY_INV, xev );
            break;
        case XK_y:
        case XK_Y:
            key_event( HPKEY_NEG, xev );
            break;
        case XK_z:
        case XK_Z:
            key_event( HPKEY_EEX, xev );
            break;
        case XK_F7:
        case XK_F10:
            please_exit = true;
            break;
        default:
            break;
    }
}

static void x11_release_all_keys( void )
{
    release_all_keys();
    for ( int b = FIRST_HPKEY; b <= LAST_HPKEY; b++ )
        DrawButton( b );
}

static inline void draw_nibble( int c, int r, int val )
{
    val &= 0x0f;
    if ( val == lcd_nibbles_buffer[ r ][ c ] )
        return;

    lcd_nibbles_buffer[ r ][ c ] = val;

    int y = ( r * 2 ) + 20;
    int x = ( c * 8 ) + 5;
    if ( r <= display.lines )
        x -= ( 2 * display.offset );

    XCopyPlane( dpy, nibble_maps[ val ], lcd.win, lcd.gc, 0, 0, 8, 2, x, y, 1 );
}

/* Identical in all ui_*.c */
static inline void draw_row( long addr, int row )
{
    int line_length = NIBBLES_PER_ROW;

    if ( ( display.offset > 3 ) && ( row <= display.lines ) )
        line_length += 2;

    for ( int i = 0; i < line_length; i++ )
        draw_nibble( i, row, read_nibble( addr + i ) );
}

// Used in init_x11_ui()
static inline bool InitDisplay( void )
{
    /*
     * open the display
     */
    dpy = XOpenDisplay( NULL );
    if ( dpy == ( Display* )0 ) {
        if ( config.verbose )
            fprintf( stderr, "can\'t open display\n" );

        return false;
    }

    /*
     * Get the default screen
     */
    screen = DefaultScreen( dpy );

    /*
     * Try to use XShm-Extension
     */
    shm_flag = true;

    if ( !XShmQueryExtension( dpy ) ) {
        shm_flag = false;
        if ( config.verbose )
            fprintf( stderr, "Xserver does not support XShm extension.\n" );
    }
    if ( config.verbose && shm_flag )
        fprintf( stderr, "using XShm extension.\n" );

    return true;
}

/**********/
/* public */
/**********/

void x11_ui_stop( void ) { XCloseDisplay( dpy ); }

void x11_get_event( void )
{
    XEvent xev;
    XClientMessageEvent* cm;
    int i, bufs = 2;
    char buf[ 2 ];
    KeySym sym;
    static int release_pending = 0;
    static XKeyEvent release_event;
    static Time last_release_time = 0;

    if ( paste_last_key ) {
        release_key( paste[ paste_count - 1 ] );
        paste_last_key = 0;
        return;
    } else if ( paste_count < paste_size ) {
        press_key( paste[ paste_count ] );
        paste_last_key = 1;
        paste_count++;
        return;
    }

    if ( release_pending ) {
        i = XLookupString( &release_event, buf, bufs, &sym, NULL );
        decode_key( ( XEvent* )&release_event, sym, buf, i );
        release_pending = 0;
        return;
    }

    do {
        while ( XPending( dpy ) > 0 ) {

            XNextEvent( dpy, &xev );

            switch ( ( int )xev.type ) {

                case KeyPress:
                    release_pending = 0;
                    if ( ( xev.xkey.time - last_release_time ) <= 1 ) {
                        release_pending = 0;
                        break;
                    }

                    i = XLookupString( &xev.xkey, buf, bufs, &sym, NULL );
                    decode_key( &xev, sym, buf, i );
                    first_key = 1;
                    break;

                case KeyRelease:
                    i = XLookupString( &xev.xkey, buf, bufs, &sym, NULL );
                    first_key = 0;
                    release_pending = 1;
                    last_release_time = xev.xkey.time;
                    memcpy( &release_event, &xev, sizeof( XKeyEvent ) );
                    break;

                case NoExpose:
                    break;

                case Expose:
                    if ( xev.xexpose.count == 0 ) {
                        if ( xev.xexpose.window == lcd.win )
                            _DrawDisp();
                        else if ( xev.xexpose.window == iconW )
                            DrawIcon();
                        else if ( xev.xexpose.window == mainW )
                            DrawKeypad( &keypad );
                        else
                            for ( i = FIRST_HPKEY; i <= LAST_HPKEY; i++ ) {
                                if ( xev.xexpose.window == buttons[ i ].xwin ) {
                                    DrawButton( i );
                                    break;
                                }
                            }
                    }
                    break;

                case UnmapNotify:
                    mapped = false;
                    break;

                case MapNotify:
                    if ( !mapped ) {
                        mapped = true;
                        x11_update_LCD();
                        last_annunc_state = -1;
                        x11_draw_annunc();
                    }
                    break;

                case ButtonPress:
                    if ( xev.xbutton.subwindow == lcd.win ) {
                        if ( xev.xbutton.button == Button2 ) {
                            if ( xev.xbutton.subwindow == lcd.win ) {
                                int x;
                                int flag = 0;
                                char* paste_in = XFetchBuffer( dpy, &x, 0 );

                                char* p = paste_in;
                                if ( x > MAX_PASTE ) {
                                    x = 0;
                                    printf( "input too long. limit is %d "
                                            "characters\n",
                                            MAX_PASTE );
                                }
                                paste_count = 0;
                                paste_size = 0;
                                while ( x-- ) {
                                    char c = *p++;
                                    switch ( c ) {
                                        case '.':
                                            paste[ paste_size++ ] = HPKEY_PERIOD;
                                            break;
                                        case '0':
                                            paste[ paste_size++ ] = HPKEY_0;
                                            break;
                                        case '1':
                                            paste[ paste_size++ ] = HPKEY_1;
                                            break;
                                        case '2':
                                            paste[ paste_size++ ] = HPKEY_2;
                                            break;
                                        case '3':
                                            paste[ paste_size++ ] = HPKEY_3;
                                            break;
                                        case '4':
                                            paste[ paste_size++ ] = HPKEY_4;
                                            break;
                                        case '5':
                                            paste[ paste_size++ ] = HPKEY_5;
                                            break;
                                        case '6':
                                            paste[ paste_size++ ] = HPKEY_6;
                                            break;
                                        case '7':
                                            paste[ paste_size++ ] = HPKEY_7;
                                            break;
                                        case '8':
                                            paste[ paste_size++ ] = HPKEY_8;
                                            break;
                                        case '9':
                                            paste[ paste_size++ ] = HPKEY_9;
                                            break;
                                        case '\n':
                                            paste[ paste_size++ ] = HPKEY_SHR;
                                            paste[ paste_size++ ] = HPKEY_PERIOD;
                                            break;
                                        case '!':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_DEL;
                                            break;
                                        case '+':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            paste[ paste_size++ ] = HPKEY_PLUS;
                                            break;
                                        case '-':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            paste[ paste_size++ ] = HPKEY_MINUS;
                                            break;
                                        case '*':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            paste[ paste_size++ ] = HPKEY_MUL;
                                            break;
                                        case '/':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            paste[ paste_size++ ] = HPKEY_DIV;
                                            break;
                                        case ' ':
                                            paste[ paste_size++ ] = 47;
                                            break;
                                        case '(':
                                            paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_DIV;
                                            break;
                                        case '[':
                                            paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_MUL;
                                            break;
                                        case '<':
                                            if ( x > 1 && *p == '<' ) {
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                                paste[ paste_size++ ] = HPKEY_MINUS;
                                                x--;
                                                p++;
                                            } else {
                                                paste[ paste_size++ ] = HPKEY_ALPHA;
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                                paste[ paste_size++ ] = HPKEY_2;
                                            }
                                            break;
                                        case '{':
                                            paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_PLUS;
                                            break;
                                        case ')':
                                        case ']':
                                        case '}':
                                            paste[ paste_size++ ] = HPKEY_RIGHT;
                                            break;
                                        case '>':
                                            if ( x > 1 && *p == '>' ) {
                                                paste[ paste_size++ ] = HPKEY_RIGHT;
                                                paste[ paste_size++ ] = HPKEY_RIGHT;
                                                paste[ paste_size++ ] = HPKEY_RIGHT;
                                                x--;
                                                p++;
                                            } else {
                                                paste[ paste_size++ ] = HPKEY_ALPHA;
                                                paste[ paste_size++ ] = HPKEY_SHR;
                                                paste[ paste_size++ ] = HPKEY_2;
                                            }
                                            break;
                                        case '#':
                                            paste[ paste_size++ ] = HPKEY_SHR;
                                            paste[ paste_size++ ] = HPKEY_DIV;
                                            break;
                                        case '_':
                                            paste[ paste_size++ ] = HPKEY_SHR;
                                            paste[ paste_size++ ] = HPKEY_MUL;
                                            break;
                                        case '"':
                                            if ( flag & 1 ) {
                                                flag &= ~1;
                                                paste[ paste_size++ ] = HPKEY_RIGHT;
                                            } else {
                                                flag |= 1;
                                                paste[ paste_size++ ] = HPKEY_SHR;
                                                paste[ paste_size++ ] = HPKEY_MINUS;
                                            }
                                            break;
                                        case ':':
                                            if ( flag & 2 ) {
                                                flag &= ~2;
                                                paste[ paste_size++ ] = HPKEY_RIGHT;
                                            } else {
                                                flag |= 2;
                                                paste[ paste_size++ ] = HPKEY_SHR;
                                                paste[ paste_size++ ] = HPKEY_PLUS;
                                            }
                                            break;
                                        case '\'':
                                            if ( flag & 4 ) {
                                                flag &= ~4;
                                                paste[ paste_size++ ] = HPKEY_RIGHT;
                                            } else {
                                                flag |= 4;
                                                paste[ paste_size++ ] = HPKEY_COLON;
                                            }
                                            break;
                                        case 'a':
                                        case 'A':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_A;
                                            break;
                                        case 'b':
                                        case 'B':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_B;
                                            break;
                                        case 'c':
                                        case 'C':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_C;
                                            break;
                                        case 'd':
                                        case 'D':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_D;
                                            break;
                                        case 'e':
                                        case 'E':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_E;
                                            break;
                                        case 'f':
                                        case 'F':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_F;
                                            break;
                                        case 'g':
                                        case 'G':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_MTH;
                                            break;
                                        case 'h':
                                        case 'H':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_PRG;
                                            break;
                                        case 'i':
                                        case 'I':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_CST;
                                            break;
                                        case 'j':
                                        case 'J':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_VAR;
                                            break;
                                        case 'k':
                                        case 'K':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_UP;
                                            break;
                                        case 'l':
                                        case 'L':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_NXT;
                                            break;

                                        case 'm':
                                        case 'M':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_COLON;
                                            break;
                                        case 'n':
                                        case 'N':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_STO;
                                            break;
                                        case 'o':
                                        case 'O':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_EVAL;
                                            break;
                                        case 'p':
                                        case 'P':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_LEFT;
                                            break;
                                        case 'q':
                                        case 'Q':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_DOWN;
                                            break;
                                        case 'r':
                                        case 'R':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_RIGHT;
                                            break;
                                        case 's':
                                        case 'S':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_SIN;
                                            break;
                                        case 't':
                                        case 'T':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_COS;
                                            break;
                                        case 'u':
                                        case 'U':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_TAN;
                                            break;
                                        case 'v':
                                        case 'V':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_SQRT;
                                            break;
                                        case 'w':
                                        case 'W':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_POWER;
                                            break;
                                        case 'x':
                                        case 'X':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_INV;
                                            break;
                                        case 'y':
                                        case 'Y':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_NEG;
                                            break;
                                        case 'z':
                                        case 'Z':
                                            paste[ paste_size++ ] = HPKEY_ALPHA;
                                            if ( islower( c ) )
                                                paste[ paste_size++ ] = HPKEY_SHL;
                                            paste[ paste_size++ ] = HPKEY_EEX;
                                            break;
                                        default:
                                            printf( "unknown %c %d\n", c, *p );
                                            break;
                                    }
                                }
                                if ( paste_in )
                                    XFree( paste_in );
                                if ( paste_size )
                                    return;
                            }
                        }
                    } else {
                        if ( xev.xbutton.button == Button1 || xev.xbutton.button == Button2 || xev.xbutton.button == Button3 ) {
                            for ( i = FIRST_HPKEY; i <= LAST_HPKEY; i++ ) {
                                if ( xev.xbutton.subwindow == buttons[ i ].xwin ) {
                                    if ( keyboard[ i ].pressed ) {
                                        if ( xev.xbutton.button == Button3 ) {
                                            release_key( i );
                                            DrawButton( i );
                                        }
                                    } else {
                                        last_button = i;
                                        press_key( i );
                                        first_key = 1;
                                        DrawButton( i );
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    break;

                case ButtonRelease:
                    first_key = 0;
                    if ( xev.xbutton.button == Button1 )
                        x11_release_all_keys();

                    if ( xev.xbutton.button == Button2 ) {
                        if ( last_button >= 0 ) {
                            release_key( last_button );
                            DrawButton( last_button );
                        }
                        last_button = -1;
                    }
                    break;

                case FocusOut:
                    first_key = 0;
                    x11_release_all_keys();
                    break;

                case MappingNotify:
                    switch ( xev.xmapping.request ) {
                        case MappingModifier:
                        case MappingKeyboard:
                            XRefreshKeyboardMapping( &xev.xmapping );
                            break;
                        case MappingPointer:
                        default:
                            break;
                    }
                    break;

                case EnterNotify:
                case LeaveNotify:
                    break;

                case ClientMessage:
                    cm = ( XClientMessageEvent* )&xev;

                    if ( cm->message_type == wm_protocols ) {
                        if ( cm->data.l[ 0 ] == ( long )wm_delete_window )
                            please_exit = true;
                        if ( cm->data.l[ 0 ] == ( long )wm_save_yourself )
                            save_command_line();
                    }
                    break;

                case KeymapNotify:
                case ConfigureNotify:
                case ReparentNotify:
                default:
                    break;
            }
        }
    } while ( first_key > 1 );

    if ( first_key )
        first_key++;
}

static void x11_refresh_LCD( void )
{
    if ( !shm_flag )
        return;

    if ( lcd.display_update & UPDATE_DISP )
        XShmPutImage( dpy, lcd.win, lcd.gc, lcd.disp_image, 2 * display.offset, 0, 5, 20, 262,
                      ( unsigned int )( ( 2 * display.lines ) + 2 ), 0 );

    if ( ( ( 2 * display.lines ) < 126 ) && ( lcd.display_update & UPDATE_MENU ) )
        XShmPutImage( dpy, lcd.win, lcd.gc, lcd.menu_image, 0, 0, 5, ( int )( ( 2 * display.lines ) + 22 ), 262,
                      ( unsigned int )( 126 - ( 2 * display.lines ) ), 0 );

    lcd.display_update = 0;
}

void x11_adjust_contrast( void )
{
    int gray = 0;
    int r = 0, g = 0, b = 0;
    unsigned long old;

    int contrast = display.contrast;

    if ( contrast < 0x3 )
        contrast = 0x3;
    if ( contrast > 0x13 )
        contrast = 0x13;

    old = colors[ PIXEL ].xcolor.pixel;
    switch ( color_mode ) {
        case COLOR_MODE_MONO:
            return;
        case COLOR_MODE_GRAY:
            gray = ( 0x13 - contrast ) * ( colors[ LCD ].gray_rgb / 0x10 );
            colors[ PIXEL ].xcolor.red = gray << 8;
            colors[ PIXEL ].xcolor.green = gray << 8;
            colors[ PIXEL ].xcolor.blue = gray << 8;
            break;
        default:
            r = ( 0x13 - contrast ) * ( colors[ LCD ].r / 0x10 );
            g = ( 0x13 - contrast ) * ( colors[ LCD ].g / 0x10 );
            b = 128 - ( ( 0x13 - contrast ) * ( ( 128 - colors[ LCD ].b ) / 0x10 ) );
            colors[ PIXEL ].xcolor.red = r << 8;
            colors[ PIXEL ].xcolor.green = g << 8;
            colors[ PIXEL ].xcolor.blue = b << 8;
            break;
    }

    if ( direct_color ) {
        colors[ PIXEL ].gray_rgb = gray;
        colors[ PIXEL ].r = r;
        colors[ PIXEL ].g = g;
        colors[ PIXEL ].b = b;

        AllocColors();

        XSetForeground( dpy, lcd.gc, COLOR( PIXEL ) );

        lcd.display_update = UPDATE_DISP | UPDATE_MENU;

        x11_refresh_LCD();

        last_annunc_state = -1;
        x11_draw_annunc();

        last_icon_state = -1;
        refresh_icon();
    } else if ( dynamic_color ) {
        XStoreColor( dpy, cmap, &colors[ PIXEL ].xcolor );
    } else {
        if ( XAllocColor( dpy, cmap, &colors[ PIXEL ].xcolor ) == 0 ) {
            colors[ PIXEL ].xcolor.pixel = old;
            if ( config.verbose )
                fprintf( stderr, "warning: can\'t alloc new pixel color.\n" );
        } else {
            XFreeColors( dpy, cmap, &old, 1, 0 );
            XSetForeground( dpy, lcd.gc, COLOR( PIXEL ) );

            lcd.display_update = UPDATE_DISP | UPDATE_MENU;

            x11_refresh_LCD();

            last_annunc_state = -1;
            x11_draw_annunc();

            last_icon_state = -1;
            refresh_icon();
        }
    }
}

void x11_update_LCD( void )
{
    int i, j;
    long addr;
    static int old_offset = -1;
    static int old_lines = -1;
    int addr_pad;
    int val, line_pad, line_length;
    word_20 data_addr, data_addr_2;

    if ( !mapped ) {
        refresh_icon();
        return;
    }
    if ( display.on ) {
        addr = display.disp_start;
        if ( shm_flag ) {
            data_addr = 0;
            data_addr_2 = lcd.disp_image->bytes_per_line;
            line_length = NIBBLES_PER_ROW;
            if ( display.offset > 3 )
                line_length += 2;
            line_pad = 2 * lcd.disp_image->bytes_per_line - line_length;
            addr_pad = display.nibs_per_line - line_length;
            for ( i = 0; i <= display.lines; i++ ) {
                for ( j = 0; j < line_length; j++ ) {
                    val = read_nibble( addr++ );
                    lcd.disp_image->data[ data_addr++ ] = nibble_bitmap[ val ];
                    lcd.disp_image->data[ data_addr_2++ ] = nibble_bitmap[ val ];
                }
                addr += addr_pad;
                data_addr += line_pad;
                data_addr_2 += line_pad;
            }
            lcd.display_update |= UPDATE_DISP;
        } else {
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
        }
        if ( i < DISP_ROWS ) {
            addr = display.menu_start;
            if ( shm_flag ) {
                data_addr = 0;
                data_addr_2 = lcd.menu_image->bytes_per_line;
                line_pad = 2 * lcd.menu_image->bytes_per_line - NIBBLES_PER_ROW;
                for ( ; i < DISP_ROWS; i++ ) {
                    for ( j = 0; j < NIBBLES_PER_ROW; j++ ) {
                        val = read_nibble( addr++ );
                        lcd.menu_image->data[ data_addr++ ] = nibble_bitmap[ val ];
                        lcd.menu_image->data[ data_addr_2++ ] = nibble_bitmap[ val ];
                    }
                    data_addr += line_pad;
                    data_addr_2 += line_pad;
                }
                lcd.display_update |= UPDATE_MENU;
            } else
                for ( ; i < DISP_ROWS; i++ ) {
                    draw_row( addr, i );
                    addr += NIBBLES_PER_ROW;
                }
        }
    } else {
        if ( shm_flag ) {
            memset( lcd.disp_image->data, 0, ( size_t )( lcd.disp_image->bytes_per_line * lcd.disp_image->height ) );
            memset( lcd.menu_image->data, 0, ( size_t )( lcd.menu_image->bytes_per_line * lcd.menu_image->height ) );
            lcd.display_update = UPDATE_DISP | UPDATE_MENU;
        } else {
            ui_init_LCD();

            for ( i = 0; i < 64; i++ )
                for ( j = 0; j < NIBBLES_PER_ROW; j++ )
                    draw_nibble( j, i, 0x00 );
        }
    }

    x11_refresh_LCD();
}

void x11_disp_draw_nibble( word_20 addr, word_4 val )
{
    long offset;
    int shm_addr;
    int x, y;

    offset = ( addr - display.disp_start );
    x = offset % display.nibs_per_line;
    if ( x < 0 || x > 35 )
        return;

    if ( display.nibs_per_line != 0 ) {
        y = offset / display.nibs_per_line;
        if ( y < 0 || y > 63 )
            return;

        if ( shm_flag ) {
            shm_addr = ( 2 * y * lcd.disp_image->bytes_per_line ) + x;
            lcd.disp_image->data[ shm_addr ] = nibble_bitmap[ val ];
            lcd.disp_image->data[ shm_addr + lcd.disp_image->bytes_per_line ] = nibble_bitmap[ val ];
            lcd.display_update |= UPDATE_DISP;
        } else
            draw_nibble( x, y, val );
    } else {
        if ( shm_flag ) {
            shm_addr = x;
            for ( y = 0; y < display.lines; y++ ) {
                lcd.disp_image->data[ shm_addr ] = nibble_bitmap[ val ];
                shm_addr += lcd.disp_image->bytes_per_line;
                lcd.disp_image->data[ shm_addr ] = nibble_bitmap[ val ];
                shm_addr += lcd.disp_image->bytes_per_line;
            }
            lcd.display_update |= UPDATE_DISP;
        } else
            for ( y = 0; y < display.lines; y++ )
                draw_nibble( x, y, val );
    }
}

void x11_menu_draw_nibble( word_20 addr, word_4 val )
{
    long offset;
    int shm_addr;
    int x, y;

    offset = ( addr - display.menu_start );
    if ( shm_flag ) {
        shm_addr = 2 * ( offset / NIBBLES_PER_ROW ) * lcd.menu_image->bytes_per_line + ( offset % NIBBLES_PER_ROW );
        lcd.menu_image->data[ shm_addr ] = nibble_bitmap[ val ];
        lcd.menu_image->data[ shm_addr + lcd.menu_image->bytes_per_line ] = nibble_bitmap[ val ];
        lcd.display_update |= UPDATE_MENU;
    } else {
        x = offset % NIBBLES_PER_ROW;
        y = display.lines + ( offset / NIBBLES_PER_ROW ) + 1;

        draw_nibble( x, y, val );
    }
}

void x11_draw_annunc( void )
{
    int val = saturn.annunc;
    if ( val == last_annunc_state )
        return;

    last_annunc_state = val;

    for ( int i = 0; i < NB_ANNUNCIATORS; i++ )
        if ( ( annunciators_bits[ i ] & val ) == annunciators_bits[ i ] )
            XCopyPlane( dpy, ann_tbl[ i ].pixmap, lcd.win, lcd.gc, 0, 0, ann_tbl[ i ].width, ann_tbl[ i ].height, ann_tbl[ i ].x,
                        ann_tbl[ i ].y, 1 );
        else
            XClearArea( dpy, lcd.win, ann_tbl[ i ].x, ann_tbl[ i ].y, ann_tbl[ i ].width, ann_tbl[ i ].height, False );

    refresh_icon();
}

void init_x11_ui( int argc, char** argv )
{
    if ( config.verbose )
        fprintf( stderr, "UI is x11\n" );

    /* Set public API to this UI's functions */
    ui_disp_draw_nibble = x11_disp_draw_nibble;
    ui_menu_draw_nibble = x11_menu_draw_nibble;
    ui_get_event = x11_get_event;
    ui_update_LCD = x11_update_LCD;
    ui_refresh_LCD = x11_refresh_LCD;
    ui_adjust_contrast = x11_adjust_contrast;
    ui_draw_annunc = x11_draw_annunc;

    save_options( argc, argv );

    if ( !InitDisplay() )
        exit( 1 );

    if ( !CreateWindows( saved_argc, saved_argv ) ) {
        fprintf( stderr, "can\'t create window\n" );
        exit( 1 );
    }

    for ( int i = 0; i < NB_ANNUNCIATORS; i++ )
        ann_tbl[ i ].pixmap = XCreateBitmapFromData( dpy, lcd.win, ( char* )ann_tbl[ i ].bits, ann_tbl[ i ].width, ann_tbl[ i ].height );

    /* init nibble_maps */
    for ( int i = 0; i < 16; i++ )
        nibble_maps[ i ] = XCreateBitmapFromData( dpy, lcd.win, ( char* )nibbles[ i ], 8, 2 );

    if ( !shm_flag )
        return;

    if ( lcd.disp_image->bitmap_bit_order == MSBFirst ) {
        nibble_bitmap[ 0x0 ] = 0x00; /* ---- */
        nibble_bitmap[ 0x1 ] = 0xc0; /* *--- */
        nibble_bitmap[ 0x2 ] = 0x30; /* -*-- */
        nibble_bitmap[ 0x3 ] = 0xf0; /* **-- */
        nibble_bitmap[ 0x4 ] = 0x0c; /* --*- */
        nibble_bitmap[ 0x5 ] = 0xcc; /* *-*- */
        nibble_bitmap[ 0x6 ] = 0x3c; /* -**- */
        nibble_bitmap[ 0x7 ] = 0xfc; /* ***- */
        nibble_bitmap[ 0x8 ] = 0x03; /* ---* */
        nibble_bitmap[ 0x9 ] = 0xc3; /* *--* */
        nibble_bitmap[ 0xa ] = 0x33; /* -*-* */
        nibble_bitmap[ 0xb ] = 0xf3; /* **-* */
        nibble_bitmap[ 0xc ] = 0x0f; /* --** */
        nibble_bitmap[ 0xd ] = 0xcf; /* *-** */
        nibble_bitmap[ 0xe ] = 0x3f; /* -*** */
        nibble_bitmap[ 0xf ] = 0xff; /* **** */
    } else {
        nibble_bitmap[ 0x0 ] = 0x00; /* ---- */
        nibble_bitmap[ 0x1 ] = 0x03; /* *--- */
        nibble_bitmap[ 0x2 ] = 0x0c; /* -*-- */
        nibble_bitmap[ 0x3 ] = 0x0f; /* **-- */
        nibble_bitmap[ 0x4 ] = 0x30; /* --*- */
        nibble_bitmap[ 0x5 ] = 0x33; /* *-*- */
        nibble_bitmap[ 0x6 ] = 0x3c; /* -**- */
        nibble_bitmap[ 0x7 ] = 0x3f; /* ***- */
        nibble_bitmap[ 0x8 ] = 0xc0; /* ---* */
        nibble_bitmap[ 0x9 ] = 0xc3; /* *--* */
        nibble_bitmap[ 0xa ] = 0xcc; /* -*-* */
        nibble_bitmap[ 0xb ] = 0xcf; /* **-* */
        nibble_bitmap[ 0xc ] = 0xf0; /* --** */
        nibble_bitmap[ 0xd ] = 0xf3; /* *-** */
        nibble_bitmap[ 0xe ] = 0xfc; /* -*** */
        nibble_bitmap[ 0xf ] = 0xff; /* **** */
    }
}
