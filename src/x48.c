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
#if defined( GUI_IS_X11 )
#include <X11/cursorfont.h>
#endif

#include "options.h"
#include "hp48.h"
#include "romio.h"
#include "x48.h"
#include "x48_errors.h"
#include "x48_resources.h"

disp_t disp;

keypad_t keypad;
color_t* colors;

#if defined( GUI_IS_X11 )
static XrmOptionDescRec options[] = {
    { "-display", ".display", XrmoptionSepArg, ( void* )0 },
    { "-geometry", "*geometry", XrmoptionSepArg, ( void* )0 },
    { "-iconGeom", "*iconGeom", XrmoptionSepArg, ( void* )0 },
    { "-iconName", "*iconName", XrmoptionSepArg, ( void* )0 },
    { "-iconic", "*iconic", XrmoptionNoArg, ( void* )"True" },
    { "-name", ( char* )0, XrmoptionSepArg, ( void* )0 },
    { "-title", "*title", XrmoptionSepArg, ( void* )0 },

    { "-xshm", "*useXShm", XrmoptionNoArg, ( void* )"True" },
    { "+xshm", "*useXShm", XrmoptionNoArg, ( void* )"False" },

    { "-visual", "*visual", XrmoptionSepArg, ( void* )0 },
    { "-mono", "*mono", XrmoptionNoArg, ( void* )"True" },
    { "-gray", "*gray", XrmoptionNoArg, ( void* )"True" },
    { "-monoIcon", "*monoIcon", XrmoptionNoArg, ( void* )"True" },

    { "-version", "*printVersion", XrmoptionNoArg, ( void* )"True" },
    { "-copyright", "*printCopyright", XrmoptionNoArg, ( void* )"True" },
    { "-warranty", "*printWarranty", XrmoptionNoArg, ( void* )"True" },

    { "-smallFont", "*smallLabelFont", XrmoptionSepArg, ( void* )0 },
    { "-mediumFont", "*mediumLabelFont", XrmoptionSepArg, ( void* )0 },
    { "-largeFont", "*largeLabelFont", XrmoptionSepArg, ( void* )0 },
    { "-connFont", "*connectionFont", XrmoptionSepArg, ( void* )0 },

    { "-verbose", "*verbose", XrmoptionNoArg, ( void* )"True" },

    { "-terminal", "*useTerminal", XrmoptionNoArg, ( void* )"True" },
    { "+terminal", "*useTerminal", XrmoptionNoArg, ( void* )"False" },
    { "-serial", "*useSerial", XrmoptionNoArg, ( void* )"True" },
    { "+serial", "*useSerial", XrmoptionNoArg, ( void* )"False" },
    { "-line", "*serialLine", XrmoptionSepArg, ( void* )0 },

    { "-initialize", "*completeInitialize", XrmoptionNoArg, ( void* )"True" },
    { "-reset", "*resetOnStartup", XrmoptionNoArg, ( void* )"True" },
    { "-rom", "*romFileName", XrmoptionSepArg, ( void* )0 },
    { "-home", "*homeDirectory", XrmoptionSepArg, ( void* )0 },

    { "-xrm", ( char* )0, XrmoptionResArg, ( void* )0 },
    { "-netbook", "*netbook", XrmoptionNoArg, ( void* )"False" },
    { "+netbook", "*netbook", XrmoptionNoArg, ( void* )"True" },

    { "-throttle", "*throttle", XrmoptionNoArg, ( void* )"False" },
    { "+throttle", "*throttle", XrmoptionNoArg, ( void* )"True" },
};

static char* defaults[] = {
    "*iconic:			False",
    "*visual:			Default",
    "*mono:			False",
    "*gray:			False",
    "*monoIcon:			False",
    "*useXShm:			True",
    "*smallLabelFont:		-*-fixed-bold-r-normal-*-14-*-*-*-*-*-iso8859-1",
    "*mediumLabelFont:		-*-fixed-bold-r-normal-*-15-*-*-*-*-*-iso8859-1",
    "*largeLabelFont:		-*-fixed-medium-r-normal-*-20-*-*-*-*-*-iso8859-1",
    "*connectionFont:		-*-fixed-medium-r-normal-*-12-*-*-*-*-*-iso8859-1",
    "*verbose:			False",
    "*printVersion:		False",
    "*printCopyright:		False",
    "*printWarranty:		False",
    "*useTerminal:		True",
    "*useSerial:		False",
    "*serialLine:		/dev/ttyS0",
    "*completeInitialize:	False",
    "*resetOnStartup:		False",
    "*romFileName:		rom.dump",
    "*homeDirectory:		.x48ng",
    0 };

static int CompletionType = -1;

extern int saved_argc;
extern char** saved_argv;

Display* dpy;
int screen;
unsigned int depth;
Colormap cmap;
GC gc;
Window mainW;
Window iconW = 0;

Atom wm_delete_window, wm_save_yourself, wm_protocols;
Atom ol_decor_del, ol_decor_icon_name;
Atom atom_type;
Visual* visual;
Pixmap icon_pix;
Pixmap icon_text_pix;
Pixmap icon_disp_pix;
static int last_icon_state = -1;

int shm_flag;
int xerror_flag;

int dynamic_color;
int direct_color;
int does_backing_store;
int color_mode;
int icon_color_mode;
#elif defined( GUI_IS_SDL1 )
// This will take the value of the defines, but can be run-time modified
unsigned KEYBOARD_HEIGHT, KEYBOARD_WIDTH, TOP_SKIP, SIDE_SKIP, BOTTOM_SKIP,
    DISP_KBD_SKIP, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_OFFSET_X,
    DISPLAY_OFFSET_Y, DISP_FRAME, KEYBOARD_OFFSET_X, KEYBOARD_OFFSET_Y,
    KBD_UPLINE;
#endif

color_t colors_sx[] = { { "white",
                          255,
                          255,
                          255
#if defined( GUI_IS_X11 )
                          ,
                          255,
                          255,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "left",
                          255,
                          166,
                          0
#if defined( GUI_IS_X11 )
                          ,
                          255,
                          230,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "right",
                          0,
                          210,
                          255
#if defined( GUI_IS_X11 )
                          ,
                          255,
                          169,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "but_top",
                          109,
                          93,
                          93
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          91,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "button",
                          90,
                          77,
                          77
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          81,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "but_bot",
                          76,
                          65,
                          65
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          69,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "lcd_col",
                          202,
                          221,
                          92
#if defined( GUI_IS_X11 )
                          ,
                          255,
                          205,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "pix_col",
                          0,
                          0,
                          128
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          20,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "pad_top",
                          109,
                          78,
                          78
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          88,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "pad",
                          90,
                          64,
                          64
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          73,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "pad_bot",
                          76,
                          54,
                          54
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          60,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "disp_pad_top",
                          155,
                          118,
                          84
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          124,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "disp_pad",
                          124,
                          94,
                          67
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          99,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "disp_pad_bot",
                          100,
                          75,
                          53
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          79,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "logo",
                          204,
                          169,
                          107
#if defined( GUI_IS_X11 )
                          ,
                          255,
                          172,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "logo_back",
                          64,
                          64,
                          64
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          65,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "label",
                          202,
                          184,
                          144
#if defined( GUI_IS_X11 )
                          ,
                          255,
                          185,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "frame",
                          0,
                          0,
                          0
#if defined( GUI_IS_X11 )
                          ,
                          255,
                          0,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "underlay",
                          60,
                          42,
                          42
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          48,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "black",
                          0,
                          0,
                          0
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          0,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { 0 } };

color_t colors_gx[] = { { "white",
                          255,
                          255,
                          255
#if defined( GUI_IS_X11 )
                          ,
                          255,
                          255,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "left",
                          255,
                          186,
                          255
#if defined( GUI_IS_X11 )
                          ,
                          255,
                          220,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "right",
                          0,
                          255,
                          204
#if defined( GUI_IS_X11 )
                          ,
                          255,
                          169,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "but_top",
                          104,
                          104,
                          104
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          104,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "button",
                          88,
                          88,
                          88
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          88,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "but_bot",
                          74,
                          74,
                          74
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          74,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "lcd_col",
                          202,
                          221,
                          92
#if defined( GUI_IS_X11 )
                          ,
                          255,
                          205,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "pix_col",
                          0,
                          0,
                          128
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          20,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "pad_top",
                          88,
                          88,
                          88
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          88,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "pad",
                          74,
                          74,
                          74
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          74,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "pad_bot",
                          64,
                          64,
                          64
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          64,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "disp_pad_top",
                          128,
                          128,
                          138
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          128,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "disp_pad",
                          104,
                          104,
                          110
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          104,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "disp_pad_bot",
                          84,
                          84,
                          90
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          84,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "logo",
                          176,
                          176,
                          184
#if defined( GUI_IS_X11 )
                          ,
                          255,
                          176,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "logo_back",
                          104,
                          104,
                          110
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          104,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "label",
                          240,
                          240,
                          240
#if defined( GUI_IS_X11 )
                          ,
                          255,
                          240,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "frame",
                          0,
                          0,
                          0
#if defined( GUI_IS_X11 )
                          ,
                          255,
                          0,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "underlay",
                          104,
                          104,
                          110
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          104,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { "black",
                          0,
                          0,
                          0
#if defined( GUI_IS_X11 )
                          ,
                          0,
                          0,
                          { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 }
#endif
                        },
                        { 0 } };

#if defined( GUI_IS_SDL1 )

// Control how the screen update is performed: at regular intervals (delayed)
// or immediatly Note: this is only for the LCD. The annunciators and the
// buttons are always immediately updated
// #define DELAYEDDISPUPDATE
// Interval in millisecond between screen updates
#define DISPUPDATEINTERVAL 200

unsigned int ARGBColors[ BLACK + 1 ];
#endif

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
      0 },

    { "MTH", 0, 0, 0x24, 0,   50,      36, 26, WHITE, "MTH",
      0,     0, 0, 0,    "G", "PRINT", 1,  0,  0,     0 },
    { "PRG", 0, 0, 0x74, 50,  50,    36, 26, WHITE, "PRG",
      0,     0, 0, 0,    "H", "I/O", 1,  0,  0,     0 },
    { "CST", 0, 0, 0x73, 100, 50,      36, 26, WHITE, "CST",
      0,     0, 0, 0,    "I", "MODES", 1,  0,  0,     0 },
    { "VAR", 0, 0, 0x72, 150, 50,       36, 26, WHITE, "VAR",
      0,     0, 0, 0,    "J", "MEMORY", 1,  0,  0,     0 },
    { "UP", 0,        0,         0x71,      200, 50,        36, 26, WHITE, 0,
      0,    up_width, up_height, up_bitmap, "K", "LIBRARY", 1,  0,  0,     0 },
    { "NXT", 0, 0, 0x70, 250, 50,     36, 26, WHITE, "NXT",
      0,     0, 0, 0,    "L", "PREV", 0,  0,  0,     0 },

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
      0 },
    { "STO", 0, 0, 0x64, 50,  100,   36, 26,    WHITE, "STO",
      0,     0, 0, 0,    "N", "DEF", 0,  "RCL", 0,     0 },
    { "EVAL", 0, 0, 0x63, 100, 100,  36, 26,     WHITE, "EVAL",
      0,      0, 0, 0,    "O", "aQ", 0,  "aNUM", 0,     0 },
    { "LEFT", 0, 0, 0x62,       150,         100,         36,  26,
      WHITE,  0, 0, left_width, left_height, left_bitmap, "P", "GRAPH",
      0,      0, 0, 0 },
    { "DOWN", 0, 0, 0x61,       200,         100,         36,  26,
      WHITE,  0, 0, down_width, down_height, down_bitmap, "Q", "REVIEW",
      0,      0, 0, 0 },
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
      0 },

    { "SIN", 0, 0, 0x34, 0,   150,    36, 26,  WHITE, "SIN",
      0,     0, 0, 0,    "S", "ASIN", 0,  "b", 0,     0 },
    { "COS", 0, 0, 0x54, 50,  150,    36, 26,  WHITE, "COS",
      0,     0, 0, 0,    "T", "ACOS", 0,  "c", 0,     0 },
    { "TAN", 0, 0, 0x53, 100, 150,    36, 26,  WHITE, "TAN",
      0,     0, 0, 0,    "U", "ATAN", 0,  "d", 0,     0 },
    { "SQRT", 0,   0, 0x52,       150,         150,         36,  26,
      WHITE,  0,   0, sqrt_width, sqrt_height, sqrt_bitmap, "V", "e",
      0,      "f", 0, 0 },
    { "POWER", 0, 0,           0x51,         200,          150, 36,  26, WHITE,
      0,       0, power_width, power_height, power_bitmap, "W", "g", 0,  "LOG",
      0,       0 },
    { "INV", 0,         0,          0x50,       250, 150, 36, 26,   WHITE, 0,
      0,     inv_width, inv_height, inv_bitmap, "X", "h", 0,  "LN", 0,     0 },

    { "ENTER", 0, 0, 0x44, 0, 200,        86, 26,       WHITE, "ENTER",
      2,       0, 0, 0,    0, "EQUATION", 0,  "MATRIX", 0,     0 },
    { "NEG", 0,      0, 0x43,    100,       200,        36,
      26,    WHITE,  0, 0,       neg_width, neg_height, neg_bitmap,
      "Y",   "EDIT", 0, "VISIT", 0,         0 },
    { "EEX", 0, 0, 0x42, 150, 200,  36, 26,   WHITE, "EEX",
      0,     0, 0, 0,    "Z", "2D", 0,  "3D", 0,     0 },
    { "DEL", 0, 0, 0x41, 200, 200,     36, 26, WHITE, "DEL",
      0,     0, 0, 0,    0,   "PURGE", 0,  0,  0,     0 },
    { "BS", 0,        0,         0x40,      250, 200,    36, 26,    WHITE, 0,
      0,    bs_width, bs_height, bs_bitmap, 0,   "DROP", 0,  "CLR", 0,     0 },

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
      0 },
    { "7", 0, 0, 0x33, 60, 250,     46, 26, WHITE, "7",
      1,   0, 0, 0,    0,  "SOLVE", 1,  0,  0,     0 },
    { "8", 0, 0, 0x32, 120, 250,    46, 26, WHITE, "8",
      1,   0, 0, 0,    0,   "PLOT", 1,  0,  0,     0 },
    { "9", 0, 0, 0x31, 180, 250,       46, 26, WHITE, "9",
      1,   0, 0, 0,    0,   "ALGEBRA", 1,  0,  0,     0 },
    { "DIV", 0,         0,          0x30,       240, 250,   46, 26,  WHITE, 0,
      0,     div_width, div_height, div_bitmap, 0,   "( )", 0,  "#", 0,     0 },

    { "SHL", 0,         0,          0x25,       0, 300, 36, 26, LEFT, 0,
      0,     shl_width, shl_height, shl_bitmap, 0, 0,   0,  0,  0,    0 },
    { "4", 0, 0, 0x23, 60, 300,    46, 26, WHITE, "4",
      1,   0, 0, 0,    0,  "TIME", 1,  0,  0,     0 },
    { "5", 0, 0, 0x22, 120, 300,    46, 26, WHITE, "5",
      1,   0, 0, 0,    0,   "STAT", 1,  0,  0,     0 },
    { "6", 0, 0, 0x21, 180, 300,     46, 26, WHITE, "6",
      1,   0, 0, 0,    0,   "UNITS", 1,  0,  0,     0 },
    { "MUL", 0,         0,          0x20,       240, 300,   46, 26,  WHITE, 0,
      0,     mul_width, mul_height, mul_bitmap, 0,   "[ ]", 0,  "_", 0,     0 },

    { "SHR", 0,         0,          0x15,       0, 350, 36, 26, RIGHT, 0,
      0,     shr_width, shr_height, shr_bitmap, 0, 0,   0,  0,  0,     0 },
    { "1", 0, 0, 0x13, 60, 350,   46, 26,      WHITE, "1",
      1,   0, 0, 0,    0,  "RAD", 0,  "POLAR", 0,     0 },
    { "2", 0, 0, 0x12, 120, 350,     46, 26,    WHITE, "2",
      1,   0, 0, 0,    0,   "STACK", 0,  "ARG", 0,     0 },
    { "3", 0, 0, 0x11, 180, 350,   46, 26,     WHITE, "3",
      1,   0, 0, 0,    0,   "CMD", 0,  "MENU", 0,     0 },
    { "MINUS", 0, 0,           0x10,         240,          350, 46,  26, WHITE,
      0,       0, minus_width, minus_height, minus_bitmap, 0,   "i", 0,  "j",
      0,       0 },

    { "ON", 0, 0, 0x8000, 0, 400,    36, 26,    WHITE,  "ON",
      0,    0, 0, 0,      0, "CONT", 0,  "OFF", "ATTN", 0 },
    { "0", 0, 0, 0x03, 60, 400,  46, 26,   WHITE, "0",
      1,   0, 0, 0,    0,  "= ", 0,  " a", 0,     0 },
    { "PERIOD", 0, 0, 0x02, 120, 400,  46, 26,   WHITE, ".",
      1,        0, 0, 0,    0,   ", ", 0,  " k", 0,     0 },
    { "SPC", 0, 0, 0x01, 180, 400,  46, 26,   WHITE, "SPC",
      0,     0, 0, 0,    0,   "l ", 0,  " m", 0,     0 },
    { "PLUS", 0,     0, 0x00,       240,         400,         46, 26,
      WHITE,  0,     0, plus_width, plus_height, plus_bitmap, 0,  "{ }",
      0,      ": :", 0, 0 },

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
      0 },

    { "MTH", 0, 0, 0x24, 0,   50,    36, 26,      WHITE, "MTH",
      0,     0, 0, 0,    "G", "RAD", 0,  "POLAR", 0,     0 },
    { "PRG", 0, 0, 0x74, 50,  50, 36, 26,      WHITE, "PRG",
      0,     0, 0, 0,    "H", 0,  0,  "CHARS", 0,     0 },
    { "CST", 0, 0, 0x73, 100, 50, 36, 26,      WHITE, "CST",
      0,     0, 0, 0,    "I", 0,  0,  "MODES", 0,     0 },
    { "VAR", 0, 0, 0x72, 150, 50, 36, 26,       WHITE, "VAR",
      0,     0, 0, 0,    "J", 0,  0,  "MEMORY", 0,     0 },
    { "UP", 0,        0,         0x71,      200, 50, 36, 26,      WHITE, 0,
      0,    up_width, up_height, up_bitmap, "K", 0,  0,  "STACK", 0,     0 },
    { "NXT", 0, 0, 0x70, 250, 50,     36, 26,     WHITE, "NXT",
      0,     0, 0, 0,    "L", "PREV", 0,  "MENU", 0,     0 },

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
      0 },
    { "STO", 0, 0, 0x64, 50,  100,   36, 26,    WHITE, "STO",
      0,     0, 0, 0,    "N", "DEF", 0,  "RCL", 0,     0 },
    { "EVAL", 0, 0, 0x63, 100, 100,    36, 26,     WHITE, "EVAL",
      0,      0, 0, 0,    "O", "aNUM", 0,  "UNDO", 0,     0 },
    { "LEFT", 0, 0, 0x62,       150,         100,         36,  26,
      WHITE,  0, 0, left_width, left_height, left_bitmap, "P", "PICTURE",
      0,      0, 0, 0 },
    { "DOWN", 0, 0, 0x61,       200,         100,         36,  26,
      WHITE,  0, 0, down_width, down_height, down_bitmap, "Q", "VIEW",
      0,      0, 0, 0 },
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
      0 },

    { "SIN", 0, 0, 0x34, 0,   150,    36, 26,  WHITE, "SIN",
      0,     0, 0, 0,    "S", "ASIN", 0,  "b", 0,     0 },
    { "COS", 0, 0, 0x54, 50,  150,    36, 26,  WHITE, "COS",
      0,     0, 0, 0,    "T", "ACOS", 0,  "c", 0,     0 },
    { "TAN", 0, 0, 0x53, 100, 150,    36, 26,  WHITE, "TAN",
      0,     0, 0, 0,    "U", "ATAN", 0,  "d", 0,     0 },
    { "SQRT", 0,   0, 0x52,       150,         150,         36,  26,
      WHITE,  0,   0, sqrt_width, sqrt_height, sqrt_bitmap, "V", "n",
      0,      "o", 0, 0 },
    { "POWER", 0, 0,           0x51,         200,          150, 36,  26, WHITE,
      0,       0, power_width, power_height, power_bitmap, "W", "p", 0,  "LOG",
      0,       0 },
    { "INV", 0,         0,          0x50,       250, 150, 36, 26,   WHITE, 0,
      0,     inv_width, inv_height, inv_bitmap, "X", "q", 0,  "LN", 0,     0 },

    { "ENTER", 0, 0, 0x44, 0, 200,        86, 26,       WHITE, "ENTER",
      2,       0, 0, 0,    0, "EQUATION", 0,  "MATRIX", 0,     0 },
    { "NEG", 0,      0, 0x43,  100,       200,        36,
      26,    WHITE,  0, 0,     neg_width, neg_height, neg_bitmap,
      "Y",   "EDIT", 0, "CMD", 0,         0 },
    { "EEX", 0, 0, 0x42, 150, 200,    36, 26,    WHITE, "EEX",
      0,     0, 0, 0,    "Z", "PURG", 0,  "ARG", 0,     0 },
    { "DEL", 0, 0, 0x41, 200, 200,     36, 26, WHITE, "DEL",
      0,     0, 0, 0,    0,   "CLEAR", 0,  0,  0,     0 },
    { "BS", 0,        0,         0x40,      250, 200,    36, 26, WHITE, 0,
      0,    bs_width, bs_height, bs_bitmap, 0,   "DROP", 0,  0,  0,     0 },

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
      0 },
    { "7", 0, 0, 0x33, 60, 250, 46, 26,      WHITE, "7",
      1,   0, 0, 0,    0,  0,   1,  "SOLVE", 0,     0 },
    { "8", 0, 0, 0x32, 120, 250, 46, 26,     WHITE, "8",
      1,   0, 0, 0,    0,   0,   1,  "PLOT", 0,     0 },
    { "9", 0, 0, 0x31, 180, 250, 46, 26,         WHITE, "9",
      1,   0, 0, 0,    0,   0,   1,  "SYMBOLIC", 0,     0 },
    { "DIV", 0,         0,          0x30,       240, 250,  46, 26,  WHITE, 0,
      0,     div_width, div_height, div_bitmap, 0,   "r ", 0,  "s", 0,     0 },

    { "SHL", 0,         0,          0x25,       0, 300, 36, 26, LEFT, 0,
      0,     shl_width, shl_height, shl_bitmap, 0, 0,   0,  0,  0,    0 },
    { "4", 0, 0, 0x23, 60, 300, 46, 26,     WHITE, "4",
      1,   0, 0, 0,    0,  0,   1,  "TIME", 0,     0 },
    { "5", 0, 0, 0x22, 120, 300, 46, 26,     WHITE, "5",
      1,   0, 0, 0,    0,   0,   1,  "STAT", 0,     0 },
    { "6", 0, 0, 0x21, 180, 300, 46, 26,      WHITE, "6",
      1,   0, 0, 0,    0,   0,   1,  "UNITS", 0,     0 },
    { "MUL", 0,         0,          0x20,       240, 300,  46, 26,  WHITE, 0,
      0,     mul_width, mul_height, mul_bitmap, 0,   "t ", 0,  "u", 0,     0 },

    { "SHR", 0,         0,          0x15,       0, 350, 36, 26,  RIGHT, 0,
      0,     shr_width, shr_height, shr_bitmap, 0, 0,   1,  " ", 0,     0 },
    { "1", 0, 0, 0x13, 60, 350, 46, 26,    WHITE, "1",
      1,   0, 0, 0,    0,  0,   1,  "I/O", 0,     0 },
    { "2", 0, 0, 0x12, 120, 350, 46, 26,        WHITE, "2",
      1,   0, 0, 0,    0,   0,   1,  "LIBRARY", 0,     0 },
    { "3", 0, 0, 0x11, 180, 350, 46, 26,       WHITE, "3",
      1,   0, 0, 0,    0,   0,   1,  "EQ LIB", 0,     0 },
    { "MINUS", 0, 0,           0x10,         240,          350, 46,   26, WHITE,
      0,       0, minus_width, minus_height, minus_bitmap, 0,   "v ", 0,  "w",
      0,       0 },

    { "ON", 0, 0, 0x8000, 0, 400,    36, 26,    WHITE,    "ON",
      0,    0, 0, 0,      0, "CONT", 0,  "OFF", "CANCEL", 0 },
    { "0", 0, 0, 0x03, 60, 400,     46, 26,     WHITE, "0",
      1,   0, 0, 0,    0,  "\004 ", 0,  "\003", 0,     0 },
    { "PERIOD", 0, 0, 0x02, 120, 400,     46, 26,     WHITE, ".",
      1,        0, 0, 0,    0,   "\002 ", 0,  "\001", 0,     0 },
    { "SPC", 0, 0, 0x01, 180, 400,     46, 26,  WHITE, "SPC",
      0,     0, 0, 0,    0,   "\005 ", 0,  "z", 0,     0 },
    { "PLUS", 0,   0, 0x00,       240,         400,         46, 26,
      WHITE,  0,   0, plus_width, plus_height, plus_bitmap, 0,  "x ",
      0,      "y", 0, 0 },

    { 0 } };

