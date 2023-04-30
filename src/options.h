#ifndef _OPTIONS_H
#define _OPTIONS_H 1

#include "config.h"

#ifdef GUI_IS_X11
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
#endif

#endif /* _OPTIONS_H */
