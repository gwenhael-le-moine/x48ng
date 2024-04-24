#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "emulator.h"
#include "emulator_inner.h"
#include "romio.h"
#include "config.h"

#define X48_MAGIC 0x48503438
#define NB_CONFIG 8

#define RAM_SIZE_SX 0x10000
#define RAM_SIZE_GX 0x40000

bool please_exit = false;
bool save_before_exit = true;

bool rom_is_new = true;
long ram_size;
long port1_size;
long port1_mask;
bool port1_is_ram;
long port2_size;
long port2_mask;
bool port2_is_ram;

hpkey_t keyboard[ 49 ] = {
    /* From top left to bottom right */
    {0x14,   0},
    {0x84,   0},
    {0x83,   0},
    {0x82,   0},
    {0x81,   0},
    {0x80,   0},

    {0x24,   0},
    {0x74,   0},
    {0x73,   0},
    {0x72,   0},
    {0x71,   0},
    {0x70,   0},

    {0x04,   0},
    {0x64,   0},
    {0x63,   0},
    {0x62,   0},
    {0x61,   0},
    {0x60,   0},

    {0x34,   0},
    {0x54,   0},
    {0x53,   0},
    {0x52,   0},
    {0x51,   0},
    {0x50,   0},

    {0x44,   0},
    {0x43,   0},
    {0x42,   0},
    {0x41,   0},
    {0x40,   0},

    {0x35,   0},
    {0x33,   0},
    {0x32,   0},
    {0x31,   0},
    {0x30,   0},

    {0x25,   0},
    {0x23,   0},
    {0x22,   0},
    {0x21,   0},
    {0x20,   0},

    {0x15,   0},
    {0x13,   0},
    {0x12,   0},
    {0x11,   0},
    {0x10,   0},

    {0x8000, 0},
    {0x03,   0},
    {0x02,   0},
    {0x01,   0},
    {0x00,   0},
};

int annunciators_bits[ NB_ANNUNCIATORS ] = { ANN_LEFT, ANN_RIGHT, ANN_ALPHA, ANN_BATTERY, ANN_BUSY, ANN_IO };

void saturn_config_init( void )
{
    saturn.version[ 0 ] = VERSION_MAJOR;
    saturn.version[ 1 ] = VERSION_MINOR;
    saturn.version[ 2 ] = PATCHLEVEL;
    memset( &device, 0, sizeof( device ) );
    device.display_touched = true;
    device.contrast_touched = true;
    device.baud_touched = true;
    device.ann_touched = true;
    saturn.rcs = 0x0;
    saturn.tcs = 0x0;
    saturn.lbr = 0x0;
}

void init_saturn( void )
{
    memset( &saturn, 0, sizeof( saturn ) - 4 * sizeof( unsigned char* ) );
    saturn.PC = 0x00000;
    saturn.magic = X48_MAGIC;
    saturn.t1_tick = 8192;
    saturn.t2_tick = 16;
    saturn.i_per_s = 0;
    saturn.version[ 0 ] = VERSION_MAJOR;
    saturn.version[ 1 ] = VERSION_MINOR;
    saturn.version[ 2 ] = PATCHLEVEL;
    saturn.hexmode = HEX;
    saturn.rstkp = -1;
    saturn.interruptable = true;
    saturn.int_pending = false;
    saturn.kbd_ien = true;
    saturn.timer1 = 0;
    saturn.timer2 = 0x2000;
    saturn.bank_switch = 0;
    for ( int i = 0; i < NB_MCTL; i++ ) {
        if ( i == 0 )
            saturn.mem_cntl[ i ].unconfigured = 1;
        else if ( i == 5 )
            saturn.mem_cntl[ i ].unconfigured = 0;
        else
            saturn.mem_cntl[ i ].unconfigured = 2;
        saturn.mem_cntl[ i ].config[ 0 ] = 0;
        saturn.mem_cntl[ i ].config[ 1 ] = 0;
    }
    dev_memory_init();
}

/***********************************************/
/* READING ~/.config/x48ng/{rom,ram,state,port1,port2} */
/***********************************************/

int read_8( FILE* fp, word_8* var )
{
    unsigned char tmp;

    if ( fread( &tmp, 1, 1, fp ) != 1 ) {
        if ( config.verbose )
            fprintf( stderr, "can\'t read word_8\n" );
        return 0;
    }
    *var = tmp;
    return 1;
}

