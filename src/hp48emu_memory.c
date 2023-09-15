#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "hp48.h"
#include "hp48emu.h"
#include "romio.h"
#include "ui.h" /* disp; disp_draw_nibble(); menu_draw_nibble(); */
#include "options.h"

#define MCTL_MMIO_SX 0
#define MCTL_SysRAM_SX 1
#define MCTL_PORT1_SX 2
#define MCTL_PORT2_SX 3
#define MCTL_EXTRA_SX 4
#define MCTL_SysROM_SX 5

#define MCTL_MMIO_GX 0
#define MCTL_SysRAM_GX 1
#define MCTL_BANK_GX 2
#define MCTL_PORT1_GX 3
#define MCTL_PORT2_GX 4
#define MCTL_SysROM_GX 5

#define DISP_INSTR_OFF 0x10

extern int device_check;
extern short port1_is_ram;
extern long port1_mask;
extern short port2_is_ram;
extern long port2_mask;

long nibble_masks[ 16 ] = { 0x0000000f, 0x000000f0, 0x00000f00, 0x0000f000,
                            0x000f0000, 0x00f00000, 0x0f000000, 0xf0000000,
                            0x0000000f, 0x000000f0, 0x00000f00, 0x0000f000,
                            0x000f0000, 0x00f00000, 0x0f000000, 0xf0000000 };

void ( *write_nibble )( long addr, int val );
int ( *read_nibble )( long addr );
int ( *read_nibble_crc )( long addr );

static int line_counter = -1;

static inline int calc_crc( int nib ) {
    saturn.crc =
        ( saturn.crc >> 4 ) ^ ( ( ( saturn.crc ^ nib ) & 0xf ) * 0x1081 );
    return nib;
}

