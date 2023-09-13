#include "options.h"

int verbose = 0;
int useTerminal = 1;
int useSerial = 0;
int throttle = 0;
int initialize = 0;
int resetOnStartup = 0;

char* serialLine = "/dev/ttyS0";
char* romFileName = "rom";
char* homeDirectory = ".x48ng";