#define MAX_PASTE 128
int paste[ MAX_PASTE * 3 ];
int paste_count = 0;
int paste_size = 0;
int paste_last_key = 0;

int first_key = 0;

int last_button = -1;

#if defined( GUI_IS_X11 )
typedef struct icon_t {
    unsigned int w;
    unsigned int h;
    int c;
    unsigned char* bits;
} icon_map_t;

#define ICON_MAP 0
#define ON_MAP 1
#define DISP_MAP 2
#define FIRST_MAP 3
#define LAST_MAP 9

icon_map_t* icon_maps;

icon_map_t icon_maps_sx[] = {
    { hp48_icon_width, hp48_icon_height, BLACK, hp48_icon_bitmap },
    { hp48_on_width, hp48_on_height, PIXEL, hp48_on_bitmap },
    { hp48_disp_width, hp48_disp_height, LCD, hp48_disp_bitmap },
    { hp48_top_width, hp48_top_height, DISP_PAD, hp48_top_bitmap },
    { hp48_bottom_width, hp48_bottom_height, PAD, hp48_bottom_bitmap },
    { hp48_logo_width, hp48_logo_height, LOGO, hp48_logo_bitmap },
    { hp48_text_width, hp48_text_height, LABEL, hp48_text_bitmap },
    { hp48_keys_width, hp48_keys_height, BLACK, hp48_keys_bitmap },
    { hp48_orange_width, hp48_orange_height, LEFT, hp48_orange_bitmap },
    { hp48_blue_width, hp48_blue_height, RIGHT, hp48_blue_bitmap } };

icon_map_t icon_maps_gx[] = {
    { hp48_icon_width, hp48_icon_height, BLACK, hp48_icon_bitmap },
    { hp48_on_width, hp48_on_height, PIXEL, hp48_on_bitmap },
    { hp48_disp_width, hp48_disp_height, LCD, hp48_disp_bitmap },
    { hp48_top_gx_width, hp48_top_gx_height, DISP_PAD, hp48_top_gx_bitmap },
    { hp48_bottom_width, hp48_bottom_height, PAD, hp48_bottom_bitmap },
    { hp48_logo_gx_width, hp48_logo_gx_height, LOGO, hp48_logo_gx_bitmap },
    { hp48_text_gx_width, hp48_text_gx_height, LABEL, hp48_text_gx_bitmap },
    { hp48_keys_width, hp48_keys_height, BLACK, hp48_keys_bitmap },
    { hp48_orange_width, hp48_orange_height, LEFT, hp48_orange_bitmap },
    { hp48_green_gx_width, hp48_green_gx_height, RIGHT,
      hp48_green_gx_bitmap } };

#define KEYBOARD_HEIGHT ( buttons[ LAST_BUTTON ].y + buttons[ LAST_BUTTON ].h )
#define KEYBOARD_WIDTH ( buttons[ LAST_BUTTON ].x + buttons[ LAST_BUTTON ].w )

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

