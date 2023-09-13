#include "options.h"

int verbose = 0;
int useTerminal = 1;
int useSerial = 0;
int throttle = 0;
int initialize = 0;
int resetOnStartup = 0;
int netbook = 0;
int useXShm = 1;
int gray = 0;
int mono = 0;

char* serialLine = "/dev/ttyS0";
char* romFileName = "rom";
char* homeDirectory = ".x48ng";

char* smallLabelFont = "-*-fixed-bold-r-normal-*-14-*-*-*-*-*-iso8859-1";
char* mediumLabelFont = "-*-fixed-bold-r-normal-*-15-*-*-*-*-*-iso8859-1";
char* largeLabelFont = "-*-fixed-medium-r-normal-*-20-*-*-*-*-*-iso8859-1";
char* connectionFont = "-*-fixed-medium-r-normal-*-12-*-*-*-*-*-iso8859-1";