int read_char( FILE* fp, char* var )
{
    char tmp;

    if ( fread( &tmp, 1, 1, fp ) != 1 ) {
        if ( config.verbose )
            fprintf( stderr, "can\'t read char\n" );
        return 0;
    }
    *var = tmp;
    return 1;
}

int read_16( FILE* fp, word_16* var )
{
    unsigned char tmp[ 2 ];

    if ( fread( &tmp[ 0 ], 1, 2, fp ) != 2 ) {
        if ( config.verbose )
            fprintf( stderr, "can\'t read word_16\n" );
        return 0;
    }
    *var = tmp[ 0 ] << 8;
    *var |= tmp[ 1 ];
    return 1;
}

int read_32( FILE* fp, word_32* var )
{
    unsigned char tmp[ 4 ];

    if ( fread( &tmp[ 0 ], 1, 4, fp ) != 4 ) {
        if ( config.verbose )
            fprintf( stderr, "can\'t read word_32\n" );
        return 0;
    }
    *var = tmp[ 0 ] << 24;
    *var |= tmp[ 1 ] << 16;
    *var |= tmp[ 2 ] << 8;
    *var |= tmp[ 3 ];
    return 1;
}

int read_u_long( FILE* fp, unsigned long* var )
{
    unsigned char tmp[ 4 ];

    if ( fread( &tmp[ 0 ], 1, 4, fp ) != 4 ) {
        if ( config.verbose )
            fprintf( stderr, "can\'t read unsigned long\n" );
        return 0;
    }
    *var = tmp[ 0 ] << 24;
    *var |= tmp[ 1 ] << 16;
    *var |= tmp[ 2 ] << 8;
    *var |= tmp[ 3 ];
    return 1;
}

