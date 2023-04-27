/*
 *  This file is part of x48, an emulator of the HP-48sx Calculator.
 *  Copyright (C) 1994  Eddie C. Dost  (ecd@dressler.de)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* $Log: global.h,v $
 * Revision 1.5  1994/12/08  22:28:39  ecd
 * added generic define for SYSV_TIME if SYSV is defined
 *
 * Revision 1.4  1994/12/07  20:16:41  ecd
 * added more functions missing in SunOS includes
 *
 * Revision 1.4  1994/12/07  20:16:41  ecd
 * added more functions missing in SunOS includes
 *
 * Revision 1.3  1994/11/02  14:51:27  ecd
 * new define for SYSV_TIME
 *
 * Revision 1.2  1994/10/06  16:28:03  ecd
 * added #define USE_SHM
 *
 * Revision 1.1  1994/09/07  12:53:20  ecd
 * Initial revision
 *
 *
 * $Id: global.h,v 1.5 1994/12/08 22:28:39 ecd Exp ecd $
 */

#ifndef _GLOBAL_H
#define _GLOBAL_H 1

#define _GNU_SOURCE

#include "config.h"

#if !defined( __GNUC__ ) || defined( __STRICT_ANSI__ )
#  define inline
#  if !defined( __STDC__ )
#    define const
#  endif
#endif

/*
 * If we are running Linux, `linux' will be defined by gcc.
 */
#if defined( linux )
#  ifndef LINUX
#    define LINUX 1
#  endif

#  define SYSV_TIME 1
#endif /* Not Linux */

#ifdef SYSV
#  ifndef SYSV_TIME
#    define SYSV_TIME 1
#  endif
#endif

#define HAVE_STDIO 1

#ifndef HAVE_STDIO
#  include <stdio.h>
#  include <sys/time.h>
#  include <sys/types.h>
extern int printf( char*, ... );
extern int fprintf( FILE*, char*, ... );
extern int sscanf( char*, char*, ... );
extern void fflush( FILE* );
extern int fseek( FILE*, long, int );
extern int fread( void*, int, int, FILE* );
extern int fwrite( void*, int, int, FILE* );
extern void fclose( FILE* );
extern int fgetc( FILE* );
extern void bzero( void*, int );
extern time_t time( time_t* );
extern int select( int, fd_set*, fd_set*, fd_set*, struct timeval* );
extern int setitimer( int, struct itimerval*, struct itimerval* );
extern int gethostname( char*, int );
#  ifdef HAVE_XSHM
#    include <sys/ipc.h>
#    include <sys/shm.h>
extern int shmget( key_t, int, int );
extern int shmat( int, void*, int );
extern int shmctl( int, int, struct shmid_ds* );
#  endif
#endif

#endif /* !_GLOBAL_H */