int AllocColors( void ) {
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
            colors[ c ].xcolor.pixel =
                ( ( colors[ c ].xcolor.red >> r_shift ) & visual->red_mask ) |
                ( ( colors[ c ].xcolor.green >> g_shift ) &
                  visual->green_mask ) |
                ( ( colors[ c ].xcolor.blue >> b_shift ) & visual->blue_mask );
            XStoreColor( dpy, cmap, &colors[ c ].xcolor );
        } else {
            if ( dynamic_color && c == PIXEL ) {
                if ( XAllocColorCells( dpy, cmap, True, ( unsigned long* )0, 0,
                                       &colors[ c ].xcolor.pixel, 1 ) == 0 ) {
                    dyn = 0;
                    if ( XAllocColor( dpy, cmap, &colors[ c ].xcolor ) == 0 ) {
                        if ( verbose )
                            fprintf( stderr, "XAllocColor failed.\n" );
                        error = c;
                        break;
                    }
                } else if ( colors[ c ].xcolor.pixel >= visual->map_entries ) {
                    dyn = 0;
                    if ( XAllocColor( dpy, cmap, &colors[ c ].xcolor ) == 0 ) {
                        if ( verbose )
                            fprintf( stderr, "XAllocColor failed.\n" );
                        error = c;
                        break;
                    }
                } else {
                    XStoreColor( dpy, cmap, &colors[ c ].xcolor );
                }
            } else {
                if ( XAllocColor( dpy, cmap, &colors[ c ].xcolor ) == 0 ) {
                    if ( verbose )
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
        if ( verbose )
            fprintf( stderr, "Using own Colormap.\n" );
        /*
         * free colors so far allocated
         */
        for ( c = WHITE; c < error; c++ ) {
            XFreeColors( dpy, cmap, &colors[ c ].xcolor.pixel, 1, 0 );
        }

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
                if ( XAllocColorCells( dpy, cmap, True, ( unsigned long* )0, 0,
                                       &colors[ c ].xcolor.pixel, 1 ) == 0 ) {
                    dyn = 0;
                    if ( XAllocColor( dpy, cmap, &colors[ c ].xcolor ) == 0 )
                        fatal_exit( "can\'t alloc Color.\n", "" );

                } else if ( colors[ c ].xcolor.pixel >= visual->map_entries ) {
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
    return 0;
}

int merge_app_defaults( char* path, XrmDatabase* db ) {
    char file[ 1024 ];
    XrmDatabase tmp;

    if ( path == ( char* )0 )
        return 0;

    sprintf( file, "%s/%s", path, res_class );

    tmp = XrmGetFileDatabase( file );
    if ( tmp == ( XrmDatabase )0 )
        return 0;

    XrmMergeDatabases( tmp, db );

    return 1;
}

int InitDisplay( int argc, char** argv ) {
    XrmDatabase cmd = NULL, tmp = NULL;
    char *res, *s;
    char buf[ 1024 ], home[ 1024 ];
    int def, i;
    struct passwd* pwd;
#ifdef SYSV
    struct utsname uts;
#else
    char hostname[ 128 ];
#endif

    /*
     * Parse the command line
     */
    XrmInitialize();
    XrmParseCommand( &cmd, options, sizeof( options ) / sizeof( *options ),
                     progname, &argc, argv );

    if ( ( argc == 2 ) && !strcmp( argv[ 1 ], "-help" ) )
        usage();
    else if ( argc > 1 ) {
        fprintf( stderr, "unknown option %s or missing argument\n", argv[ 1 ] );
        usage();
    }

    res_name = progname;
    res_class = strdup( res_name );
    *res_class = islower( *res_class ) ? toupper( *res_class ) : *res_class;

    /*
     * look for argument -name
     */
    res = get_string_resource_from_db( cmd, "name", "Name" );
    if ( res ) {
        if ( !( res_name = strdup( res ) ) )
            fatal_exit( "out of memory in InitDisplay()\n", "" );

        for ( s = res_name; *s; s++ )
            *s = isupper( *s ) ? tolower( *s ) : *s;

        free( res_class );
        res_class = strdup( res_name );
        *res_class = islower( *res_class ) ? toupper( *res_class ) : *res_class;

        argc = saved_argc;
        argv = ( char** )malloc( ( argc + 1 ) * sizeof( char* ) );
        if ( argv == ( char** )0 )
            fatal_exit( "out of memory in InitDisplay()\n", "" );

        argv[ argc ] = ( char* )0;
        for ( i = 0; i < argc; i++ )
            argv[ i ] = saved_argv[ i ];

        XrmParseCommand( &cmd, options, sizeof( options ) / sizeof( *options ),
                         res_name, &argc, argv );
    }

    /*
     * Open the display
     */
    res = get_string_resource_from_db( cmd, "display", "Display" );

    dpy = XOpenDisplay( res );
    if ( dpy == ( Display* )0 ) {
        if ( res ) {
            if ( verbose )
                fprintf( stderr, "can\'t open display %s\n", res );
        } else {
            if ( verbose )
                fprintf( stderr, "can\'t open display\n" );
        }
        return -1;
    }

    /*
     * Load all those Resources.
     *
     * 1. Hardcoded Defaults
     *
     * 2. /usr/lib/X11/app-defaults/X48NG
     *
     * 3. Values in $XUSERFILESEARCHPATH/X48NG or, if not set,
     *    $XAPPLRESDIR/X48NG
     *
     * 4. Values from XResourceManagerString() or, if empty,
     *    ~/.Xdefaults
     *
     * 5. Values in $XENVIRONMENT or, if not set,
     *    ~/.Xdefaults-hostname
     *
     * 6. Command line arguments
     */

    /* 1.  Hardcoded Defaults */

    for ( def = 0; defaults[ def ]; def++ ) {
        if ( ( tmp = XrmGetStringDatabase( defaults[ def ] ) ) )
            XrmMergeDatabases( tmp, &rdb );
    }

    /* 2. /usr/lib/X11/app-defaults/X48NG */

    merge_app_defaults( "/usr/lib/X11/app-defaults", &rdb );

    /* 3. Values in $XUSERFILESEARCHPATH/X48NG, or $XAPPLRESDIR/X48NG */

    if ( !merge_app_defaults( getenv( "XUSERFILESEARCHPATH" ), &rdb ) )
        merge_app_defaults( getenv( "XAPPLRESDIR" ), &rdb );

    /* 4. Values from XResourceManagerString() or ~/.Xdefaults */

    res = XResourceManagerString( dpy );
    if ( res ) {
        if ( ( tmp = XrmGetStringDatabase( res ) ) )
            XrmMergeDatabases( tmp, &rdb );
    } else {
        res = getenv( "HOME" );
        if ( res )
            strcpy( home, res );
        else {
            pwd = getpwuid( getuid() );
            if ( pwd )
                strcpy( home, pwd->pw_dir );
        }
        sprintf( buf, "%s/.Xdefaults", home );
        if ( ( tmp = XrmGetFileDatabase( buf ) ) )
            XrmMergeDatabases( tmp, &rdb );
    }

    /* 5. Values in $XENVIRONMENT or ~/.Xdefaults-hostname */

    res = getenv( "XENVIRONMENT" );
    if ( res ) {
        if ( ( tmp = XrmGetFileDatabase( res ) ) )
            XrmMergeDatabases( tmp, &rdb );
    } else {
        res = getenv( "HOME" );
        if ( res )
            strcpy( home, res );
        else {
            pwd = getpwuid( getuid() );
            if ( pwd )
                strcpy( home, pwd->pw_dir );
        }
        tmp = ( XrmDatabase )0;
#ifdef SYSV
        if ( uname( &uts ) >= 0 ) {
            sprintf( buf, "%s/.Xdefaults-%s", home, uts.nodename );
            tmp = XrmGetFileDatabase( buf );
        }
#else
        if ( gethostname( hostname, 128 ) >= 0 ) {
            sprintf( buf, "%s/.Xdefaults-%s", home, hostname );
            tmp = XrmGetFileDatabase( buf );
        }
#endif
        if ( tmp )
            XrmMergeDatabases( tmp, &rdb );
    }

    /* 6. Command line arguments */

    if ( cmd )
        XrmMergeDatabases( cmd, &rdb );

    get_resources();

    /*
     * Get the default screen
     */
    screen = DefaultScreen( dpy );

    /*
     * Does the Xserver do backing-store?
     */
    does_backing_store = XDoesBackingStore( XScreenOfDisplay( dpy, screen ) );

    /*
     * Try to use XShm-Extension
     */
    shm_flag = useXShm;

    if ( !XShmQueryExtension( dpy ) ) {
        shm_flag = 0;
        if ( verbose )
            fprintf( stderr, "Xserver does not support XShm extension.\n" );
    }
    if ( shm_flag )
        fprintf( stderr, "using XShm extension.\n" );

    return 0;
}

#elif defined( GUI_IS_SDL1 )

typedef struct sdltohpkeymap_t {
    SDLKey sdlkey;
    int hpkey;
} sdltohpkeymap_t;

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
#endif

#if defined( GUI_IS_X11 )
void adjust_contrast( int contrast ) {
    int gray = 0;
    int r = 0, g = 0, b = 0;
    unsigned long old;

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
            b = 128 -
                ( ( 0x13 - contrast ) * ( ( 128 - colors[ LCD ].b ) / 0x10 ) );
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
        XSetForeground( dpy, disp.gc, COLOR( PIXEL ) );
        disp.display_update = UPDATE_DISP | UPDATE_MENU;
        refresh_display();
        redraw_annunc();
        last_icon_state = -1;
        refresh_icon();
    } else if ( dynamic_color ) {
        XStoreColor( dpy, cmap, &colors[ PIXEL ].xcolor );
    } else {
        if ( XAllocColor( dpy, cmap, &colors[ PIXEL ].xcolor ) == 0 ) {
            colors[ PIXEL ].xcolor.pixel = old;
            if ( verbose )
                fprintf( stderr, "warning: can\'t alloc new pixel color.\n" );
        } else {
            XFreeColors( dpy, cmap, &old, 1, 0 );
            XSetForeground( dpy, disp.gc, COLOR( PIXEL ) );
            disp.display_update = UPDATE_DISP | UPDATE_MENU;
            refresh_display();
            redraw_annunc();
            last_icon_state = -1;
            refresh_icon();
        }
    }
}
#elif defined( GUI_IS_SDL1 )
void adjust_contrast( int contrast ) {
    SDLCreateColors();
    SDLCreateAnnunc();
    redraw_display();
}
#endif

int SmallTextWidth( const char* string, unsigned int length ) {
    int i, w;

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

void exit_x48( int tell_x11 ) {
    exit_emulator();

#if defined( GUI_IS_X11 )
    if ( tell_x11 )
        XCloseDisplay( dpy );
#endif

    exit( 0 );
}

#if defined( GUI_IS_X11 )
int DrawSmallString( Display* the_dpy, Drawable d, GC the_gc, int x, int y,
                     const char* string, unsigned int length ) {
    int i;
    Pixmap pix;

    for ( i = 0; i < length; i++ ) {
        if ( small_font[ ( int )string[ i ] ].h != 0 ) {
            pix = XCreateBitmapFromData(
                the_dpy, d, ( char* )small_font[ ( int )string[ i ] ].bits,
                small_font[ ( int )string[ i ] ].w,
                small_font[ ( int )string[ i ] ].h );
            XCopyPlane( the_dpy, pix, d, the_gc, 0, 0,
                        small_font[ ( int )string[ i ] ].w,
                        small_font[ ( int )string[ i ] ].h, x,
                        ( int )( y - small_font[ ( int )string[ i ] ].h ), 1 );
            XFreePixmap( the_dpy, pix );
        }
        x += SmallTextWidth( &string[ i ], 1 );
    }
    return 0;
}

void CreateButton( int i, int off_x, int off_y, XFontStruct* f_small,
                   XFontStruct* f_med, XFontStruct* f_big ) {
    int x, y;
    XSetWindowAttributes xswa;
    XFontStruct* finfo;
    XGCValues val;
    unsigned long gc_mask;
    Pixmap pix;
    XCharStruct xchar;
    int dir, fa, fd;
    unsigned long pixel;

    {
        if ( i < BUTTON_MTH )
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
        buttons[ i ].xwin = XCreateSimpleWindow(
            dpy, mainW, off_x + buttons[ i ].x, off_y + buttons[ i ].y,
            buttons[ i ].w, buttons[ i ].h, 0, COLOR( BLACK ), pixel );

        XDefineCursor( dpy, buttons[ i ].xwin,
                       XCreateFontCursor( dpy, XC_hand1 ) );

        xswa.event_mask = LeaveWindowMask | ExposureMask | StructureNotifyMask;
        xswa.backing_store = Always;

        XChangeWindowAttributes( dpy, buttons[ i ].xwin,
                                 CWEventMask | CWBackingStore, &xswa );

        /*
         * draw the released button
         */
        buttons[ i ].map = XCreatePixmap(
            dpy, buttons[ i ].xwin, buttons[ i ].w, buttons[ i ].h, depth );

        XSetForeground( dpy, gc, pixel );
        XFillRectangle( dpy, buttons[ i ].map, gc, 0, 0, buttons[ i ].w,
                        buttons[ i ].h );

        XSetForeground( dpy, gc, COLOR( BUTTON ) );
        XFillRectangle( dpy, buttons[ i ].map, gc, 1, 1, buttons[ i ].w - 2,
                        buttons[ i ].h - 2 );

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

            XTextExtents( finfo, buttons[ i ].label,
                          ( int )strlen( buttons[ i ].label ), &dir, &fa, &fd,
                          &xchar );
            x = ( buttons[ i ].w - xchar.width ) / 2;
            y = ( 1 + buttons[ i ].h - ( xchar.ascent + xchar.descent ) ) / 2 +
                xchar.ascent + 1;
            XDrawImageString( dpy, buttons[ i ].map, gc, x, y,
                              buttons[ i ].label,
                              ( int )strlen( buttons[ i ].label ) );

            XSetBackground( dpy, gc, COLOR( BLACK ) );

        } else if ( buttons[ i ].lw != 0 ) {

            /*
             * draw pixmap centered in button
             */
            XSetBackground( dpy, gc, COLOR( BUTTON ) );
            XSetForeground( dpy, gc, COLOR( buttons[ i ].lc ) );

            pix = XCreateBitmapFromData( dpy, buttons[ i ].xwin,
                                         ( char* )buttons[ i ].lb,
                                         buttons[ i ].lw, buttons[ i ].lh );

            x = ( 1 + buttons[ i ].w - buttons[ i ].lw ) / 2;
            y = ( 1 + buttons[ i ].h - buttons[ i ].lh ) / 2 + 1;

            XCopyPlane( dpy, pix, buttons[ i ].map, gc, 0, 0, buttons[ i ].lw,
                        buttons[ i ].lh, x, y, 1 );

            XFreePixmap( dpy, pix );

            XSetBackground( dpy, gc, COLOR( BLACK ) );
        }

        /*
         * draw edge of button
         */
        XSetForeground( dpy, gc, COLOR( BUT_TOP ) );

        XDrawLine( dpy, buttons[ i ].map, gc, 1, ( int )( buttons[ i ].h - 2 ),
                   1, 1 );
        XDrawLine( dpy, buttons[ i ].map, gc, 2, ( int )( buttons[ i ].h - 3 ),
                   2, 2 );
        XDrawLine( dpy, buttons[ i ].map, gc, 3, ( int )( buttons[ i ].h - 4 ),
                   3, 3 );

        XDrawLine( dpy, buttons[ i ].map, gc, 1, 1,
                   ( int )( buttons[ i ].w - 2 ), 1 );
        XDrawLine( dpy, buttons[ i ].map, gc, 2, 2,
                   ( int )( buttons[ i ].w - 3 ), 2 );
        XDrawLine( dpy, buttons[ i ].map, gc, 3, 3,
                   ( int )( buttons[ i ].w - 4 ), 3 );
        XDrawLine( dpy, buttons[ i ].map, gc, 4, 4,
                   ( int )( buttons[ i ].w - 5 ), 4 );

        XDrawPoint( dpy, buttons[ i ].map, gc, 4, 5 );

        XSetForeground( dpy, gc, COLOR( BUT_BOT ) );

        XDrawLine( dpy, buttons[ i ].map, gc, 3, ( int )( buttons[ i ].h - 2 ),
                   ( int )( buttons[ i ].w - 2 ),
                   ( int )( buttons[ i ].h - 2 ) );
        XDrawLine( dpy, buttons[ i ].map, gc, 4, ( int )( buttons[ i ].h - 3 ),
                   ( int )( buttons[ i ].w - 3 ),
                   ( int )( buttons[ i ].h - 3 ) );

        XDrawLine( dpy, buttons[ i ].map, gc, ( int )( buttons[ i ].w - 2 ),
                   ( int )( buttons[ i ].h - 2 ), ( int )( buttons[ i ].w - 2 ),
                   3 );
        XDrawLine( dpy, buttons[ i ].map, gc, ( int )( buttons[ i ].w - 3 ),
                   ( int )( buttons[ i ].h - 3 ), ( int )( buttons[ i ].w - 3 ),
                   4 );
        XDrawLine( dpy, buttons[ i ].map, gc, ( int )( buttons[ i ].w - 4 ),
                   ( int )( buttons[ i ].h - 4 ), ( int )( buttons[ i ].w - 4 ),
                   5 );

        XDrawPoint( dpy, buttons[ i ].map, gc, ( int )( buttons[ i ].w - 5 ),
                    ( int )( buttons[ i ].h - 4 ) );

        /*
         * draw frame around button
         */
        XSetForeground( dpy, gc, COLOR( FRAME ) );

        XDrawLine( dpy, buttons[ i ].map, gc, 0, ( int )( buttons[ i ].h - 3 ),
                   0, 2 );
        XDrawLine( dpy, buttons[ i ].map, gc, 2, 0,
                   ( int )( buttons[ i ].w - 3 ), 0 );
        XDrawLine( dpy, buttons[ i ].map, gc, 2, ( int )( buttons[ i ].h - 1 ),
                   ( int )( buttons[ i ].w - 3 ),
                   ( int )( buttons[ i ].h - 1 ) );
        XDrawLine( dpy, buttons[ i ].map, gc, ( int )( buttons[ i ].w - 1 ),
                   ( int )( buttons[ i ].h - 3 ), ( int )( buttons[ i ].w - 1 ),
                   2 );

        if ( i == BUTTON_ON ) {
            XDrawLine( dpy, buttons[ i ].map, gc, 1, 1,
                       ( int )( buttons[ i ].w - 2 ), 1 );
            XDrawPoint( dpy, buttons[ i ].map, gc, 1, 2 );
            XDrawPoint( dpy, buttons[ i ].map, gc,
                        ( int )( buttons[ i ].w - 2 ), 2 );
        } else {
            XDrawPoint( dpy, buttons[ i ].map, gc, 1, 1 );
            XDrawPoint( dpy, buttons[ i ].map, gc,
                        ( int )( buttons[ i ].w - 2 ), 1 );
        }
        XDrawPoint( dpy, buttons[ i ].map, gc, 1,
                    ( int )( buttons[ i ].h - 2 ) );
        XDrawPoint( dpy, buttons[ i ].map, gc, ( int )( buttons[ i ].w - 2 ),
                    ( int )( buttons[ i ].h - 2 ) );

        /*
         * draw the depressed button
         */
        buttons[ i ].down = XCreatePixmap(
            dpy, buttons[ i ].xwin, buttons[ i ].w, buttons[ i ].h, depth );

        XSetForeground( dpy, gc, pixel );
        XFillRectangle( dpy, buttons[ i ].down, gc, 0, 0, buttons[ i ].w,
                        buttons[ i ].h );

        XSetForeground( dpy, gc, COLOR( BUTTON ) );
        XFillRectangle( dpy, buttons[ i ].down, gc, 1, 1, buttons[ i ].w - 2,
                        buttons[ i ].h - 2 );

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

            XTextExtents( finfo, buttons[ i ].label,
                          ( int )strlen( buttons[ i ].label ), &dir, &fa, &fd,
                          &xchar );
            x = ( buttons[ i ].w - xchar.width ) / 2;
            y = ( 1 + buttons[ i ].h - ( xchar.ascent + xchar.descent ) ) / 2 +
                xchar.ascent;
            XDrawImageString( dpy, buttons[ i ].down, gc, x, y,
                              buttons[ i ].label,
                              ( int )strlen( buttons[ i ].label ) );

            XSetBackground( dpy, gc, COLOR( BLACK ) );

        } else {

            /*
             * draw pixmap centered in button
             */
            XSetBackground( dpy, gc, COLOR( BUTTON ) );
            XSetForeground( dpy, gc, COLOR( buttons[ i ].lc ) );

            pix = XCreateBitmapFromData( dpy, buttons[ i ].xwin,
                                         ( char* )buttons[ i ].lb,
                                         buttons[ i ].lw, buttons[ i ].lh );

            x = ( 1 + buttons[ i ].w - buttons[ i ].lw ) / 2;
            y = ( 1 + buttons[ i ].h - buttons[ i ].lh ) / 2;

            XCopyPlane( dpy, pix, buttons[ i ].down, gc, 0, 0, buttons[ i ].lw,
                        buttons[ i ].lh, x, y, 1 );

            XFreePixmap( dpy, pix );

            XSetBackground( dpy, gc, COLOR( BLACK ) );
        }

        /*
         * draw edge of button
         */
        XSetForeground( dpy, gc, COLOR( BUT_TOP ) );

        XDrawLine( dpy, buttons[ i ].down, gc, 2, ( int )( buttons[ i ].h - 4 ),
                   2, 2 );
        XDrawLine( dpy, buttons[ i ].down, gc, 3, ( int )( buttons[ i ].h - 5 ),
                   3, 3 );

        XDrawLine( dpy, buttons[ i ].down, gc, 2, 2,
                   ( int )( buttons[ i ].w - 4 ), 2 );
        XDrawLine( dpy, buttons[ i ].down, gc, 3, 3,
                   ( int )( buttons[ i ].w - 5 ), 3 );

        XDrawPoint( dpy, buttons[ i ].down, gc, 4, 4 );

        XSetForeground( dpy, gc, COLOR( BUT_BOT ) );

        XDrawLine( dpy, buttons[ i ].down, gc, 3, ( int )( buttons[ i ].h - 3 ),
                   ( int )( buttons[ i ].w - 3 ),
                   ( int )( buttons[ i ].h - 3 ) );
        XDrawLine( dpy, buttons[ i ].down, gc, 4, ( int )( buttons[ i ].h - 4 ),
                   ( int )( buttons[ i ].w - 4 ),
                   ( int )( buttons[ i ].h - 4 ) );

        XDrawLine( dpy, buttons[ i ].down, gc, ( int )( buttons[ i ].w - 3 ),
                   ( int )( buttons[ i ].h - 3 ), ( int )( buttons[ i ].w - 3 ),
                   3 );
        XDrawLine( dpy, buttons[ i ].down, gc, ( int )( buttons[ i ].w - 4 ),
                   ( int )( buttons[ i ].h - 4 ), ( int )( buttons[ i ].w - 4 ),
                   4 );

        XDrawPoint( dpy, buttons[ i ].down, gc, ( int )( buttons[ i ].w - 5 ),
                    ( int )( buttons[ i ].h - 5 ) );

        /*
         * draw frame around button
         */
        XSetForeground( dpy, gc, COLOR( FRAME ) );

        XDrawLine( dpy, buttons[ i ].down, gc, 0, ( int )( buttons[ i ].h - 3 ),
                   0, 2 );
        XDrawLine( dpy, buttons[ i ].down, gc, 2, 0,
                   ( int )( buttons[ i ].w - 3 ), 0 );
        XDrawLine( dpy, buttons[ i ].down, gc, 2, ( int )( buttons[ i ].h - 1 ),
                   ( int )( buttons[ i ].w - 3 ),
                   ( int )( buttons[ i ].h - 1 ) );
        XDrawLine( dpy, buttons[ i ].down, gc, ( int )( buttons[ i ].w - 1 ),
                   ( int )( buttons[ i ].h - 3 ), ( int )( buttons[ i ].w - 1 ),
                   2 );

        if ( i == BUTTON_ON ) {
            XDrawLine( dpy, buttons[ i ].down, gc, 1, 1,
                       ( int )( buttons[ i ].w - 2 ), 1 );
            XDrawPoint( dpy, buttons[ i ].down, gc, 1, 2 );
            XDrawPoint( dpy, buttons[ i ].down, gc,
                        ( int )( buttons[ i ].w - 2 ), 2 );
        } else {
            XDrawPoint( dpy, buttons[ i ].down, gc, 1, 1 );
            XDrawPoint( dpy, buttons[ i ].down, gc,
                        ( int )( buttons[ i ].w - 2 ), 1 );
        }
        XDrawPoint( dpy, buttons[ i ].down, gc, 1,
                    ( int )( buttons[ i ].h - 2 ) );
        XDrawPoint( dpy, buttons[ i ].down, gc, ( int )( buttons[ i ].w - 2 ),
                    ( int )( buttons[ i ].h - 2 ) );

        if ( i == BUTTON_ON ) {
            XDrawRectangle( dpy, buttons[ i ].down, gc, 1, 2,
                            buttons[ i ].w - 3, buttons[ i ].h - 4 );
            XDrawPoint( dpy, buttons[ i ].down, gc, 2, 3 );
            XDrawPoint( dpy, buttons[ i ].down, gc,
                        ( int )( buttons[ i ].w - 3 ), 3 );
        } else {
            XDrawRectangle( dpy, buttons[ i ].down, gc, 1, 1,
                            buttons[ i ].w - 3, buttons[ i ].h - 3 );
            XDrawPoint( dpy, buttons[ i ].down, gc, 2, 2 );
            XDrawPoint( dpy, buttons[ i ].down, gc,
                        ( int )( buttons[ i ].w - 3 ), 2 );
        }
        XDrawPoint( dpy, buttons[ i ].down, gc, 2,
                    ( int )( buttons[ i ].h - 3 ) );
        XDrawPoint( dpy, buttons[ i ].down, gc, ( int )( buttons[ i ].w - 3 ),
                    ( int )( buttons[ i ].h - 3 ) );
    }
}

void DrawButtons( void ) {
    int i;

    for ( i = BUTTON_A; i <= LAST_BUTTON; i++ ) {
        if ( buttons[ i ].pressed ) {
            XCopyArea( dpy, buttons[ i ].down, buttons[ i ].xwin, gc, 0, 0,
                       buttons[ i ].w, buttons[ i ].h, 0, 0 );
        } else {
            XCopyArea( dpy, buttons[ i ].map, buttons[ i ].xwin, gc, 0, 0,
                       buttons[ i ].w, buttons[ i ].h, 0, 0 );
        }
    }
}

int DrawButton( int i ) {
    if ( buttons[ i ].pressed ) {
        XCopyArea( dpy, buttons[ i ].down, buttons[ i ].xwin, gc, 0, 0,
                   buttons[ i ].w, buttons[ i ].h, 0, 0 );
    } else {
        XCopyArea( dpy, buttons[ i ].map, buttons[ i ].xwin, gc, 0, 0,
                   buttons[ i ].w, buttons[ i ].h, 0, 0 );
    }
    return 0;
}

void CreateBackground( int width, int height, int w_top, int h_top,
                       keypad_t* keypad ) {
    XSetBackground( dpy, gc, COLOR( PAD ) );
    XSetForeground( dpy, gc, COLOR( PAD ) );

    XFillRectangle( dpy, keypad->pixmap, gc, 0, 0, w_top, h_top );

    XSetBackground( dpy, gc, COLOR( DISP_PAD ) );
    XSetForeground( dpy, gc, COLOR( DISP_PAD ) );

    XFillRectangle( dpy, keypad->pixmap, gc, 0, 0, width, height );
}

void CreateKeypad( unsigned int w, unsigned int h, unsigned int offset_y,
                   unsigned int offset_x, keypad_t* keypad ) {
    int i, x, y;
    int wl, wr, ws;
    Pixmap pix;
    unsigned long pixel;
    unsigned int pw, ph;
    XFontStruct *f_small, *f_med, *f_big;

    f_small = get_font_resource( dpy, "smallLabelFont", "SmallLabelFont" );
    f_med = get_font_resource( dpy, "mediumLabelFont", "MediumLabelFont" );
    f_big = get_font_resource( dpy, "largeLabelFont", "LargeLabelFont" );

    /*
     * draw the character labels
     */
    for ( i = BUTTON_A; i <= LAST_BUTTON; i++ ) {

        CreateButton( i, offset_x, offset_y, f_small, f_med, f_big );

        if ( i < BUTTON_MTH )
            pixel = COLOR( DISP_PAD );
        else
            pixel = COLOR( PAD );

        if ( buttons[ i ].letter != ( char* )0 ) {

            XSetBackground( dpy, gc, pixel );
            XSetForeground( dpy, gc, COLOR( WHITE ) );

            if ( opt_gx ) {
                x = offset_x + buttons[ i ].x + buttons[ i ].w + 3;
                y = offset_y + buttons[ i ].y + buttons[ i ].h + 1;
            } else {
                x = offset_x + buttons[ i ].x + buttons[ i ].w -
                    SmallTextWidth( buttons[ i ].letter, 1 ) / 2 + 5;
                y = offset_y + buttons[ i ].y + buttons[ i ].h - 2;
            }

            DrawSmallString( dpy, keypad->pixmap, gc, x, y, buttons[ i ].letter,
                             1 );
        }
    }

    XFreeFont( dpy, f_big );
    XFreeFont( dpy, f_med );
    XFreeFont( dpy, f_small );

    /*
     * draw the bottom labels
     */
    for ( i = BUTTON_A; i <= LAST_BUTTON; i++ ) {

        if ( buttons[ i ].sub != ( char* )0 ) {

            XSetBackground( dpy, gc, pixel );
            XSetForeground( dpy, gc, COLOR( WHITE ) );

            x = offset_x + buttons[ i ].x +
                ( 1 + buttons[ i ].w -
                  SmallTextWidth( buttons[ i ].sub,
                                  strlen( buttons[ i ].sub ) ) ) /
                    2;
            y = offset_y + buttons[ i ].y + buttons[ i ].h + small_ascent + 2;

            DrawSmallString( dpy, keypad->pixmap, gc, x, y, buttons[ i ].sub,
                             strlen( buttons[ i ].sub ) );
        }
    }

    /*
     * draw the left labels
     */
    for ( i = BUTTON_A; i <= LAST_BUTTON; i++ ) {

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

                pix = XCreatePixmap( dpy, keypad->pixmap, pw, ph, depth );

                XSetForeground( dpy, gc, COLOR( UNDERLAY ) );

                XFillRectangle( dpy, pix, gc, 0, 0, pw, ph );

                XSetBackground( dpy, gc, COLOR( UNDERLAY ) );
                XSetForeground( dpy, gc, COLOR( LEFT ) );

                x = ( pw + 1 -
                      SmallTextWidth( buttons[ i ].left,
                                      strlen( buttons[ i ].left ) ) ) /
                    2;
                if ( opt_gx )
                    y = 14;
                else
                    y = 9;

                DrawSmallString( dpy, pix, gc, x, y, buttons[ i ].left,
                                 strlen( buttons[ i ].left ) );

                XSetForeground( dpy, gc, pixel );

                if ( !opt_gx ) {
                    XDrawPoint( dpy, pix, gc, 0, 0 );
                    XDrawPoint( dpy, pix, gc, 0, ph - 1 );
                    XDrawPoint( dpy, pix, gc, pw - 1, 0 );
                    XDrawPoint( dpy, pix, gc, pw - 1, ph - 1 );
                }

                if ( opt_gx ) {
                    x = offset_x + buttons[ i ].x - 6;
                    y = offset_y + buttons[ i ].y - small_ascent -
                        small_descent - 6;
                } else {
                    x = offset_x + buttons[ i ].x + ( buttons[ i ].w - pw ) / 2;
                    y = offset_y + buttons[ i ].y - small_ascent -
                        small_descent;
                }

                XCopyArea( dpy, pix, keypad->pixmap, gc, 0, 0, pw, ph, x, y );

                XFreePixmap( dpy, pix );

            } else {

                XSetBackground( dpy, gc, pixel );
                XSetForeground( dpy, gc, COLOR( LEFT ) );

                if ( buttons[ i ].right == ( char* )0 ) { /* centered label */

                    x = offset_x + buttons[ i ].x +
                        ( 1 + buttons[ i ].w -
                          SmallTextWidth( buttons[ i ].left,
                                          strlen( buttons[ i ].left ) ) ) /
                            2;

                } else { /* label to the left */

                    wl = SmallTextWidth( buttons[ i ].left,
                                         strlen( buttons[ i ].left ) );
                    wr = SmallTextWidth( buttons[ i ].right,
                                         strlen( buttons[ i ].right ) );
                    ws = SmallTextWidth( " ", 1 );

                    x = offset_x + buttons[ i ].x +
                        ( 1 + buttons[ i ].w - ( wl + wr + ws ) ) / 2;
                }

                y = offset_y + buttons[ i ].y - small_descent;

                DrawSmallString( dpy, keypad->pixmap, gc, x, y,
                                 buttons[ i ].left,
                                 strlen( buttons[ i ].left ) );
            }
        }
    }

    /*
     * draw the right labels
     */
    for ( i = BUTTON_A; i <= LAST_BUTTON; i++ ) {

        if ( i < BUTTON_MTH )
            pixel = COLOR( DISP_PAD );
        else
            pixel = COLOR( PAD );

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

                pix = XCreatePixmap( dpy, keypad->pixmap, pw, ph, depth );

                XSetForeground( dpy, gc, COLOR( UNDERLAY ) );

                XFillRectangle( dpy, pix, gc, 0, 0, pw, ph );

                XSetBackground( dpy, gc, COLOR( UNDERLAY ) );
                XSetForeground( dpy, gc, COLOR( RIGHT ) );

                x = ( pw + 1 -
                      SmallTextWidth( buttons[ i ].right,
                                      strlen( buttons[ i ].right ) ) ) /
                    2;
                if ( opt_gx )
                    y = 14;
                else
                    y = 8;

                DrawSmallString( dpy, pix, gc, x, y, buttons[ i ].right,
                                 strlen( buttons[ i ].right ) );

                XSetForeground( dpy, gc, pixel );

                if ( !opt_gx ) {
                    XDrawPoint( dpy, pix, gc, 0, 0 );
                    XDrawPoint( dpy, pix, gc, 0, ph - 1 );
                    XDrawPoint( dpy, pix, gc, pw - 1, 0 );
                    XDrawPoint( dpy, pix, gc, pw - 1, ph - 1 );
                }

                if ( opt_gx ) {
                    x = offset_x + buttons[ i ].x - 6;
                    y = offset_y + buttons[ i ].y - small_ascent -
                        small_descent - 6;
                } else {
                    x = offset_x + buttons[ i ].x + ( buttons[ i ].w - pw ) / 2;
                    y = offset_y + buttons[ i ].y - small_ascent -
                        small_descent;
                }

                XCopyArea( dpy, pix, keypad->pixmap, gc, 0, 0, pw, ph, x, y );

                XFreePixmap( dpy, pix );

            } else {

                XSetBackground( dpy, gc, pixel );
                XSetForeground( dpy, gc, COLOR( RIGHT ) );

                if ( buttons[ i ].left == ( char* )0 ) { /* centered label */

                    x = offset_x + buttons[ i ].x +
                        ( 1 + buttons[ i ].w -
                          SmallTextWidth( buttons[ i ].right,
                                          strlen( buttons[ i ].right ) ) ) /
                            2;

                } else { /* label to the right */

                    wl = SmallTextWidth( buttons[ i ].left,
                                         strlen( buttons[ i ].left ) );
                    wr = SmallTextWidth( buttons[ i ].right,
                                         strlen( buttons[ i ].right ) );
                    ws = SmallTextWidth( " ", 1 );

                    x = offset_x + buttons[ i ].x +
                        ( 1 + buttons[ i ].w - ( wl + wr + ws ) ) / 2 + wl + ws;
                }

                y = offset_y + buttons[ i ].y - small_descent;

                DrawSmallString( dpy, keypad->pixmap, gc, x, y,
                                 buttons[ i ].right,
                                 strlen( buttons[ i ].right ) );
            }
        }
    }

    /*
     * at last draw the v--- LAST ---v thing
     */

    if ( !opt_gx ) {
        XSetBackground( dpy, gc, COLOR( PAD ) );
        XSetForeground( dpy, gc, COLOR( WHITE ) );

        pix = XCreateBitmapFromData( dpy, keypad->pixmap, ( char* )last_bitmap,
                                     last_width, last_height );

        x = offset_x + buttons[ BUTTON_1 ].x + buttons[ BUTTON_1 ].w +
            ( buttons[ BUTTON_2 ].x - buttons[ BUTTON_1 ].x -
              buttons[ BUTTON_1 ].w ) /
                2;
        y = offset_y + buttons[ BUTTON_5 ].y + buttons[ BUTTON_5 ].h + 2;

        XCopyPlane( dpy, pix, keypad->pixmap, gc, 0, 0, last_width, last_height,
                    x, y, 1 );

        XFreePixmap( dpy, pix );
    }
}

