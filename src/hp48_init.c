#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>

#include "options.h"
#include "hp48.h"
#include "hp48emu.h" /* dev_memory_init(); */
#include "romio.h"

#define X48_MAGIC 0x48503438
#define NR_CONFIG 8

#define RAM_SIZE_SX 0x10000
#define RAM_SIZE_GX 0x40000

short rom_is_new = 1;
long ram_size;
long port1_size;
long port1_mask;
short port1_is_ram;
long port2_size;
long port2_mask;
short port2_is_ram;

void get_home_directory( char* path ) {
    char* p;
    struct passwd* pwd;

    if ( homeDirectory[ 0 ] == '/' )
        strcpy( path, homeDirectory );
    else {
        p = getenv( "HOME" );
        if ( p ) {
            strcpy( path, p );
            strcat( path, "/" );
        } else {
            pwd = getpwuid( getuid() );
            if ( pwd ) {
                strcpy( path, pwd->pw_dir );
                strcat( path, "/" );
            } else {
                if ( verbose )
                    fprintf( stderr, "can\'t figure out your home directory, "
                                     "trying /tmp\n" );
                strcpy( path, "/tmp" );
            }
        }
        strcat( path, homeDirectory );
    }
}

void saturn_config_init( void ) {
    saturn.version[ 0 ] = VERSION_MAJOR;
    saturn.version[ 1 ] = VERSION_MINOR;
    saturn.version[ 2 ] = PATCHLEVEL;
    memset( &device, 0, sizeof( device ) );
    device.display_touched = 1;
    device.contrast_touched = 1;
    device.baud_touched = 1;
    device.ann_touched = 1;
    saturn.rcs = 0x0;
    saturn.tcs = 0x0;
    saturn.lbr = 0x0;
}

