#ifndef _DEVICE_H
#define _DEVICE_H 1

#include "hp48.h"

#define DISP_INSTR_OFF 0x10

#define ANN_LEFT 0x81
#define ANN_RIGHT 0x82
#define ANN_ALPHA 0x84
#define ANN_BATTERY 0x88
#define ANN_BUSY 0x90
#define ANN_IO 0xa0

typedef struct device_t {

    int display_touched;

    char contrast_touched;

    char disp_test_touched;

    char crc_touched;

    char power_status_touched;
    char power_ctrl_touched;

    char mode_touched;

    char ann_touched;

    char baud_touched;

    char card_ctrl_touched;
    char card_status_touched;

    char ioc_touched;

    char tcs_touched;
    char rcs_touched;

    char rbr_touched;
    char tbr_touched;

    char sreq_touched;

    char ir_ctrl_touched;

    char base_off_touched;

    char lcr_touched;
    char lbr_touched;

    char scratch_touched;
    char base_nibble_touched;

    char unknown_touched;

    char t1_ctrl_touched;
    char t2_ctrl_touched;

    char unknown2_touched;

    char t1_touched;
    char t2_touched;

} device_t;

extern device_t device;
extern void check_devices( void ); /* device.c */

/* extern void	check_out_register( void ); */

extern void update_display( void );                       /* device.c */
extern void redraw_display( void );                       /* device.c */
extern void disp_draw_nibble( word_20 addr, word_4 val ); /* device.c */
extern void menu_draw_nibble( word_20 addr, word_4 val ); /* device.c */
extern void draw_annunc( void );                          /* device.c */
extern void redraw_annunc( void );                        /* device.c */

#endif /* !_DEVICE_H */
