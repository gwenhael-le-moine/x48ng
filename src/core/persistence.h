#ifndef _PERSISTENCE_H
#  define _PERSISTENCE_H 1

#  include <stdbool.h>

extern bool port1_is_ram;
extern long port1_mask;
extern bool port2_is_ram;
extern long port2_mask;

extern int read_rom( const char* fname );
extern int read_files( void );      /* used in debugger.c */
extern int write_files( void );     /* used in debugger.c */

#endif /* !_PERSISTENCE_H */
