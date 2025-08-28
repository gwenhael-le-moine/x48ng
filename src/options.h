#ifndef _OPTIONS_H
#  define _OPTIONS_H 1

#  include <stdbool.h>

typedef enum { FRONTEND_SDL, FRONTEND_NCURSES, FRONTEND_GTK } frontend_t;

typedef enum { MODEL_48SX = 485, MODEL_48GX = 486, MODEL_40G = 406, MODEL_49G = 496, MODEL_50G = 506 } model_t;

typedef struct {
    char* progname;

    model_t model;
    bool verbose;
    bool shiftless;
    bool big_screen;
    bool black_lcd;

    frontend_t frontend;
    bool mono;
    bool gray;

    /* sdl */
    bool chromeless;
    bool fullscreen;
    double scale;

    /* tui */
    bool tiny;
    bool small;

    char* wire_name;
    char* ir_name;

    bool print_config;
    bool useTerminal;
    bool useSerial;
    bool useDebugger;
    bool throttle;
    bool resetOnStartup;
    bool inhibit_shutdown;

    char* serialLine;
} config_t;
extern config_t config;

#  define MAX_LENGTH_FILENAME 2048
extern char normalized_config_path[ MAX_LENGTH_FILENAME ];
extern char normalized_config_file[ MAX_LENGTH_FILENAME ];
extern char normalized_rom_path[ MAX_LENGTH_FILENAME ];
extern char normalized_ram_path[ MAX_LENGTH_FILENAME ];
extern char normalized_state_path[ MAX_LENGTH_FILENAME ];
extern char normalized_port1_path[ MAX_LENGTH_FILENAME ];
extern char normalized_port2_path[ MAX_LENGTH_FILENAME ];

/*************/
/* functions */
/*************/
extern config_t* config_init( int argc, char* argv[] );

#endif /* !_OPTIONS_H */
