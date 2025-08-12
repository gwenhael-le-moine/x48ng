#ifndef _OPTIONS_H
#define _OPTIONS_H 1

#include <stdbool.h>

typedef enum { FRONTEND_TEXT, FRONTEND_SDL } frontends_t;

typedef struct {
    char* progname;

    bool verbose;
    bool shiftless;

    bool print_config;
    bool useTerminal;
    bool useSerial;
    bool useDebugger;
    bool throttle;
    bool resetOnStartup;
    int frontend_type;

    bool inhibit_shutdown;

    bool mono;
    bool gray;

    /* sdl */
    bool chromeless;
    bool fullscreen;
    double scale;

    /* tui */
    bool tiny;
    bool small;

    char* serialLine;
} config_t;
extern config_t config;

#define MAX_LENGTH_FILENAME 2048
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
extern int config_init( int argc, char* argv[] );

#endif /* !_OPTIONS_H */