int read_state_file( FILE* fp )
{
    int i;

    /*
     * version 0.4.x, read in the saturn_t struct
     */
    for ( i = 0; i < 16; i++ )
        if ( !read_8( fp, &saturn.A[ i ] ) )
            return 0;
    for ( i = 0; i < 16; i++ )
        if ( !read_8( fp, &saturn.B[ i ] ) )
            return 0;
    for ( i = 0; i < 16; i++ )
        if ( !read_8( fp, &saturn.C[ i ] ) )
            return 0;
    for ( i = 0; i < 16; i++ )
        if ( !read_8( fp, &saturn.D[ i ] ) )
            return 0;
    if ( !read_32( fp, &saturn.d[ 0 ] ) )
        return 0;
    if ( !read_32( fp, &saturn.d[ 1 ] ) )
        return 0;
    if ( !read_8( fp, &saturn.P ) )
        return 0;
    if ( !read_32( fp, &saturn.PC ) )
        return 0;
    for ( i = 0; i < 16; i++ )
        if ( !read_8( fp, &saturn.R0[ i ] ) )
            return 0;
    for ( i = 0; i < 16; i++ )
        if ( !read_8( fp, &saturn.R1[ i ] ) )
            return 0;
    for ( i = 0; i < 16; i++ )
        if ( !read_8( fp, &saturn.R2[ i ] ) )
            return 0;
    for ( i = 0; i < 16; i++ )
        if ( !read_8( fp, &saturn.R3[ i ] ) )
            return 0;
    for ( i = 0; i < 16; i++ )
        if ( !read_8( fp, &saturn.R4[ i ] ) )
            return 0;
    for ( i = 0; i < 4; i++ )
        if ( !read_8( fp, &saturn.IN[ i ] ) )
            return 0;
    for ( i = 0; i < 3; i++ )
        if ( !read_8( fp, &saturn.OUT[ i ] ) )
            return 0;
    if ( !read_8( fp, &saturn.CARRY ) )
        return 0;
    for ( i = 0; i < NB_PSTAT; i++ )
        if ( !read_8( fp, &saturn.PSTAT[ i ] ) )
            return 0;
    if ( !read_8( fp, &saturn.XM ) )
        return 0;
    if ( !read_8( fp, &saturn.SB ) )
        return 0;
    if ( !read_8( fp, &saturn.SR ) )
        return 0;
    if ( !read_8( fp, &saturn.MP ) )
        return 0;
    if ( !read_8( fp, &saturn.hexmode ) )
        return 0;
    for ( i = 0; i < NB_RSTK; i++ )
        if ( !read_32( fp, &saturn.RSTK[ i ] ) )
            return 0;
    if ( !read_16( fp, ( word_16* )&saturn.rstkp ) )
        return 0;
    for ( i = 0; i < 9; i++ )
        if ( !read_16( fp, ( word_16* )&saturn.keybuf.rows[ i ] ) )
            return 0;
    if ( !read_8( fp, &saturn.interruptable ) )
        return 0;
    if ( !read_8( fp, &saturn.int_pending ) )
        return 0;
    if ( !read_8( fp, &saturn.kbd_ien ) )
        return 0;
    if ( !read_8( fp, &saturn.disp_io ) )
        return 0;
    if ( !read_8( fp, &saturn.contrast_ctrl ) )
        return 0;
    if ( !read_8( fp, &saturn.disp_test ) )
        return 0;
    if ( !read_16( fp, &saturn.crc ) )
        return 0;
    if ( !read_8( fp, &saturn.power_status ) )
        return 0;
    if ( !read_8( fp, &saturn.power_ctrl ) )
        return 0;
    if ( !read_8( fp, &saturn.mode ) )
        return 0;
    if ( !read_8( fp, &saturn.annunc ) )
        return 0;
    if ( !read_8( fp, &saturn.baud ) )
        return 0;
    if ( !read_8( fp, &saturn.card_ctrl ) )
        return 0;
    if ( !read_8( fp, &saturn.card_status ) )
        return 0;
    if ( !read_8( fp, &saturn.io_ctrl ) )
        return 0;
    if ( !read_8( fp, &saturn.rcs ) )
        return 0;
    if ( !read_8( fp, &saturn.tcs ) )
        return 0;
    if ( !read_8( fp, &saturn.rbr ) )
        return 0;
    if ( !read_8( fp, &saturn.tbr ) )
        return 0;
    if ( !read_8( fp, &saturn.sreq ) )
        return 0;
    if ( !read_8( fp, &saturn.ir_ctrl ) )
        return 0;
    if ( !read_8( fp, &saturn.base_off ) )
        return 0;
    if ( !read_8( fp, &saturn.lcr ) )
        return 0;
    if ( !read_8( fp, &saturn.lbr ) )
        return 0;
    if ( !read_8( fp, &saturn.scratch ) )
        return 0;
    if ( !read_8( fp, &saturn.base_nibble ) )
        return 0;
    if ( !read_32( fp, &saturn.disp_addr ) )
        return 0;
    if ( !read_16( fp, &saturn.line_offset ) )
        return 0;
    if ( !read_8( fp, &saturn.line_count ) )
        return 0;
    if ( !read_16( fp, &saturn.unknown ) )
        return 0;
    if ( !read_8( fp, &saturn.t1_ctrl ) )
        return 0;
    if ( !read_8( fp, &saturn.t2_ctrl ) )
        return 0;
    if ( !read_32( fp, &saturn.menu_addr ) )
        return 0;
    if ( !read_8( fp, &saturn.unknown2 ) )
        return 0;
    if ( !read_char( fp, &saturn.timer1 ) )
        return 0;
    if ( !read_32( fp, &saturn.timer2 ) )
        return 0;
    if ( !read_32( fp, &saturn.t1_instr ) )
        return 0;
    if ( !read_32( fp, &saturn.t2_instr ) )
        return 0;
    if ( !read_16( fp, ( word_16* )&saturn.t1_tick ) )
        return 0;
    if ( !read_16( fp, ( word_16* )&saturn.t2_tick ) )
        return 0;
    if ( !read_32( fp, &saturn.i_per_s ) )
        return 0;
    if ( !read_16( fp, ( word_16* )&saturn.bank_switch ) )
        return 0;
    for ( i = 0; i < NB_MCTL; i++ ) {
        if ( !read_16( fp, &saturn.mem_cntl[ i ].unconfigured ) )
            return 0;
        if ( !read_32( fp, &saturn.mem_cntl[ i ].config[ 0 ] ) )
            return 0;
        if ( !read_32( fp, &saturn.mem_cntl[ i ].config[ 1 ] ) )
            return 0;
    }
    return 1;
}