void write_dev_mem( long addr, int val ) {
    static int old_line_offset = -1;

    device_check = 1;
    schedule_event = 0;
    switch ( ( int )addr ) {
        case 0x100: /* DISPIO */
            if ( val != saturn.disp_io ) {
                saturn.disp_io = val;
                display.on = ( val & 0x8 ) >> 3;
                display.offset = val & 0x7;
                disp.offset = 2 * display.offset;
                if ( display.offset > 3 )
                    display.nibs_per_line =
                        ( NIBBLES_PER_ROW + saturn.line_offset + 2 ) & 0xfff;
                else
                    display.nibs_per_line =
                        ( NIBBLES_PER_ROW + saturn.line_offset ) & 0xfff;
                display.disp_end =
                    display.disp_start +
                    ( display.nibs_per_line * ( display.lines + 1 ) );
                device.display_touched = DISP_INSTR_OFF;
            }
            return;
        case 0x101: /* CONTRAST CONTROL */
            saturn.contrast_ctrl = val;
            display.contrast &= ~0x0f;
            display.contrast |= val;
            device.contrast_touched = 1;
            return;
        case 0x102: /* DISPLAY TEST */
            display.contrast &= ~0xf0;
            display.contrast |= ( ( val & 0x1 ) << 4 );
            device.contrast_touched = 1;
            /* Fall through */
        case 0x103: /* DISPLAY TEST */
            saturn.disp_test &= ~nibble_masks[ addr - 0x102 ];
            saturn.disp_test |= val << ( ( addr - 0x102 ) * 4 );
            device.disp_test_touched = 1;
            return;
        case 0x104:
        case 0x105:
        case 0x106:
        case 0x107: /* CRC */
            saturn.crc &= ~nibble_masks[ addr - 0x104 ];
            saturn.crc |= val << ( ( addr - 0x104 ) * 4 );
            return;
        case 0x108: /* POWER STATUS */
            saturn.power_status = val;
            device.power_status_touched = 1;
            return;
        case 0x109: /* POWER CONTROL */
            saturn.power_ctrl = val;
            device.power_ctrl_touched = 1;
            return;
        case 0x10a: /* MODE */
            saturn.mode = val;
            device.mode_touched = 1;
            return;
        case 0x10b:
        case 0x10c: /* ANNUNC */
            saturn.annunc &= ~nibble_masks[ addr - 0x10b ];
            saturn.annunc |= val << ( ( addr - 0x10b ) * 4 );
            display.annunc = saturn.annunc;
            device.ann_touched = 1;
            return;
        case 0x10d: /* BAUD */
            saturn.baud = val;
            device.baud_touched = 1;
            return;
        case 0x10e: /* CARD CONTROL */
            saturn.card_ctrl = val;
            if ( saturn.card_ctrl & 0x02 )
                saturn.MP = 1;
            if ( saturn.card_ctrl & 0x01 )
                do_interupt();
            device.card_ctrl_touched = 1;
            return;
        case 0x10f: /* CARD STATUS */
            return;
        case 0x110: /* IO CONTROL */
            saturn.io_ctrl = val;
            device.ioc_touched = 1;
            return;
        case 0x111: /* RCS */
            saturn.rcs = val;
            return;
        case 0x112: /* TCS */
            saturn.tcs = val;
            return;
        case 0x113: /* CRER */
            saturn.rcs &= 0x0b;
            return;
        case 0x114:
        case 0x115: /* RBR */
            return;
        case 0x116:
        case 0x117: /* TBR */
            saturn.tbr &= ~nibble_masks[ addr - 0x116 ];
            saturn.tbr |= val << ( ( addr - 0x116 ) * 4 );
            saturn.tcs |= 0x01;
            device.tbr_touched = 1;
            return;
        case 0x118:
        case 0x119: /* SERVICE REQ */
            saturn.sreq &= ~nibble_masks[ addr - 0x118 ];
            saturn.sreq |= val << ( ( addr - 0x118 ) * 4 );
            device.sreq_touched = 1;
            return;
        case 0x11a: /* IR CONTROL */
            saturn.ir_ctrl = val;
            device.ir_ctrl_touched = 1;
            return;
        case 0x11b: /* BASE NIB OFFSET */
            saturn.base_off = val;
            device.base_off_touched = 1;
            return;
        case 0x11c: /* LED CONTROL */
            saturn.lcr = val;
            device.lcr_touched = 1;
            return;
        case 0x11d: /* LED BUFFER */
            saturn.lbr = val;
            device.lbr_touched = 1;
            return;
        case 0x11e: /* SCRATCH PAD */
            saturn.scratch = val;
            device.scratch_touched = 1;
            return;
        case 0x11f: /* BASENIBBLE */
            saturn.base_nibble = val;
            device.base_nibble_touched = 1;
            return;
        case 0x120:
        case 0x121:
        case 0x122:
        case 0x123: /* DISP_ADDR */
        case 0x124:
            saturn.disp_addr &= ~nibble_masks[ addr - 0x120 ];
            saturn.disp_addr |= val << ( ( addr - 0x120 ) * 4 );
            if ( display.disp_start != ( saturn.disp_addr & 0xffffe ) ) {
                display.disp_start = saturn.disp_addr & 0xffffe;
                display.disp_end =
                    display.disp_start +
                    ( display.nibs_per_line * ( display.lines + 1 ) );
                device.display_touched = DISP_INSTR_OFF;
            }
            return;
        case 0x125:
        case 0x126:
        case 0x127: /* LINE_OFFSET */
            saturn.line_offset &= ~nibble_masks[ addr - 0x125 ];
            saturn.line_offset |= val << ( ( addr - 0x125 ) * 4 );
            if ( saturn.line_offset != old_line_offset ) {
                old_line_offset = saturn.line_offset;
                if ( display.offset > 3 )
                    display.nibs_per_line =
                        ( NIBBLES_PER_ROW + saturn.line_offset + 2 ) & 0xfff;
                else
                    display.nibs_per_line =
                        ( NIBBLES_PER_ROW + saturn.line_offset ) & 0xfff;
                display.disp_end =
                    display.disp_start +
                    ( display.nibs_per_line * ( display.lines + 1 ) );
                device.display_touched = DISP_INSTR_OFF;
            }
            return;
        case 0x128:
        case 0x129: /* LINE_COUNT */
            saturn.line_count &= ~nibble_masks[ addr - 0x128 ];
            saturn.line_count |= val << ( ( addr - 0x128 ) * 4 );
            line_counter = -1;
            if ( display.lines != ( saturn.line_count & 0x3f ) ) {
                display.lines = saturn.line_count & 0x3f;
                if ( display.lines == 0 )
                    display.lines = 63;
                disp.lines = 2 * display.lines;
                display.disp_end =
                    display.disp_start +
                    ( display.nibs_per_line * ( display.lines + 1 ) );
                device.display_touched = DISP_INSTR_OFF;
            }
            return;
        case 0x12a:
        case 0x12b:
        case 0x12c:
        case 0x12d: /* Dont know yet */
            saturn.unknown &= ~nibble_masks[ addr - 0x12a ];
            saturn.unknown |= val << ( ( addr - 0x12a ) * 4 );
            device.unknown_touched = 1;
            return;
        case 0x12e: /* TIMER 1 CONTROL */
            saturn.t1_ctrl = val;
            device.t1_ctrl_touched = 1;
            return;
        case 0x12f: /* TIMER 2 CONTROL */
            saturn.t2_ctrl = val;
            device.t2_ctrl_touched = 1;
            return;
        case 0x130:
        case 0x131:
        case 0x132:
        case 0x133: /* MENU_ADDR */
        case 0x134:
            saturn.menu_addr &= ~nibble_masks[ addr - 0x130 ];
            saturn.menu_addr |= val << ( ( addr - 0x130 ) * 4 );
            if ( display.menu_start != saturn.menu_addr ) {
                display.menu_start = saturn.menu_addr;
                display.menu_end = display.menu_start + 0x110;
                device.display_touched = DISP_INSTR_OFF;
            }
            return;
        case 0x135:
        case 0x136: /* Dont know yet 2 */
            saturn.unknown2 &= ~nibble_masks[ addr - 0x135 ];
            saturn.unknown2 |= val << ( ( addr - 0x135 ) * 4 );
            device.unknown2_touched = 1;
            return;
        case 0x137: /* TIMER1 */
            saturn.timer1 = val;
            device.t1_touched = 1;
            return;
        case 0x138:
        case 0x139:
        case 0x13a:
        case 0x13b:
        case 0x13c:
        case 0x13d:
        case 0x13e:
        case 0x13f: /* TIMER2 */
            saturn.timer2 &= ~nibble_masks[ addr - 0x138 ];
            saturn.timer2 |= val << ( ( addr - 0x138 ) * 4 );
            device.t2_touched = 1;
            return;
        default:
            if ( verbose )
                fprintf( stderr, "%.5lx: UNKNOWN DEVICE WRITE AT 0x%lx !!!\n",
                         saturn.PC, addr );
            return;
    }
}

