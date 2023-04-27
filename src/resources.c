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

/* $Log: resources.c,v $
 * Revision 1.3  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.2  1994/12/07  20:20:50  ecd
 * more resource get functions
 *
 * Revision 1.2  1994/12/07  20:20:50  ecd
 * more resource get functions
 *
 * Revision 1.1  1994/12/07  10:15:47  ecd
 * Initial revision
 *
 *
 * $Id: resources.c,v 1.3 1995/01/11 18:20:01 ecd Exp ecd $
 */

/* xscreensaver, Copyright (c) 1992 Jamie Zawinski <jwz@lucid.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 */

#include "global.h"

#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <stdio.h>

#include "disasm.h"
#include "errors.h"
#include "resources.h"

XrmDatabase rdb = ( XrmDatabase )0;

int verbose;
int quiet;
int useTerminal;
int useSerial;
char* serialLine;
int useXShm;
int useDebugger;
int netbook;
int throttle;
int initialize;
int resetOnStartup;
char* romFileName;
char* homeDirectory;

void get_resources( void ) {
    if ( get_boolean_resource( "printVersion", "PrintVersion" ) )
        show_version();
    if ( get_boolean_resource( "printCopyright", "PrintCopyright" ) )
        show_copyright();
    if ( get_boolean_resource( "printWarranty", "PrintWarranty" ) )
        show_warranty();

    verbose = get_boolean_resource( "verbose", "Verbose" );
    quiet = get_boolean_resource( "quiet", "Quiet" );

    useXShm = get_boolean_resource( "useXShm", "UseXShm" );

    useTerminal = get_boolean_resource( "useTerminal", "UseTerminal" );
    useSerial = get_boolean_resource( "useSerial", "UseSerial" );
    serialLine = get_string_resource( "serialLine", "SerialLine" );

    initialize =
        get_boolean_resource( "completeInitialize", "CompleteInitialize" );
    resetOnStartup = get_boolean_resource( "resetOnStartup", "ResetOnStartup" );
    romFileName = get_string_resource( "romFileName", "RomFileName" );
    homeDirectory = get_string_resource( "homeDirectory", "HomeDirectory" );

    useDebugger = get_boolean_resource( "useDebugger", "UseDebugger" );
    disassembler_mode = get_mnemonic_resource( "disassemblerMnemonics",
                                               "DisassemblerMnemonics" );

    netbook = get_boolean_resource( "netbook", "Netbook" );

    throttle = get_boolean_resource( "throttle", "Throttle" );
}

char* get_string_resource_from_db( XrmDatabase db, char* name, char* class ) {
    XrmValue value;
    char* type;
    char full_name[ 1024 ], full_class[ 1024 ];

    strcpy( full_name, res_name );
    strcat( full_name, "." );
    strcat( full_name, name );
    strcpy( full_class, res_class );
    strcat( full_class, "." );
    strcat( full_class, class );
    if ( XrmGetResource( db, full_name, full_class, &type, &value ) ) {
        char* str = ( char* )malloc( value.size + 1 );
        strncpy( str, ( char* )value.addr, value.size );
        str[ value.size ] = 0;
        return str;
    }
    return ( char* )0;
}

char* get_string_resource( char* name, char* class ) {
    return get_string_resource_from_db( rdb, name, class );
}

int get_mnemonic_resource( char* name, char* class ) {
    char *tmp, buf[ 100 ];
    char* s = get_string_resource( name, class );
    char* os = s;

    if ( !s )
        return CLASS_MNEMONICS;
    for ( tmp = buf; *s; s++ )
        *tmp++ = isupper( *s ) ? _tolower( *s ) : *s;
    *tmp = 0;
    free( os );

    if ( !strcmp( buf, "hp" ) )
        return HP_MNEMONICS;
    if ( !strcmp( buf, "class" ) )
        return CLASS_MNEMONICS;
    fprintf( stderr, "%s: %s must be one of \'HP\' or \'class\', not %s.\n",
             progname, name, buf );
    return CLASS_MNEMONICS;
}

int get_boolean_resource( char* name, char* class ) {
    char *tmp, buf[ 100 ];
    char* s = get_string_resource( name, class );
    char* os = s;
    if ( !s )
        return 0;
    for ( tmp = buf; *s; s++ )
        *tmp++ = isupper( *s ) ? _tolower( *s ) : *s;
    *tmp = 0;
    free( os );

    if ( !strcmp( buf, "on" ) || !strcmp( buf, "true" ) ||
         !strcmp( buf, "yes" ) )
        return 1;
    if ( !strcmp( buf, "off" ) || !strcmp( buf, "false" ) ||
         !strcmp( buf, "no" ) )
        return 0;
    fprintf( stderr, "%s: %s must be boolean, not %s.\n", progname, name, buf );
    return 0;
}

int get_integer_resource( char* name, char* class ) {
    int val;
    char c, *s = get_string_resource( name, class );
    if ( !s )
        return 0;
    if ( 1 == sscanf( s, " %d %c", &val, &c ) ) {
        free( s );
        return val;
    }
    fprintf( stderr, "%s: %s must be an integer, not %s.\n", progname, name,
             s );
    free( s );
    return 0;
}