int read_mem_file( char* name, word_4* mem, int size )
{
    struct stat st;
    FILE* fp;
    word_8* tmp_mem;
    word_8 byte;
    int i, j;

    if ( NULL == ( fp = fopen( name, "r" ) ) ) {
        if ( config.verbose )
            fprintf( stderr, "ct open %s\n", name );
        return 0;
    }

    if ( stat( name, &st ) < 0 ) {
        if ( config.verbose )
            fprintf( stderr, "can\'t stat %s\n", name );
        return 0;
    }

    if ( st.st_size == size ) {
        /*
         * size is same as memory size, old version file
         */
        if ( fread( mem, 1, ( size_t )size, fp ) != ( unsigned long )size ) {
            if ( config.verbose )
                fprintf( stderr, "can\'t read %s\n", name );
            fclose( fp );
            return 0;
        }
    } else {
        /*
         * size is different, check size and decompress memory
         */

        if ( st.st_size != size / 2 ) {
            if ( config.verbose )
                fprintf( stderr, "strange size %s, expected %d, found %ld\n", name, size / 2, st.st_size );
            fclose( fp );
            return 0;
        }

        if ( NULL == ( tmp_mem = ( word_8* )malloc( ( size_t )st.st_size ) ) ) {
            for ( i = 0, j = 0; i < size / 2; i++ ) {
                if ( 1 != fread( &byte, 1, 1, fp ) ) {
                    if ( config.verbose )
                        fprintf( stderr, "can\'t read %s\n", name );
                    fclose( fp );
                    return 0;
                }
                mem[ j++ ] = ( word_4 )( ( int )byte & 0xf );
                mem[ j++ ] = ( word_4 )( ( ( int )byte >> 4 ) & 0xf );
            }
        } else {
            if ( fread( tmp_mem, 1, ( size_t )size / 2, fp ) != ( unsigned long )( size / 2 ) ) {
                if ( config.verbose )
                    fprintf( stderr, "can\'t read %s\n", name );
                fclose( fp );
                free( tmp_mem );
                return 0;
            }

            for ( i = 0, j = 0; i < size / 2; i++ ) {
                mem[ j++ ] = ( word_4 )( ( int )tmp_mem[ i ] & 0xf );
                mem[ j++ ] = ( word_4 )( ( ( int )tmp_mem[ i ] >> 4 ) & 0xf );
            }

            free( tmp_mem );
        }
    }

    fclose( fp );

    if ( config.verbose )
        printf( "read %s\n", name );

    return 1;
}