int read_dev_mem( long addr ) {
    switch ( ( int )addr ) {
        case 0x100: /* DISPLAY IO */
            return saturn.disp_io & 0x0f;
        case 0x101: /* CONTRAST CONTROL */
            return saturn.contrast_ctrl & 0x0f;
        case 0x102:
        case 0x103: /* DISPLAY TEST */
            return ( saturn.disp_test >> ( ( addr - 0x102 ) * 4 ) ) & 0x0f;
        case 0x104:
        case 0x105:
        case 0x106:
        case 0x107: /* CRC */
            return ( saturn.crc >> ( ( addr - 0x104 ) * 4 ) ) & 0x0f;
        case 0x108: /* POWER STATUS */
            return saturn.power_status & 0x0f;
        case 0x109: /* POWER CONTROL */
            return saturn.power_ctrl & 0x0f;
        case 0x10a: /* MODE */
            return saturn.mode & 0x0f;
        case 0x10b:
        case 0x10c: /* ANNUNC */
            return ( saturn.annunc >> ( ( addr - 0x10b ) * 4 ) ) & 0x0f;
        case 0x10d: /* BAUD */
            return saturn.baud & 0x0f;
        case 0x10e: /* CARD CONTROL */
            return saturn.card_ctrl & 0x0f;
        case 0x10f: /* CARD STATUS */
            return saturn.card_status & 0x0f;
        case 0x110: /* IO CONTROL */
            return saturn.io_ctrl & 0x0f;
        case 0x111: /* RCS */
            return saturn.rcs & 0x0f;
        case 0x112: /* TCS */
            return saturn.tcs & 0x0f;
        case 0x113: /* CRER */
            return 0x00;
        case 0x114:
        case 0x115: /* RBR */
            saturn.rcs &= 0x0e;
            device.rbr_touched = 1;
            device_check = 1;
            schedule_event = 0;
            return ( saturn.rbr >> ( ( addr - 0x114 ) * 4 ) ) & 0x0f;
        case 0x116:
        case 0x117: /* TBR */
            return 0x00;
        case 0x118:
        case 0x119: /* SERVICE REQ */
            return ( saturn.sreq >> ( ( addr - 0x118 ) * 4 ) ) & 0x0f;
        case 0x11a: /* IR CONTROL */
            return saturn.ir_ctrl & 0x0f;
        case 0x11b: /* BASE NIB OFFSET */
            return saturn.base_off & 0x0f;
        case 0x11c: /* LED CONTROL */
            return saturn.lcr & 0x0f;
        case 0x11d: /* LED BUFFER */
            return saturn.lbr & 0x0f;
        case 0x11e: /* SCRATCH PAD */
            return saturn.scratch & 0x0f;
        case 0x11f: /* BASENIBBLE */
            return saturn.base_nibble & 0x0f;
        case 0x120:
        case 0x121:
        case 0x122:
        case 0x123: /* DISP_ADDR */
        case 0x124:
            return ( saturn.disp_addr >> ( ( addr - 0x120 ) * 4 ) ) & 0x0f;
        case 0x125:
        case 0x126:
        case 0x127: /* LINE_OFFSET */
            return ( saturn.line_offset >> ( ( addr - 0x125 ) * 4 ) ) & 0x0f;
        case 0x128:
        case 0x129: /* LINE_COUNT */
            line_counter++;
            if ( line_counter > 0x3f )
                line_counter = -1;
            return ( ( ( saturn.line_count & 0xc0 ) |
                       ( line_counter & 0x3f ) ) >>
                     ( ( addr - 0x128 ) * 4 ) ) &
                   0x0f;
        case 0x12a:
        case 0x12b:
        case 0x12c:
        case 0x12d: /* Dont know yet */
            return ( saturn.unknown >> ( ( addr - 0x12a ) * 4 ) ) & 0x0f;
        case 0x12e: /* TIMER 1 CONTROL */
            return saturn.t1_ctrl & 0x0f;
        case 0x12f: /* TIMER 2 CONTROL */
            return saturn.t2_ctrl & 0x0f;
        case 0x130:
        case 0x131:
        case 0x132:
        case 0x133: /* MENU_ADDR */
        case 0x134:
            return ( saturn.menu_addr >> ( ( addr - 0x130 ) * 4 ) ) & 0x0f;
        case 0x135:
        case 0x136: /* Dont know yet 2 */
            return ( saturn.unknown2 >> ( ( addr - 0x135 ) * 4 ) ) & 0x0f;
        case 0x137:
            return saturn.timer1 & 0xf;
        case 0x138:
        case 0x139:
        case 0x13a:
        case 0x13b:
        case 0x13c:
        case 0x13d:
        case 0x13e:
        case 0x13f:
            return ( saturn.timer2 >> ( ( addr - 0x138 ) * 4 ) ) & 0xf;
        default:
            if ( verbose )
                fprintf( stderr, "%.5lx: UNKNOWN DEVICE READ AT 0x%lx !!!\n",
                         saturn.PC, addr );
            return 0x00;
    }
}

