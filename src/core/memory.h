#ifndef _MEMORY_H
#  define _MEMORY_H 1

#  include "../ui4x/api.h"

#  include "types.h"

#  define NIBBLES_PER_ROW 34
#  define NIBBLES_NB_BITS 4

#  define DISP_ROWS 64
#  define NIBS_PER_BUFFER_ROW ( NIBBLES_PER_ROW + 2 )

#  define KEYS_BUFFER_SIZE 9

typedef struct display_t {
    int on;

    long disp_start;
    long disp_end;

    int offset;
    int lines;
    int nibs_per_line;

    int contrast;

    long menu_start;
    long menu_end;
} display_t;

extern long nibble_masks[ 16 ];

extern display_t display;

extern void ( *bus_write_nibble )( Address addr, Nibble val );
extern int ( *read_nibble_crc )( Address addr );
extern void dev_memory_init( void ); /*  */

extern Nibble ( *bus_fetch_nibble )( Address addr );       /* used in debugger.c; ui_*.c */
extern long read_nibbles( Address addr, int len ); /* used in debugger.c */

#endif /* !_MEMORY_H */
