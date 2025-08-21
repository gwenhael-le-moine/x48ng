#ifndef _SERIAL_H
#define _SERIAL_H 1

extern void serial_baud( int baud );
extern void transmit_char( void );
extern void receive_char( void );
extern int init_serial( void ); /* used in main.c */

#endif /* !_SERIAL_H */