void write_nibble_sx( long addr, int val ) {
    addr &= 0xfffff;
    val &= 0x0f;
    switch ( ( int )( addr >> 16 ) & 0x0f ) {
        case 0:
            if ( addr < 0x140 && addr >= 0x100 &&
                 saturn.mem_cntl[ MCTL_MMIO_SX ].config[ 0 ] == 0x100 ) {
                write_dev_mem( addr, val );
                return;
            }
            return;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            return;
        case 7:
            if ( saturn.mem_cntl[ MCTL_SysRAM_SX ].config[ 0 ] == 0x70000 ) {
                if ( saturn.mem_cntl[ MCTL_SysRAM_SX ].config[ 1 ] == 0xfc000 &&
                     addr < 0x74000 ) {
                    saturn.ram[ addr - 0x70000 ] = val;
                    break;
                }
                if ( saturn.mem_cntl[ MCTL_SysRAM_SX ].config[ 1 ] == 0xfe000 &&
                     addr < 0x72000 ) {
                    saturn.ram[ addr - 0x70000 ] = val;
                    break;
                }
                if ( saturn.mem_cntl[ MCTL_SysRAM_SX ].config[ 1 ] ==
                     0xf0000 ) {
                    saturn.ram[ addr - 0x70000 ] = val;
                    break;
                }
            }
            return;
        case 8:
        case 9:
        case 0xa:
        case 0xb:
            if ( saturn.mem_cntl[ MCTL_PORT1_SX ].config[ 0 ] == 0x80000 ) {
                if ( port1_is_ram )
                    saturn.port1[ ( addr - 0x80000 ) & port1_mask ] = val;
                return;
            }
            if ( saturn.mem_cntl[ MCTL_PORT2_SX ].config[ 0 ] == 0x80000 ) {
                if ( port2_is_ram )
                    saturn.port2[ ( addr - 0x80000 ) & port2_mask ] = val;
                return;
            }
            return;
        case 0xc:
        case 0xd:
        case 0xe:
            if ( saturn.mem_cntl[ MCTL_PORT1_SX ].config[ 0 ] == 0xc0000 ) {
                if ( port1_is_ram )
                    saturn.port1[ ( addr - 0xc0000 ) & port1_mask ] = val;
                return;
            }
            if ( saturn.mem_cntl[ MCTL_PORT2_SX ].config[ 0 ] == 0xc0000 ) {
                if ( port2_is_ram )
                    saturn.port2[ ( addr - 0xc0000 ) & port2_mask ] = val;
                return;
            }
            return;
        case 0xf:
            if ( saturn.mem_cntl[ MCTL_SysRAM_SX ].config[ 0 ] == 0xf0000 ) {
                saturn.ram[ addr - 0xf0000 ] = val;
                break;
            }
            if ( saturn.mem_cntl[ MCTL_PORT1_SX ].config[ 0 ] == 0xc0000 ) {
                if ( port1_is_ram )
                    saturn.port1[ ( addr - 0xc0000 ) & port1_mask ] = val;
                return;
            }
            if ( saturn.mem_cntl[ MCTL_PORT2_SX ].config[ 0 ] == 0xc0000 ) {
                if ( port2_is_ram )
                    saturn.port2[ ( addr - 0xc0000 ) & port2_mask ] = val;
                return;
            }
            return;
    }
    if ( device.display_touched || !disp.mapped )
        return;
    if ( addr >= display.disp_start && addr < display.disp_end ) {
        disp_draw_nibble( addr, val );
    }
    if ( display.lines == 63 )
        return;
    if ( addr >= display.menu_start && addr < display.menu_end ) {
        menu_draw_nibble( addr, val );
    }
}

void write_nibble_gx( long addr, int val ) {
    addr &= 0xfffff;
    val &= 0x0f;
    switch ( ( int )( addr >> 16 ) & 0x0f ) {
        case 0:
            if ( addr < 0x140 && addr >= 0x100 &&
                 saturn.mem_cntl[ MCTL_MMIO_GX ].config[ 0 ] == 0x100 ) {
                write_dev_mem( addr, val );
                return;
            }
            return;
        case 1:
        case 2:
        case 3:
        case 5:
        case 6:
            return;
        case 4:
            if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 0 ] == 0x40000 ) {
                saturn.ram[ addr - 0x40000 ] = val;
                break;
            }
            return;
        case 7:
            if ( addr >= 0x7f000 &&
                 saturn.mem_cntl[ MCTL_BANK_GX ].config[ 0 ] == 0x7f000 ) {
                return;
            }
            if ( addr >= 0x7e000 && addr < 0x7f000 &&
                 saturn.mem_cntl[ MCTL_PORT1_GX ].config[ 0 ] == 0x7e000 ) {
                return;
            }
            if ( addr >= 0x7e000 && addr < 0x7f000 &&
                 saturn.mem_cntl[ MCTL_PORT2_GX ].config[ 0 ] == 0x7e000 ) {
                return;
            }
            return;
        case 8:
            if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 0 ] == 0x80000 ) {
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xfc000 &&
                     addr < 0x84000 ) {
                    saturn.ram[ addr - 0x80000 ] = val;
                    break;
                }
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xfe000 &&
                     addr < 0x82000 ) {
                    saturn.ram[ addr - 0x80000 ] = val;
                    break;
                }
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] ==
                     0xf0000 ) {
                    saturn.ram[ addr - 0x80000 ] = val;
                    break;
                }
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] ==
                     0xc0000 ) {
                    saturn.ram[ addr - 0x80000 ] = val;
                    break;
                }
            }
            return;
        case 9:
            if ( saturn.mem_cntl[ MCTL_BANK_GX ].config[ 0 ] == 0x90000 ) {
                if ( addr < 0x91000 ) {
                    return;
                }
            }
            if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 0 ] == 0x80000 )
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] ==
                     0xc0000 ) {
                    saturn.ram[ addr - 0x80000 ] = val;
                    break;
                }
            return;
        case 0xa:
            if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 0 ] == 0x80000 )
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] ==
                     0xc0000 ) {
                    saturn.ram[ addr - 0x80000 ] = val;
                    break;
                }
            if ( saturn.mem_cntl[ MCTL_PORT1_GX ].config[ 0 ] == 0xa0000 ) {
                if ( port1_is_ram )
                    saturn.port1[ ( addr - 0xa0000 ) & port1_mask ] = val;
                return;
            }
            return;
        case 0xb:
            if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 0 ] == 0x80000 )
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] ==
                     0xc0000 ) {
                    saturn.ram[ addr - 0x80000 ] = val;
                    break;
                }
            if ( saturn.mem_cntl[ MCTL_PORT2_GX ].config[ 0 ] == 0xb0000 ) {
                if ( port2_is_ram )
                    saturn.port2[ ( ( saturn.bank_switch << 18 ) +
                                    ( addr - 0xb0000 ) ) &
                                  port2_mask ] = val;
                /*
                            if (port2_size > (saturn.bank_switch << 18))
                              {
                                if (port2_is_ram)
                                  saturn.port2[(saturn.bank_switch << 18)
                                               + (addr - 0xb0000)] = val;
                              }
                */
                return;
            }
            return;
        case 0xc:
            if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 0 ] == 0xc0000 ) {
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xfc000 &&
                     addr < 0xc4000 ) {
                    saturn.ram[ addr - 0xc0000 ] = val;
                    break;
                }
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xfe000 &&
                     addr < 0xc2000 ) {
                    saturn.ram[ addr - 0xc0000 ] = val;
                    break;
                }
                saturn.ram[ addr - 0xc0000 ] = val;
                break;
            }
            if ( saturn.mem_cntl[ MCTL_PORT1_GX ].config[ 0 ] == 0xc0000 ) {
                if ( port1_is_ram )
                    saturn.port1[ ( addr - 0xc0000 ) & port1_mask ] = val;
                return;
            }
            if ( saturn.mem_cntl[ MCTL_PORT2_GX ].config[ 0 ] == 0xc0000 ) {
                if ( port2_is_ram )
                    saturn.port2[ ( ( saturn.bank_switch << 18 ) +
                                    ( addr - 0xc0000 ) ) &
                                  port2_mask ] = val;
                /*
                            if (port2_size > (saturn.bank_switch << 18))
                              {
                                if (port2_is_ram)
                                  saturn.port2[(saturn.bank_switch << 18)
                                               + (addr - 0xc0000)] = val;
                              }
                */
                return;
            }
            return;
        case 0xd:
        case 0xe:
        case 0xf:
            if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 0 ] == 0xc0000 )
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] ==
                     0xc0000 ) {
                    saturn.ram[ addr - 0xc0000 ] = val;
                    break;
                }
            if ( saturn.mem_cntl[ MCTL_PORT1_GX ].config[ 0 ] == 0xc0000 )
                if ( saturn.mem_cntl[ MCTL_PORT1_GX ].config[ 1 ] == 0xc0000 ) {
                    if ( port1_is_ram )
                        saturn.port1[ ( addr - 0xc0000 ) & port1_mask ] = val;
                    return;
                }
            if ( saturn.mem_cntl[ MCTL_PORT2_GX ].config[ 0 ] == 0xc0000 )
                if ( saturn.mem_cntl[ MCTL_PORT2_GX ].config[ 1 ] == 0xc0000 ) {
                    if ( port2_is_ram )
                        saturn.port2[ ( ( saturn.bank_switch << 18 ) +
                                        ( addr - 0xc0000 ) ) &
                                      port2_mask ] = val;
                    /*
                                  if (port2_size > (saturn.bank_switch << 18))
                                    {
                                      if (port2_is_ram)
                                        saturn.port2[(saturn.bank_switch << 18)
                                                     + (addr - 0xc0000)] = val;
                                    }
                    */
                    return;
                }
            return;
    }
    if ( device.display_touched || !disp.mapped )
        return;
    if ( addr >= display.disp_start && addr < display.disp_end ) {
        disp_draw_nibble( addr, val );
    }
    if ( display.lines == 63 )
        return;
    if ( addr >= display.menu_start && addr < display.menu_end ) {
        menu_draw_nibble( addr, val );
    }
}

