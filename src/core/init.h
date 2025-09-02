#ifndef _INIT_H
#  define _INIT_H 1

#  include <stdbool.h>

#  include "ui4x/api.h"

#define X48_MAGIC 0x48503438

extern int annunciators_bits[ NB_ANNUNCIATORS ];
extern bool save_before_exit;

extern void init_saturn( void );
extern void saturn_config_init( void );

extern void start_emulator( void ); /* used in main.c */
extern void stop_emulator( void );  /* used in debugger.c; ui_*.c */

#endif /* !_INIT_H */
