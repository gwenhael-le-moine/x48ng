#ifndef _ROMIO_H
#define _ROMIO_H 1

extern unsigned int opt_gx;
extern unsigned int rom_size;

extern int read_rom_file( const char* name, unsigned char** mem,
                          unsigned int* size );

#endif /* !_ROMIO_H */
