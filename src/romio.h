#ifndef _ROMIO_H
#define _ROMIO_H 1

#include "global.h"

#define ROM_SIZE_SX 0x080000
#define ROM_SIZE_GX 0x100000

extern unsigned int opt_gx;
extern unsigned int rom_size;

extern int read_rom_file( char* name, unsigned char** mem, unsigned int* size );

#endif /* !_ROMIO_H */