void CreateBezel( unsigned int width, unsigned int height,
                  unsigned int offset_y, unsigned int offset_x,
                  keypad_t* keypad ) {
    int i;
    int display_height = DISPLAY_HEIGHT;
    int display_width = DISPLAY_WIDTH;

    /*
     * draw the frame around the display
     */
    XSetForeground( dpy, gc, COLOR( DISP_PAD_TOP ) );

    for ( i = 0; i < DISP_FRAME; i++ ) {
        XDrawLine( dpy, keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X - i ),
                   ( int )( DISPLAY_OFFSET_Y + display_height + 2 * i ),
                   ( int )( DISPLAY_OFFSET_X + display_width + i ),
                   ( int )( DISPLAY_OFFSET_Y + display_height + 2 * i ) );
        XDrawLine( dpy, keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X - i ),
                   ( int )( DISPLAY_OFFSET_Y + display_height + 2 * i + 1 ),
                   ( int )( DISPLAY_OFFSET_X + display_width + i ),
                   ( int )( DISPLAY_OFFSET_Y + display_height + 2 * i + 1 ) );
        XDrawLine( dpy, keypad->pixmap, gc,
                   ( int )( DISPLAY_OFFSET_X + display_width + i ),
                   ( int )( DISPLAY_OFFSET_Y - i ),
                   ( int )( DISPLAY_OFFSET_X + display_width + i ),
                   ( int )( DISPLAY_OFFSET_Y + display_height + 2 * i ) );
    }

    XSetForeground( dpy, gc, COLOR( DISP_PAD_BOT ) );

    for ( i = 0; i < DISP_FRAME; i++ ) {
        XDrawLine( dpy, keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X - i - 1 ),
                   ( int )( DISPLAY_OFFSET_Y - i - 1 ),
                   ( int )( DISPLAY_OFFSET_X + display_width + i - 1 ),
                   ( int )( DISPLAY_OFFSET_Y - i - 1 ) );
        XDrawLine( dpy, keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X - i - 1 ),
                   ( int )( DISPLAY_OFFSET_Y - i - 1 ),
                   ( int )( DISPLAY_OFFSET_X - i - 1 ),
                   ( int )( DISPLAY_OFFSET_Y + display_height + 2 * i - 1 ) );
    }

    /*
     * round off corners
     */
    XSetForeground( dpy, gc, COLOR( DISP_PAD ) );

    XDrawLine( dpy, keypad->pixmap, gc,
               ( int )( DISPLAY_OFFSET_X - DISP_FRAME ),
               ( int )( DISPLAY_OFFSET_Y - DISP_FRAME ),
               ( int )( DISPLAY_OFFSET_X - DISP_FRAME + 3 ),
               ( int )( DISPLAY_OFFSET_Y - DISP_FRAME ) );
    XDrawLine( dpy, keypad->pixmap, gc,
               ( int )( DISPLAY_OFFSET_X - DISP_FRAME ),
               ( int )( DISPLAY_OFFSET_Y - DISP_FRAME ),
               ( int )( DISPLAY_OFFSET_X - DISP_FRAME ),
               ( int )( DISPLAY_OFFSET_Y - DISP_FRAME + 3 ) );
    XDrawPoint( dpy, keypad->pixmap, gc,
                ( int )( DISPLAY_OFFSET_X - DISP_FRAME + 1 ),
                ( int )( DISPLAY_OFFSET_Y - DISP_FRAME + 1 ) );

    XDrawLine( dpy, keypad->pixmap, gc,
               ( int )( DISPLAY_OFFSET_X + display_width + DISP_FRAME - 4 ),
               ( int )( DISPLAY_OFFSET_Y - DISP_FRAME ),
               ( int )( DISPLAY_OFFSET_X + display_width + DISP_FRAME - 1 ),
               ( int )( DISPLAY_OFFSET_Y - DISP_FRAME ) );
    XDrawLine( dpy, keypad->pixmap, gc,
               ( int )( DISPLAY_OFFSET_X + display_width + DISP_FRAME - 1 ),
               ( int )( DISPLAY_OFFSET_Y - DISP_FRAME ),
               ( int )( DISPLAY_OFFSET_X + display_width + DISP_FRAME - 1 ),
               ( int )( DISPLAY_OFFSET_Y - DISP_FRAME + 3 ) );
    XDrawPoint( dpy, keypad->pixmap, gc,
                ( int )( DISPLAY_OFFSET_X + display_width + DISP_FRAME - 2 ),
                ( int )( DISPLAY_OFFSET_Y - DISP_FRAME + 1 ) );

    XDrawLine(
        dpy, keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X - DISP_FRAME ),
        ( int )( DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 4 ),
        ( int )( DISPLAY_OFFSET_X - DISP_FRAME ),
        ( int )( DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 1 ) );
    XDrawLine(
        dpy, keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X - DISP_FRAME ),
        ( int )( DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 1 ),
        ( int )( DISPLAY_OFFSET_X - DISP_FRAME + 3 ),
        ( int )( DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 1 ) );
    XDrawPoint(
        dpy, keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X - DISP_FRAME + 1 ),
        ( int )( DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 2 ) );

    XDrawLine(
        dpy, keypad->pixmap, gc,
        ( int )( DISPLAY_OFFSET_X + display_width + DISP_FRAME - 1 ),
        ( int )( DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 4 ),
        ( int )( DISPLAY_OFFSET_X + display_width + DISP_FRAME - 1 ),
        ( int )( DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 1 ) );
    XDrawLine(
        dpy, keypad->pixmap, gc,
        ( int )( DISPLAY_OFFSET_X + display_width + DISP_FRAME - 4 ),
        ( int )( DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 1 ),
        ( int )( DISPLAY_OFFSET_X + display_width + DISP_FRAME - 1 ),
        ( int )( DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 1 ) );
    XDrawPoint(
        dpy, keypad->pixmap, gc,
        ( int )( DISPLAY_OFFSET_X + display_width + DISP_FRAME - 2 ),
        ( int )( DISPLAY_OFFSET_Y + display_height + 2 * DISP_FRAME - 2 ) );

    /*
     * simulate rounded lcd corners
     */
    XSetForeground( dpy, gc, COLOR( LCD ) );

    XDrawLine( dpy, keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X - 1 ),
               ( int )( DISPLAY_OFFSET_Y + 1 ), ( int )( DISPLAY_OFFSET_X - 1 ),
               ( int )( DISPLAY_OFFSET_Y + display_height - 2 ) );
    XDrawLine( dpy, keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X + 1 ),
               ( int )( DISPLAY_OFFSET_Y - 1 ),
               ( int )( DISPLAY_OFFSET_X + display_width - 2 ),
               ( int )( DISPLAY_OFFSET_Y - 1 ) );
    XDrawLine( dpy, keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X + 1 ),
               ( int )( DISPLAY_OFFSET_Y + display_height ),
               ( int )( DISPLAY_OFFSET_X + display_width - 2 ),
               ( int )( DISPLAY_OFFSET_Y + display_height ) );
    XDrawLine( dpy, keypad->pixmap, gc,
               ( int )( DISPLAY_OFFSET_X + display_width ),
               ( int )( DISPLAY_OFFSET_Y + 1 ),
               ( int )( DISPLAY_OFFSET_X + display_width ),
               ( int )( DISPLAY_OFFSET_Y + display_height - 2 ) );
}

