#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>

#include "emulate.h"
#include "init.h"
#include "options.h"
#include "romio.h"
#include "types.h"

#define NB_CONFIG 8

#define RAM_SIZE_SX 0x10000
#define RAM_SIZE_GX 0x40000

long port1_mask;
bool port1_is_ram;
long port2_mask;
bool port2_is_ram;

static bool rom_is_new = true;
static long port1_size;
static long port2_size;

/***********************************************/
/* READING ~/.config/x48ng/{rom,ram,state,port1,port2} */
/***********************************************/

static int read_Byte( FILE* fp, Byte* var )
{
    unsigned char tmp;

    if ( fread( &tmp, 1, 1, fp ) != 1 ) {
        if ( config.verbose )
            fprintf( stderr, "can\'t read Byte\n" );
        return 0;
    }
    *var = tmp;
    return 1;
}

static int read_char( FILE* fp, char* var )
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

static int read_16( FILE* fp, word_16* var )
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

static int read_32( FILE* fp, word_32* var )
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

static int read_u_long( FILE* fp, unsigned long* var )
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

static int read_state_file( FILE* fp )
{
    int i;

    /*
     * version 0.4.x, read in the saturn_t struct
     */
    for ( i = 0; i < 16; i++ )
        if ( !read_Byte( fp, &saturn.reg[ A ][ i ] ) )
            return 0;
    for ( i = 0; i < 16; i++ )
        if ( !read_Byte( fp, &saturn.reg[ B ][ i ] ) )
            return 0;
    for ( i = 0; i < 16; i++ )
        if ( !read_Byte( fp, &saturn.reg[ C ][ i ] ) )
            return 0;
    for ( i = 0; i < 16; i++ )
        if ( !read_Byte( fp, &saturn.reg[ D ][ i ] ) )
            return 0;
    if ( !read_32( fp, &saturn.d[ 0 ] ) )
        return 0;
    if ( !read_32( fp, &saturn.d[ 1 ] ) )
        return 0;
    if ( !read_Byte( fp, &saturn.p ) )
        return 0;
    if ( !read_32( fp, &saturn.pc ) )
        return 0;
    for ( i = 0; i < 16; i++ )
        if ( !read_Byte( fp, &saturn.reg_r[ 0 ][ i ] ) )
            return 0;
    for ( i = 0; i < 16; i++ )
        if ( !read_Byte( fp, &saturn.reg_r[ 1 ][ i ] ) )
            return 0;
    for ( i = 0; i < 16; i++ )
        if ( !read_Byte( fp, &saturn.reg_r[ 2 ][ i ] ) )
            return 0;
    for ( i = 0; i < 16; i++ )
        if ( !read_Byte( fp, &saturn.reg_r[ 3 ][ i ] ) )
            return 0;
    for ( i = 0; i < 16; i++ )
        if ( !read_Byte( fp, &saturn.reg_r[ 4 ][ i ] ) )
            return 0;
    for ( i = 0; i < 4; i++ )
        if ( !read_Byte( fp, &saturn.in[ i ] ) )
            return 0;
    for ( i = 0; i < 3; i++ )
        if ( !read_Byte( fp, &saturn.out[ i ] ) )
            return 0;
    if ( !read_Byte( fp, &saturn.carry ) )
        return 0;
    for ( i = 0; i < NB_PSTAT; i++ )
        if ( !read_Byte( fp, &saturn.pstat[ i ] ) )
            return 0;
    if ( !read_Byte( fp, &saturn.st[ XM ] ) )
        return 0;
    if ( !read_Byte( fp, &saturn.st[ SB ] ) )
        return 0;
    if ( !read_Byte( fp, &saturn.st[ SR ] ) )
        return 0;
    if ( !read_Byte( fp, &saturn.st[ MP ] ) )
        return 0;
    if ( !read_Byte( fp, &saturn.hexmode ) )
        return 0;
    for ( i = 0; i < NB_RSTK; i++ )
        if ( !read_32( fp, &saturn.rstk[ i ] ) )
            return 0;
    if ( !read_16( fp, ( word_16* )&saturn.rstk_ptr ) )
        return 0;
    for ( i = 0; i < KEYS_BUFFER_SIZE; i++ )
        if ( !read_16( fp, ( word_16* )&saturn.keybuf[ i ] ) )
            return 0;
    if ( !read_Byte( fp, &saturn.interruptable ) )
        return 0;
    if ( !read_Byte( fp, &saturn.int_pending ) )
        return 0;
    if ( !read_Byte( fp, &saturn.kbd_ien ) )
        return 0;
    if ( !read_Byte( fp, &saturn.disp_io ) )
        return 0;
    if ( !read_Byte( fp, &saturn.contrast_ctrl ) )
        return 0;
    if ( !read_Byte( fp, &saturn.disp_test ) )
        return 0;
    if ( !read_16( fp, &saturn.crc ) )
        return 0;
    if ( !read_Byte( fp, &saturn.power_status ) )
        return 0;
    if ( !read_Byte( fp, &saturn.power_ctrl ) )
        return 0;
    if ( !read_Byte( fp, &saturn.mode ) )
        return 0;
    if ( !read_Byte( fp, &saturn.annunc ) )
        return 0;
    if ( !read_Byte( fp, &saturn.baud ) )
        return 0;
    if ( !read_Byte( fp, &saturn.card_ctrl ) )
        return 0;
    if ( !read_Byte( fp, &saturn.card_status ) )
        return 0;
    if ( !read_Byte( fp, &saturn.io_ctrl ) )
        return 0;
    if ( !read_Byte( fp, &saturn.rcs ) )
        return 0;
    if ( !read_Byte( fp, &saturn.tcs ) )
        return 0;
    if ( !read_Byte( fp, &saturn.rbr ) )
        return 0;
    if ( !read_Byte( fp, &saturn.tbr ) )
        return 0;
    if ( !read_Byte( fp, &saturn.sreq ) )
        return 0;
    if ( !read_Byte( fp, &saturn.ir_ctrl ) )
        return 0;
    if ( !read_Byte( fp, &saturn.base_off ) )
        return 0;
    if ( !read_Byte( fp, &saturn.lcr ) )
        return 0;
    if ( !read_Byte( fp, &saturn.lbr ) )
        return 0;
    if ( !read_Byte( fp, &saturn.scratch ) )
        return 0;
    if ( !read_Byte( fp, &saturn.base_nibble ) )
        return 0;
    if ( !read_32( fp, &saturn.disp_addr ) )
        return 0;
    if ( !read_16( fp, &saturn.line_offset ) )
        return 0;
    if ( !read_Byte( fp, &saturn.line_count ) )
        return 0;
    if ( !read_16( fp, &saturn.unknown ) )
        return 0;
    if ( !read_Byte( fp, &saturn.t1_ctrl ) )
        return 0;
    if ( !read_Byte( fp, &saturn.t2_ctrl ) )
        return 0;
    if ( !read_32( fp, &saturn.menu_addr ) )
        return 0;
    if ( !read_Byte( fp, &saturn.unknown2 ) )
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

static int read_mem_file( char* name, Nibble* mem, int size )
{
    struct stat st;
    FILE* fp;
    Byte* tmp_mem;
    Byte byte;
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

        if ( NULL == ( tmp_mem = ( Byte* )malloc( ( size_t )st.st_size ) ) ) {
            for ( i = 0, j = 0; i < size / 2; i++ ) {
                if ( 1 != fread( &byte, 1, 1, fp ) ) {
                    if ( config.verbose )
                        fprintf( stderr, "can\'t read %s\n", name );
                    fclose( fp );
                    return 0;
                }
                mem[ j++ ] = ( Nibble )( ( int )byte & 0xf );
                mem[ j++ ] = ( Nibble )( ( ( int )byte >> 4 ) & 0xf );
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
                mem[ j++ ] = ( Nibble )( ( int )tmp_mem[ i ] & 0xf );
                mem[ j++ ] = ( Nibble )( ( ( int )tmp_mem[ i ] >> 4 ) & 0xf );
            }

            free( tmp_mem );
        }
    }

    fclose( fp );

    if ( config.verbose )
        printf( "read %s\n", name );

    return 1;
}

