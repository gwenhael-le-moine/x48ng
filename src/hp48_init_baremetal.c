#include <string.h>
#include "hp48.h"
#include "hp48emu.h"
#include "romio.h"
#include "x48_resources.h"

#include "hp48_rom.h"

#define X48_MAGIC 0x48503438
#define NR_CONFIG 8

short rom_is_new = 1;
long ram_size;
long port1_size;
long port1_mask;
short port1_is_ram;
long port2_size;
long port2_mask;
short port2_is_ram;

void saturn_config_init( void ) {
    saturn.version[ 0 ] = VERSION_MAJOR;
    saturn.version[ 1 ] = VERSION_MINOR;
    saturn.version[ 2 ] = PATCHLEVEL;
    saturn.version[ 3 ] = COMPILE_VERSION;
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
    saturn.version[ 3 ] = COMPILE_VERSION;
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

    return 0;
}

int exit_emulator( void ) {
    write_files();

    return 1;
}

/***********************************************/
/* READING ~/.x48ng/{rom,ram,hp48,port1,port2} */
/***********************************************/
static char hp48_ram[RAM_SIZE_SX];
int read_files( void ) {

    /*************************************************/
    /* 1. read ROM from ~/.x48ng/rom into saturn.rom */
    /*************************************************/
    saturn.rom = ( word_4* ) hp48_rom; // read at link time
    rom_is_new = 0;

    /**************************************************/
    /* 2. read saved state from ~/.x48ng/hp48 into fp */
    /**************************************************/
    init_saturn(); // resume not supported

    dev_memory_init();

    saturn_config_init();


    /*************************************************/
    /* 3. read RAM from ~/.x48ng/ram into saturn.ram */
    /*************************************************/
    ram_size = opt_gx ? RAM_SIZE_GX : RAM_SIZE_SX;
    saturn.ram = ( word_4* ) hp48_ram; // static allocatiom at link time

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


    // not supported

    /********************************************************/
    /* 5. read card 2 from ~/.x48ng/port2 into saturn.port2 */
    /********************************************************/
    port2_size = 0;
    port2_mask = 0;
    port2_is_ram = 0;
    saturn.port2 = ( unsigned char* )0;

    // not supported

    /************************/
    /* All files are loaded */
    /************************/
    return 1;
}

/***********************************************/
/* WRITING ~/.x48ng/{rom,ram,hp48,port1,port2} */
/***********************************************/
int write_files( void ) {
	// We might want to write to EEPROM or flash
	// but now, off is deepsleep. Like the real HP49r
    return 1;
}