void DrawMore( unsigned int w, unsigned int h, unsigned int offset_y,
               unsigned int offset_x, keypad_t* keypad ) {
    Pixmap pix;
    int cut = 0;
    int x, y;

    // int display_height = DISPLAY_HEIGHT;
    int display_width = DISPLAY_WIDTH;
    /*
     * lower the whole thing
     */
    XSetForeground( dpy, gc, COLOR( PAD_TOP ) );

    /* bottom lines */
    int keypad_width = keypad->width;
    XDrawLine( dpy, keypad->pixmap, gc, 1, ( int )( keypad->height - 1 ),
               ( int )( keypad_width - 1 ), ( int )( keypad->height - 1 ) );
    XDrawLine( dpy, keypad->pixmap, gc, 2, ( int )( keypad->height - 2 ),
               ( int )( keypad_width - 2 ), ( int )( keypad->height - 2 ) );

    /* right lines */
    XDrawLine( dpy, keypad->pixmap, gc, ( int )( keypad->width - 1 ),
               ( int )( keypad->height - 1 ), ( int )( keypad->width - 1 ),
               cut );
    XDrawLine( dpy, keypad->pixmap, gc, ( int )( keypad->width - 2 ),
               ( int )( keypad->height - 2 ), ( int )( keypad->width - 2 ),
               cut );

    XSetForeground( dpy, gc, COLOR( DISP_PAD_TOP ) );

    /* right lines */
    XDrawLine( dpy, keypad->pixmap, gc, ( int )( keypad->width - 1 ), cut - 1,
               ( int )( keypad->width - 1 ), 1 );
    XDrawLine( dpy, keypad->pixmap, gc, ( int )( keypad->width - 2 ), cut - 1,
               ( int )( keypad->width - 2 ), 2 );

    XSetForeground( dpy, gc, COLOR( DISP_PAD_BOT ) );

    /* top lines */
    XDrawLine( dpy, keypad->pixmap, gc, 0, 0, ( int )( keypad->width - 2 ), 0 );
    XDrawLine( dpy, keypad->pixmap, gc, 1, 1, ( int )( keypad->width - 3 ), 1 );

    /* left lines */
    XDrawLine( dpy, keypad->pixmap, gc, 0, cut - 1, 0, 0 );
    XDrawLine( dpy, keypad->pixmap, gc, 1, cut - 1, 1, 1 );

    XSetForeground( dpy, gc, COLOR( PAD_BOT ) );

    /* left lines */
    XDrawLine( dpy, keypad->pixmap, gc, 0, ( int )( keypad->height - 2 ), 0,
               cut );
    XDrawLine( dpy, keypad->pixmap, gc, 1, ( int )( keypad->height - 3 ), 1,
               cut );

    /*
     * lower the menu buttons
     */
    XSetForeground( dpy, gc, COLOR( PAD_TOP ) );

    /* bottom lines */
    XDrawLine( dpy, keypad->pixmap, gc, 3, ( int )( keypad->height - 3 ),
               ( int )( keypad->width - 3 ), ( int )( keypad->height - 3 ) );
    XDrawLine( dpy, keypad->pixmap, gc, 4, ( int )( keypad->height - 4 ),
               ( int )( keypad->width - 4 ), ( int )( keypad->height - 4 ) );

    /* right lines */
    XDrawLine( dpy, keypad->pixmap, gc, ( int )( keypad->width - 3 ),
               ( int )( keypad->height - 3 ), ( int )( keypad->width - 3 ),
               cut );
    XDrawLine( dpy, keypad->pixmap, gc, ( int )( keypad->width - 4 ),
               ( int )( keypad->height - 4 ), ( int )( keypad->width - 4 ),
               cut );

    XSetForeground( dpy, gc, COLOR( DISP_PAD_TOP ) );

    /* right lines */
    XDrawLine( dpy, keypad->pixmap, gc, ( int )( keypad->width - 3 ), cut - 1,
               ( int )( keypad->width - 3 ), offset_y - 24 );
    XDrawLine( dpy, keypad->pixmap, gc, ( int )( keypad->width - 4 ), cut - 1,
               ( int )( keypad->width - 4 ), offset_y - 23 );

    XSetForeground( dpy, gc, COLOR( DISP_PAD_BOT ) );

    /* top lines */
    XDrawLine( dpy, keypad->pixmap, gc, 2, offset_y - 25,
               ( int )( keypad->width - 4 ), offset_y - 25 );
    XDrawLine( dpy, keypad->pixmap, gc, 3, offset_y - 24,
               ( int )( keypad->width - 5 ), offset_y - 24 );

    /* left lines */
    XDrawLine( dpy, keypad->pixmap, gc, 2, cut - 1, 2, offset_y - 24 );
    XDrawLine( dpy, keypad->pixmap, gc, 3, cut - 1, 3, offset_y - 23 );

    XSetForeground( dpy, gc, COLOR( PAD_BOT ) );

    /* left lines */
    XDrawLine( dpy, keypad->pixmap, gc, 2, ( int )( keypad->height - 4 ), 2,
               cut );
    XDrawLine( dpy, keypad->pixmap, gc, 3, ( int )( keypad->height - 5 ), 3,
               cut );

    /*
     * lower the keyboard
     */
    XSetForeground( dpy, gc, COLOR( PAD_TOP ) );

    /* bottom lines */
    XDrawLine( dpy, keypad->pixmap, gc, 5, ( int )( keypad->height - 5 ),
               ( int )( keypad->width - 3 ), ( int )( keypad->height - 5 ) );
    XDrawLine( dpy, keypad->pixmap, gc, 6, ( int )( keypad->height - 6 ),
               ( int )( keypad->width - 4 ), ( int )( keypad->height - 6 ) );

    /* right lines */
    XDrawLine( dpy, keypad->pixmap, gc, ( int )( keypad->width - 5 ),
               ( int )( keypad->height - 5 ), ( int )( keypad->width - 5 ),
               cut + 1 );
    XDrawLine( dpy, keypad->pixmap, gc, ( int )( keypad->width - 6 ),
               ( int )( keypad->height - 6 ), ( int )( keypad->width - 6 ),
               cut + 2 );

    XSetForeground( dpy, gc, COLOR( DISP_PAD_BOT ) );

    /* top lines */
    XDrawLine( dpy, keypad->pixmap, gc, 4, cut, ( int )( keypad->width - 6 ),
               cut );
    XDrawLine( dpy, keypad->pixmap, gc, 5, cut + 1,
               ( int )( keypad->width - 7 ), cut + 1 );

    XSetForeground( dpy, gc, COLOR( PAD_BOT ) );

    /* left lines */
    XDrawLine( dpy, keypad->pixmap, gc, 4, ( int )( keypad->height - 6 ), 4,
               cut + 1 );
    XDrawLine( dpy, keypad->pixmap, gc, 5, ( int )( keypad->height - 7 ), 5,
               cut + 2 );

    /*
     * round off the bottom edge
     */
    XSetForeground( dpy, gc, COLOR( PAD_TOP ) );

    XDrawLine( dpy, keypad->pixmap, gc, ( int )( keypad->width - 7 ),
               ( int )( keypad->height - 7 ), ( int )( keypad->width - 7 ),
               ( int )( keypad->height - 14 ) );
    XDrawLine( dpy, keypad->pixmap, gc, ( int )( keypad->width - 8 ),
               ( int )( keypad->height - 8 ), ( int )( keypad->width - 8 ),
               ( int )( keypad->height - 11 ) );
    XDrawLine( dpy, keypad->pixmap, gc, ( int )( keypad->width - 7 ),
               ( int )( keypad->height - 7 ), ( int )( keypad->width - 14 ),
               ( int )( keypad->height - 7 ) );
    XDrawLine( dpy, keypad->pixmap, gc, ( int )( keypad->width - 7 ),
               ( int )( keypad->height - 8 ), ( int )( keypad->width - 11 ),
               ( int )( keypad->height - 8 ) );
    XDrawPoint( dpy, keypad->pixmap, gc, ( int )( keypad->width - 9 ),
                ( int )( keypad->height - 9 ) );

    XDrawLine( dpy, keypad->pixmap, gc, 7, ( int )( keypad->height - 7 ), 13,
               ( int )( keypad->height - 7 ) );
    XDrawLine( dpy, keypad->pixmap, gc, 8, ( int )( keypad->height - 8 ), 10,
               ( int )( keypad->height - 8 ) );
    XSetForeground( dpy, gc, COLOR( PAD_BOT ) );
    XDrawLine( dpy, keypad->pixmap, gc, 6, ( int )( keypad->height - 8 ), 6,
               ( int )( keypad->height - 14 ) );
    XDrawLine( dpy, keypad->pixmap, gc, 7, ( int )( keypad->height - 9 ), 7,
               ( int )( keypad->height - 11 ) );

    /*
     * insert the HP Logo
     */

    XSetBackground( dpy, gc, COLOR( LOGO_BACK ) );
    XSetForeground( dpy, gc, COLOR( LOGO ) );

    pix = XCreateBitmapFromData( dpy, keypad->pixmap, ( char* )hp_bitmap,
                                 hp_width, hp_height );

    x = opt_gx ? DISPLAY_OFFSET_X - 6 : DISPLAY_OFFSET_X;

    XCopyPlane( dpy, pix, keypad->pixmap, gc, 0, 0, hp_width, hp_height, x, 10,
                1 );

    XFreePixmap( dpy, pix );

    if ( !opt_gx ) {
        XSetForeground( dpy, gc, COLOR( FRAME ) );

        XDrawLine( dpy, keypad->pixmap, gc, ( int )DISPLAY_OFFSET_X, 9,
                   ( int )( DISPLAY_OFFSET_X + hp_width - 1 ), 9 );
        XDrawLine( dpy, keypad->pixmap, gc, ( int )( DISPLAY_OFFSET_X - 1 ), 10,
                   ( int )( DISPLAY_OFFSET_X - 1 ), 10 + hp_height - 1 );
        XDrawLine( dpy, keypad->pixmap, gc, ( int )DISPLAY_OFFSET_X,
                   10 + hp_height, ( int )( DISPLAY_OFFSET_X + hp_width - 1 ),
                   10 + hp_height );
        XDrawLine( dpy, keypad->pixmap, gc,
                   ( int )( DISPLAY_OFFSET_X + hp_width ), 10,
                   ( int )( DISPLAY_OFFSET_X + hp_width ), 10 + hp_height - 1 );
    }

    /*
     * write the name of it
     */
    XSetBackground( dpy, gc, COLOR( DISP_PAD ) );
    XSetForeground( dpy, gc, COLOR( LABEL ) );

    if ( opt_gx ) {
        x = DISPLAY_OFFSET_X + display_width - gx_128K_ram_width +
            gx_128K_ram_x_hot + 2;
        y = 10 + gx_128K_ram_y_hot;
        pix = XCreateBitmapFromData( dpy, keypad->pixmap,
                                     ( char* )gx_128K_ram_bitmap,
                                     gx_128K_ram_width, gx_128K_ram_height );
        XCopyPlane( dpy, pix, keypad->pixmap, gc, 0, 0, gx_128K_ram_width,
                    gx_128K_ram_height, x, y, 1 );
        XFreePixmap( dpy, pix );

        XSetForeground( dpy, gc, COLOR( LOGO ) );
        x = DISPLAY_OFFSET_X + hp_width;
        y = hp_height + 8 - hp48gx_height;
        pix =
            XCreateBitmapFromData( dpy, keypad->pixmap, ( char* )hp48gx_bitmap,
                                   hp48gx_width, hp48gx_height );
        XCopyPlane( dpy, pix, keypad->pixmap, gc, 0, 0, hp48gx_width,
                    hp48gx_height, x, y, 1 );
        XFreePixmap( dpy, pix );

        XSetFillStyle( dpy, gc, FillStippled );
        x = DISPLAY_OFFSET_X + DISPLAY_WIDTH - gx_128K_ram_width +
            gx_silver_x_hot + 2;
        y = 10 + gx_silver_y_hot;
        pix = XCreateBitmapFromData( dpy, keypad->pixmap,
                                     ( char* )gx_silver_bitmap, gx_silver_width,
                                     gx_silver_height );
        XSetStipple( dpy, gc, pix );
        XSetTSOrigin( dpy, gc, x, y );
        XFillRectangle( dpy, keypad->pixmap, gc, x, y, gx_silver_width,
                        gx_silver_height );
        XFreePixmap( dpy, pix );

        XSetForeground( dpy, gc, COLOR( RIGHT ) );
        x = DISPLAY_OFFSET_X + display_width - gx_128K_ram_width +
            gx_green_x_hot + 2;
        y = 10 + gx_green_y_hot;
        pix = XCreateBitmapFromData( dpy, keypad->pixmap,
                                     ( char* )gx_green_bitmap, gx_green_width,
                                     gx_green_height );
        XSetStipple( dpy, gc, pix );
        XSetTSOrigin( dpy, gc, x, y );
        XFillRectangle( dpy, keypad->pixmap, gc, x, y, gx_green_width,
                        gx_green_height );
        XFreePixmap( dpy, pix );

        XSetTSOrigin( dpy, gc, 0, 0 );
        XSetFillStyle( dpy, gc, FillSolid );
    } else {
        x = DISPLAY_OFFSET_X;
        y = TOP_SKIP - DISP_FRAME - hp48sx_height - 3;

        pix =
            XCreateBitmapFromData( dpy, keypad->pixmap, ( char* )hp48sx_bitmap,
                                   hp48sx_width, hp48sx_height );

        XCopyPlane( dpy, pix, keypad->pixmap, gc, 0, 0, hp48sx_width,
                    hp48sx_height, x, y, 1 );

        XFreePixmap( dpy, pix );

        x = DISPLAY_OFFSET_X + display_width - 1 - science_width;
        y = TOP_SKIP - DISP_FRAME - science_height - 4;

        pix =
            XCreateBitmapFromData( dpy, keypad->pixmap, ( char* )science_bitmap,
                                   science_width, science_height );

        XCopyPlane( dpy, pix, keypad->pixmap, gc, 0, 0, science_width,
                    science_height, x, y, 1 );
    }
}

void DrawKeypad( keypad_t* keypad ) {
    XCopyArea( dpy, keypad->pixmap, mainW, gc, 0, 0, keypad->width,
               keypad->height, 0, 0 );
}

void CreateIcon( void ) {
    XSetWindowAttributes xswa;
    XWindowAttributes xwa;
    Pixmap tmp_pix;
    int p;

    XGetWindowAttributes( dpy, iconW, &xwa );
    xswa.event_mask = xwa.your_event_mask | ExposureMask;
    xswa.backing_store = Always;
    XChangeWindowAttributes( dpy, iconW, CWEventMask | CWBackingStore, &xswa );

    icon_pix =
        XCreatePixmap( dpy, iconW, hp48_icon_width, hp48_icon_height, depth );

    /*
     * draw the icon pixmap
     */
    if ( icon_color_mode == COLOR_MODE_MONO ) {
        tmp_pix = XCreateBitmapFromData(
            dpy, iconW, ( char* )icon_maps[ ICON_MAP ].bits,
            icon_maps[ ICON_MAP ].w, icon_maps[ ICON_MAP ].h );
        XSetForeground( dpy, gc, COLOR( BLACK ) );
        XSetBackground( dpy, gc, COLOR( WHITE ) );
        XCopyPlane( dpy, tmp_pix, icon_pix, gc, 0, 0, icon_maps[ ICON_MAP ].w,
                    icon_maps[ ICON_MAP ].h, 0, 0, 1 );
        XFreePixmap( dpy, tmp_pix );
    } else {
        XSetFillStyle( dpy, gc, FillStippled );
        for ( p = FIRST_MAP; p <= LAST_MAP; p++ ) {
            tmp_pix =
                XCreateBitmapFromData( dpy, iconW, ( char* )icon_maps[ p ].bits,
                                       icon_maps[ p ].w, icon_maps[ p ].h );
            XSetStipple( dpy, gc, tmp_pix );
            XSetForeground( dpy, gc, COLOR( icon_maps[ p ].c ) );
            XFillRectangle( dpy, icon_pix, gc, 0, 0, icon_maps[ p ].w,
                            icon_maps[ p ].h );
            XFreePixmap( dpy, tmp_pix );
        }
        XSetFillStyle( dpy, gc, FillSolid );

        /*
         * draw frame around icon
         */
        XSetForeground( dpy, gc, COLOR( BLACK ) );
        XDrawRectangle( dpy, icon_pix, gc, 0, 0, icon_maps[ ICON_MAP ].w - 1,
                        icon_maps[ ICON_MAP ].h - 1 );
    }

    /*
     * draw the display
     */
    XSetFillStyle( dpy, gc, FillStippled );
    icon_disp_pix = XCreateBitmapFromData(
        dpy, iconW, ( char* )icon_maps[ DISP_MAP ].bits,
        icon_maps[ DISP_MAP ].w, icon_maps[ DISP_MAP ].h );
    XSetStipple( dpy, gc, icon_disp_pix );
    if ( icon_color_mode == COLOR_MODE_MONO )
        XSetForeground( dpy, gc, COLOR( WHITE ) );
    else
        XSetForeground( dpy, gc, COLOR( icon_maps[ DISP_MAP ].c ) );
    XFillRectangle( dpy, icon_pix, gc, 0, 0, icon_maps[ DISP_MAP ].w,
                    icon_maps[ DISP_MAP ].h );

    /*
     * draw the 'x48' string
     */
    icon_text_pix =
        XCreateBitmapFromData( dpy, iconW, ( char* )icon_maps[ ON_MAP ].bits,
                               icon_maps[ ON_MAP ].w, icon_maps[ ON_MAP ].h );
    XSetStipple( dpy, gc, icon_text_pix );
    if ( icon_color_mode == COLOR_MODE_MONO )
        XSetForeground( dpy, gc, COLOR( BLACK ) );
    else
        XSetForeground( dpy, gc, COLOR( icon_maps[ ON_MAP ].c ) );
    XFillRectangle( dpy, icon_pix, gc, 0, 0, icon_maps[ ON_MAP ].w,
                    icon_maps[ ON_MAP ].h );
    XSetFillStyle( dpy, gc, FillSolid );
}

void refresh_icon( void ) {
    int icon_state;

    icon_state =
        ( ( display.on && !( ( ANN_IO & display.annunc ) == ANN_IO ) ) ||
          ( display.on && !( ( ANN_ALPHA & display.annunc ) == ANN_ALPHA ) ) );
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
        XFillRectangle( dpy, icon_pix, gc, 0, 0, icon_maps[ ON_MAP ].w,
                        icon_maps[ ON_MAP ].h );
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
        XFillRectangle( dpy, icon_pix, gc, 0, 0, icon_maps[ DISP_MAP ].w,
                        icon_maps[ DISP_MAP ].h );
    }
    XSetFillStyle( dpy, gc, FillSolid );
    if ( iconW ) {
        XCopyArea( dpy, icon_pix, iconW, gc, 0, 0, hp48_icon_width,
                   hp48_icon_height, 0, 0 );
    }
}

void DrawIcon( void ) {
    XCopyArea( dpy, icon_pix, iconW, gc, 0, 0, hp48_icon_width,
               hp48_icon_height, 0, 0 );
}

int handle_xerror( Display* the_dpy, XErrorEvent* eev ) {
    xerror_flag = 1;

    return 0;
}