int read_files( void )
{
    unsigned long v1, v2;
    int i, read_version;
    int ram_size;
    struct stat st;
    FILE* fp;

    /*************************************************/
    /* 1. read ROM from ~/.x48ng/rom into saturn.rom */
    /*************************************************/
    saturn.rom = ( word_4* )NULL;
    if ( !read_rom_file( normalized_rom_path, &saturn.rom, &rom_size ) )
        return 0;

    if ( config.verbose )
        printf( "read %s\n", normalized_rom_path );

    rom_is_new = false;

    /**************************************************/
    /* 2. read saved state from ~/.x48ng/state into fp */
    /**************************************************/
    if ( NULL == ( fp = fopen( normalized_state_path, "r" ) ) ) {
        if ( config.verbose )
            fprintf( stderr, "can\'t open %s\n", normalized_state_path );
        return 0;
    }

    /*
     * ok, file is open, try to read the MAGIC number
     */
    read_u_long( fp, &saturn.magic );

    if ( X48_MAGIC != saturn.magic ) {
        /*
         * no MAGIC number, exiting
         */
        fprintf( stderr, "You seem to try to load an old saved state. Try "
                         "running a recent x48 (0.6.4) before trying x48ng." );
        exit( 99 );
    } else {
        /*
         * MAGIC ok, read and compare the version
         */
        read_version = 1;
        for ( i = 0; i < 4; i++ ) {
            if ( !read_char( fp, &saturn.version[ i ] ) ) {
                if ( config.verbose )
                    fprintf( stderr, "can\'t read version\n" );
                read_version = 0;
            }
        }

        if ( read_version ) {
            v1 = ( ( int )saturn.version[ 0 ] & 0xff ) << 24;
            v1 |= ( ( int )saturn.version[ 1 ] & 0xff ) << 16;
            v1 |= ( ( int )saturn.version[ 2 ] & 0xff ) << 8;
            v1 |= ( ( int )saturn.version[ 3 ] & 0xff );
            v2 = ( ( int )VERSION_MAJOR & 0xff ) << 24;
            v2 |= ( ( int )VERSION_MINOR & 0xff ) << 16;
            v2 |= ( ( int )PATCHLEVEL & 0xff ) << 8;

            /*
             * try to read latest version file
             */
            if ( !read_state_file( fp ) ) {
                if ( config.verbose )
                    fprintf( stderr, "can\'t handle %s\n", normalized_state_path );
                init_saturn();
            } else if ( config.verbose )
                printf( "read %s\n", normalized_state_path );
        }
    }
    fclose( fp );

    dev_memory_init();

    saturn_config_init();

    ram_size = opt_gx ? RAM_SIZE_GX : RAM_SIZE_SX;

    saturn.ram = ( word_4* )NULL;
    if ( NULL == ( saturn.ram = ( word_4* )malloc( ram_size ) ) ) {
        if ( config.verbose )
            fprintf( stderr, "can\'t malloc RAM[%d]\n", ram_size );
        exit( 1 );
    }

    /*************************************************/
    /* 3. read RAM from ~/.x48ng/ram into saturn.ram */
    /*************************************************/
    if ( ( fp = fopen( normalized_ram_path, "r" ) ) == NULL ) {
        if ( config.verbose )
            fprintf( stderr, "can\'t open %s\n", normalized_ram_path );
        return 0;
    }
    if ( !read_mem_file( normalized_ram_path, saturn.ram, ram_size ) )
        return 0;

    /**************************************/
    /* Onto reading port1 and port2 cards */
    /**************************************/
    saturn.card_status = 0;

    /********************************************************/
    /* 4. read card 1 from ~/.x48ng/port1 into saturn.port1 */
    /********************************************************/
    port1_size = 0;
    port1_mask = 0;
    port1_is_ram = 0;
    saturn.port1 = ( unsigned char* )0;

    if ( stat( normalized_port1_path, &st ) >= 0 ) {
        port1_size = 2 * st.st_size;
        if ( ( port1_size == 0x10000 ) || ( port1_size == 0x40000 ) ) {
            if ( NULL == ( saturn.port1 = ( word_4* )malloc( port1_size ) ) ) {
                if ( config.verbose )
                    fprintf( stderr, "can\'t malloc PORT1[%ld]\n", port1_size );
            } else if ( !read_mem_file( normalized_port1_path, saturn.port1, port1_size ) ) {
                port1_size = 0;
                port1_is_ram = false;
            } else {
                port1_is_ram = st.st_mode & S_IWUSR;
                port1_mask = port1_size - 1;
            }
        }
    }

    if ( opt_gx ) {
        saturn.card_status |= ( port1_size > 0 ) ? 2 : 0;
        saturn.card_status |= port1_is_ram ? 8 : 0;
    } else {
        saturn.card_status |= ( port1_size > 0 ) ? 1 : 0;
        saturn.card_status |= port1_is_ram ? 4 : 0;
    }

    /********************************************************/
    /* 5. read card 2 from ~/.x48ng/port2 into saturn.port2 */
    /********************************************************/
    port2_size = 0;
    port2_mask = 0;
    port2_is_ram = false;
    saturn.port2 = ( unsigned char* )0;

    if ( stat( normalized_port2_path, &st ) >= 0 ) {
        port2_size = 2 * st.st_size;
        if ( ( opt_gx && ( ( port2_size % 0x40000 ) == 0 ) ) ||
             ( !opt_gx && ( ( port2_size == 0x10000 ) || ( port2_size == 0x40000 ) ) ) ) {
            if ( NULL == ( saturn.port2 = ( word_4* )malloc( port2_size ) ) ) {
                if ( config.verbose )
                    fprintf( stderr, "can\'t malloc PORT2[%ld]\n", port2_size );
            } else if ( !read_mem_file( normalized_port2_path, saturn.port2, port2_size ) ) {
                port2_size = 0;
                port2_is_ram = false;
            } else {
                port2_is_ram = st.st_mode & S_IWUSR;
                port2_mask = port2_size - 1;
            }
        }
    }

    if ( opt_gx ) {
        saturn.card_status |= ( port2_size > 0 ) ? 1 : 0;
        saturn.card_status |= port2_is_ram ? 4 : 0;
    } else {
        saturn.card_status |= ( port2_size > 0 ) ? 2 : 0;
        saturn.card_status |= port2_is_ram ? 8 : 0;
    }

    /************************/
    /* All files are loaded */
    /************************/
    return 1;
}

/***********************************************/
/* WRITING ~/.x48ng/{rom,ram,state,port1,port2} */
/***********************************************/

int write_8( FILE* fp, word_8* var )
{
    unsigned char tmp;

    tmp = *var;
    if ( fwrite( &tmp, 1, 1, fp ) != 1 ) {
        if ( config.verbose )
            fprintf( stderr, "can\'t write word_8\n" );
        return 0;
    }
    return 1;
}

