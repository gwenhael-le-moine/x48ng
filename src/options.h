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

/* $Log: options.h,v $
 * Revision 1.5  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.4  1994/12/07  20:16:41  ecd
 * added more options
 *
 * Revision 1.4  1994/12/07  20:16:41  ecd
 * added more options
 *
 * Revision 1.3  1994/11/28  02:19:22  ecd
 * complete rewrite
 *
 * Revision 1.2  1994/11/04  03:44:47  ecd
 * added first options
 *
 * Revision 1.1  1994/11/02  14:51:27  ecd
 * Initial revision
 *
 *
 * $Id: options.h,v 1.5 1995/01/11 18:20:01 ecd Exp ecd $
 */

#ifndef _OPTIONS_H
#define _OPTIONS_H 1

#include "global.h"

#include <X11/Xlib.h>
#include <X11/Xresource.h>

static XrmOptionDescRec options[] = {
    { "-display", ".display", XrmoptionSepArg, ( void* )0 },
    { "-geometry", "*geometry", XrmoptionSepArg, ( void* )0 },
    { "-iconGeom", "*iconGeom", XrmoptionSepArg, ( void* )0 },
    { "-iconName", "*iconName", XrmoptionSepArg, ( void* )0 },
    { "-iconic", "*iconic", XrmoptionNoArg, ( void* )"True" },
    { "-name", ( char* )0, XrmoptionSepArg, ( void* )0 },
    { "-title", "*title", XrmoptionSepArg, ( void* )0 },

    { "-xshm", "*useXShm", XrmoptionNoArg, ( void* )"True" },
    { "+xshm", "*useXShm", XrmoptionNoArg, ( void* )"False" },

    { "-visual", "*visual", XrmoptionSepArg, ( void* )0 },
    { "-mono", "*mono", XrmoptionNoArg, ( void* )"True" },
    { "-gray", "*gray", XrmoptionNoArg, ( void* )"True" },
    { "-monoIcon", "*monoIcon", XrmoptionNoArg, ( void* )"True" },

    { "-version", "*printVersion", XrmoptionNoArg, ( void* )"True" },
    { "-copyright", "*printCopyright", XrmoptionNoArg, ( void* )"True" },
    { "-warranty", "*printWarranty", XrmoptionNoArg, ( void* )"True" },

    { "-smallFont", "*smallLabelFont", XrmoptionSepArg, ( void* )0 },
    { "-mediumFont", "*mediumLabelFont", XrmoptionSepArg, ( void* )0 },
    { "-largeFont", "*largeLabelFont", XrmoptionSepArg, ( void* )0 },
    { "-connFont", "*connectionFont", XrmoptionSepArg, ( void* )0 },

    { "-verbose", "*verbose", XrmoptionNoArg, ( void* )"True" },
    { "-quiet", "*quiet", XrmoptionNoArg, ( void* )"True" },

    { "-terminal", "*useTerminal", XrmoptionNoArg, ( void* )"True" },
    { "+terminal", "*useTerminal", XrmoptionNoArg, ( void* )"False" },
    { "-serial", "*useSerial", XrmoptionNoArg, ( void* )"True" },
    { "+serial", "*useSerial", XrmoptionNoArg, ( void* )"False" },
    { "-line", "*serialLine", XrmoptionSepArg, ( void* )0 },

    { "-initialize", "*completeInitialize", XrmoptionNoArg, ( void* )"True" },
    { "-reset", "*resetOnStartup", XrmoptionNoArg, ( void* )"True" },
    { "-rom", "*romFileName", XrmoptionSepArg, ( void* )0 },
    { "-home", "*homeDirectory", XrmoptionSepArg, ( void* )0 },

    { "-debug", "*useDebugger", XrmoptionNoArg, ( void* )"False" },
    { "+debug", "*useDebugger", XrmoptionNoArg, ( void* )"True" },
    { "-disasm", "*disassemblerMnemonics", XrmoptionSepArg, ( void* )0 },

    { "-xrm", ( char* )0, XrmoptionResArg, ( void* )0 },
    { "-netbook", "*netbook", XrmoptionNoArg, ( void* )"False" },
    { "+netbook", "*netbook", XrmoptionNoArg, ( void* )"True" },

    { "-throttle", "*throttle", XrmoptionNoArg, ( void* )"False" },
    { "+throttle", "*throttle", XrmoptionNoArg, ( void* )"True" },

    /*
     * these are parsed for compatibility, but not used yet.
     */
    { "-bg", "*background", XrmoptionSepArg, ( void* )0 },
    { "-background", "*background", XrmoptionSepArg, ( void* )0 },
    { "-bd", "*borderColor", XrmoptionSepArg, ( void* )0 },
    { "-bordercolor", "*borderColor", XrmoptionSepArg, ( void* )0 },
    { "-bw", "*borderWidth", XrmoptionSepArg, ( void* )0 },
    { "-borderwidth", "*borderWidth", XrmoptionSepArg, ( void* )0 },
    { "-fg", "*foreground", XrmoptionSepArg, ( void* )0 },
    { "-foreground", "*foreground", XrmoptionSepArg, ( void* )0 },
    { "-fn", "*fontName", XrmoptionSepArg, ( void* )0 },
    { "-font", "*fontName", XrmoptionSepArg, ( void* )0 },
    { "-rv", "*reverseVideo", XrmoptionNoArg, ( void* )"True" },
    { "+rv", "*reverseVideo", XrmoptionNoArg, ( void* )"False" },
    { "-reverse", "*reverseVideo", XrmoptionNoArg, ( void* )"True" },

};

#endif /* _OPTIONS_H */