void CreateDispWindow( void ) {
    XSetWindowAttributes xswa;
    XGCValues val;
    unsigned long gc_mask;
    static XRectangle rect;

    /*
     * create the display subwindow
     */
    disp.w = DISPLAY_WIDTH;
    disp.h = DISPLAY_HEIGHT;

    disp.win = XCreateSimpleWindow( dpy, mainW, ( int )DISPLAY_OFFSET_X,
                                    ( int )DISPLAY_OFFSET_Y, disp.w, disp.h, 0,
                                    COLOR( BLACK ), COLOR( LCD ) );

    disp.mapped = 1;

    xswa.event_mask = ExposureMask | StructureNotifyMask;
    xswa.backing_store = Always;

    XChangeWindowAttributes( dpy, disp.win, CWEventMask | CWBackingStore,
                             &xswa );

    /*
     * set up the gc
     */
    val.foreground = COLOR( LCD );
    val.background = COLOR( LCD );
    val.function = GXcopy;
    gc_mask = GCForeground | GCBackground | GCFunction;
    disp.gc = XCreateGC( dpy, mainW, gc_mask, &val );

    XSetForeground( dpy, disp.gc, COLOR( PIXEL ) );

    disp.display_update = UPDATE_DISP | UPDATE_MENU;

    xerror_flag = 0;
    XSetErrorHandler( handle_xerror );
    XFlush( dpy );

    disp.disp_image = NULL;
    disp.menu_image = NULL;
    if ( shm_flag ) {

        /*
         * create XShmImage for DISP
         */
        disp.disp_image = XShmCreateImage( dpy, None, 1, XYBitmap, NULL,
                                           &disp.disp_info, 262, 128 );
        if ( disp.disp_image == NULL ) {
            shm_flag = 0;
            if ( verbose )
                fprintf( stderr,
                         "XShm error in CreateImage(DISP), disabling.\n" );
            goto shm_error;
        }

        /*
         * get ID of shared memory block for DISP
         */
        disp.disp_info.shmid = shmget(
            IPC_PRIVATE,
            ( disp.disp_image->bytes_per_line * disp.disp_image->height ),
            IPC_CREAT | 0777 );
        if ( disp.disp_info.shmid < 0 ) {
            XDestroyImage( disp.disp_image );
            disp.disp_image = NULL;
            shm_flag = 0;
            if ( verbose )
                fprintf( stderr, "XShm error in shmget(DISP), disabling.\n" );
            goto shm_error;
        }

        /*
         * get address of shared memory block for DISP
         */
        disp.disp_info.shmaddr = ( char* )shmat( disp.disp_info.shmid, 0, 0 );
        if ( disp.disp_info.shmaddr == ( ( char* )-1 ) ) {
            XDestroyImage( disp.disp_image );
            disp.disp_image = NULL;
            shm_flag = 0;
            if ( verbose )
                fprintf( stderr, "XShm error in shmat(DISP), disabling.\n" );
            goto shm_error;
        }
        disp.disp_image->data = disp.disp_info.shmaddr;
        disp.disp_info.readOnly = False;
        XShmAttach( dpy, &disp.disp_info );

        /*
         * create XShmImage for MENU
         */
        disp.menu_image = XShmCreateImage( dpy, None, 1, XYBitmap, NULL,
                                           &disp.menu_info, 262, 128 );
        if ( disp.menu_image == NULL ) {
            XDestroyImage( disp.disp_image );
            disp.disp_image = NULL;
            shm_flag = 0;
            if ( verbose )
                fprintf( stderr,
                         "XShm error in CreateImage(MENU), disabling.\n" );
            goto shm_error;
        }

        /*
         * get ID of shared memory block for MENU
         */
        disp.menu_info.shmid = shmget(
            IPC_PRIVATE,
            ( disp.menu_image->bytes_per_line * disp.menu_image->height ),
            IPC_CREAT | 0777 );
        if ( disp.menu_info.shmid < 0 ) {
            XDestroyImage( disp.disp_image );
            disp.disp_image = NULL;
            XDestroyImage( disp.menu_image );
            disp.menu_image = NULL;
            shm_flag = 0;
            if ( verbose )
                fprintf( stderr, "XShm error in shmget(MENU), disabling.\n" );
            goto shm_error;
        }

        /*
         * get address of shared memory block for MENU
         */
        disp.menu_info.shmaddr = ( char* )shmat( disp.menu_info.shmid, 0, 0 );
        if ( disp.menu_info.shmaddr == ( ( char* )-1 ) ) {
            XDestroyImage( disp.disp_image );
            disp.disp_image = NULL;
            XDestroyImage( disp.menu_image );
            disp.menu_image = NULL;
            shm_flag = 0;
            if ( verbose )
                fprintf( stderr, "XShm error in shmat(MENU), disabling.\n" );
            goto shm_error;
        }
        disp.menu_image->data = disp.menu_info.shmaddr;
        disp.menu_info.readOnly = False;
        XShmAttach( dpy, &disp.menu_info );

        XFlush( dpy );
        XSync( dpy, 0 );
        sleep( 1 );

        if ( xerror_flag ) {
            XDestroyImage( disp.disp_image );
            disp.disp_image = NULL;
            XDestroyImage( disp.menu_image );
            disp.menu_image = NULL;
            shm_flag = 0;
            if ( verbose )
                fprintf( stderr, "XShm error in shmget(MENU), disabling.\n" );
            goto shm_error;
        } else {
            shmctl( disp.disp_info.shmid, IPC_RMID, 0 );
            shmctl( disp.menu_info.shmid, IPC_RMID, 0 );
        }

        memset( disp.disp_image->data, 0,
                ( size_t )( disp.disp_image->bytes_per_line *
                            disp.disp_image->height ) );
        memset( disp.menu_image->data, 0,
                ( size_t )( disp.menu_image->bytes_per_line *
                            disp.menu_image->height ) );

        if ( verbose )
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
        rect.height = disp.h;
        XSetClipRectangles( dpy, disp.gc, 0, 0, &rect, 1, Unsorted );
    }
}

int CreateWindows( int argc, char** argv ) {
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
    char *name, *user_geom, def_geom[ 40 ];
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

    if ( netbook ) {
        int i;
        for ( i = 0; i < 6; i++ ) {
            buttons[ i ].x -= 3;
            buttons[ i ].y += 300;
        }
        for ( ; i <= LAST_BUTTON; i++ ) {
            buttons[ i ].x += 317;
            buttons[ i ].y -= 3;
        }
    }

    class = InputOutput;
    visual = get_visual_resource( dpy, "visual", "Visual", &depth );
    if ( visual != DefaultVisual( dpy, screen ) ) {
        if ( visual->class == DirectColor )
            cmap = XCreateColormap( dpy, RootWindow( dpy, screen ), visual,
                                    AllocAll );
        else
            cmap = XCreateColormap( dpy, RootWindow( dpy, screen ), visual,
                                    AllocNone );
    } else
        cmap = DefaultColormap( dpy, screen );

    direct_color = 0;
    switch ( visual->class ) {
        case DirectColor:
            direct_color = 1;
        case GrayScale:
        case PseudoColor:
            dynamic_color = 1;
            break;
        case StaticGray:
        case StaticColor:
        case TrueColor:
        default:
            dynamic_color = 0;
            break;
    }

    if ( ( visual->class == StaticGray ) || ( visual->class == GrayScale ) )
        color_mode = COLOR_MODE_GRAY;
    else
        color_mode = COLOR_MODE_COLOR;
    if ( get_boolean_resource( "gray", "Gray" ) )
        color_mode = COLOR_MODE_GRAY;

    if ( get_boolean_resource( "mono", "Mono" ) )
        color_mode = COLOR_MODE_MONO;
    if ( depth == 1 )
        color_mode = COLOR_MODE_MONO;

    icon_color_mode = color_mode;
    if ( get_boolean_resource( "monoIcon", "Mono" ) )
        icon_color_mode = COLOR_MODE_MONO;

    clh.res_name = res_name;
    clh.res_class = res_class;
    if ( !XStringListToTextProperty( &progname, 1, &iname ) )
        return -1;

    if ( ( name = get_string_resource( "title", "Title" ) ) == ( char* )0 ) {
        name = ( char* )malloc( 128 );
        if ( name == ( char* )0 )
            fatal_exit( "malloc failed.\n", "" );

        sprintf( name, "%s-%d.%d.%d", progname, saturn.version[ 0 ],
                 saturn.version[ 1 ], saturn.version[ 2 ] );
    }

    if ( !XStringListToTextProperty( &name, 1, &wname ) )
        return -1;

    /*
     * Set some Window Attributes
     */
    xswa.colormap = cmap;
    mask = CWColormap;

    /*
     * create the window
     */
    width = KEYBOARD_WIDTH + 2 * SIDE_SKIP;
    if ( netbook ) {
        height = KEYBOARD_HEIGHT;
    } else {
        height = DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + DISP_KBD_SKIP +
                 KEYBOARD_HEIGHT + BOTTOM_SKIP;
    }

    mainW = XCreateWindow( dpy, RootWindow( dpy, screen ), 0, 0, width, height,
                           0, ( int )depth, class, visual, mask, &xswa );

    if ( mainW == 0 )
        return -1;

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
    user_geom = get_string_resource( "geometry", "Geometry" );

    info = XWMGeometry( dpy, screen, user_geom, def_geom, 0, &hint, &x, &y, &w,
                        &h, &hint.win_gravity );

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
    if ( get_boolean_resource( "iconic", "Iconic" ) )
        wmh.initial_state = IconicState;
    else
        wmh.initial_state = NormalState;
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
    xswa.event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask |
                      ButtonReleaseMask | ExposureMask | KeymapStateMask |
                      EnterWindowMask | StructureNotifyMask | FocusChangeMask;
    mask = CWBackPixel | CWBorderPixel | CWBackingStore | CWEventMask |
           CWBitGravity | CWWinGravity;
    XChangeWindowAttributes( dpy, mainW, mask, &xswa );
    XMoveWindow( dpy, mainW, hint.x, hint.y );

    /*
     * create the icon
     */
    xswa.colormap = cmap;
    mask = CWColormap;
    iconW = XCreateWindow( dpy, RootWindow( dpy, screen ), 0, 0,
                           hp48_icon_width, hp48_icon_height, 0, ( int )depth,
                           class, visual, mask, &xswa );

    if ( iconW == 0 )
        return -1;

    wmh.icon_window = iconW;
    wmh.window_group = mainW;
    wmh.flags |= ( IconWindowHint | WindowGroupHint );

    /*
     * set icon position if requested
     */
    ih.x = ih.y = 0;
    ih.min_width = ih.max_width = ih.base_width = ih.width = hp48_icon_width;
    ih.min_height = ih.max_height = ih.base_height = ih.height =
        hp48_icon_height;
    ih.win_gravity = NorthWestGravity;
    ih.flags = PSize | PMinSize | PMaxSize | PBaseSize | PWinGravity;

    user_geom = get_string_resource( "iconGeom", "IconGeom" );
    info = XWMGeometry( dpy, screen, user_geom, ( char* )0, 0, &ih, &x, &y, &w,
                        &h, &ih.win_gravity );

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
    mask = CWBackPixel | CWBorderPixel | CWBackingStore | CWBitGravity |
           CWWinGravity;
    XChangeWindowAttributes( dpy, iconW, mask, &xswa );

    /*
     * tell window manager all the stuff we dug out
     */
    XSetWMProperties( dpy, mainW, &wname, &iname, argv, argc, &hint, &wmh,
                      &clh );

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
    XChangeProperty( dpy, mainW, ol_decor_del, atom_type, 32, PropModeReplace,
                     ( unsigned char* )&ol_decor_icon_name, 1 );

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
    CreateIcon();

    /*
     * create the display
     */
    CreateDispWindow();

    /*
     * create the keypad
     */
    /*
     * draw the nice labels around the buttons
     */
    keypad.width = width;
    keypad.height = height;

    keypad.pixmap = XCreatePixmap( dpy, mainW, width, height, depth );

    if ( netbook ) {
        int cut =
            buttons[ BUTTON_MTH ].y - ( small_ascent + small_descent + 6 + 4 );
        CreateBackground( width / 2, height, width, height, &keypad );
        DrawMore( width, height, KEYBOARD_OFFSET_Y, KEYBOARD_OFFSET_X,
                  &keypad );
        CreateBezel( width / 2, height, KEYBOARD_OFFSET_Y, KEYBOARD_OFFSET_X,
                     &keypad );
        CreateKeypad( width, height, -cut, KEYBOARD_OFFSET_X, &keypad );
    } else {
        int cut = buttons[ BUTTON_MTH ].y + KEYBOARD_OFFSET_Y - 19;
        CreateBackground( width, cut, width, height, &keypad );
        DrawMore( width, height, KEYBOARD_OFFSET_Y, KEYBOARD_OFFSET_X,
                  &keypad );
        CreateBezel( width, cut, KEYBOARD_OFFSET_Y, KEYBOARD_OFFSET_X,
                     &keypad );
        CreateKeypad( width, height, KEYBOARD_OFFSET_Y, KEYBOARD_OFFSET_X,
                      &keypad );
    }

    /*
     * map the window
     */
    XMapWindow( dpy, mainW );
    XMapSubwindows( dpy, mainW );

    DrawKeypad( &keypad );
    DrawButtons();
    DrawIcon();

    if ( shm_flag ) {
        XSetForeground( dpy, disp.gc, COLOR( PIXEL ) );
        XFillRectangle( dpy, disp.win, disp.gc, 5, 20, 262, 128 );
    }

    return 0;
}

int key_event( int b, XEvent* xev ) {
    int code;
    int i, r, c;

    code = buttons[ b ].code;
    if ( xev->type == KeyPress ) {
        buttons[ b ].pressed = 1;
        DrawButton( b );
        if ( code == 0x8000 ) {
            for ( i = 0; i < 9; i++ )
                saturn.keybuf.rows[ i ] |= 0x8000;
            do_kbd_int();
        } else {
            r = code >> 4;
            c = 1 << ( code & 0xf );
            if ( ( saturn.keybuf.rows[ r ] & c ) == 0 ) {
                if ( saturn.kbd_ien ) {
                    do_kbd_int();
                }
                saturn.keybuf.rows[ r ] |= c;
            }
        }
    } else {
        if ( code == 0x8000 ) {
            for ( i = 0; i < 9; i++ )
                saturn.keybuf.rows[ i ] &= ~0x8000;
            memset( &saturn.keybuf, 0, sizeof( saturn.keybuf ) );
        } else {
            r = code >> 4;
            c = 1 << ( code & 0xf );
            saturn.keybuf.rows[ r ] &= ~c;
        }
        buttons[ b ].pressed = 0;
        DrawButton( b );
    }

    return 0;
}

void refresh_display( void ) {
    if ( !shm_flag )
        return;

    if ( disp.display_update & UPDATE_DISP ) {
        XShmPutImage( dpy, disp.win, disp.gc, disp.disp_image, disp.offset, 0,
                      5, 20, 262, ( unsigned int )( disp.lines + 2 ), 0 );
    }
    if ( ( disp.lines < 126 ) && ( disp.display_update & UPDATE_MENU ) ) {
        XShmPutImage( dpy, disp.win, disp.gc, disp.menu_image, 0, 0, 5,
                      ( int )( disp.lines + 22 ), 262,
                      ( unsigned int )( 126 - disp.lines ), 0 );
    }
    disp.display_update = 0;
}

void DrawDisp( void ) {
    if ( shm_flag ) {
        XShmPutImage( dpy, disp.win, disp.gc, disp.disp_image, disp.offset, 0,
                      5, 20, 262, ( unsigned int )( disp.lines + 2 ), 0 );
        if ( display.lines < 63 ) {
            XShmPutImage( dpy, disp.win, disp.gc, disp.menu_image, 0,
                          disp.lines - 110, 5, 22 + disp.lines, 262,
                          ( unsigned int )( 126 - disp.lines ), 0 );
        }
        disp.display_update = 0;
    } else {
        redraw_display();
    }

    redraw_annunc();
}

void get_geometry_string( Window win, char* s, int allow_off_screen ) {
    XWindowAttributes xwa;
    Window root, parent, window;
    Window* children = ( Window* )0;
    unsigned int rw, rh, rbw, rd;
    unsigned int w, h, bw, d;
    unsigned int nc;
    int rx, ry, x, y;
    int x_pos, x_neg, x_s, y_pos, y_neg, y_s;

    window = win;
    nc = 0;
    XQueryTree( dpy, window, &root, &parent, &children, &nc );
    XFree( ( char* )children );
    while ( parent != root ) {
        window = parent;
        nc = 0;
        XQueryTree( dpy, window, &root, &parent, &children, &nc );
        XFree( ( char* )children );
    }
    XGetGeometry( dpy, window, &root, &x, &y, &w, &h, &bw, &d );
    XGetGeometry( dpy, root, &root, &rx, &ry, &rw, &rh, &rbw, &rd );

    x_s = 1;
    x_pos = x;
    x_neg = rw - ( x + w );
    if ( abs( x_pos ) > abs( x_neg ) ) {
        x = x_neg;
        x_s = -1;
    }
    y_s = 1;
    y_pos = y;
    y_neg = rh - ( y + h );
    if ( abs( y_pos ) > abs( y_neg ) ) {
        y = y_neg;
        y_s = -1;
    }

    if ( !allow_off_screen ) {
        if ( x < 0 )
            x = 0;
        if ( y < 0 )
            y = 0;
    }

    XGetWindowAttributes( dpy, win, &xwa );
    sprintf( s, "%ux%u%s%d%s%d", xwa.width, xwa.height, ( x_s > 0 ) ? "+" : "-",
             x, ( y_s > 0 ) ? "+" : "-", y );
}