int write_char( FILE* fp, char* var )
{
    char tmp;

    tmp = *var;
    if ( fwrite( &tmp, 1, 1, fp ) != 1 ) {
        if ( config.verbose )
            fprintf( stderr, "can\'t write char\n" );
        return 0;
    }
    return 1;
}

int write_16( FILE* fp, word_16* var )
{
    unsigned char tmp[ 2 ];

    tmp[ 0 ] = ( *var >> 8 ) & 0xff;
    tmp[ 1 ] = *var & 0xff;
    if ( fwrite( &tmp[ 0 ], 1, 2, fp ) != 2 ) {
        if ( config.verbose )
            fprintf( stderr, "can\'t write word_16\n" );
        return 0;
    }
    return 1;
}

int write_32( FILE* fp, word_32* var )
{
    unsigned char tmp[ 4 ];

    tmp[ 0 ] = ( *var >> 24 ) & 0xff;
    tmp[ 1 ] = ( *var >> 16 ) & 0xff;
    tmp[ 2 ] = ( *var >> 8 ) & 0xff;
    tmp[ 3 ] = *var & 0xff;
    if ( fwrite( &tmp[ 0 ], 1, 4, fp ) != 4 ) {
        if ( config.verbose )
            fprintf( stderr, "can\'t write word_32\n" );
        return 0;
    }
    return 1;
}

int write_u_long( FILE* fp, unsigned long* var )
{
    unsigned char tmp[ 4 ];

    tmp[ 0 ] = ( *var >> 24 ) & 0xff;
    tmp[ 1 ] = ( *var >> 16 ) & 0xff;
    tmp[ 2 ] = ( *var >> 8 ) & 0xff;
    tmp[ 3 ] = *var & 0xff;
    if ( fwrite( &tmp[ 0 ], 1, 4, fp ) != 4 ) {
        if ( config.verbose )
            fprintf( stderr, "can\'t write unsigned long\n" );
        return 0;
    }
    return 1;
}

int write_mem_file( char* name, word_4* mem, int size )
{
    FILE* fp;
    word_8* tmp_mem;
    word_8 byte;
    int i, j;

    if ( NULL == ( fp = fopen( name, "w" ) ) ) {
        if ( config.verbose )
            fprintf( stderr, "can\'t open %s\n", name );
        return 0;
    }

    if ( NULL == ( tmp_mem = ( word_8* )malloc( ( size_t )size / 2 ) ) ) {
        for ( i = 0, j = 0; i < size / 2; i++ ) {
            byte = ( mem[ j++ ] & 0x0f );
            byte |= ( mem[ j++ ] << 4 ) & 0xf0;
            if ( 1 != fwrite( &byte, 1, 1, fp ) ) {
                if ( config.verbose )
                    fprintf( stderr, "can\'t write %s\n", name );
                fclose( fp );
                return 0;
            }
        }
    } else {
        for ( i = 0, j = 0; i < size / 2; i++ ) {
            tmp_mem[ i ] = ( mem[ j++ ] & 0x0f );
            tmp_mem[ i ] |= ( mem[ j++ ] << 4 ) & 0xf0;
        }

        if ( fwrite( tmp_mem, 1, ( size_t )size / 2, fp ) != ( unsigned long )size / 2 ) {
            if ( config.verbose )
                fprintf( stderr, "can\'t write %s\n", name );
            fclose( fp );
            free( tmp_mem );
            return 0;
        }

        free( tmp_mem );
    }

    fclose( fp );

    if ( config.verbose )
        printf( "wrote %s\n", name );

    return 1;
}