void init_saturn( void ) {
    int i;

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
    saturn.intenable = 1;
    saturn.int_pending = 0;
    saturn.kbd_ien = 1;
    saturn.timer1 = 0;
    saturn.timer2 = 0x2000;
    saturn.bank_switch = 0;
    for ( i = 0; i < NR_MCTL; i++ ) {
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

int init_emulator( void ) {
    if ( !initialize && read_files() ) {
        if ( resetOnStartup )
            saturn.PC = 0x00000;
        return 0;
    }

    init_saturn();

    if ( !read_rom( romFileName ) )
        exit( 1 );

    return 0;
}

int exit_emulator( void ) {
    write_files();

    return 1;
}

/***********************************************/
/* READING ~/.x48ng/{rom,ram,hp48,port1,port2} */
/***********************************************/

int read_8( FILE* fp, word_8* var ) {
    unsigned char tmp;

    if ( fread( &tmp, 1, 1, fp ) != 1 ) {
        if ( verbose )
            fprintf( stderr, "can\'t read word_8\n" );
        return 0;
    }
    *var = tmp;
    return 1;
}

int read_char( FILE* fp, char* var ) {
    char tmp;

    if ( fread( &tmp, 1, 1, fp ) != 1 ) {
        if ( verbose )
            fprintf( stderr, "can\'t read char\n" );
        return 0;
    }
    *var = tmp;
    return 1;
}

int read_16( FILE* fp, word_16* var ) {
    unsigned char tmp[ 2 ];

    if ( fread( &tmp[ 0 ], 1, 2, fp ) != 2 ) {
        if ( verbose )
            fprintf( stderr, "can\'t read word_16\n" );
        return 0;
    }
    *var = tmp[ 0 ] << 8;
    *var |= tmp[ 1 ];
    return 1;
}

int read_32( FILE* fp, word_32* var ) {
    unsigned char tmp[ 4 ];

    if ( fread( &tmp[ 0 ], 1, 4, fp ) != 4 ) {
        if ( verbose )
            fprintf( stderr, "can\'t read word_32\n" );
        return 0;
    }
    *var = tmp[ 0 ] << 24;
    *var |= tmp[ 1 ] << 16;
    *var |= tmp[ 2 ] << 8;
    *var |= tmp[ 3 ];
    return 1;
}

int read_u_long( FILE* fp, unsigned long* var ) {
    unsigned char tmp[ 4 ];

    if ( fread( &tmp[ 0 ], 1, 4, fp ) != 4 ) {
        if ( verbose )
            fprintf( stderr, "can\'t read unsigned long\n" );
        return 0;
    }
    *var = tmp[ 0 ] << 24;
    *var |= tmp[ 1 ] << 16;
    *var |= tmp[ 2 ] << 8;
    *var |= tmp[ 3 ];
    return 1;
}

int read_hp48_file( FILE* fp ) {
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
    for ( i = 0; i < NR_PSTAT; i++ )
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
    for ( i = 0; i < NR_RSTK; i++ )
        if ( !read_32( fp, &saturn.rstk[ i ] ) )
            return 0;
    if ( !read_16( fp, ( word_16* )&saturn.rstkp ) )
        return 0;
    for ( i = 0; i < 9; i++ )
        if ( !read_16( fp, ( word_16* )&saturn.keybuf.rows[ i ] ) )
            return 0;
    if ( !read_8( fp, &saturn.intenable ) )
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
    for ( i = 0; i < NR_MCTL; i++ ) {
        if ( !read_16( fp, &saturn.mem_cntl[ i ].unconfigured ) )
            return 0;
        if ( !read_32( fp, &saturn.mem_cntl[ i ].config[ 0 ] ) )
            return 0;
        if ( !read_32( fp, &saturn.mem_cntl[ i ].config[ 1 ] ) )
            return 0;
    }
    return 1;
}

int read_mem_file( char* name, word_4* mem, int size ) {
    struct stat st;
    FILE* fp;
    word_8* tmp_mem;
    word_8 byte;
    int i, j;

    if ( NULL == ( fp = fopen( name, "r" ) ) ) {
        if ( verbose )
            fprintf( stderr, "ct open %s\n", name );
        return 0;
    }

    if ( stat( name, &st ) < 0 ) {
        if ( verbose )
            fprintf( stderr, "can\'t stat %s\n", name );
        return 0;
    }

    if ( st.st_size == size ) {
        /*
         * size is same as memory size, old version file
         */
        if ( fread( mem, 1, ( size_t )size, fp ) != size ) {
            if ( verbose )
                fprintf( stderr, "can\'t read %s\n", name );
            fclose( fp );
            return 0;
        }
    } else {
        /*
         * size is different, check size and decompress memory
         */

        if ( st.st_size != size / 2 ) {
            if ( verbose )
                fprintf( stderr, "strange size %s, expected %d, found %ld\n",
                         name, size / 2, st.st_size );
            fclose( fp );
            return 0;
        }

        if ( NULL == ( tmp_mem = ( word_8* )malloc( ( size_t )st.st_size ) ) ) {
            for ( i = 0, j = 0; i < size / 2; i++ ) {
                if ( 1 != fread( &byte, 1, 1, fp ) ) {
                    if ( verbose )
                        fprintf( stderr, "can\'t read %s\n", name );
                    fclose( fp );
                    return 0;
                }
                mem[ j++ ] = ( word_4 )( ( int )byte & 0xf );
                mem[ j++ ] = ( word_4 )( ( ( int )byte >> 4 ) & 0xf );
            }
        } else {
            if ( fread( tmp_mem, 1, ( size_t )size / 2, fp ) != size / 2 ) {
                if ( verbose )
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

    if ( verbose )
        printf( "read %s\n", name );

    return 1;
}

int read_rom( const char* fname ) {
    int ram_size;

    if ( !read_rom_file( fname, &saturn.rom, &rom_size ) )
        return 0;

    if ( verbose )
        printf( "read %s\n", fname );

    dev_memory_init();

    ram_size = opt_gx ? RAM_SIZE_GX : RAM_SIZE_SX;

    if ( NULL == ( saturn.ram = ( word_4* )malloc( ram_size ) ) ) {
        if ( verbose )
            fprintf( stderr, "can\'t malloc RAM\n" );
        return 0;
    }

    memset( saturn.ram, 0, ram_size );

    port1_size = 0;
    port1_mask = 0;
    port1_is_ram = 0;
    saturn.port1 = ( unsigned char* )0;

    port2_size = 0;
    port2_mask = 0;
    port2_is_ram = 0;
    saturn.port2 = ( unsigned char* )0;

    saturn.card_status = 0;

    return 1;
}

int read_files( void ) {
    char path[ 1024 ];
    char fnam[ 1024 ];
    unsigned long v1, v2;
    int i, read_version;
    int ram_size;
    struct stat st;
    FILE* fp;

    get_home_directory( path );
    strcat( path, "/" );

    /*************************************************/
    /* 1. read ROM from ~/.x48ng/rom into saturn.rom */
    /*************************************************/
    saturn.rom = ( word_4* )NULL;
    strcpy( fnam, path );
    strcat( fnam, "rom" );
    if ( !read_rom_file( fnam, &saturn.rom, &rom_size ) )
        return 0;

    if ( verbose )
        printf( "read %s\n", fnam );

    rom_is_new = 0;

    /**************************************************/
    /* 2. read saved state from ~/.x48ng/hp48 into fp */
    /**************************************************/
    strcpy( fnam, path );
    strcat( fnam, "hp48" );
    if ( NULL == ( fp = fopen( fnam, "r" ) ) ) {
        if ( verbose )
            fprintf( stderr, "can\'t open %s\n", fnam );
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
                if ( verbose )
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
            if ( !read_hp48_file( fp ) ) {
                if ( verbose )
                    fprintf( stderr, "can\'t handle %s\n", fnam );
                init_saturn();
            } else if ( verbose )
                printf( "read %s\n", fnam );
        }
    }
    fclose( fp );

    dev_memory_init();

    saturn_config_init();

    ram_size = opt_gx ? RAM_SIZE_GX : RAM_SIZE_SX;

    saturn.ram = ( word_4* )NULL;
    if ( NULL == ( saturn.ram = ( word_4* )malloc( ram_size ) ) ) {
        if ( verbose )
            fprintf( stderr, "can\'t malloc RAM[%d]\n", ram_size );
        exit( 1 );
    }

    /*************************************************/
    /* 3. read RAM from ~/.x48ng/ram into saturn.ram */
    /*************************************************/
    strcpy( fnam, path );
    strcat( fnam, "ram" );
    if ( ( fp = fopen( fnam, "r" ) ) == NULL ) {
        if ( verbose )
            fprintf( stderr, "can\'t open %s\n", fnam );
        return 0;
    }
    if ( !read_mem_file( fnam, saturn.ram, ram_size ) )
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

    strcpy( fnam, path );
    strcat( fnam, "port1" );
    if ( stat( fnam, &st ) >= 0 ) {
        port1_size = 2 * st.st_size;
        if ( ( port1_size == 0x10000 ) || ( port1_size == 0x40000 ) ) {
            if ( NULL == ( saturn.port1 = ( word_4* )malloc( port1_size ) ) ) {
                if ( verbose )
                    fprintf( stderr, "can\'t malloc PORT1[%ld]\n", port1_size );
            } else if ( !read_mem_file( fnam, saturn.port1, port1_size ) ) {
                port1_size = 0;
                port1_is_ram = 0;
            } else {
                port1_is_ram = ( st.st_mode & S_IWUSR ) ? 1 : 0;
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
    port2_is_ram = 0;
    saturn.port2 = ( unsigned char* )0;

    strcpy( fnam, path );
    strcat( fnam, "port2" );
    if ( stat( fnam, &st ) >= 0 ) {
        port2_size = 2 * st.st_size;
        if ( ( opt_gx && ( ( port2_size % 0x40000 ) == 0 ) ) ||
             ( !opt_gx &&
               ( ( port2_size == 0x10000 ) || ( port2_size == 0x40000 ) ) ) ) {
            if ( NULL == ( saturn.port2 = ( word_4* )malloc( port2_size ) ) ) {
                if ( verbose )
                    fprintf( stderr, "can\'t malloc PORT2[%ld]\n", port2_size );
            } else if ( !read_mem_file( fnam, saturn.port2, port2_size ) ) {
                port2_size = 0;
                port2_is_ram = 0;
            } else {
                port2_is_ram = ( st.st_mode & S_IWUSR ) ? 1 : 0;
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
/* WRITING ~/.x48ng/{rom,ram,hp48,port1,port2} */
/***********************************************/

int write_8( FILE* fp, word_8* var ) {
    unsigned char tmp;

    tmp = *var;
    if ( fwrite( &tmp, 1, 1, fp ) != 1 ) {
        if ( verbose )
            fprintf( stderr, "can\'t write word_8\n" );
        return 0;
    }
    return 1;
}

int write_char( FILE* fp, char* var ) {
    char tmp;

    tmp = *var;
    if ( fwrite( &tmp, 1, 1, fp ) != 1 ) {
        if ( verbose )
            fprintf( stderr, "can\'t write char\n" );
        return 0;
    }
    return 1;
}

int write_16( FILE* fp, word_16* var ) {
    unsigned char tmp[ 2 ];

    tmp[ 0 ] = ( *var >> 8 ) & 0xff;
    tmp[ 1 ] = *var & 0xff;
    if ( fwrite( &tmp[ 0 ], 1, 2, fp ) != 2 ) {
        if ( verbose )
            fprintf( stderr, "can\'t write word_16\n" );
        return 0;
    }
    return 1;
}

int write_32( FILE* fp, word_32* var ) {
    unsigned char tmp[ 4 ];

    tmp[ 0 ] = ( *var >> 24 ) & 0xff;
    tmp[ 1 ] = ( *var >> 16 ) & 0xff;
    tmp[ 2 ] = ( *var >> 8 ) & 0xff;
    tmp[ 3 ] = *var & 0xff;
    if ( fwrite( &tmp[ 0 ], 1, 4, fp ) != 4 ) {
        if ( verbose )
            fprintf( stderr, "can\'t write word_32\n" );
        return 0;
    }
    return 1;
}

int write_u_long( FILE* fp, unsigned long* var ) {
    unsigned char tmp[ 4 ];

    tmp[ 0 ] = ( *var >> 24 ) & 0xff;
    tmp[ 1 ] = ( *var >> 16 ) & 0xff;
    tmp[ 2 ] = ( *var >> 8 ) & 0xff;
    tmp[ 3 ] = *var & 0xff;
    if ( fwrite( &tmp[ 0 ], 1, 4, fp ) != 4 ) {
        if ( verbose )
            fprintf( stderr, "can\'t write unsigned long\n" );
        return 0;
    }
    return 1;
}

int write_mem_file( char* name, word_4* mem, int size ) {
    FILE* fp;
    word_8* tmp_mem;
    word_8 byte;
    int i, j;

    if ( NULL == ( fp = fopen( name, "w" ) ) ) {
        if ( verbose )
            fprintf( stderr, "can\'t open %s\n", name );
        return 0;
    }

    if ( NULL == ( tmp_mem = ( word_8* )malloc( ( size_t )size / 2 ) ) ) {
        for ( i = 0, j = 0; i < size / 2; i++ ) {
            byte = ( mem[ j++ ] & 0x0f );
            byte |= ( mem[ j++ ] << 4 ) & 0xf0;
            if ( 1 != fwrite( &byte, 1, 1, fp ) ) {
                if ( verbose )
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

        if ( fwrite( tmp_mem, 1, ( size_t )size / 2, fp ) != size / 2 ) {
            if ( verbose )
                fprintf( stderr, "can\'t write %s\n", name );
            fclose( fp );
            free( tmp_mem );
            return 0;
        }

        free( tmp_mem );
    }

    fclose( fp );

    if ( verbose )
        printf( "wrote %s\n", name );

    return 1;
}

int write_files( void ) {
    char path[ 1024 ];
    char fnam[ 1024 ];
    struct stat st;
    int i, make_dir;
    int ram_size;
    FILE* fp;

    make_dir = 0;
    get_home_directory( path );

    if ( stat( path, &st ) == -1 ) {
        if ( errno == ENOENT ) {
            make_dir = 1;
        } else {
            if ( verbose )
                fprintf( stderr, "can\'t stat %s, saving to /tmp\n", path );
            strcpy( path, "/tmp" );
        }
    } else {
        if ( !S_ISDIR( st.st_mode ) ) {
            if ( verbose )
                fprintf( stderr, "%s is no directory, saving to /tmp\n", path );
            strcpy( path, "/tmp" );
        }
    }

    if ( make_dir ) {
        if ( mkdir( path, 0777 ) == -1 ) {
            if ( verbose )
                fprintf( stderr, "can\'t mkdir %s, saving to /tmp\n", path );
            strcpy( path, "/tmp" );
        }
    }

    strcat( path, "/" );

    strcpy( fnam, path );
    strcat( fnam, "hp48" );
    if ( ( fp = fopen( fnam, "w" ) ) == NULL ) {
        if ( verbose )
            fprintf( stderr, "can\'t open %s, no saving done\n", fnam );
        return 0;
    }

    /*
     * write the hp48 config file
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
    for ( i = 0; i < NR_PSTAT; i++ )
        write_8( fp, &saturn.PSTAT[ i ] );
    write_8( fp, &saturn.XM );
    write_8( fp, &saturn.SB );
    write_8( fp, &saturn.SR );
    write_8( fp, &saturn.MP );
    write_8( fp, &saturn.hexmode );
    for ( i = 0; i < NR_RSTK; i++ )
        write_32( fp, &saturn.rstk[ i ] );
    write_16( fp, ( word_16* )&saturn.rstkp );
    for ( i = 0; i < 9; i++ )
        write_16( fp, ( word_16* )&saturn.keybuf.rows[ i ] );
    write_8( fp, &saturn.intenable );
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
    for ( i = 0; i < NR_MCTL; i++ ) {
        write_16( fp, &saturn.mem_cntl[ i ].unconfigured );
        write_32( fp, &saturn.mem_cntl[ i ].config[ 0 ] );
        write_32( fp, &saturn.mem_cntl[ i ].config[ 1 ] );
    }
    fclose( fp );
    if ( verbose )
        printf( "wrote %s\n", fnam );

    if ( rom_is_new ) {
        strcpy( fnam, path );
        strcat( fnam, "rom" );
        if ( !write_mem_file( fnam, saturn.rom, rom_size ) )
            return 0;
    }

    if ( opt_gx )
        ram_size = RAM_SIZE_GX;
    else
        ram_size = RAM_SIZE_SX;

    strcpy( fnam, path );
    strcat( fnam, "ram" );
    if ( !write_mem_file( fnam, saturn.ram, ram_size ) )
        return 0;

    if ( ( port1_size > 0 ) && port1_is_ram ) {
        strcpy( fnam, path );
        strcat( fnam, "port1" );
        if ( !write_mem_file( fnam, saturn.port1, port1_size ) )
            return 0;
    }

    if ( ( port2_size > 0 ) && port2_is_ram ) {
        strcpy( fnam, path );
        strcat( fnam, "port2" );
        if ( !write_mem_file( fnam, saturn.port2, port2_size ) )
            return 0;
    }

    return 1;
}