int read_nibble_sx( long addr ) {
    addr &= 0xfffff;
    switch ( ( int )( addr >> 16 ) & 0x0f ) {
        case 0:
            if ( addr < 0x140 && addr >= 0x100 ) {
                if ( saturn.mem_cntl[ MCTL_MMIO_SX ].config[ 0 ] == 0x100 )
                    return read_dev_mem( addr );
                else
                    return 0x00;
            }
            return saturn.rom[ addr ];
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            return saturn.rom[ addr ];
        case 7:
            if ( saturn.mem_cntl[ MCTL_SysRAM_SX ].config[ 0 ] == 0x70000 ) {
                if ( saturn.mem_cntl[ MCTL_SysRAM_SX ].config[ 1 ] == 0xfc000 &&
                     addr < 0x74000 )
                    return saturn.ram[ addr - 0x70000 ];
                if ( saturn.mem_cntl[ MCTL_SysRAM_SX ].config[ 1 ] == 0xfe000 &&
                     addr < 0x72000 )
                    return saturn.ram[ addr - 0x70000 ];
                if ( saturn.mem_cntl[ MCTL_SysRAM_SX ].config[ 1 ] == 0xf0000 )
                    return saturn.ram[ addr - 0x70000 ];
            }
            return saturn.rom[ addr ];
        case 8:
        case 9:
        case 0xa:
        case 0xb:
            if ( saturn.mem_cntl[ MCTL_PORT1_SX ].config[ 0 ] == 0x80000 ) {
                return saturn.port1[ ( addr - 0x80000 ) & port1_mask ];
            }
            if ( saturn.mem_cntl[ MCTL_PORT2_SX ].config[ 0 ] == 0x80000 ) {
                return saturn.port2[ ( addr - 0x80000 ) & port2_mask ];
            }
            return 0x00;
        case 0xc:
        case 0xd:
        case 0xe:
            if ( saturn.mem_cntl[ MCTL_PORT1_SX ].config[ 0 ] == 0xc0000 ) {
                return saturn.port1[ ( addr - 0xc0000 ) & port1_mask ];
            }
            if ( saturn.mem_cntl[ MCTL_PORT2_SX ].config[ 0 ] == 0xc0000 ) {
                return saturn.port2[ ( addr - 0xc0000 ) & port2_mask ];
            }
            return 0x00;
        case 0xf:
            if ( saturn.mem_cntl[ MCTL_SysRAM_SX ].config[ 0 ] == 0xf0000 )
                return saturn.ram[ addr - 0xf0000 ];
            if ( saturn.mem_cntl[ MCTL_PORT1_SX ].config[ 0 ] == 0xf0000 ) {
                return saturn.port1[ ( addr - 0xf0000 ) & port1_mask ];
            }
            if ( saturn.mem_cntl[ MCTL_PORT2_SX ].config[ 0 ] == 0xf0000 ) {
                return saturn.port2[ ( addr - 0xf0000 ) & port2_mask ];
            }
            return 0x00;
    }
    return 0x00;
}