int write_state_file( char* filename )
{
    int i;
    FILE* fp;

    if ( ( fp = fopen( filename, "w" ) ) == NULL ) {
        if ( config.verbose )
            fprintf( stderr, "can\'t open %s, no saving done\n", filename );
        return 0;
    }

    /*
     * write the state config file
     */
    write_32( fp, ( word_32* )&saturn.magic );
    for ( i = 0; i < 4; i++ )
        write_char( fp, &saturn.version[ i ] );
    for ( i = 0; i < 16; i++ )
        write_8( fp, &saturn.A[ i ] );
    for ( i = 0; i < 16; i++ )
        write_8( fp, &saturn.B[ i ] );
    for ( i = 0; i < 16; i++ )
        write_8( fp, &saturn.C[ i ] );
    for ( i = 0; i < 16; i++ )
        write_8( fp, &saturn.D[ i ] );
    write_32( fp, &saturn.d[ 0 ] );
    write_32( fp, &saturn.d[ 1 ] );
    write_8( fp, &saturn.P );
    write_32( fp, &saturn.PC );
    for ( i = 0; i < 16; i++ )
        write_8( fp, &saturn.R0[ i ] );
    for ( i = 0; i < 16; i++ )
        write_8( fp, &saturn.R1[ i ] );
    for ( i = 0; i < 16; i++ )
        write_8( fp, &saturn.R2[ i ] );
    for ( i = 0; i < 16; i++ )
        write_8( fp, &saturn.R3[ i ] );
    for ( i = 0; i < 16; i++ )
        write_8( fp, &saturn.R4[ i ] );
    for ( i = 0; i < 4; i++ )
        write_8( fp, &saturn.IN[ i ] );
    for ( i = 0; i < 3; i++ )
        write_8( fp, &saturn.OUT[ i ] );
    write_8( fp, &saturn.CARRY );
    for ( i = 0; i < NB_PSTAT; i++ )
        write_8( fp, &saturn.PSTAT[ i ] );
    write_8( fp, &saturn.XM );
    write_8( fp, &saturn.SB );
    write_8( fp, &saturn.SR );
    write_8( fp, &saturn.MP );
    write_8( fp, &saturn.hexmode );
    for ( i = 0; i < NB_RSTK; i++ )
        write_32( fp, &saturn.RSTK[ i ] );
    write_16( fp, ( word_16* )&saturn.rstkp );
    for ( i = 0; i < 9; i++ )
        write_16( fp, ( word_16* )&saturn.keybuf.rows[ i ] );
    write_8( fp, &saturn.interruptable );
    write_8( fp, &saturn.int_pending );
    write_8( fp, &saturn.kbd_ien );
    write_8( fp, &saturn.disp_io );
    write_8( fp, &saturn.contrast_ctrl );
    write_8( fp, &saturn.disp_test );
    write_16( fp, &saturn.crc );
    write_8( fp, &saturn.power_status );
    write_8( fp, &saturn.power_ctrl );
    write_8( fp, &saturn.mode );
    write_8( fp, &saturn.annunc );
    write_8( fp, &saturn.baud );
    write_8( fp, &saturn.card_ctrl );
    write_8( fp, &saturn.card_status );
    write_8( fp, &saturn.io_ctrl );
    write_8( fp, &saturn.rcs );
    write_8( fp, &saturn.tcs );
    write_8( fp, &saturn.rbr );
    write_8( fp, &saturn.tbr );
    write_8( fp, &saturn.sreq );
    write_8( fp, &saturn.ir_ctrl );
    write_8( fp, &saturn.base_off );
    write_8( fp, &saturn.lcr );
    write_8( fp, &saturn.lbr );
    write_8( fp, &saturn.scratch );
    write_8( fp, &saturn.base_nibble );
    write_32( fp, &saturn.disp_addr );
    write_16( fp, &saturn.line_offset );
    write_8( fp, &saturn.line_count );
    write_16( fp, &saturn.unknown );
    write_8( fp, &saturn.t1_ctrl );
    write_8( fp, &saturn.t2_ctrl );
    write_32( fp, &saturn.menu_addr );
    write_8( fp, &saturn.unknown2 );
    write_char( fp, &saturn.timer1 );
    write_32( fp, &saturn.timer2 );
    write_32( fp, &saturn.t1_instr );
    write_32( fp, &saturn.t2_instr );
    write_16( fp, ( word_16* )&saturn.t1_tick );
    write_16( fp, ( word_16* )&saturn.t2_tick );
    write_32( fp, &saturn.i_per_s );
    write_16( fp, &saturn.bank_switch );
    for ( i = 0; i < NB_MCTL; i++ ) {
        write_16( fp, &saturn.mem_cntl[ i ].unconfigured );
        write_32( fp, &saturn.mem_cntl[ i ].config[ 0 ] );
        write_32( fp, &saturn.mem_cntl[ i ].config[ 1 ] );
    }

    fclose( fp );

    if ( config.verbose )
        printf( "wrote %s\n", filename );

    return 1;
}

