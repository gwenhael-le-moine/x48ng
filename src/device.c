#include "global.h"

#include <stdlib.h>

#include "device.h"
#include "hp48.h"
#include "hp48_emu.h"
#include "timer.h"
#include "x48_x11.h"

extern int device_check;

device_t device;

void check_devices( void ) {
    if ( device.display_touched > 0 && device.display_touched-- == 1 ) {
        device.display_touched = 0;
        update_display();
    }
    if ( device.display_touched > 0 ) {
        device_check = 1;
    }
    if ( device.contrast_touched ) {
        device.contrast_touched = 0;
        adjust_contrast( display.contrast );
    }
    if ( device.ann_touched ) {
        device.ann_touched = 0;
        draw_annunc();
    }
    if ( device.baud_touched ) {
        device.baud_touched = 0;
        serial_baud( saturn.baud );
    }
    if ( device.ioc_touched ) {
        device.ioc_touched = 0;
        if ( ( saturn.io_ctrl & 0x02 ) && ( saturn.rcs & 0x01 ) ) {
            do_interupt();
        }
    }
    if ( device.rbr_touched ) {
        device.rbr_touched = 0;
        receive_char();
    }
    if ( device.tbr_touched ) {
        device.tbr_touched = 0;
        transmit_char();
    }
    if ( device.t1_touched ) {
        saturn.t1_instr = 0;
        sched_timer1 = saturn.t1_tick;
        restart_timer( T1_TIMER );
        set_t1 = saturn.timer1;
        device.t1_touched = 0;
    }
    if ( device.t2_touched ) {
        saturn.t2_instr = 0;
        sched_timer2 = saturn.t2_tick;
        device.t2_touched = 0;
    }
#if 0
  if (device.disp_test_touched) {
    device.disp_test_touched = 0;
  }
  if (device.crc_touched) {
    device.crc_touched = 0;
  }
  if (device.power_status_touched) {
    device.power_status_touched = 0;
  }
  if (device.power_ctrl_touched) {
    device.power_ctrl_touched = 0;
  }
  if (device.mode_touched) {
    device.mode_touched = 0;
  }
  if (device.card_ctrl_touched) {
    device.card_ctrl_touched = 0;
  }
  if (device.card_status_touched) {
    device.card_status_touched = 0;
  }
  if (device.tcs_touched) {
    device.tcs_touched = 0;
  }
  if (device.rcs_touched) {
    device.rcs_touched = 0;
  }
  if (device.sreq_touched) {
    device.sreq_touched = 0;
  }
  if (device.ir_ctrl_touched) {
    device.ir_ctrl_touched = 0;
  }
  if (device.base_off_touched) {
    device.base_off_touched = 0;
  }
  if (device.lcr_touched) {
    device.lcr_touched = 0;
  }
  if (device.lbr_touched) {
    device.lbr_touched = 0;
  }
  if (device.scratch_touched) {
    device.scratch_touched = 0;
  }
  if (device.base_nibble_touched) {
    device.base_nibble_touched = 0;
  }
  if (device.unknown_touched) {
    device.unknown_touched = 0;
  }
  if (device.t1_ctrl_touched) {
    device.t1_ctrl_touched = 0;
  }
  if (device.t2_ctrl_touched) {
    device.t2_ctrl_touched = 0;
  }
  if (device.unknown2_touched) {
    device.unknown2_touched = 0;
  }
#endif
}

#if 0

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void check_out_register(void) {
  static int au = -2;
  unsigned char c[] = { 0xff, 0x00 };

  if (au == -2)
    if ((au = open("/dev/audio", O_WRONLY)) < 0)
  if (au < 0)
    return;
  if (saturn.OUT[2] & 0x8)
    write(au, c, 1);
  else
    write(au, &c[1], 1);
}

#endif