int read_nibble_gx( long addr ) {
    addr &= 0xfffff;
    switch ( ( int )( addr >> 16 ) & 0x0f ) {
        case 0:
            if ( addr < 0x140 && addr >= 0x100 ) {
                if ( saturn.mem_cntl[ 0 ].config[ 0 ] == 0x100 )
                    return read_dev_mem( addr );
                else
                    return 0x00;
            }
            return saturn.rom[ addr ];
        case 1:
        case 2:
        case 3:
        case 5:
        case 6:
            return saturn.rom[ addr ];
        case 4:
            if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 0 ] == 0x40000 )
                return saturn.ram[ addr - 0x40000 ];
            return saturn.rom[ addr ];
        case 7:
            if ( addr >= 0x7f000 &&
                 saturn.mem_cntl[ MCTL_BANK_GX ].config[ 0 ] == 0x7f000 ) {
                if ( addr == 0x7f000 ) {
                    saturn.bank_switch = 0;
                }
                if ( addr >= 0x7f040 && addr < 0x7f080 ) {
                    saturn.bank_switch = ( addr - 0x7f040 ) / 2;
                }
                return 0x7;
            }
            if ( addr >= 0x7e000 && addr < 0x7f000 &&
                 saturn.mem_cntl[ MCTL_PORT1_GX ].config[ 0 ] == 0x7e000 ) {
                return 0x7;
            }
            if ( addr >= 0x7e000 && addr < 0x7f000 &&
                 saturn.mem_cntl[ MCTL_PORT2_GX ].config[ 0 ] == 0x7e000 ) {
                return 0x7;
            }
            return saturn.rom[ addr ];
        case 8:
            if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 0 ] == 0x80000 ) {
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xfc000 &&
                     addr < 0x84000 )
                    return saturn.ram[ addr - 0x80000 ];
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xfe000 &&
                     addr < 0x82000 )
                    return saturn.ram[ addr - 0x80000 ];
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xf0000 )
                    return saturn.ram[ addr - 0x80000 ];
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xc0000 )
                    return saturn.ram[ addr - 0x80000 ];
            }
            return saturn.rom[ addr ];
        case 9:
            if ( saturn.mem_cntl[ 0 ].config[ 0 ] == 0x90000 ) {
                if ( addr < 0x91000 ) {
                    if ( addr == 0x90000 ) {
                        saturn.bank_switch = 0;
                    }
                    if ( addr >= 0x90040 && addr < 0x90080 ) {
                        saturn.bank_switch = ( addr - 0x90040 ) / 2;
                    }
                    return 0x7;
                }
            }
            if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 0 ] == 0x80000 )
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xc0000 )
                    return saturn.ram[ addr - 0x80000 ];
            return saturn.rom[ addr ];
        case 0xa:
            if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 0 ] == 0x80000 )
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xc0000 )
                    return saturn.ram[ addr - 0x80000 ];
            if ( saturn.mem_cntl[ MCTL_PORT1_GX ].config[ 0 ] == 0xa0000 ) {
                return saturn.port1[ ( addr - 0xa0000 ) & port1_mask ];
            }
            return saturn.rom[ addr ];
        case 0xb:
            if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 0 ] == 0x80000 )
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xc0000 )
                    return saturn.ram[ addr - 0x80000 ];
            if ( saturn.mem_cntl[ MCTL_PORT2_GX ].config[ 0 ] == 0xb0000 ) {
                return saturn.port2[ ( ( saturn.bank_switch << 18 ) +
                                       ( addr - 0xb0000 ) ) &
                                     port2_mask ];
                /*
                            if (port2_size > (saturn.bank_switch << 18))
                              {
                                return saturn.port2[(saturn.bank_switch << 18)
                                                    + (addr - 0xb0000)];
                              }
                            return 0x00;
                */
            }
            return saturn.rom[ addr ];
        case 0xc:
            if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 0 ] == 0xc0000 ) {
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xfc000 &&
                     addr < 0xc4000 )
                    return saturn.ram[ addr - 0xc0000 ];
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xfe000 &&
                     addr < 0xc2000 )
                    return saturn.ram[ addr - 0xc0000 ];
                return saturn.ram[ addr - 0xc0000 ];
            }
            if ( saturn.mem_cntl[ MCTL_PORT1_GX ].config[ 0 ] == 0xc0000 ) {
                return saturn.port1[ ( addr - 0xc0000 ) & port1_mask ];
            }
            if ( saturn.mem_cntl[ MCTL_PORT2_GX ].config[ 0 ] == 0xc0000 ) {
                return saturn.port2[ ( ( saturn.bank_switch << 18 ) +
                                       ( addr - 0xc0000 ) ) &
                                     port2_mask ];
                /*
                            if (port2_size > (saturn.bank_switch << 18))
                              {
                                return saturn.port2[(saturn.bank_switch << 18)
                                                    + (addr - 0xc0000)];
                              }
                            return 0x00;
                */
            }
            return saturn.rom[ addr ];
        case 0xd:
        case 0xe:
        case 0xf:
            if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 0 ] == 0xc0000 )
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xc0000 )
                    return saturn.ram[ addr - 0xc0000 ];
            if ( saturn.mem_cntl[ MCTL_PORT1_GX ].config[ 0 ] == 0xc0000 )
                if ( saturn.mem_cntl[ MCTL_PORT1_GX ].config[ 1 ] == 0xc0000 ) {
                    return saturn.port1[ ( addr - 0xc0000 ) & port1_mask ];
                }
            if ( saturn.mem_cntl[ MCTL_PORT2_GX ].config[ 0 ] == 0xc0000 )
                if ( saturn.mem_cntl[ MCTL_PORT2_GX ].config[ 1 ] == 0xc0000 ) {
                    return saturn.port2[ ( ( saturn.bank_switch << 18 ) +
                                           ( addr - 0xc0000 ) ) &
                                         port2_mask ];
                    /*
                                  if (port2_size > (saturn.bank_switch << 18))
                                    {
                                      return saturn.port2[(saturn.bank_switch <<
                       18)
                                                          + (addr - 0xc0000)];
                                    }
                                  return 0x00;
                    */
                }
            return saturn.rom[ addr ];
    }
    return 0x00;
}

