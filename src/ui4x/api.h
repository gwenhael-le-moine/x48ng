#ifndef _UI4x_API_H
#  define _UI4x_API_H 1

#  include <stdbool.h>

#  define LCD_WIDTH ( 131 )

#  define NB_ANNUNCIATORS ( 6 )

// HP 48{G,S}X Keys
typedef enum {
    HP48_KEY_A = 0,
    HP48_KEY_B,
    HP48_KEY_C,
    HP48_KEY_D,
    HP48_KEY_E,
    HP48_KEY_F,

    HP48_KEY_G,
    HP48_KEY_H,
    HP48_KEY_I,
    HP48_KEY_J,
    HP48_KEY_K,
    HP48_KEY_L,

    HP48_KEY_M,
    HP48_KEY_N,
    HP48_KEY_O,
    HP48_KEY_P,
    HP48_KEY_Q,
    HP48_KEY_R,

    HP48_KEY_S,
    HP48_KEY_T,
    HP48_KEY_U,
    HP48_KEY_V,
    HP48_KEY_W,
    HP48_KEY_X,

    HP48_KEY_ENTER,
    HP48_KEY_Y,
    HP48_KEY_Z,
    HP48_KEY_DEL,
    HP48_KEY_BACKSPACE,

    HP48_KEY_ALPHA,
    HP48_KEY_7,
    HP48_KEY_8,
    HP48_KEY_9,
    HP48_KEY_DIVIDE,

    HP48_KEY_LEFTSHIFT,
    HP48_KEY_4,
    HP48_KEY_5,
    HP48_KEY_6,
    HP48_KEY_MUL,

    HP48_KEY_RIGHTSHIFT,
    HP48_KEY_1,
    HP48_KEY_2,
    HP48_KEY_3,
    HP48_KEY_MINUS,

    HP48_KEY_ON,
    HP48_KEY_0,
    HP48_KEY_PERIOD,
    HP48_KEY_SPACE,
    HP48_KEY_PLUS,

    NB_HP48_KEYS
} hp48sx_gx_keynames_t;

typedef enum {
    HP4950_KEY_A = 0,
    HP4950_KEY_B,
    HP4950_KEY_C,
    HP4950_KEY_D,
    HP4950_KEY_E,
    HP4950_KEY_F,

    HP4950_KEY_G,
    HP4950_KEY_H,
    HP4950_KEY_I,
    HP4950_KEY_UP,
    HP4950_KEY_J, /* 10 */

    HP4950_KEY_K,
    HP4950_KEY_L,
    HP4950_KEY_LEFT,
    HP4950_KEY_DOWN,
    HP4950_KEY_RIGHT,

    HP4950_KEY_M,
    HP4950_KEY_N,
    HP4950_KEY_O,
    HP4950_KEY_P,
    HP4950_KEY_BACKSPACE, /* 20 */

    HP4950_KEY_Q,
    HP4950_KEY_R,
    HP4950_KEY_S,
    HP4950_KEY_T,
    HP4950_KEY_U,

    HP4950_KEY_V,
    HP4950_KEY_W,
    HP4950_KEY_X,
    HP4950_KEY_Y,
    HP4950_KEY_Z,

    HP4950_KEY_ALPHA,
    HP4950_KEY_7,
    HP4950_KEY_8,
    HP4950_KEY_9,
    HP4950_KEY_MULTIPLY,

    HP4950_KEY_LEFTSHIFT,
    HP4950_KEY_4,
    HP4950_KEY_5,
    HP4950_KEY_6,
    HP4950_KEY_MINUS,

    HP4950_KEY_RIGHTSHIFT,
    HP4950_KEY_1,
    HP4950_KEY_2,
    HP4950_KEY_3,
    HP4950_KEY_PLUS,

    HP4950_KEY_ON,
    HP4950_KEY_0,
    HP4950_KEY_PERIOD,
    HP4950_KEY_SPACE,
    HP4950_KEY_ENTER,

    NB_HP4950_KEYS
} hp49g_50g_keynames_t;

typedef enum { FRONTEND_SDL, FRONTEND_NCURSES, FRONTEND_GTK } ui4x_frontend_t;

typedef enum { MODEL_48SX = 485, MODEL_48GX = 486, MODEL_40G = 406, MODEL_49G = 496, MODEL_50G = 506 } ui4x_model_t;

typedef struct ui4x_config_t {
    ui4x_model_t model;
    bool shiftless;
    bool black_lcd;
    bool newrpl_keyboard;

    ui4x_frontend_t frontend;
    bool mono;
    bool gray;

    bool chromeless;
    bool fullscreen;

    bool tiny;
    bool small;

    bool verbose;

    double zoom;
    bool netbook;
    int netbook_pivot_line;

    char* name;
    char* progname;
    char* progpath;
    char* wire_name;
    char* ir_name;

    char* datadir;
    char* style_filename;

    char* sd_dir;
} ui4x_config_t;

typedef struct ui4x_emulator_api_t {
    /* keyboard */
    void ( *press_key )( int hpkey );
    void ( *release_key )( int hpkey );
    bool ( *is_key_pressed )( int hpkey );
    /* display */
    bool ( *is_display_on )( void );
    unsigned char ( *get_annunciators )( void );
    void ( *get_lcd_buffer )( int* target );
    int ( *get_contrast )( void );
    /* SD card */
    int ( *do_mount_sd )( char* filename );
    void ( *do_unmount_sd )( void );
    bool ( *is_sd_mounted )( void );
    void ( *get_sd_path )( char** filename );
    /* machine */
    void ( *do_reset )( void );
    void ( *do_stop )( void );
    void ( *do_sleep )( void );
    void ( *do_wake )( void );
    /* debugger */
    void ( *do_debug )( void );
} ui4x_emulator_api_t;

extern int ui_get_lcd_height( void );
extern int ui_get_nb_keys( void );

extern void ui_handle_pending_inputs( void );
extern void ui_refresh_output( void );

extern void init_ui( ui4x_config_t* opt, ui4x_emulator_api_t* emulator_api );
extern void exit_ui( void );

#endif /* !_UI4x_API_H */