void save_options( int argc, char** argv ) {
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

void save_command_line( void ) {
    XWindowAttributes xwa;
    int wm_argc, ac;
    char **wm_argv, geom[ 128 ], icon_geom[ 128 ];

    ac = wm_argc = 0;

    wm_argv = ( char** )malloc( ( saved_argc + 5 ) * sizeof( char* ) );
    if ( wm_argv == ( char** )0 ) {
        if ( verbose )
            fprintf( stderr, "warning: malloc failed in wm_save_yourself.\n" );
        XSetCommand( dpy, mainW, saved_argv, saved_argc );
        return;
    }

    while ( saved_argv[ ac ] ) {
        if ( !strcmp( saved_argv[ ac ], "-geometry" ) ) {
            ac += 2;
            continue;
        }
        if ( !strcmp( saved_argv[ ac ], "-iconGeom" ) ) {
            ac += 2;
            continue;
        }
        if ( !strcmp( saved_argv[ ac ], "-iconic" ) ) {
            ac++;
            continue;
        }
        wm_argv[ wm_argc++ ] = saved_argv[ ac++ ];
    }

    wm_argv[ wm_argc++ ] = "-geometry";
    get_geometry_string( mainW, geom, 1 );
    wm_argv[ wm_argc++ ] = geom;

    wm_argv[ wm_argc++ ] = "-iconGeom";
    get_geometry_string( iconW, icon_geom, 0 );
    wm_argv[ wm_argc++ ] = icon_geom;

    XGetWindowAttributes( dpy, mainW, &xwa );
    if ( xwa.map_state == IsUnmapped ) {
        wm_argv[ wm_argc++ ] = "-iconic";
    }
    wm_argv[ wm_argc ] = ( char* )0;

    XSetCommand( dpy, mainW, wm_argv, wm_argc );
}

int decode_key( XEvent* xev, KeySym sym, char* buf, int buflen ) {
    int wake;

    wake = 0;
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
            key_event( BUTTON_0, xev );
            wake = 1;
            break;
        case XK_KP_1:
        case XK_1:
            key_event( BUTTON_1, xev );
            wake = 1;
            break;
        case XK_KP_2:
        case XK_2:
            key_event( BUTTON_2, xev );
            wake = 1;
            break;
        case XK_KP_3:
        case XK_3:
            key_event( BUTTON_3, xev );
            wake = 1;
            break;
        case XK_KP_4:
        case XK_4:
            key_event( BUTTON_4, xev );
            wake = 1;
            break;
        case XK_KP_5:
        case XK_5:
            key_event( BUTTON_5, xev );
            wake = 1;
            break;
        case XK_KP_6:
        case XK_6:
            key_event( BUTTON_6, xev );
            wake = 1;
            break;
        case XK_KP_7:
        case XK_7:
            key_event( BUTTON_7, xev );
            wake = 1;
            break;
        case XK_KP_8:
        case XK_8:
            key_event( BUTTON_8, xev );
            wake = 1;
            break;
        case XK_KP_9:
        case XK_9:
            key_event( BUTTON_9, xev );
            wake = 1;
            break;
        case XK_KP_Add:
        case XK_plus:
        case XK_equal:
            key_event( BUTTON_PLUS, xev );
            wake = 1;
            break;
        case XK_KP_Subtract:
        case XK_minus:
            key_event( BUTTON_MINUS, xev );
            wake = 1;
            break;
#ifdef XK_F25
        case XK_F25:
#endif
        case XK_KP_Divide:
        case XK_slash:
            key_event( BUTTON_DIV, xev );
            wake = 1;
            break;
#ifdef XK_F26
        case XK_F26:
#endif
        case XK_KP_Multiply:
        case XK_asterisk:
        case XK_comma:
            key_event( BUTTON_MUL, xev );
            wake = 1;
            break;
        case XK_KP_Enter:
        case XK_Return:
            key_event( BUTTON_ENTER, xev );
            wake = 1;
            break;
        case XK_KP_Decimal:
        case XK_KP_Separator:
        case XK_period:
            key_event( BUTTON_PERIOD, xev );
            wake = 1;
            break;
        case XK_space:
            key_event( BUTTON_SPC, xev );
            wake = 1;
            break;
        case XK_Delete:
            key_event( BUTTON_DEL, xev );
            wake = 1;
            break;
        case XK_BackSpace:
            key_event( BUTTON_BS, xev );
            wake = 1;
            break;
        case XK_Escape:
            key_event( BUTTON_ON, xev );
            wake = 1;
            break;
        case XK_Shift_L:
        case XK_Control_R:
            key_event( BUTTON_SHL, xev );
            wake = 1;
            break;
        case XK_Shift_R:
        case XK_Control_L:
            key_event( BUTTON_SHR, xev );
            wake = 1;
            break;
        case XK_Alt_L:
        case XK_Alt_R:
        case XK_Meta_L:
        case XK_Meta_R:
            key_event( BUTTON_ALPHA, xev );
            wake = 1;
            break;
        case XK_a:
        case XK_A:
        case XK_F1:
            key_event( BUTTON_A, xev );
            wake = 1;
            break;
        case XK_b:
        case XK_B:
        case XK_F2:
            key_event( BUTTON_B, xev );
            wake = 1;
            break;
        case XK_c:
        case XK_C:
        case XK_F3:
            key_event( BUTTON_C, xev );
            wake = 1;
            break;
        case XK_d:
        case XK_D:
        case XK_F4:
            key_event( BUTTON_D, xev );
            wake = 1;
            break;
        case XK_e:
        case XK_E:
        case XK_F5:
            key_event( BUTTON_E, xev );
            wake = 1;
            break;
        case XK_f:
        case XK_F:
        case XK_F6:
            key_event( BUTTON_F, xev );
            wake = 1;
            break;
        case XK_g:
        case XK_G:
            key_event( BUTTON_MTH, xev );
            wake = 1;
            break;
        case XK_h:
        case XK_H:
            key_event( BUTTON_PRG, xev );
            wake = 1;
            break;
        case XK_i:
        case XK_I:
            key_event( BUTTON_CST, xev );
            wake = 1;
            break;
        case XK_j:
        case XK_J:
            key_event( BUTTON_VAR, xev );
            wake = 1;
            break;
        case XK_k:
        case XK_K:
        case XK_Up:
            key_event( BUTTON_UP, xev );
            wake = 1;
            break;
        case XK_l:
        case XK_L:
            key_event( BUTTON_NXT, xev );
            wake = 1;
            break;
        case XK_m:
        case XK_M:
            key_event( BUTTON_COLON, xev );
            wake = 1;
            break;
        case XK_n:
        case XK_N:
            key_event( BUTTON_STO, xev );
            wake = 1;
            break;
        case XK_o:
        case XK_O:
            key_event( BUTTON_EVAL, xev );
            wake = 1;
            break;
        case XK_p:
        case XK_P:
        case XK_Left:
            key_event( BUTTON_LEFT, xev );
            wake = 1;
            break;
        case XK_q:
        case XK_Q:
        case XK_Down:
            key_event( BUTTON_DOWN, xev );
            wake = 1;
            break;
        case XK_r:
        case XK_R:
        case XK_Right:
            key_event( BUTTON_RIGHT, xev );
            wake = 1;
            break;
        case XK_s:
        case XK_S:
            key_event( BUTTON_SIN, xev );
            wake = 1;
            break;
        case XK_t:
        case XK_T:
            key_event( BUTTON_COS, xev );
            wake = 1;
            break;
        case XK_u:
        case XK_U:
            key_event( BUTTON_TAN, xev );
            wake = 1;
            break;
        case XK_v:
        case XK_V:
            key_event( BUTTON_SQRT, xev );
            wake = 1;
            break;
        case XK_w:
        case XK_W:
            key_event( BUTTON_POWER, xev );
            wake = 1;
            break;
        case XK_x:
        case XK_X:
            key_event( BUTTON_INV, xev );
            wake = 1;
            break;
        case XK_y:
        case XK_Y:
            key_event( BUTTON_NEG, xev );
            wake = 1;
            break;
        case XK_z:
        case XK_Z:
            key_event( BUTTON_EEX, xev );
            wake = 1;
            break;
        default:
            break;
    }
    return wake;
}
#elif defined( GUI_IS_SDL1 )
void SDLCreateHP( void ) {
    /* int x, y, w, h; */
    unsigned int width, height;

    // SDL port: we allocate memory for the buttons because we need to modify
    // their coordinates, and we don't want to change the original buttons_gx or
    // buttons_sx
    if ( buttons ) {
        free( buttons );
        buttons = 0;
    }
    buttons = ( button_t* )malloc( sizeof( buttons_gx ) );

    if ( opt_gx ) {
        // buttons = buttons_gx;
        memcpy( buttons, buttons_gx, sizeof( buttons_gx ) );
        colors = colors_gx;
    } else {
        // buttons = buttons_sx;
        memcpy( buttons, buttons_sx, sizeof( buttons_sx ) );
        colors = colors_sx;
    }

    int cut;

    ///////////////////////////////////////////////
    // SDL PORT
    ///////////////////////////////////////////////
    SDLCreateColors();

    width = KEYBOARD_WIDTH + 2 * SIDE_SKIP;
    height = DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + DISP_KBD_SKIP +
             KEYBOARD_HEIGHT + BOTTOM_SKIP;

    disp.mapped = 1;
    disp.w = DISPLAY_WIDTH;
    disp.h = DISPLAY_HEIGHT;

    keypad.width = width;
    keypad.height = height;

    cut = buttons[ BUTTON_MTH ].y + KEYBOARD_OFFSET_Y - 19;
    SDLDrawBackground( width, cut, width, height );
    SDLDrawMore( width, height, cut, KEYBOARD_OFFSET_Y, KEYBOARD_OFFSET_X,
                 keypad.width, keypad.height );
    SDLDrawLogo( width, height, cut, KEYBOARD_OFFSET_Y, KEYBOARD_OFFSET_X,
                 keypad.width, keypad.height );
    SDLDrawBezel( width, height, KEYBOARD_OFFSET_Y, KEYBOARD_OFFSET_X );
    SDLDrawKeypad();

    SDL_UpdateRect( sdlwindow, 0, 0, 0, 0 );
}

// Find which key is pressed, if any.
// Returns -1 is no key is pressed
int SDLCoordinateToKey( int x, int y ) {
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
    int i;
    i = 0;
    while ( sdltohpkeymap[ i ].sdlkey ) {
        if ( sdltohpkeymap[ i ].sdlkey == k )
            return sdltohpkeymap[ i ].hpkey;
        i++;
    }
    return -1;
}