/***********************************************/
/* WRITING ~/.x48ng/{rom,ram,state,port1,port2} */
/***********************************************/

static int write_Byte( FILE* fp, Byte* var )
{
    unsigned char tmp;

    tmp = *var;
    if ( fwrite( &tmp, 1, 1, fp ) != 1 ) {
        if ( config.verbose )
            fprintf( stderr, "can\'t write Byte\n" );
        return 0;
    }
    return 1;
}

static int write_char( FILE* fp, char* var )
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

static int write_16( FILE* fp, word_16* var )
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

static int write_32( FILE* fp, word_32* var )
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

static int write_u_long( FILE* fp, unsigned long* var )
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

static int write_mem_file( char* name, Nibble* mem, int size )
{
    FILE* fp;
    Byte* tmp_mem;
    Byte byte;
    int i, j;

    if ( NULL == ( fp = fopen( name, "w" ) ) ) {
        if ( config.verbose )
            fprintf( stderr, "can\'t open %s\n", name );
        return 0;
    }

    if ( NULL == ( tmp_mem = ( Byte* )malloc( ( size_t )size / 2 ) ) ) {
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

static int write_state_file( char* filename )
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
    for ( i = 0; i < 3; i++ )
        write_char( fp, &saturn.version[ i ] );
    for ( i = 0; i < 16; i++ )
        write_Byte( fp, &saturn.reg[ A ][ i ] );
    for ( i = 0; i < 16; i++ )
        write_Byte( fp, &saturn.reg[ B ][ i ] );
    for ( i = 0; i < 16; i++ )
        write_Byte( fp, &saturn.reg[ C ][ i ] );
    for ( i = 0; i < 16; i++ )
        write_Byte( fp, &saturn.reg[ D ][ i ] );
    write_32( fp, &saturn.d[ 0 ] );
    write_32( fp, &saturn.d[ 1 ] );
    write_Byte( fp, &saturn.p );
    write_32( fp, &saturn.pc );
    for ( i = 0; i < 16; i++ )
        write_Byte( fp, &saturn.reg_r[ 0 ][ i ] );
    for ( i = 0; i < 16; i++ )
        write_Byte( fp, &saturn.reg_r[ 1 ][ i ] );
    for ( i = 0; i < 16; i++ )
        write_Byte( fp, &saturn.reg_r[ 2 ][ i ] );
    for ( i = 0; i < 16; i++ )
        write_Byte( fp, &saturn.reg_r[ 3 ][ i ] );
    for ( i = 0; i < 16; i++ )
        write_Byte( fp, &saturn.reg_r[ 4 ][ i ] );
    for ( i = 0; i < 4; i++ )
        write_Byte( fp, &saturn.in[ i ] );
    for ( i = 0; i < 3; i++ )
        write_Byte( fp, &saturn.out[ i ] );
    write_Byte( fp, &saturn.carry );
    for ( i = 0; i < NB_PSTAT; i++ )
        write_Byte( fp, &saturn.pstat[ i ] );
    write_Byte( fp, &saturn.st[ XM ] );
    write_Byte( fp, &saturn.st[ SB ] );
    write_Byte( fp, &saturn.st[ SR ] );
    write_Byte( fp, &saturn.st[ MP ] );
    write_Byte( fp, &saturn.hexmode );
    for ( i = 0; i < NB_RSTK; i++ )
        write_32( fp, &saturn.rstk[ i ] );
    write_16( fp, ( word_16* )&saturn.rstk_ptr );
    for ( i = 0; i < KEYS_BUFFER_SIZE; i++ )
        write_16( fp, ( word_16* )&saturn.keybuf[ i ] );
    write_Byte( fp, &saturn.interruptable );
    write_Byte( fp, &saturn.int_pending );
    write_Byte( fp, &saturn.kbd_ien );
    write_Byte( fp, &saturn.disp_io );
    write_Byte( fp, &saturn.contrast_ctrl );
    write_Byte( fp, &saturn.disp_test );
    write_16( fp, &saturn.crc );
    write_Byte( fp, &saturn.power_status );
    write_Byte( fp, &saturn.power_ctrl );
    write_Byte( fp, &saturn.mode );
    write_Byte( fp, &saturn.annunc );
    write_Byte( fp, &saturn.baud );
    write_Byte( fp, &saturn.card_ctrl );
    write_Byte( fp, &saturn.card_status );
    write_Byte( fp, &saturn.io_ctrl );
    write_Byte( fp, &saturn.rcs );
    write_Byte( fp, &saturn.tcs );
    write_Byte( fp, &saturn.rbr );
    write_Byte( fp, &saturn.tbr );
    write_Byte( fp, &saturn.sreq );
    write_Byte( fp, &saturn.ir_ctrl );
    write_Byte( fp, &saturn.base_off );
    write_Byte( fp, &saturn.lcr );
    write_Byte( fp, &saturn.lbr );
    write_Byte( fp, &saturn.scratch );
    write_Byte( fp, &saturn.base_nibble );
    write_32( fp, &saturn.disp_addr );
    write_16( fp, &saturn.line_offset );
    write_Byte( fp, &saturn.line_count );
    write_16( fp, &saturn.unknown );
    write_Byte( fp, &saturn.t1_ctrl );
    write_Byte( fp, &saturn.t2_ctrl );
    write_32( fp, &saturn.menu_addr );
    write_Byte( fp, &saturn.unknown2 );
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

/**********/
/* public */
/**********/

int read_rom( const char* fname )
{
    int ram_size;

    if ( !read_rom_file( fname, &saturn.rom, &rom_size ) )
        return 0;

    if ( config.verbose )
        printf( "read %s\n", fname );

    dev_memory_init();

    ram_size = opt_gx ? RAM_SIZE_GX : RAM_SIZE_SX;

    if ( NULL == ( saturn.ram = ( Nibble* )malloc( ram_size ) ) ) {
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

int read_files( void )
{
    /* unsigned long v1, v2; */
    int i;
    bool read_version;
    int ram_size;
    struct stat st;
    FILE* fp;

    /*************************************************/
    /* 1. read ROM from ~/.x48ng/rom into saturn.rom */
    /*************************************************/
    saturn.rom = ( Nibble* )NULL;
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
        read_version = true;
        for ( i = 0; i < 4; i++ ) {
            if ( !read_char( fp, &saturn.version[ i ] ) ) {
                if ( config.verbose )
                    fprintf( stderr, "can\'t read version\n" );
                read_version = false;
            }
        }

        if ( read_version ) {
            /* v1 = ( ( int )saturn.version[ 0 ] & 0xff ) << 24; */
            /* v1 |= ( ( int )saturn.version[ 1 ] & 0xff ) << 16; */
            /* v1 |= ( ( int )saturn.version[ 2 ] & 0xff ) << 8; */
            /* v1 |= ( ( int )saturn.version[ 3 ] & 0xff ); */
            /* v2 = ( ( int )VERSION_MAJOR & 0xff ) << 24; */
            /* v2 |= ( ( int )VERSION_MINOR & 0xff ) << 16; */
            /* v2 |= ( ( int )PATCHLEVEL & 0xff ) << 8; */

            /*
             * try to read latest version file
             */
            if ( !read_state_file( fp ) ) {
                if ( config.verbose )
                    fprintf( stderr, "can't handle %s\n", normalized_state_path );
                init_saturn();
            } else if ( config.verbose )
                printf( "read %s\n", normalized_state_path );
        }
    }
    fclose( fp );

    dev_memory_init();

    saturn_config_init();

    ram_size = opt_gx ? RAM_SIZE_GX : RAM_SIZE_SX;

    saturn.ram = ( Nibble* )NULL;
    if ( NULL == ( saturn.ram = ( Nibble* )malloc( ram_size ) ) ) {
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
            if ( NULL == ( saturn.port1 = ( Nibble* )malloc( port1_size ) ) ) {
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
            if ( NULL == ( saturn.port2 = ( Nibble* )malloc( port2_size ) ) ) {
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
