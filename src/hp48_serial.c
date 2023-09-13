#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

#include "options.h"
#include "hp48.h"
#include "hp48emu.h"

static int wire_fd;
static int ir_fd;
static int ttyp;

extern int rece_instr;

char* wire_name = ( char* )0;
char* ir_name = ( char* )0;

void update_connection_display( void ) {
    if ( wire_fd == -1 ) {
        if ( wire_name )
            free( wire_name );
        wire_name = ( char* )0;
    }
    if ( ir_fd == -1 ) {
        if ( ir_name )
            free( ir_name );
        ir_name = ( char* )0;
    }
}

int serial_init( void ) {
    int c;
    int n;
    char tty_dev_name[ 128 ];
    struct termios ttybuf;

    wire_fd = -1;
    ttyp = -1;
    if ( useTerminal ) {
        /* Unix98 PTY (Preferred) */
        if ( ( wire_fd = open( "/dev/ptmx", O_RDWR | O_NONBLOCK, 0666 ) ) >=
             0 ) {
            grantpt( wire_fd );
            unlockpt( wire_fd );
            if ( ptsname_r( wire_fd, tty_dev_name, 128 ) ) {
                perror( "Could not get the name of the wire device." );
                exit( -1 );
            }
            if ( ( ttyp = open( tty_dev_name, O_RDWR | O_NDELAY, 0666 ) ) >=
                 0 ) {
                if ( verbose )
                    printf( "wire connection on %s\n", tty_dev_name );
                wire_name = strdup( tty_dev_name );
            }
        }
        /* BSD PTY (Legacy) */
        else {
            c = 'p';
            do {
                for ( n = 0; n < 16; n++ ) {
                    sprintf( tty_dev_name, "/dev/pty%c%x", c, n );
                    if ( ( wire_fd =
                               open( tty_dev_name, O_RDWR | O_EXCL | O_NDELAY,
                                     0666 ) ) >= 0 ) {
                        ttyp = wire_fd;
                        sprintf( tty_dev_name, "/dev/tty%c%x", c, n );
                        if ( verbose )
                            printf( "wire connection on %s\n", tty_dev_name );
                        wire_name = strdup( tty_dev_name );
                        break;
                    }
                }
                c++;
            } while ( ( wire_fd < 0 ) && ( errno != ENOENT ) );
        }
    }

    if ( ttyp >= 0 ) {
#if defined( TCSANOW )
        if ( tcgetattr( ttyp, &ttybuf ) < 0 )
#else
        if ( ioctl( ttyp, TCGETS, ( char* )&ttybuf ) < 0 )
#endif
        {
            if ( verbose )
                fprintf( stderr, "ioctl(wire, TCGETS) failed, errno = %d\n",
                         errno );
            wire_fd = -1;
            ttyp = -1;
        }
    }

    ttybuf.c_lflag = 0;
    ttybuf.c_iflag = 0;
    ttybuf.c_oflag = 0;
    ttybuf.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    for ( n = 0; n < NCCS; n++ )
        ttybuf.c_cc[ n ] = 0;
    ttybuf.c_cc[ VTIME ] = 0;
    ttybuf.c_cc[ VMIN ] = 1;

    if ( ttyp >= 0 ) {
#if defined( TCSANOW )
        if ( tcsetattr( ttyp, TCSANOW, &ttybuf ) < 0 )
#else
        if ( ioctl( ttyp, TCSETS, ( char* )&ttybuf ) < 0 )
#endif
        {
            if ( verbose )
                fprintf( stderr, "ioctl(wire, TCSETS) failed, errno = %d\n",
                         errno );
            wire_fd = -1;
            ttyp = -1;
        }
    }

    ir_fd = -1;
    if ( useSerial ) {
        sprintf( tty_dev_name, "%s", serialLine );
        if ( ( ir_fd = open( tty_dev_name, O_RDWR | O_NDELAY ) ) >= 0 ) {
            if ( verbose )
                printf( "IR connection on %s\n", tty_dev_name );
            ir_name = strdup( tty_dev_name );
        }
    }

    if ( ir_fd >= 0 ) {
#if defined( TCSANOW )
        if ( tcgetattr( ir_fd, &ttybuf ) < 0 )
#else
        if ( ioctl( ir_fd, TCGETS, ( char* )&ttybuf ) < 0 )
#endif
        {
            if ( verbose )
                fprintf( stderr, "ioctl(IR, TCGETS) failed, errno = %d\n",
                         errno );
            ir_fd = -1;
        }
    }

    ttybuf.c_lflag = 0;
    ttybuf.c_iflag = 0;
    ttybuf.c_oflag = 0;
    ttybuf.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    for ( n = 0; n < NCCS; n++ )
        ttybuf.c_cc[ n ] = 0;
    ttybuf.c_cc[ VTIME ] = 0;
    ttybuf.c_cc[ VMIN ] = 1;

    if ( ir_fd >= 0 ) {
#if defined( TCSANOW )
        if ( tcsetattr( ir_fd, TCSANOW, &ttybuf ) < 0 )
#else
        if ( ioctl( ir_fd, TCSETS, ( char* )&ttybuf ) < 0 )
#endif
        {
            if ( verbose )
                fprintf( stderr, "ioctl(IR, TCSETS) failed, errno = %d\n",
                         errno );
            ir_fd = -1;
        }
    }
    update_connection_display();
    return 1;
}

