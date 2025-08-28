#ifndef _INIT_H
#  define _INIT_H 1

#  include <stdbool.h>

#  include "emulator_ui4x_api.h"

extern bool port1_is_ram;
extern long port1_mask;
extern bool port2_is_ram;
extern long port2_mask;

extern int annunciators_bits[ NB_ANNUNCIATORS ];
extern bool save_before_exit;

extern int read_files( void );      /* used in debugger.c */
extern int write_files( void );     /* used in debugger.c */
extern void start_emulator( void ); /* used in main.c */
extern void stop_emulator( void );  /* used in debugger.c; ui_*.c */

#endif /* !_INIT_H */
