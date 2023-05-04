#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>

#include "resources.h"
#include "disasm.h"
#include "errors.h"

int verbose;
int quiet;
int useTerminal;
int useSerial;
int useXShm;
int useDebugger;
int netbook;
int throttle;
int initialize;
int resetOnStartup;
#if defined( GUI_IS_X11 )
char* serialLine;
char* romFileName;
char* homeDirectory;
#elif defined( GUI_IS_SDL1 )
char serialLine[ 1024 ];
char romFileName[ 1024 ];
char homeDirectory[ 1024 ];
#endif

#if defined( GUI_IS_X11 )
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>

XrmDatabase rdb = ( XrmDatabase )0;

void usage( void ) {
    fprintf( stdout, "\n\
%s Version %d.%d.%d\n\
\n\
usage:\n\t%s [-options ...]\n\
\n\
where options include:\n\
    -help                        print out this message\n\
    -display    <displayname>    X server to contact\n\
    -name	<string>         set application name to <string>\n\
    -title      <string>         set window title to <string>\n\
    -geometry   <geometry>       position of window\n\
    -iconGeom   <geometry>       position of icon window\n\
    -iconic                      start iconic\n\
    -visual     <visualname>     use visual <visualname>\n\
    -mono                        force monochrome\n\
    -gray                        force grayscale\n\
    -monoIcon                    force monochrome icon\n\
    -smallFont  <fontname>       <fontname> to draw small labels (MTH - DEL)\n\
    -mediumFont <fontname>       <fontname> to draw medium label (ENTER)\n\
    -largeFont  <fontname>       <fontname> to draw large labels (Numbers)\n\
    -connFont   <fontname>       <fontname> to display wire & IR connections\n\
    -/+xshm                      turn on/off XShm extension\n\
    -version                     print out version information\n\
    -copyright                   print out copyright information\n\
    -warranty                    print out warranty information\n\
    -verbose                     run verbosive\n\
    -quiet                       run quietly\n\
    -/+terminal                  turn on/off pseudo terminal interface\n\
    -/+serial                    turn on/off serial interface\n\
    -line       <devicename>     use serial line <devicename> for IR connection\n\
    -/+debug                     turn on/off debugger\n\
    -disasm     <string>         use <string> (\'HP\' or \'class\') mnemonics\n\
    -reset                       perform a reset (PC = 0) on startup\n\
    -initialize                  force initialization x48ng from ROM-dump\n\
    -rom        <filename>       if initializing, read ROM from <filename>\n\
    -home       <directory>      use directory ~/<directory> to save x48ng files\n\
    -xrm        <resource>       set Xresource <resource>\n\
    -/+throttle			 turn off/on speed emulation\n\
    -/+netbook			 turn off/on netbook layout\n\
\n",
             progname, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL, progname );

    fflush( stdout );
    exit( 1 );
}

void show_version( void ) {
    fprintf( stdout, "\n\
%s Version %d.%d.%d",
             progname, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );
}

void show_copyright( void ) {
    fprintf( stdout, "\n\
                               COPYRIGHT\n\
\n\
x48ng is an Emulator/Debugger for the HP-48 Handheld Calculator.\n\
\n\
This program is free software; you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation; either version 2 of the License, or\n\
(at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program; if not, write to the Free Software\n\
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n\n" );
}

void show_warranty( void ) {
    fprintf( stdout, "\n\
                              NO WARRANTY\n\
\n\
      BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY\n\
FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN\n\
OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES\n\
PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED\n\
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\n\
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS\n\
TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE\n\
PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,\n\
REPAIR OR CORRECTION.\n\
\n\
      IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING\n\
WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR\n\
REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,\n\
INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING\n\
OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED\n\
TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY\n\
YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER\n\
PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE\n\
POSSIBILITY OF SUCH DAMAGES.\n\n" );
}

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
        *tmp++ = isupper( *s ) ? tolower( *s ) : *s;
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
        *tmp++ = isupper( *s ) ? tolower( *s ) : *s;
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
                *tmp = tolower( *tmp );

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

#elif defined( GUI_IS_SDL1 )

void get_resources( void ) {
    verbose = 0;
    quiet = 0;
    useTerminal = 1;
    useSerial = 0;
    strcpy( serialLine, "/dev/ttyS0" );
    initialize = 0;
    resetOnStartup = 0;

    // There are two directories that can contain files:
    // homeDirectory:		Directory in which the live files (hp state,
    // ram, but also a copy of the rom) are stored
    //							homeDirectory is the
    // first directory in which x48ng attempts to load the emulator data It
    // is also in homeDirectory that state files are saved
    // romFileName:		if loading files from homeDirectory fails, the
    // emulator instead initializes the state and ram from scratch, and attempts
    //							to load the ROM
    // romFileName. This is just for bootstrapping: afterwards then the emulator
    // will save the state to homeDirectory

    // Have homeDirectory in the user's home
    strcpy( homeDirectory, ".x48ng" ); // live files are stored in ~/.x48ng

    // As a fallback, assume that a ROM will be available at the same location
    // as the executable We assume that the rom file is in the same
    int rv;
    rv = readlink(
        "/proc/self/exe", romFileName,
        sizeof( romFileName ) ); // Find the full path name of the executable
                                 // (this is linux/cygwin only)
    if ( rv > 0 && rv < sizeof( romFileName ) ) {
        // If found...
        romFileName[ rv ] = 0;
        // find the last slash and terminate
        char* slash = strrchr( romFileName, '/' );
        *slash = 0;
        // append the name of the rom file
        strcat( romFileName, "/rom" );
    } else {
        // Couldn't find path to executable... just use some default
        strcpy( romFileName, "rom.dump" );
    }

    useDebugger = 1;
    disassembler_mode = CLASS_MNEMONICS; // HP_MNEMONICS
    netbook = 0;
    throttle = 0;
}
#endif
