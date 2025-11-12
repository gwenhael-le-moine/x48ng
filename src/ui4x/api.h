#ifndef _UI4x_API_H
#  define _UI4x_API_H 1

#  include <stdbool.h>

// LCD
#  define NIBBLES_PER_ROW 34
#  define LCD_WIDTH 131
#  define LCD_HEIGHT ( ui4x_config.big_screen ? 80 : 64 )

// HP 48{G,S}X Keys
typedef enum {
    HP48_KEY_A = 0,
    HP48_KEY_B,
    HP48_KEY_C,
    HP48_KEY_D,
    HP48_KEY_E,
    HP48_KEY_F,
    HP48_KEY_MTH,
    HP48_KEY_PRG,
    HP48_KEY_CST,
    HP48_KEY_VAR,
    HP48_KEY_UP,
    HP48_KEY_NXT,
    HP48_KEY_QUOTE,
    HP48_KEY_STO,
    HP48_KEY_EVAL,
    HP48_KEY_LEFT,
    HP48_KEY_DOWN,
    HP48_KEY_RIGHT,
    HP48_KEY_SIN,
    HP48_KEY_COS,
    HP48_KEY_TAN,
    HP48_KEY_SQRT,
    HP48_KEY_POWER,
    HP48_KEY_INV,
    HP48_KEY_ENTER,
    HP48_KEY_NEG,
    HP48_KEY_EEX,
    HP48_KEY_DEL,
    HP48_KEY_BS,
    HP48_KEY_ALPHA,
    HP48_KEY_7,
    HP48_KEY_8,
    HP48_KEY_9,
    HP48_KEY_DIV,
    HP48_KEY_SHL,
    HP48_KEY_4,
    HP48_KEY_5,
    HP48_KEY_6,
    HP48_KEY_MUL,
    HP48_KEY_SHR,
    HP48_KEY_1,
    HP48_KEY_2,
    HP48_KEY_3,
    HP48_KEY_MINUS,
    HP48_KEY_ON,
    HP48_KEY_0,
    HP48_KEY_PERIOD,
    HP48_KEY_SPC,
    HP48_KEY_PLUS,
    NB_HP48_KEYS
} hp48_keynames_t;

// HP 4{0,9}G Keys
typedef enum {
    HP49_KEY_A = 0,
    HP49_KEY_B,
    HP49_KEY_C,
    HP49_KEY_D,
    HP49_KEY_E,
    HP49_KEY_F,
    HP49_KEY_APPS,
    HP49_KEY_MODE,
    HP49_KEY_TOOL,
    HP49_KEY_VAR,
    HP49_KEY_STO,
    HP49_KEY_NXT,
    HP49_KEY_LEFT,
    HP49_KEY_UP,
    HP49_KEY_RIGHT,
    HP49_KEY_DOWN,
    HP49_KEY_HIST,
    HP49_KEY_CAT,
    HP49_KEY_EQW,
    HP49_KEY_SYMB,
    HP49_KEY_BS,
    HP49_KEY_POWER,
    HP49_KEY_SQRT,
    HP49_KEY_SIN,
    HP49_KEY_COS,
    HP49_KEY_TAN,
    HP49_KEY_EEX,
    HP49_KEY_NEG,
    HP49_KEY_X,
    HP49_KEY_INV,
    HP49_KEY_DIV,
    HP49_KEY_ALPHA,
    HP49_KEY_7,
    HP49_KEY_8,
    HP49_KEY_9,
    HP49_KEY_MUL,
    HP49_KEY_SHL,
    HP49_KEY_4,
    HP49_KEY_5,
    HP49_KEY_6,
    HP49_KEY_MINUS,
    HP49_KEY_SHR,
    HP49_KEY_1,
    HP49_KEY_2,
    HP49_KEY_3,
    HP49_KEY_PLUS,
    HP49_KEY_ON,
    HP49_KEY_0,
    HP49_KEY_PERIOD,
    HP49_KEY_SPC,
    HP49_KEY_ENTER,
    NB_HP49_KEYS
} hp49_keynames_t;

#  define NB_KEYS ( ui4x_config.model == MODEL_48GX || ui4x_config.model == MODEL_48SX ? NB_HP48_KEYS : NB_HP49_KEYS )

typedef enum { FRONTEND_SDL, FRONTEND_NCURSES, FRONTEND_GTK } ui4x_frontend_t;

typedef enum { MODEL_48SX = 485, MODEL_48GX = 486, MODEL_40G = 406, MODEL_49G = 496, MODEL_50G = 506 } ui4x_model_t;

typedef struct ui4x_config_t {
    ui4x_model_t model;
    bool shiftless;
    bool big_screen;
    bool black_lcd;

    ui4x_frontend_t frontend;
    bool mono;
    bool gray;

    bool chromeless;
    bool fullscreen;
    double scale;

    bool tiny;
    bool small;

    bool verbose;

    char* progname;
    char* wire_name;
    char* ir_name;
} ui4x_config_t;

extern ui4x_config_t ui4x_config;

/*************************************************/
/* public API: if it's there it's used elsewhere */
/*************************************************/
extern void ( *ui_get_event )( void );
extern void ( *ui_update_display )( void );

extern void ( *ui_start )( void );
extern void ( *ui_stop )( void );

extern void setup_ui( ui4x_config_t* conf, void ( *emulator_api_press_key )( int hpkey ), void ( *emulator_api_release_key )( int hpkey ),
                      bool ( *emulator_api_is_key_pressed )( int hpkey ), unsigned char ( *emulator_api_get_annunciators )( void ),
                      bool ( *emulator_api_get_display_state )( void ), void ( *emulator_api_get_lcd_buffer )( int* target ),
                      int ( *emulator_api_get_contrast )( void ), void ( *exit_emulator )( void ) );
extern void close_and_exit( void );

#endif /* !_UI4x_API_H */