int write_files( void )
{
    struct stat st;
    bool make_dir = false;
    int ram_size = opt_gx ? RAM_SIZE_GX : RAM_SIZE_SX;

    if ( stat( normalized_config_path, &st ) == -1 ) {
        if ( errno == ENOENT ) {
            make_dir = true;
        } else {
            if ( config.verbose )
                fprintf( stderr, "can\'t stat %s, saving to /tmp\n", normalized_config_path );
            strcpy( normalized_config_path, "/tmp" );
        }
    } else {
        if ( !S_ISDIR( st.st_mode ) ) {
            if ( config.verbose )
                fprintf( stderr, "%s is no directory, saving to /tmp\n", normalized_config_path );
            strcpy( normalized_config_path, "/tmp" );
        }
    }

    if ( make_dir ) {
        if ( mkdir( normalized_config_path, 0777 ) == -1 ) {
            if ( config.verbose )
                fprintf( stderr, "can\'t mkdir %s, saving to /tmp\n", normalized_config_path );
            strcpy( normalized_config_path, "/tmp" );
        }
    }

    if ( !write_state_file( normalized_state_path ) )
        return 0;

    rom_is_new = make_dir;
    if ( rom_is_new ) {
        char new_rom_path[ MAX_LENGTH_FILENAME ];

        strcpy( new_rom_path, normalized_config_path );
        strcat( new_rom_path, "rom" );

        if ( !write_mem_file( new_rom_path, saturn.rom, rom_size ) )
            return 0;

        if ( config.verbose )
            printf( "wrote %s\n", new_rom_path );
    }

    if ( !write_mem_file( normalized_ram_path, saturn.ram, ram_size ) )
        return 0;

    if ( ( port1_size > 0 ) && port1_is_ram ) {
        if ( !write_mem_file( normalized_port1_path, saturn.port1, port1_size ) )
            return 0;
    }

    if ( ( port2_size > 0 ) && port2_is_ram ) {
        if ( !write_mem_file( normalized_port2_path, saturn.port2, port2_size ) )
            return 0;
    }

    return 1;
}

int read_rom( const char* fname )
{
    int ram_size;

    if ( !read_rom_file( fname, &saturn.rom, &rom_size ) )
        return 0;

    if ( config.verbose )
        printf( "read %s\n", fname );

    dev_memory_init();

    ram_size = opt_gx ? RAM_SIZE_GX : RAM_SIZE_SX;

    if ( NULL == ( saturn.ram = ( word_4* )malloc( ram_size ) ) ) {
        if ( config.verbose )
            fprintf( stderr, "can\'t malloc RAM\n" );
        return 0;
    }

    memset( saturn.ram, 0, ram_size );

    port1_size = 0;
    port1_mask = 0;
    port1_is_ram = false;
    saturn.port1 = ( unsigned char* )0;

    port2_size = 0;
    port2_mask = 0;
    port2_is_ram = false;
    saturn.port2 = ( unsigned char* )0;

    saturn.card_status = 0;

    return 1;
}

void init_display( void )
{
    display.on = ( int )( saturn.disp_io & 0x8 ) >> 3;

    display.disp_start = ( saturn.disp_addr & 0xffffe );
    display.offset = ( saturn.disp_io & 0x7 );

    display.lines = ( saturn.line_count & 0x3f );
    if ( display.lines == 0 )
        display.lines = 63;

    if ( display.offset > 3 )
        display.nibs_per_line = ( NIBBLES_PER_ROW + saturn.line_offset + 2 ) & 0xfff;
    else
        display.nibs_per_line = ( NIBBLES_PER_ROW + saturn.line_offset ) & 0xfff;

    display.disp_end = display.disp_start + ( display.nibs_per_line * ( display.lines + 1 ) );

    display.menu_start = saturn.menu_addr;
    display.menu_end = saturn.menu_addr + 0x110;

    display.contrast = saturn.contrast_ctrl;
    display.contrast |= ( ( saturn.disp_test & 0x1 ) << 4 );
}

void start_emulator( void )
{
    please_exit = false;
    save_before_exit = true;

    /* If files are successfully read => return and let's go */
    if ( read_files() ) {
        if ( config.resetOnStartup )
            saturn.PC = 0x00000;
    } else {
        /* if files were not readable => initialize */
        if ( config.verbose )
            fprintf( stderr, "initialization of %s\n", normalized_config_path );

        init_saturn();
        if ( !read_rom( normalized_rom_path ) )
            exit( 1 ); /* can't read ROM */
    }

    init_serial();
    init_display();
}

void exit_emulator( void )
{
    if ( save_before_exit )
        write_files();
}