void serial_baud( int baud ) {
    int error = 0;
    struct termios ttybuf;

    if ( ir_fd >= 0 ) {
#if defined( TCSANOW )
        if ( tcgetattr( ir_fd, &ttybuf ) < 0 )
#else
        if ( ioctl( ir_fd, TCGETS, ( char* )&ttybuf ) < 0 )
#endif
        {
            if ( verbose )
                fprintf( stderr, "ioctl(IR,  TCGETS) failed, errno = %d\n",
                         errno );
            ir_fd = -1;
            error = 1;
        }
    }

#if defined( __APPLE__ )
    baud &= 0x7;
    switch ( baud ) {
        case 0: /* 1200 */
            ttybuf.c_cflag |= B1200;
            break;
        case 1: /* 1920 */
#ifdef B1920
            ttybuf.c_cflag |= B1920;
#endif
            break;
        case 2: /* 2400 */
            ttybuf.c_cflag |= B2400;
            break;
        case 3: /* 3840 */
#ifdef B3840
            ttybuf.c_cflag |= B3840;
#endif
            break;
        case 4: /* 4800 */
            ttybuf.c_cflag |= B4800;
            break;
        case 5: /* 7680 */
#ifdef B7680
            ttybuf.c_cflag |= B7680;
#endif
            break;
        case 6: /* 9600 */
            ttybuf.c_cflag |= B9600;
            break;
        case 7: /* 15360 */
#ifdef B15360
            ttybuf.c_cflag |= B15360;
#endif
            break;
    }

    if ( ( ir_fd >= 0 ) && ( ( ttybuf.c_ospeed ) == 0 ) ) {
        if ( verbose )
            fprintf( stderr, "can\'t set baud rate, using 9600\n" );
        ttybuf.c_cflag |= B9600;
    }
#else
    ttybuf.c_cflag &= ~CBAUD;

    baud &= 0x7;
    switch ( baud ) {
        case 0: /* 1200 */
            ttybuf.c_cflag |= B1200;
            break;
        case 1: /* 1920 */
#ifdef B1920
            ttybuf.c_cflag |= B1920;
#endif
            break;
        case 2: /* 2400 */
            ttybuf.c_cflag |= B2400;
            break;
        case 3: /* 3840 */
#ifdef B3840
            ttybuf.c_cflag |= B3840;
#endif
            break;
        case 4: /* 4800 */
            ttybuf.c_cflag |= B4800;
            break;
        case 5: /* 7680 */
#ifdef B7680
            ttybuf.c_cflag |= B7680;
#endif
            break;
        case 6: /* 9600 */
            ttybuf.c_cflag |= B9600;
            break;
        case 7: /* 15360 */
#ifdef B15360
            ttybuf.c_cflag |= B15360;
#endif
            break;
    }

    if ( ( ir_fd >= 0 ) && ( ( ttybuf.c_cflag & CBAUD ) == 0 ) ) {
        if ( verbose )
            fprintf( stderr, "can\'t set baud rate, using 9600\n" );
        ttybuf.c_cflag |= B9600;
    }
#endif
    if ( ir_fd >= 0 ) {
#if defined( TCSANOW )
        if ( tcsetattr( ir_fd, TCSANOW, &ttybuf ) < 0 )
#else
        if ( ioctl( ir_fd, TCSETS, ( char* )&ttybuf ) < 0 )
#endif
        {
            if ( verbose )
                fprintf( stderr, "ioctl(IR,  TCSETS) failed, errno = %d\n",
                         errno );
            ir_fd = -1;
            error = 1;
        }
    }

    if ( ttyp >= 0 ) {
#if defined( TCSANOW )
        if ( tcgetattr( ttyp, &ttybuf ) < 0 )
#else
        if ( ioctl( ttyp, TCGETS, ( char* )&ttybuf ) < 0 )
#endif
        {
            if ( verbose )
                fprintf( stderr, "ioctl(wire, TCGETS) failed, errno = %d\n",
                         errno );
            wire_fd = -1;
            ttyp = -1;
            error = 1;
        }
    }

#if defined( __APPLE__ )
#else
    ttybuf.c_cflag &= ~CBAUD;

    baud &= 0x7;
    switch ( baud ) {
        case 0: /* 1200 */
            ttybuf.c_cflag |= B1200;
            break;
        case 1: /* 1920 */
#ifdef B1920
            ttybuf.c_cflag |= B1920;
#endif
            break;
        case 2: /* 2400 */
            ttybuf.c_cflag |= B2400;
            break;
        case 3: /* 3840 */
#ifdef B3840
            ttybuf.c_cflag |= B3840;
#endif
            break;
        case 4: /* 4800 */
            ttybuf.c_cflag |= B4800;
            break;
        case 5: /* 7680 */
#ifdef B7680
            ttybuf.c_cflag |= B7680;
#endif
            break;
        case 6: /* 9600 */
            ttybuf.c_cflag |= B9600;
            break;
        case 7: /* 15360 */
#ifdef B15360
            ttybuf.c_cflag |= B15360;
#endif
            break;
    }

    if ( ( ttyp >= 0 ) && ( ( ttybuf.c_cflag & CBAUD ) == 0 ) ) {
        if ( verbose )
            fprintf( stderr, "can\'t set baud rate, using 9600\n" );
        ttybuf.c_cflag |= B9600;
    }
#endif
    if ( ttyp >= 0 ) {
#if defined( TCSANOW )
        if ( tcsetattr( ttyp, TCSANOW, &ttybuf ) < 0 )
#else
        if ( ioctl( ttyp, TCSETS, ( char* )&ttybuf ) < 0 )
#endif
        {
            if ( verbose )
                fprintf( stderr, "ioctl(wire, TCSETS) failed, errno = %d\n",
                         errno );
            wire_fd = -1;
            ttyp = -1;
            error = 1;
        }
    }
    if ( error )
        update_connection_display();
}