int read_nibble_crc_sx( long addr ) {
    addr &= 0xfffff;
    switch ( ( int )( addr >> 16 ) & 0x0f ) {
        case 0:
            if ( addr < 0x140 && addr >= 0x100 ) {
                if ( saturn.mem_cntl[ MCTL_MMIO_SX ].config[ 0 ] == 0x100 )
                    return read_dev_mem( addr );
                else
                    return calc_crc( 0x00 );
            }
            return calc_crc( saturn.rom[ addr ] );
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            return calc_crc( saturn.rom[ addr ] );
        case 7:
            if ( saturn.mem_cntl[ MCTL_SysRAM_SX ].config[ 0 ] == 0x70000 ) {
                if ( saturn.mem_cntl[ MCTL_SysRAM_SX ].config[ 1 ] == 0xfc000 &&
                     addr < 0x74000 )
                    return calc_crc( saturn.ram[ addr - 0x70000 ] );
                if ( saturn.mem_cntl[ MCTL_SysRAM_SX ].config[ 1 ] == 0xfe000 &&
                     addr < 0x72000 )
                    return calc_crc( saturn.ram[ addr - 0x70000 ] );
                if ( saturn.mem_cntl[ MCTL_SysRAM_SX ].config[ 1 ] == 0xf0000 )
                    return calc_crc( saturn.ram[ addr - 0x70000 ] );
            }
            return calc_crc( saturn.rom[ addr ] );
        case 8:
        case 9:
        case 0xa:
        case 0xb:
            if ( saturn.mem_cntl[ MCTL_PORT1_SX ].config[ 0 ] == 0x80000 ) {
                return calc_crc(
                    saturn.port1[ ( addr - 0x80000 ) & port1_mask ] );
            }
            if ( saturn.mem_cntl[ MCTL_PORT2_SX ].config[ 0 ] == 0x80000 ) {
                return calc_crc(
                    saturn.port2[ ( addr - 0x80000 ) & port2_mask ] );
            }
            return 0x00;
        case 0xc:
        case 0xd:
        case 0xe:
            if ( saturn.mem_cntl[ MCTL_PORT1_SX ].config[ 0 ] == 0xc0000 ) {
                return calc_crc(
                    saturn.port1[ ( addr - 0xc0000 ) & port1_mask ] );
            }
            if ( saturn.mem_cntl[ MCTL_PORT2_SX ].config[ 0 ] == 0xc0000 ) {
                return calc_crc(
                    saturn.port2[ ( addr - 0xc0000 ) & port2_mask ] );
            }
            return 0x00;
        case 0xf:
            if ( saturn.mem_cntl[ MCTL_SysRAM_SX ].config[ 0 ] == 0xf0000 )
                return calc_crc( saturn.ram[ addr - 0xf0000 ] );
            if ( saturn.mem_cntl[ MCTL_PORT1_SX ].config[ 0 ] == 0xc0000 ) {
                return calc_crc(
                    saturn.port1[ ( addr - 0xc0000 ) & port1_mask ] );
            }
            if ( saturn.mem_cntl[ MCTL_PORT2_SX ].config[ 0 ] == 0xc0000 ) {
                return calc_crc(
                    saturn.port2[ ( addr - 0xc0000 ) & port2_mask ] );
            }
            return 0x00;
    }
    return 0x00;
}