void SDLDrawMore( unsigned int w, unsigned int h, unsigned int cut,
                  unsigned int offset_y, unsigned int offset_x,
                  int keypad_width, int keypad_height ) {

    int display_height = DISPLAY_HEIGHT;
    int display_width = DISPLAY_WIDTH;

    // lower the whole thing

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

void SDLDrawLogo( unsigned int w, unsigned int h, unsigned int cut,
                  unsigned int offset_y, unsigned int offset_x,
                  int keypad_width, int keypad_height ) {
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
void SDLDrawKeyLabelLeft( void ) {
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
void SDLDrawKeyLabelRight( void ) {
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

// Draw the letter bottom right of the key
void SDLDrawKeyLetter( void ) {
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
void SDLDrawKeyLabelBottom( void ) {
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
    SDLDrawKeyLetter();
    SDLDrawKeyLabelBottom();
    SDLDrawKeyLabelLeft();
    SDLDrawKeyLabelRight();
    SDLCreateKeys();
    SDLDrawButtons();
}

void SDLDrawSmallString( int x, int y, const char* string, unsigned int length,
                         unsigned int coloron, unsigned int coloroff ) {
    int i;

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

void SDLDrawBezel( unsigned int width, unsigned int height,
                   unsigned int offset_y, unsigned int offset_x ) {
    int i;
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

    // LCD
    rect.x = DISPLAY_OFFSET_X;
    rect.y = DISPLAY_OFFSET_Y;
    rect.w = DISPLAY_WIDTH;
    rect.h = DISPLAY_HEIGHT;
    SDL_FillRect( sdlwindow, &rect, ARGBColors[ LCD ] );
}

SDL_Surface* sdlwindow;
SDL_Surface* sdlsurface;

void SDLInit( void ) {
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

    unsigned width =
        ( buttons_gx[ LAST_BUTTON ].x + buttons_gx[ LAST_BUTTON ].w ) +
        2 * SIDE_SKIP;
    unsigned height = DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + DISP_KBD_SKIP +
                      buttons_gx[ LAST_BUTTON ].y +
                      buttons_gx[ LAST_BUTTON ].h + BOTTOM_SKIP;

    sdlwindow = SDL_SetVideoMode( width, height, 32, SDL_SWSURFACE );

    if ( sdlwindow == NULL ) {
        printf( "Couldn't set video mode: %s\n", SDL_GetError() );
        exit( 1 );
    }
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

void SDLDrawNibble( int _x, int _y, int val ) {
    int x, y;
    int xoffset = DISPLAY_OFFSET_X + 5;
    int yoffset = DISPLAY_OFFSET_Y + 20;

    SDL_LockSurface( sdlwindow );
    unsigned char* buffer = ( unsigned char* )sdlwindow->pixels;
    unsigned int pitch = sdlwindow->pitch;

    for ( y = 0; y < 2; y++ ) {
        unsigned int* lineptr;
        lineptr =
            ( unsigned int* )( buffer + pitch * ( yoffset + 2 * _y + y ) );

        for ( x = 0; x < 4; x++ ) {
            // Check if bit is on
            // char c = lcd_buffer[y/2][x>>2];		// The 4 lower bits
            // in a byte are used (1 nibble per byte)
            if ( _x + x >= 131 ) // Clip at 131 pixels (some nibble writes may
                                 // go beyond the range, but are not visible)
                break;
            char c = val;
            char b = c & ( 1 << ( x & 3 ) );
            if ( b ) {
                lineptr[ xoffset + 2 * ( _x + x ) ] = ARGBColors[ PIXEL ];
                lineptr[ xoffset + 2 * ( _x + x ) + 1 ] = ARGBColors[ PIXEL ];
            } else {
                lineptr[ xoffset + 2 * ( _x + x ) ] = ARGBColors[ LCD ];
                lineptr[ xoffset + 2 * ( _x + x ) + 1 ] = ARGBColors[ LCD ];
            }
        }
    }
    SDL_UnlockSurface( sdlwindow );

#ifndef DELAYEDDISPUPDATE
    // Either update immediately or with a delay the display
    SDL_UpdateRect( sdlwindow, xoffset + 2 * _x, yoffset + 2 * _y, 8, 2 );
#endif
}

/*
        Create a surface from binary bitmap data
*/
SDL_Surface* SDLCreateSurfFromData( unsigned int w, unsigned int h,
                                    unsigned char* data, unsigned int coloron,
                                    unsigned int coloroff ) {
    int x, y;
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

// Create a ARGB surface from RGB data. Allows to set a transparent color
SDL_Surface* SDLCreateARGBSurfFromData( unsigned int w, unsigned int h,
                                        unsigned char* data,
                                        unsigned int xpcolor ) {
    int x, y;
    SDL_Surface* surf;

    surf = SDL_CreateRGBSurface( SDL_SWSURFACE, w, h, 32, 0x00ff0000,
                                 0x0000ff00, 0x000000ff, 0xff000000 );

    SDL_LockSurface( surf );

    unsigned char* pixels = ( unsigned char* )surf->pixels;
    unsigned int pitch = surf->pitch;
    for ( y = 0; y < h; y++ ) {
        unsigned int* lineptr = ( unsigned int* )( pixels + y * pitch );
        for ( x = 0; x < w; x++ ) {
            unsigned r, g, b, color;
            r = data[ y * w * 3 + x * 3 ];
            g = data[ y * w * 3 + x * 3 + 1 ];
            b = data[ y * w * 3 + x * 3 + 2 ];
            color = SDLToARGB( 255, r, g, b );

            // Set alpha to 0 for transparent colors
            if ( ( color & 0xffffff ) == ( xpcolor & 0xffffff ) ) {
                color = color & 0xffffff;
            }
            lineptr[ x ] = color;
        }
    }
    SDL_UnlockSurface( surf );
    return surf;
}

unsigned SDLBGRA2ARGB( unsigned color ) {
    unsigned a, r, g, b;
    SDLARGBTo( color, &a, &r, &g, &b );

    color = a | ( r << 24 ) | ( g << 16 ) | ( b << 8 );
    return color;
}

// State to displayed zoomed last pressed key
SDL_Surface* showkeylastsurf = 0;
int showkeylastx, showkeylasty, showkeylastkey;
// Show the hp key which is being pressed
void SDLUIShowKey( int hpkey ) {
    SDL_Rect /* rect,  */ srect, drect;
    SDL_Surface *ssurf, *zsurf;
    int x;
    int y;
    // double zoomfactor = 1.5;
    double zoomfactor = 3;

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

    // Zoomed surface
    zsurf = zoomSurface( ssurf, zoomfactor, zoomfactor, 0 );

    // Background backup
    showkeylastsurf =
        SDL_CreateRGBSurface( SDL_SWSURFACE, zsurf->w, zsurf->h, 32, 0x00ff0000,
                              0x0000ff00, 0x000000ff, 0xff000000 );

    // Where to
    x = KEYBOARD_OFFSET_X + buttons[ hpkey ].x -
        ( zsurf->w - ssurf->w + 1 ) / 2;
    y = KEYBOARD_OFFSET_Y + buttons[ hpkey ].y -
        ( zsurf->h - ssurf->h + 1 ) / 2;
    // blitting does not clip to screen, so if we are out of the screen we
    // shift the button to fit
    if ( x < 0 )
        x = 0;
    if ( y < 0 )
        y = 0;
    if ( x + zsurf->w > sdlwindow->w )
        x = sdlwindow->w - zsurf->w;
    if ( y + zsurf->h > sdlwindow->h )
        y = sdlwindow->h - zsurf->h;

    // Backup where to
    showkeylastx = x;
    showkeylasty = y;

    // Backup old surface
    srect.x = x;
    srect.y = y;
    srect.w = zsurf->w;
    srect.h = zsurf->h;
    drect.x = 0;
    drect.y = 0;
    SDL_BlitSurface( sdlwindow, &srect, showkeylastsurf, &drect );

    // Blit the zoomed button
    drect.x = x;
    drect.y = y;
    SDL_BlitSurface( zsurf, 0, sdlwindow, &drect );

    // Free zoomed surface
    SDL_FreeSurface( zsurf );

    // Update
    SDL_UpdateRect( sdlwindow, x, y, zsurf->w, zsurf->h );
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

void SDLUIFeedback( void ) {
    // This function should give some UI feedback to indicate that a key was
    // pressed E.g. by beeping, vibrating or flashing something

    SDL_Surface *surf1, *surf2;
    surf1 =
        SDL_CreateRGBSurface( SDL_SWSURFACE, sdlwindow->w, sdlwindow->h, 32,
                              0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 );
    surf2 =
        SDL_CreateRGBSurface( SDL_SWSURFACE, sdlwindow->w, sdlwindow->h, 32,
                              0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 );

    // Copy screen
    SDL_BlitSurface( sdlwindow, 0, surf1, 0 );

    // Overlay something
    SDL_FillRect( surf2, 0, 0x80000000 );
    SDL_BlitSurface( surf2, 0, sdlwindow, 0 );
    SDL_UpdateRect( sdlwindow, 0, 0, 0, 0 );

    SDL_Delay( 20 );

    // Restore screen
    SDL_BlitSurface( surf1, 0, sdlwindow, 0 );

    SDL_UpdateRect( sdlwindow, 0, 0, 0, 0 );

    SDL_FreeSurface( surf1 );
    SDL_FreeSurface( surf2 );
}

// Simple 'show window' function
SDLWINDOW_t SDLCreateWindow( int x, int y, int w, int h, unsigned color,
                             int framewidth, int inverted ) {
    SDLWINDOW_t win;

    // Backup the screen
    win.oldsurf = SDL_CreateRGBSurface( SDL_SWSURFACE, w, h, 32, 0x00ff0000,
                                        0x0000ff00, 0x000000ff, 0xff000000 );
    win.surf = SDL_CreateRGBSurface( SDL_SWSURFACE, w, h, 32, 0x00ff0000,
                                     0x0000ff00, 0x000000ff, 0xff000000 );
    win.x = x;
    win.y = y;
    // Copy screen
    SDL_Rect srect;
    SDL_Rect drect;
    srect.x = x;
    srect.y = y;
    srect.w = w;
    srect.h = h;
    drect.x = 0;
    drect.y = 0;
    drect.w = w;
    drect.h = h;
    SDL_BlitSurface( sdlwindow, &srect, win.oldsurf, &drect );

    // Draw window background
    SDL_FillRect( win.surf, &drect, color );

    // Compute the frame color
    int a, r, g, b;
    int r2, g2, b2;
    int contrast = 5;
    unsigned colorlight, colordark;
    SDLARGBTo( color, ( unsigned* )&a, ( unsigned* )&r, ( unsigned* )&g,
               ( unsigned* )&b );
    r2 = r + r / contrast;
    g2 = g + g / contrast;
    b2 = b + b / contrast;
    if ( r2 > 255 )
        r2 = 255;
    if ( g2 > 255 )
        g2 = 255;
    if ( b2 > 255 )
        b2 = 255;
    colorlight = SDLToARGB( a, r2, g2, b2 );
    r2 = r - r / contrast;
    g2 = g - g / contrast;
    b2 = b - b / contrast;
    if ( r2 < 0 )
        r2 = 0;
    if ( g2 < 0 )
        g2 = 0;
    if ( b2 < 0 )
        b2 = 0;
    colordark = SDLToARGB( a, r2, g2, b2 );

    if ( inverted ) {
        unsigned t = colorlight;
        colorlight = colordark;
        colordark = t;
    }

    // Draw the frame
    int i;
    for ( i = 0; i < framewidth; i++ ) {
        lineColor( win.surf, 0, i, w - 2 - i, i,
                   SDLBGRA2ARGB( colorlight ) ); // Horizontal top
        lineColor( win.surf, i, 0, i, h - 2 - i,
                   SDLBGRA2ARGB( colorlight ) ); // Vertical left

        lineColor( win.surf, 1 + i, h - 1 - i, w - 1, h - 1 - i,
                   SDLBGRA2ARGB( colordark ) ); // Horizontal bottom
        lineColor( win.surf, w - 1 - i, 1 + i, w - 1 - i, h - 1,
                   SDLBGRA2ARGB( colordark ) ); // Vertical right
    }

    return win;
}

void SDLShowWindow( SDLWINDOW_t* win ) {
    // Blit the window
    SDL_Rect srect;
    SDL_Rect drect;
    srect.x = 0;
    srect.y = 0;
    srect.w = win->surf->w;
    srect.h = win->surf->h;
    drect.x = win->x;
    drect.y = win->y;
    drect.w = win->surf->w;
    drect.h = win->surf->h;

    SDL_BlitSurface( win->surf, &srect, sdlwindow, &drect );

    SDL_UpdateRect( sdlwindow, 0, 0, 0, 0 );
}

void SDLHideWindow( SDLWINDOW_t* win ) {
    // Restore the screen
    SDL_Rect srect;
    SDL_Rect drect;
    srect.x = 0;
    srect.y = 0;
    srect.w = win->oldsurf->w;
    srect.h = win->oldsurf->h;
    drect.x = win->x;
    drect.y = win->y;
    drect.w = win->oldsurf->w;
    drect.h = win->oldsurf->h;

    SDL_BlitSurface( win->oldsurf, &srect, sdlwindow, &drect );

    SDL_UpdateRect( sdlwindow, 0, 0, 0, 0 );
    SDL_FreeSurface( win->surf );
    SDL_FreeSurface( win->oldsurf );
    win->surf = 0;
}

// Convert a 32-bit aarrggbb number to individual components
void SDLARGBTo( unsigned color, unsigned* a, unsigned* r, unsigned* g,
                unsigned* b ) {
    *a = ( color >> 24 ) & 0xff;
    *r = ( color >> 16 ) & 0xff;
    *g = ( color >> 8 ) & 0xff;
    *b = color & 0xff;
}
// Convert a,r,g,b to a 32-bit aarrggbb number
unsigned SDLToARGB( unsigned a, unsigned r, unsigned g, unsigned b ) {
    return ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | ( b );
}

void SDLMessageBox( int w, int h, const char* title, const char* text[],
                    unsigned color, unsigned colortext, int center ) {
    int x, y;
    int framewidth = 3;
    int texthspace = 9;

    // Compute the coordinates of the window (center on screen)
    x = ( sdlwindow->w - w ) / 2;
    y = ( sdlwindow->h - h ) / 2;

    SDLWINDOW_t win;
    win = SDLCreateWindow( x, y, w, h, color, framewidth, 0 );

    stringColor( win.surf, ( w - strlen( title ) * 8 ) / 2,
                 framewidth + texthspace, title, SDLBGRA2ARGB( colortext ) );

    int i, numlines;
    for ( numlines = 0; text[ numlines ]; numlines++ )
        ; // Count number of lines
    for ( i = 0; text[ i ]; i++ ) {
        if ( center )
            x = ( w - strlen( text[ i ] ) * 8 ) / 2;
        else
            x = framewidth + 8;
        if ( 0 )
            stringColor( win.surf, x, framewidth + 8 + 16 + i * texthspace,
                         text[ i ], SDLBGRA2ARGB( colortext ) );
        else
            stringColor( win.surf, x,
                         ( h - numlines * texthspace ) / 2 + i * texthspace,
                         text[ i ], SDLBGRA2ARGB( colortext ) );
    }

    SDLShowWindow( &win );
    SDLEventWaitClickOrKey();
    SDLHideWindow( &win );
}

void SDLEventWaitClickOrKey( void ) {
    SDL_Event event;
    while ( 1 ) {
        SDL_WaitEvent( &event );
        switch ( event.type ) {
            case SDL_QUIT:
                return;
                break;

            case SDL_MOUSEBUTTONDOWN:
                return;
                break;

            case SDL_KEYDOWN:
                return;
                break;
        }
    }
}

void SDLShowInformation( void ) {
    const char* info_title = "x48ng - HP48 emulator";

    const char* info_text[] = { //"12345678901234567890123456789012345",
                                "",
                                "License: GPL",
                                "",
                                "In order to work the emulator needs",
                                "the following files:",
                                "  rom:   an HP48 rom dump",
                                "  ram:   ram file",
                                "  hp48:  HP state file",
                                "",
                                "The following files are optional:",
                                "  port1: card 1 memory",
                                "  port2: card 2 memory",
                                "",
                                "These files must be in ~/.x48ng",
                                "",
                                0 };
    SDLMessageBox( 310, 280, info_title, info_text, 0xf0c0c0e0, 0xff000000, 0 );

    const char* info_text2[] = { //"12345678901234567890123456789012345",
                                 "",
                                 "Do a long key press (about 1 sec)",
                                 "to keep the key down (e.g. for",
                                 "ON-C, ON-+, ON-A-F).",
                                 "",
                                 "There are issues with throttling",
                                 "and key repeat rate. Therefore key",
                                 "repeat for arrows and backspace is",
                                 "disabled.",
                                 "",
                                 0 };

    SDLMessageBox( 310, 280, info_title, info_text2, 0xf0c0c0e0, 0xff000000,
                   0 );
}
#endif

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

    if ( buttons[ b ].pressed ==
         0 ) // Check not already released (not critical)
        return 0;

    buttons[ b ].pressed = 0;

    code = buttons[ b ].code;
    if ( code == 0x8000 ) {
        int i;
        for ( i = 0; i < 9; i++ )
            saturn.keybuf.rows[ i ] &= ~0x8000;
    } else {
        int r, c;
        r = code >> 4;
        c = 1 << ( code & 0xf );
        saturn.keybuf.rows[ r ] &= ~c;
    }

    return 0;
}

#if defined( GUI_IS_X11 )
static int button_release_all( void ) {
    for ( int b = BUTTON_A; b <= LAST_BUTTON; b++ )
        if ( buttons[ b ].pressed ) {
            int code = buttons[ b ].code;
            if ( code == 0x8000 ) {
                int i;
                for ( i = 0; i < 9; i++ )
                    saturn.keybuf.rows[ i ] &= ~0x8000;
            } else {
                int r, c;
                r = code >> 4;
                c = 1 << ( code & 0xf );
                saturn.keybuf.rows[ r ] &= ~c;
            }
            buttons[ b ].pressed = 0;
            DrawButton( b );
        }

    return 0;
}
#elif defined( GUI_IS_SDL1 )
static int button_release_all( void ) {
    for ( int b = BUTTON_A; b <= LAST_BUTTON; b++ )
        if ( buttons[ b ].pressed ) {
            button_released( b );
        }

    return 0;
}
#endif

#if defined( GUI_IS_X11 )
void ShowConnections( char* wire, char* ir ) {
    char name[ 128 ];
    int x, y, w, h;
    int conn_top;
    XFontStruct* finfo;
    XGCValues val;
    unsigned long gc_mask;
    XCharStruct xchar;
    int dir, fa, fd;
    Pixmap pix;

    finfo = get_font_resource( dpy, "connectionFont", "ConnectionFont" );
    val.font = finfo->fid;
    gc_mask = GCFont;
    XChangeGC( dpy, gc, gc_mask, &val );

    conn_top = DISPLAY_OFFSET_Y + DISPLAY_HEIGHT + 18;

    XTextExtents( finfo, "TEST", ( int )strlen( "TEST" ), &dir, &fa, &fd,
                 &xchar );
    w = DISPLAY_WIDTH;
    h = fa + fd;

    pix = XCreatePixmap( dpy, keypad.pixmap, w, h, depth ); /* FIXME keypad? */
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
    XCopyArea( dpy, pix, keypad.pixmap, gc, 0, 0, w, h, x,
              y ); /* FIXME keypad? */

    DrawKeypad( &keypad );

    XFreePixmap( dpy, pix );
    XFreeFont( dpy, finfo );
}
#elif defined( GUI_IS_SDL1 )
void ShowConnections( char* wire, char* ir ) {
    char name[ 128 ];
    fprintf( stderr, "%s\n", name );
}
#endif

#if defined( GUI_IS_X11 )
int get_ui_event( void ) {
    XEvent xev;
    XClientMessageEvent* cm;
    int i, wake, bufs = 2;
    char buf[ 2 ];
    KeySym sym;
    // int button_expose;
    // static int button_leave = -1;
    static int release_pending = 0;
    static XKeyEvent release_event;
    static Time last_release_time = 0;

    wake = 0;
    if ( paste_last_key ) {
        button_released( paste[ paste_count - 1 ] );
        paste_last_key = 0;
        return 1;
    } else if ( paste_count < paste_size ) {
        button_pressed( paste[ paste_count ] );
        paste_last_key = 1;
        paste_count++;
        return 1;
    }

    if ( release_pending ) {
        i = XLookupString( &release_event, buf, bufs, &sym, NULL );
        wake = decode_key( ( XEvent* )&release_event, sym, buf, i );
        release_pending = 0;
        return wake;
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
                wake = decode_key( &xev, sym, buf, i );
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
                    if ( xev.xexpose.window == disp.win ) {
                        DrawDisp();
                    } else if ( xev.xexpose.window == iconW ) {
                        DrawIcon();
                    } else if ( xev.xexpose.window == mainW ) {
                        DrawKeypad( &keypad );
                    } else
                            for ( i = BUTTON_A; i <= LAST_BUTTON; i++ ) {
                                if ( xev.xexpose.window == buttons[ i ].xwin ) {
                                    DrawButton( i );
                                    break;
                                }
                            }
                }
                break;
            case UnmapNotify:

                disp.mapped = 0;
                break;

            case MapNotify:

                if ( !disp.mapped ) {
                    disp.mapped = 1;
                    update_display();
                    redraw_annunc();
                }
                break;

            case ButtonPress:

                if ( xev.xbutton.subwindow == disp.win ) {
                    if ( xev.xbutton.button == Button2 ) {
                        if ( xev.xbutton.subwindow == disp.win ) {
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
                                    paste[ paste_size++ ] =
                                        BUTTON_PERIOD;
                                    break;
                                case '0':
                                    paste[ paste_size++ ] = BUTTON_0;
                                    break;
                                case '1':
                                    paste[ paste_size++ ] = BUTTON_1;
                                    break;
                                case '2':
                                    paste[ paste_size++ ] = BUTTON_2;
                                    break;
                                case '3':
                                    paste[ paste_size++ ] = BUTTON_3;
                                    break;
                                case '4':
                                    paste[ paste_size++ ] = BUTTON_4;
                                    break;
                                case '5':
                                    paste[ paste_size++ ] = BUTTON_5;
                                    break;
                                case '6':
                                    paste[ paste_size++ ] = BUTTON_6;
                                    break;
                                case '7':
                                    paste[ paste_size++ ] = BUTTON_7;
                                    break;
                                case '8':
                                    paste[ paste_size++ ] = BUTTON_8;
                                    break;
                                case '9':
                                    paste[ paste_size++ ] = BUTTON_9;
                                    break;
                                case '\n':
                                    paste[ paste_size++ ] = BUTTON_SHR;
                                    paste[ paste_size++ ] =
                                        BUTTON_PERIOD;
                                    break;
                                case '!':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    paste[ paste_size++ ] = BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_DEL;
                                    break;
                                case '+':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    paste[ paste_size++ ] = BUTTON_PLUS;
                                    break;
                                case '-':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    paste[ paste_size++ ] =
                                        BUTTON_MINUS;
                                    break;
                                case '*':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    paste[ paste_size++ ] = BUTTON_MUL;
                                    break;
                                case '/':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    paste[ paste_size++ ] = BUTTON_DIV;
                                    break;
                                case ' ':
                                    paste[ paste_size++ ] = 47;
                                    break;
                                case '(':
                                    paste[ paste_size++ ] = BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_DIV;
                                    break;
                                case '[':
                                    paste[ paste_size++ ] = BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_MUL;
                                    break;
                                case '<':
                                    if ( x > 1 && *p == '<' ) {
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                        paste[ paste_size++ ] =
                                            BUTTON_MINUS;
                                        x--;
                                        p++;
                                    } else {
                                        paste[ paste_size++ ] =
                                            BUTTON_ALPHA;
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                        paste[ paste_size++ ] =
                                            BUTTON_2;
                                    }
                                    break;
                                case '{':
                                    paste[ paste_size++ ] = BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_PLUS;
                                    break;
                                case ')':
                                case ']':
                                case '}':
                                    paste[ paste_size++ ] =
                                        BUTTON_RIGHT;
                                    break;
                                case '>':
                                    if ( x > 1 && *p == '>' ) {
                                        paste[ paste_size++ ] =
                                            BUTTON_RIGHT;
                                        paste[ paste_size++ ] =
                                            BUTTON_RIGHT;
                                        paste[ paste_size++ ] =
                                            BUTTON_RIGHT;
                                        x--;
                                        p++;
                                    } else {
                                        paste[ paste_size++ ] =
                                            BUTTON_ALPHA;
                                        paste[ paste_size++ ] =
                                            BUTTON_SHR;
                                        paste[ paste_size++ ] =
                                            BUTTON_2;
                                    }
                                    break;
                                case '#':
                                    paste[ paste_size++ ] = BUTTON_SHR;
                                    paste[ paste_size++ ] = BUTTON_DIV;
                                    break;
                                case '_':
                                    paste[ paste_size++ ] = BUTTON_SHR;
                                    paste[ paste_size++ ] = BUTTON_MUL;
                                    break;
                                case '"':
                                    if ( flag & 1 ) {
                                        flag &= ~1;
                                        paste[ paste_size++ ] =
                                            BUTTON_RIGHT;
                                    } else {
                                        flag |= 1;
                                        paste[ paste_size++ ] =
                                            BUTTON_SHR;
                                        paste[ paste_size++ ] =
                                            BUTTON_MINUS;
                                    }
                                    break;
                                case ':':
                                    if ( flag & 2 ) {
                                        flag &= ~2;
                                        paste[ paste_size++ ] =
                                            BUTTON_RIGHT;
                                    } else {
                                        flag |= 2;
                                        paste[ paste_size++ ] =
                                            BUTTON_SHR;
                                        paste[ paste_size++ ] =
                                            BUTTON_PLUS;
                                    }
                                    break;
                                case '\'':
                                    if ( flag & 4 ) {
                                        flag &= ~4;
                                        paste[ paste_size++ ] =
                                            BUTTON_RIGHT;
                                    } else {
                                        flag |= 4;
                                        paste[ paste_size++ ] =
                                            BUTTON_COLON;
                                    }
                                    break;
                                case 'a':
                                case 'A':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_A;
                                    break;
                                case 'b':
                                case 'B':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_B;
                                    break;
                                case 'c':
                                case 'C':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_C;
                                    break;
                                case 'd':
                                case 'D':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_D;
                                    break;
                                case 'e':
                                case 'E':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_E;
                                    break;
                                case 'f':
                                case 'F':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_F;
                                    break;
                                case 'g':
                                case 'G':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_MTH;
                                    break;
                                case 'h':
                                case 'H':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_PRG;
                                    break;
                                case 'i':
                                case 'I':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_CST;
                                    break;
                                case 'j':
                                case 'J':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_VAR;
                                    break;
                                case 'k':
                                case 'K':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_UP;
                                    break;
                                case 'l':
                                case 'L':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_NXT;
                                    break;

                                case 'm':
                                case 'M':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] =
                                        BUTTON_COLON;
                                    break;
                                case 'n':
                                case 'N':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_STO;
                                    break;
                                case 'o':
                                case 'O':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_EVAL;
                                    break;
                                case 'p':
                                case 'P':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_LEFT;
                                    break;
                                case 'q':
                                case 'Q':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_DOWN;
                                    break;
                                case 'r':
                                case 'R':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] =
                                        BUTTON_RIGHT;
                                    break;
                                case 's':
                                case 'S':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_SIN;
                                    break;
                                case 't':
                                case 'T':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_COS;
                                    break;
                                case 'u':
                                case 'U':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_TAN;
                                    break;
                                case 'v':
                                case 'V':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_SQRT;
                                    break;
                                case 'w':
                                case 'W':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] =
                                        BUTTON_POWER;
                                    break;
                                case 'x':
                                case 'X':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_INV;
                                    break;
                                case 'y':
                                case 'Y':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_NEG;
                                    break;
                                case 'z':
                                case 'Z':
                                    paste[ paste_size++ ] =
                                        BUTTON_ALPHA;
                                    if ( islower( c ) )
                                        paste[ paste_size++ ] =
                                            BUTTON_SHL;
                                    paste[ paste_size++ ] = BUTTON_EEX;
                                    break;
                                default:
                                    printf( "unknown %c %d\n", c, *p );
                                    break;
                                }
                            }
                            if ( paste_in )
                                XFree( paste_in );
                            if ( paste_size ) {
                                return 1;
                            }
                        }
                    }
                } else {
                    if ( xev.xbutton.button == Button1 ||
                         xev.xbutton.button == Button2 ||
                         xev.xbutton.button == Button3 ) {
                        for ( i = BUTTON_A; i <= LAST_BUTTON; i++ ) {
                            if ( xev.xbutton.subwindow ==
                                 buttons[ i ].xwin ) {
                                if ( buttons[ i ].pressed ) {
                                    if ( xev.xbutton.button == Button3 ) {
                                        button_released( i );
                                        DrawButton( i );
                                    }
                                } else {
                                    last_button = i;
                                    button_pressed( i );
                                    wake = 1;
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
                if ( xev.xbutton.button == Button1 ) {
                    button_release_all();
                }
                if ( xev.xbutton.button == Button2 ) {
                    if ( last_button >= 0 ) {
                        button_released( last_button );
                        DrawButton( last_button );
                    }
                    last_button = -1;
                }
                break;

            case FocusOut:
                first_key = 0;
                button_release_all();
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
                    if ( cm->data.l[ 0 ] == wm_delete_window ) {
                        /*
                         * Quit selected from window managers menu
                         */
                        exit_x48( 1 );
                    }

                    if ( cm->data.l[ 0 ] == wm_save_yourself ) {
                        save_command_line();
                    }
                }
                break;

            default:

            case KeymapNotify:
            case ConfigureNotify:
            case ReparentNotify:
                break;
            }
        }
    } while ( first_key > 1 );

    if ( first_key )
        first_key++;

    return wake;
}
#elif defined( GUI_IS_SDL1 )
int get_ui_event( void ) {
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
    static int dispupdate_t1 = -1,
               dispupdate_t2; // Logic to display at regular intervals

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
                exit_x48( 0 );
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
                // React only to left mouse button
                if ( event.type == SDL_MOUSEBUTTONDOWN &&
                     event.button.button == SDL_BUTTON_LEFT ) {

                    if ( event.button.y < DISPLAY_OFFSET_Y ||
                         event.button.y <
                             48 ) // If we resized the screen, then there's
                                  // no space above offset_y...clicking on
                                  // the screen has to do something
                        SDLShowInformation();
                }
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
                if ( event.type == SDL_KEYDOWN &&
                     event.key.keysym.sym == SDLK_F1 ) {
                    SDLShowInformation();
                }

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
#endif