void transmit_char( void ) {
    if ( saturn.ir_ctrl & 0x04 ) {
        if ( ir_fd == -1 ) {
            saturn.tcs &= 0x0e;
            if ( saturn.io_ctrl & 0x04 ) {
                do_interupt();
            }
            return;
        }
    } else {
        if ( wire_fd == -1 ) {
            saturn.tcs &= 0x0e;
            if ( saturn.io_ctrl & 0x04 ) {
                do_interupt();
            }
            return;
        }
    }

    if ( saturn.ir_ctrl & 0x04 ) {
        if ( write( ir_fd, &saturn.tbr, 1 ) == 1 ) {
            saturn.tcs &= 0x0e;
            if ( saturn.io_ctrl & 0x04 ) {
                do_interupt();
            }
        } else {
            if ( errno != EAGAIN ) {
                fprintf( stderr, "serial write error: %d\n", errno );
            }
            saturn.tcs &= 0x0e;
            if ( saturn.io_ctrl & 0x04 ) {
                do_interupt();
            }
        }
    } else {
        if ( write( wire_fd, &saturn.tbr, 1 ) == 1 ) {
            saturn.tcs &= 0x0e;
            if ( saturn.io_ctrl & 0x04 ) {
                do_interupt();
            }
        } else {
            if ( errno != EAGAIN ) {
                if ( verbose )
                    fprintf( stderr, "serial write error: %d\n", errno );
            }
            saturn.tcs &= 0x0e;
            if ( saturn.io_ctrl & 0x04 ) {
                do_interupt();
            }
        }
    }
}

#define NR_BUFFER 256

void receive_char( void ) {
    struct timeval tout;
    fd_set rfds;
    int nfd;
    static unsigned char buf[ NR_BUFFER + 1 ];
    static int nrd = 0, bp = 0;

    rece_instr = 0;

    if ( saturn.ir_ctrl & 0x04 ) {
        if ( ir_fd == -1 )
            return;
    } else if ( wire_fd == -1 )
        return;

    if ( saturn.rcs & 0x01 )
        return;

    if ( nrd == 0 ) {
        tout.tv_sec = 0;
        tout.tv_usec = 0;
        FD_ZERO( &rfds );
        if ( saturn.ir_ctrl & 0x04 ) {
            FD_SET( ir_fd, &rfds );
            nfd = ir_fd + 1;
        } else {
            FD_SET( wire_fd, &rfds );
            nfd = wire_fd + 1;
        }
        if ( ( nfd = select( nfd, &rfds, ( fd_set* )0, ( fd_set* )0, &tout ) ) >
             0 ) {
            if ( saturn.ir_ctrl & 0x04 ) {
                if ( FD_ISSET( ir_fd, &rfds ) ) {
                    nrd = read( ir_fd, buf, NR_BUFFER );
                    if ( nrd < 0 ) {
                        nrd = 0;
                        return;
                    }
                    bp = 0;
                } else
                    return;

            } else {
                if ( FD_ISSET( wire_fd, &rfds ) ) {
                    nrd = read( wire_fd, buf, NR_BUFFER );
                    if ( nrd < 0 ) {
                        nrd = 0;
                        return;
                    }
                    bp = 0;
                } else
                    return;
            }
        } else
            return;
    }
    if ( nrd == 0 )
        return;

    if ( !( saturn.io_ctrl & 0x08 ) ) {
        nrd = 0;
        return;
    }
    saturn.rbr = buf[ bp++ ];
    nrd--;
    saturn.rcs |= 0x01;

    if ( saturn.io_ctrl & 0x02 )
        do_interupt();
}