unsigned int get_pixel_resource( char* name, char* class, Display* dpy,
                                 Colormap cmap ) {
    XColor color;
    char* s = get_string_resource( name, class );
    if ( !s )
        goto DEFAULT;

    if ( !XParseColor( dpy, cmap, s, &color ) ) {
        fprintf( stderr, "%s: can't parse color %s\n", progname, s );
        goto DEFAULT;
    }
    if ( !XAllocColor( dpy, cmap, &color ) ) {
        fprintf( stderr, "%s: couldn't allocate color %s\n", progname, s );
        goto DEFAULT;
    }
    free( s );
    return color.pixel;
DEFAULT:
    if ( s )
        free( s );
    return ( strcmp( class, "Background" )
                 ? WhitePixel( dpy, DefaultScreen( dpy ) )
                 : BlackPixel( dpy, DefaultScreen( dpy ) ) );
}

static Visual* pick_visual_of_class( Display* dpy, int visual_class,
                                     unsigned int* depth ) {
    XVisualInfo vi_in, *vi_out;
    int out_count;

    vi_in.class = visual_class;
    vi_in.screen = DefaultScreen( dpy );
    vi_out = XGetVisualInfo( dpy, VisualClassMask | VisualScreenMask, &vi_in,
                             &out_count );
    if ( vi_out ) { /* choose the 'best' one, if multiple */
        int i, best;
        Visual* visual;
        for ( i = 0, best = 0; i < out_count; i++ )
            if ( vi_out[ i ].depth > vi_out[ best ].depth )
                best = i;
        visual = vi_out[ best ].visual;
        *depth = vi_out[ best ].depth;
        XFree( ( char* )vi_out );
        return visual;
    } else {
        *depth = DefaultDepth( dpy, DefaultScreen( dpy ) );
        return DefaultVisual( dpy, DefaultScreen( dpy ) );
    }
}

static Visual* id_to_visual( Display* dpy, int id, unsigned int* depth ) {
    XVisualInfo vi_in, *vi_out;
    int out_count;

    vi_in.screen = DefaultScreen( dpy );
    vi_in.visualid = id;
    vi_out = XGetVisualInfo( dpy, VisualScreenMask | VisualIDMask, &vi_in,
                             &out_count );
    if ( vi_out ) {
        Visual* v = vi_out[ 0 ].visual;
        *depth = vi_out[ 0 ].depth;
        XFree( ( char* )vi_out );
        return v;
    }
    return 0;
}

Visual* get_visual_resource( Display* dpy, char* name, char* class,
                             unsigned int* depth ) {
    char c;
    char *tmp, *s;
    int vclass;
    int id;

    s = get_string_resource( name, class );
    if ( s )
        for ( tmp = s; *tmp; tmp++ )
            if ( isupper( *tmp ) )
                *tmp = _tolower( *tmp );

    if ( !s || !strcmp( s, "default" ) )
        vclass = -1;
    else if ( !strcmp( s, "staticgray" ) )
        vclass = StaticGray;
    else if ( !strcmp( s, "staticcolor" ) )
        vclass = StaticColor;
    else if ( !strcmp( s, "truecolor" ) )
        vclass = TrueColor;
    else if ( !strcmp( s, "grayscale" ) )
        vclass = GrayScale;
    else if ( !strcmp( s, "pseudocolor" ) )
        vclass = PseudoColor;
    else if ( !strcmp( s, "directcolor" ) )
        vclass = DirectColor;
    else if ( 1 == sscanf( s, " %d %c", &id, &c ) )
        vclass = -2;
    else if ( 1 == sscanf( s, " 0x%x %c", &id, &c ) )
        vclass = -2;
    else {
        fprintf( stderr, "%s: unrecognized visual \"%s\".\n", progname, s );
        vclass = -1;
    }
    if ( s )
        free( s );

    if ( vclass == -1 ) {
        *depth = DefaultDepth( dpy, DefaultScreen( dpy ) );
        return DefaultVisual( dpy, DefaultScreen( dpy ) );
    } else if ( vclass == -2 ) {
        Visual* v = id_to_visual( dpy, id, depth );
        if ( v )
            return v;
        fprintf( stderr, "%s: no visual with id 0x%x.\n", progname, id );
        *depth = DefaultDepth( dpy, DefaultScreen( dpy ) );
        return DefaultVisual( dpy, DefaultScreen( dpy ) );
    } else
        return pick_visual_of_class( dpy, vclass, depth );
}

XFontStruct* get_font_resource( Display* dpy, char* name, char* class ) {
    char* s;
    XFontStruct* f = ( XFontStruct* )0;

    s = get_string_resource( name, class );

    if ( s )
        f = XLoadQueryFont( dpy, s );
    else {
        sprintf( errbuf, "can\'t get resource \'%s\'", name );
        fatal_exit();
    }
    if ( f == ( XFontStruct* )0 ) {
        sprintf( errbuf, "can\'t load font \'%s\'", s );
        sprintf( fixbuf, "Please change resource \'%s\'", name );
        fatal_exit();
    }
    return f;
}