int read_nibble_crc_gx( long addr ) {
    addr &= 0xfffff;
    switch ( ( int )( addr >> 16 ) & 0x0f ) {
        case 0:
            if ( addr < 0x140 && addr >= 0x100 ) {
                if ( saturn.mem_cntl[ MCTL_MMIO_GX ].config[ 0 ] == 0x100 )
                    return read_dev_mem( addr );
                else
                    return calc_crc( 0x00 );
            }
            return calc_crc( saturn.rom[ addr ] );
        case 1:
        case 2:
        case 3:
        case 5:
        case 6:
            return calc_crc( saturn.rom[ addr ] );
        case 4:
            if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 0 ] == 0x40000 )
                return calc_crc( saturn.ram[ addr - 0x40000 ] );
            return calc_crc( saturn.rom[ addr ] );
        case 7:
            if ( addr >= 0x7f000 &&
                 saturn.mem_cntl[ MCTL_BANK_GX ].config[ 0 ] == 0x7f000 ) {
                if ( addr == 0x7f000 ) {
                    saturn.bank_switch = 0;
                }
                if ( addr >= 0x7f040 && addr < 0x7f080 ) {
                    saturn.bank_switch = ( addr - 0x7f040 ) / 2;
                }
                return 0x7;
            }
            if ( addr >= 0x7e000 && addr < 0x7f000 &&
                 saturn.mem_cntl[ MCTL_PORT1_GX ].config[ 0 ] == 0x7e000 ) {
                return 0x7;
            }
            if ( addr >= 0x7e000 && addr < 0x7f000 &&
                 saturn.mem_cntl[ MCTL_PORT2_GX ].config[ 0 ] == 0x7e000 ) {
                return 0x7;
            }
            return calc_crc( saturn.rom[ addr ] );
        case 8:
            if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 0 ] == 0x80000 ) {
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xfc000 &&
                     addr < 0x84000 )
                    return calc_crc( saturn.ram[ addr - 0x80000 ] );
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xfe000 &&
                     addr < 0x82000 )
                    return calc_crc( saturn.ram[ addr - 0x80000 ] );
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xf0000 )
                    return calc_crc( saturn.ram[ addr - 0x80000 ] );
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xc0000 )
                    return calc_crc( saturn.ram[ addr - 0x80000 ] );
            }
            return calc_crc( saturn.rom[ addr ] );
        case 9:
            if ( saturn.mem_cntl[ 0 ].config[ 0 ] == 0x90000 ) {
                if ( addr < 0x91000 ) {
                    if ( addr == 0x90000 ) {
                        saturn.bank_switch = 0;
                    }
                    if ( addr >= 0x90040 && addr < 0x90080 ) {
                        saturn.bank_switch = ( addr - 0x90040 ) / 2;
                    }
                    return 0x7;
                }
            }
            if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 0 ] == 0x80000 )
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xc0000 )
                    return calc_crc( saturn.ram[ addr - 0x80000 ] );
            return calc_crc( saturn.rom[ addr ] );
        case 0xa:
            if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 0 ] == 0x80000 )
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xc0000 )
                    return calc_crc( saturn.ram[ addr - 0x80000 ] );
            if ( saturn.mem_cntl[ MCTL_PORT1_GX ].config[ 0 ] == 0xa0000 ) {
                return calc_crc(
                    saturn.port1[ ( addr - 0xa0000 ) & port1_mask ] );
            }
            return calc_crc( saturn.rom[ addr ] );
        case 0xb:
            if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 0 ] == 0x80000 )
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xc0000 )
                    return calc_crc( saturn.ram[ addr - 0x80000 ] );
            if ( saturn.mem_cntl[ MCTL_PORT2_GX ].config[ 0 ] == 0xb0000 ) {
                return calc_crc( saturn.port2[ ( ( saturn.bank_switch << 18 ) +
                                                 ( addr - 0xb0000 ) ) &
                                               port2_mask ] );
                /*
                            if (port2_size > (saturn.bank_switch << 18))
                              {
                                return calc_crc(saturn.port2[(saturn.bank_switch
                   << 18)
                                                             + (addr -
                   0xb0000)]);
                              }
                            return 0x00;
                */
            }
            return calc_crc( saturn.rom[ addr ] );
        case 0xc:
            if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 0 ] == 0xc0000 ) {
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xfc000 &&
                     addr < 0xc4000 )
                    return calc_crc( saturn.ram[ addr - 0xc0000 ] );
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xfe000 &&
                     addr < 0xc2000 )
                    return calc_crc( saturn.ram[ addr - 0xc0000 ] );
                return calc_crc( saturn.ram[ addr - 0xc0000 ] );
            }
            if ( saturn.mem_cntl[ MCTL_PORT1_GX ].config[ 0 ] == 0xc0000 ) {
                return calc_crc(
                    saturn.port1[ ( addr - 0xc0000 ) & port1_mask ] );
            }
            if ( saturn.mem_cntl[ MCTL_PORT2_GX ].config[ 0 ] == 0xc0000 ) {
                return calc_crc( saturn.port2[ ( ( saturn.bank_switch << 18 ) +
                                                 ( addr - 0xc0000 ) ) &
                                               port2_mask ] );
                /*
                            if (port2_size > (saturn.bank_switch << 18))
                              {
                                return calc_crc(saturn.port2[(saturn.bank_switch
                   << 18)
                                                             + (addr -
                   0xc0000)]);
                              }
                            return 0x00;
                */
            }
            return calc_crc( saturn.rom[ addr ] );
        case 0xd:
        case 0xe:
        case 0xf:
            if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 0 ] == 0xc0000 )
                if ( saturn.mem_cntl[ MCTL_SysRAM_GX ].config[ 1 ] == 0xc0000 )
                    return calc_crc( saturn.ram[ addr - 0xc0000 ] );
            if ( saturn.mem_cntl[ MCTL_PORT1_GX ].config[ 0 ] == 0xc0000 )
                if ( saturn.mem_cntl[ MCTL_PORT1_GX ].config[ 1 ] == 0xc0000 ) {
                    return calc_crc(
                        saturn.port1[ ( addr - 0xc0000 ) & port1_mask ] );
                }
            if ( saturn.mem_cntl[ MCTL_PORT2_GX ].config[ 0 ] == 0xc0000 )
                if ( saturn.mem_cntl[ MCTL_PORT2_GX ].config[ 1 ] == 0xc0000 ) {
                    return calc_crc(
                        saturn.port2[ ( ( saturn.bank_switch << 18 ) +
                                        ( addr - 0xc0000 ) ) &
                                      port2_mask ] );
                    /*
                                  if (port2_size > (saturn.bank_switch << 18))
                                    {
                                      return
                       calc_crc(saturn.port2[(saturn.bank_switch << 18)
                                                                   + (addr -
                       0xc0000)]);
                                    }
                                  return 0x00;
                    */
                }
            return calc_crc( saturn.rom[ addr ] );
    }
    return 0x00;
}

long read_nibbles( long addr, int len ) {
    long val = 0;

    addr += len;
    while ( len-- > 0 )
        val = ( val << 4 ) | read_nibble( --addr );

    return val;
}

void write_nibbles( long addr, long val, int len ) {
    while ( len-- > 0 ) {
        write_nibble( addr++, val );
        val >>= 4;
    }
}

void dev_memory_init( void ) {
    if ( opt_gx ) {
        read_nibble = read_nibble_gx;
        read_nibble_crc = read_nibble_crc_gx;
        write_nibble = write_nibble_gx;
    } else {
        read_nibble = read_nibble_sx;
        read_nibble_crc = read_nibble_crc_sx;
        write_nibble = write_nibble_sx;
    }
    memset( &device, 0, sizeof( device ) );
}
