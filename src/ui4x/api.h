#ifndef _UI4x_EMULATOR_H
#  define _UI4x_EMULATOR_H 1

#  include <stdbool.h>

#  include "../options.h"

// LCD
#  define LCD_WIDTH 131
#  define LCD_HEIGHT 64

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

#  define NB_KEYS NB_HP48_KEYS

// Annunciators
typedef enum {
    ANN_LEFT = 0x81,
    ANN_RIGHT = 0x82,
    ANN_ALPHA = 0x84,
    ANN_BATTERY = 0x88,
    ANN_BUSY = 0x90,
    ANN_IO = 0xa0,
    NB_ANNUNCIATORS = 6
} annunciators_bits_t;

/*************************************************/
/* public API: if it's there it's used elsewhere */
/*************************************************/
extern void press_key( int hpkey );
extern void release_key( int hpkey );
extern bool is_key_pressed( int hpkey );

extern void init_emulator( config_t* conf );
extern void exit_emulator( void );

extern unsigned char get_annunciators( void );
extern bool get_display_state( void );
extern void get_lcd_buffer( int* target );
extern int get_contrast( void );

#endif /* !_UI4x_EMULATOR_H */
