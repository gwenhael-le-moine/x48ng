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

/* $Log: x48_x11.h,v $
 * Revision 1.11  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.10  1994/12/07  20:16:41  ecd
 * added "refresh_icon"
 *
 * Revision 1.10  1994/12/07  20:16:41  ecd
 * added "refresh_icon"
 *
 * Revision 1.9  1994/11/28  02:19:22  ecd
 * added support for contrast adjustment
 *
 * Revision 1.8  1994/11/04  03:44:47  ecd
 * added support for mono and gray displays
 *
 * Revision 1.7  1994/11/02  14:51:27  ecd
 * minor fix
 *
 * Revision 1.6  1994/10/09  20:26:35  ecd
 * changed disp_t
 *
 * Revision 1.5  1994/10/06  16:29:28  ecd
 * added XShm - Extension stuff
 *
 * Revision 1.4  1994/10/05  08:33:22  ecd
 * changed disp_t: removed Pixmap
 *
 * Revision 1.3  1994/09/30  12:32:49  ecd
 * changed display stuff, added detection of interrupts in GetEvent
 *
 * Revision 1.2  1994/09/18  15:31:58  ecd
 * started Real Time support
 *
 * Revision 1.1  1994/09/13  15:05:11  ecd
 * Initial revision
 *
 *
 * $Id: x48_x11.h,v 1.11 1995/01/11 18:20:01 ecd Exp ecd $
 */

#ifndef _X48_X11_H
#define _X48_X11_H 1

#include "global.h"

#include <X11/Xlib.h>
#ifdef HAVE_XSHM
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

#define WHITE 0
#define LEFT 1
#define RIGHT 2
#define BUT_TOP 3
#define BUTTON 4
#define BUT_BOT 5
#define LCD 6
#define PIXEL 7
#define PAD_TOP 8
#define PAD 9
#define PAD_BOT 10
#define DISP_PAD_TOP 11
#define DISP_PAD 12
#define DISP_PAD_BOT 13
#define LOGO 14
#define LOGO_BACK 15
#define LABEL 16
#define FRAME 17
#define UNDERLAY 18
#define BLACK 19

typedef struct color_t {
    char* name;
    int r, g, b;
    int mono_rgb;
    int gray_rgb;
    XColor xcolor;
} color_t;

extern color_t* colors;

#define COLOR( c ) ( colors[ ( c ) ].xcolor.pixel )

#define UPDATE_MENU 1
#define UPDATE_DISP 2

typedef struct disp_t {
    unsigned int w, h;
    Window win;
    GC gc;
    short mapped;
    int offset;
    int lines;
#ifdef HAVE_XSHM
    int display_update;
    XShmSegmentInfo disp_info;
    XImage* disp_image;
    XShmSegmentInfo menu_info;
    XImage* menu_image;
#endif
} disp_t;

extern disp_t disp;

#ifdef HAVE_XSHM
extern int shm_flag;
#endif

extern Display* dpy;
extern int screen;

extern int InitDisplay( int argc, char** argv );
extern int CreateWindows( int argc, char** argv );
extern int GetEvent( void );

extern void adjust_contrast( int contrast );
extern void refresh_icon( void );

extern void ShowConnections( char* w, char* i );

extern void exit_x48( int tell_x11 );

#ifdef HAVE_XSHM
extern void refresh_display( void );
#endif

#endif /* !_X48_X11_H */
